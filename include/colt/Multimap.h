#ifndef HG_COLT_MULTIMAP
#define HG_COLT_MULTIMAP

#include <utility>

#include "Hash.h"
#include "Vector.h"

namespace colt
{
  namespace details
  {
    /// @brief Specifies the state of a map slot.
    /// An ACTIVE sentinel holds 7 bits worth of hash.
    /// An ACTIVE sentinel is one whose highest bit is 0.
    /// An EMPTY sentinel specifies that the slot is empty.
    /// A DELETED sentinel specifies that find should continue searching past that object.
    enum KeySentinel
      : uint8_t
    {
      ACTIVE = 0b00000000,
      EMPTY = 0b10000000,
      DELETED = 0b10000001,
    };

    /// @brief Creates an ACTIVE sentinel holding the 7th lowest bits of 'hash'
    /// @param hash The hash whose 7th lowest bits to encode
    /// @return An ACTIVE sentinel
    constexpr KeySentinel create_active_sentinel(size_t hash) noexcept
    {
      return static_cast<KeySentinel>(hash & 0b01111111);
    }

    /// @brief Check if a sentinel is ACTIVE
    /// @param key The sentinel to check
    /// @return True if the sentinel represents an ACTIVE sentinel
    constexpr bool is_sentinel_active(KeySentinel key) noexcept
    {
      return (key & 0b10000000) == 0;
    }
    
    /// @brief Check if a sentinel is EMPTY
    /// @param key The sentinel to check
    /// @return True if the sentinel equal EMPTY
    constexpr bool is_sentinel_empty(KeySentinel key) noexcept
    {
      return key == EMPTY;
    }

    /// @brief Check if a sentinel is DELETED
    /// @param key The sentinel to check
    /// @return True if the sentinel equal DELETED
    constexpr bool is_sentinel_deleted(KeySentinel key) noexcept
    {
      return key == DELETED;
    }

    /// @brief Check if a hash and a sentinel are equal.
    /// Precondition: is_sentinel_active(key).
    /// The check performed checks the lower 7-bits of the hash.
    /// @param key The sentinel to check for
    /// @param hash The hash whose 7 lowest bits to compare with
    /// @return True if the lowest 7-bits of the hash matches the sentinel
    constexpr bool is_sentinel_equal(KeySentinel key, size_t hash) noexcept
    {
      assert(is_sentinel_active(key));
      return hash & (key & 0b01111111);
    }
  }

  /// @brief Represents the result of an insert/insertOrAssign operation
  enum class InsertionResult
    : uint8_t
  {
    /// @brief Insertion successful
    SUCCESS,
    /// @brief The key already exists
    EXISTS,
    /// @brief Performed an assignment rather than an insertion
    ASSIGNED
  };

  template<typename Key, typename Value>
  class Map
  {
    static_assert(traits::is_hashable_v<Key>, "Key of a Map should be hashable!");
    static_assert(traits::is_equal_comparable_v<Key>, "Key of a Map should implement operator==!");

    using Slot = std::pair<const Key, Value>;
    
    /// @brief Contains meta-data information about the slots of the map
    Vector<details::KeySentinel> sentinel_metadata;
    /// @brief Memory block of the key/value pair
    memory::TypedBlock<Slot> slots;
    /// @brief The load factor (or capacity before growth)
    float load_factor;

    /// @brief Increments a probing index, faster than a modulo operation.
    /// Equivalent to a faster 'prob + 1 % slots.getSize()', but all increment
    /// done one a probe should pass by this functions.
    /// @param prob The probing index to increment
    /// @return The incremented index
    constexpr size_t advance_prob(size_t prob) const noexcept
    {
      assert(prob / slots.getSize() < 2);
      return prob == slots.getSize() ? 0 : prob + 1;
    }

    /// @brief Finds a EMPTY/ACTIVE/DELETED slot matching 'key_hash'
    /// @param key_hash The hash of 'key', obtained through 'GetHash'.
    /// This function does not perform a hash of 'key' as usually the function
    /// that calls this function already possesses that hash.
    /// @param key The key to search for.
    /// This key will not be hashed by the function.
    /// @param prob The reference where to write the offset to the slot
    /// @return True if the slot found is empty/deleted, false if the slot is already occupied
    constexpr bool find_key(size_t key_hash, traits::copy_if_trivial_t<const Key&> key, size_t& prob) const noexcept
    {
      assert(key_hash == GetHash(key));
      size_t prob_index = key_hash % slots.getSize();
      for (;;)
      {        
        if (auto sentinel = sentinel_metadata[prob_index];
          details::is_sentinel_empty(sentinel) || details::is_sentinel_deleted(sentinel))
        {
          prob = prob_index;          
          return true;
        }
        else if (details::is_sentinel_equal(sentinel, key_hash))
        {
          if (slots.getPtr()[prob_index].first == key)
          {
            prob = prob_index;
            return false;
          }
        }
        prob_index = advance_prob(prob_index);
      }
    }

  public:
    Map() = delete;
    
    constexpr Map(size_t reserve_size) noexcept
      : sentinel_metadata(reserve_size, InPlace, details::EMPTY)
      , slots(memory::allocate({ reserve_size * sizeof(Slot) }))
    {}

    ~Map()
      noexcept(std::is_nothrow_destructible_v<Key>&& std::is_nothrow_destructible_v<Value>)
    {
      for (size_t i = 0; i < sentinel_metadata.getSize(); i++)
      {
        auto sentinel = sentinel_metadata[i];
        if (details::is_sentinel_active(sentinel))
          slots.getPtr()[i].~Slot(); //destroy active slots
      }
      if (slots)
        memory::deallocate(slots);
    }

    bool contains(traits::copy_if_trivial_t<const Key&> key) const noexcept
    {
      size_t prob;
      return !find_key(GetHash(key), key, prob);
    }

    Slot* find(traits::copy_if_trivial_t<const Key&> key) noexcept
    {
      const size_t key_hash = GetHash(key);
      size_t prob_index = key_hash % slots.getSize();
      for (;;)
      {
        auto sentinel = sentinel_metadata[prob_index];
        if (details::is_sentinel_empty(sentinel))
        {
          return nullptr; //not found
        }
        else if (details::is_sentinel_deleted(sentinel))
        {
          prob_index = advance_prob(prob_index);
          continue;
        }
        else if (details::is_sentinel_equal(sentinel, key_hash))
        {
          if (slots.getPtr()[prob_index].first == key)
            return slots.getPtr() + prob_index;
        }
        prob_index = advance_prob(prob_index);
      }
    }

    std::pair<Slot*, InsertionResult> insert(traits::copy_if_trivial_t<const Key&> key, traits::copy_if_trivial_t<const Value&> value)
      noexcept(std::is_nothrow_copy_constructible_v<Value>)
    {
      const size_t key_hash = GetHash(key);
      size_t prob_index;
      if (find_key(key_hash, key, prob_index))
      {
        new(slots.getPtr() + prob_index) Slot(key, value);
        //Set the slot to ACTIVE
        sentinel_metadata[prob_index] = details::create_active_sentinel(key_hash);
        return { slots.getPtr() + prob_index, InsertionResult::SUCCESS };
      }
      else
        return { slots.getPtr() + prob_index, InsertionResult::EXISTS };
    }

    std::pair<Slot*, InsertionResult> insertOrAssign(traits::copy_if_trivial_t<const Key&> key, traits::copy_if_trivial_t<const Value&> value)
      noexcept(std::is_nothrow_copy_constructible_v<Value> && std::is_nothrow_copy_assignable_v<Value>)
    {
      const size_t key_hash = GetHash(key);
      size_t prob_index;
      if (find_key(key_hash, key, prob_index))
      {
        new(slots.getPtr() + prob_index) Slot(key, value);
        //Set the slot to ACTIVE
        sentinel_metadata[prob_index] = details::create_active_sentinel(key_hash);
        return { slots.getPtr() + prob_index, InsertionResult::SUCCESS };
      }
      else
      {
        slots.getPtr()[prob_index].second = value;
        return { slots.getPtr() + prob_index, InsertionResult::ASSIGNED };
      }
    }
  };
}

#endif //!HG_COLT_MULTIMAP