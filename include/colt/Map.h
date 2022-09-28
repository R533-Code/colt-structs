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

    /// @brief Check if a sentinel is EMPTY or DELETED
    /// @param key The sentinel to check
    /// @return True if the sentinel represents an ACTIVE sentinel
    constexpr bool is_sentinel_empty_or_deleted(KeySentinel key) noexcept
    {
      return (key & 0b10000000) != 0;
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

    /// @brief Increments a probing index, faster than a modulo operation.
    /// Equivalent to a faster 'prob + 1 % slots.getSize()', but all increment
    /// done one a probe should pass by this functions.
    /// @param prob The probing index to increment
    /// @return The incremented index
    constexpr size_t advance_prob(size_t prob, size_t mod) noexcept
    {
      assert(prob / mod < 2);
      return  (prob + 1) * (prob == mod - 1);
    }
  }

  /// @brief Represents the result of an insert/insertOrAssign operation
  enum class InsertionResult
    : uint8_t
  {
    /// @brief Insertion successful
    SUCCESS,
    /// @brief The key already exists, and nothing was performed
    EXISTS,
    /// @brief Performed an assignment rather than an insertion
    ASSIGNED
  };

  template<typename Key, typename Value>
  /// @brief A unordered associative container that contains key/value pairs with unique keys.
  /// @tparam Key The Key that can be hashed through colt::hash or std::hash
  /// @tparam Value The Value that is accessed through the Key
  class Map
  {
    static_assert(traits::is_hashable_v<Key>, "Key of a Map should be hashable!");
    static_assert(traits::is_equal_comparable_v<Key>, "Key of a Map should implement operator==!");

    using Slot = std::pair<const Key, Value>;

    /// @brief Contains meta-data information about the slots of the map
    Vector<details::KeySentinel> sentinel_metadata = {};
    /// @brief Memory block of the key/value pair
    memory::TypedBlock<Slot> slots = {};
    /// @brief The count of active objects in the container
    size_t size = 0;
    /// @brief The load factor before reallocation
    float load_factor = 0.70f;

  public:
    /// @brief Constructs an empty Map, of load factor 0.7
    constexpr Map() = default;

    /// @brief Constructs a Map of load factor 0.7, reserving memory for 'reserve_size' objects
    /// @param reserve_size The count of object to reserve for
    constexpr Map(size_t reserve_size) noexcept
      : sentinel_metadata(reserve_size, InPlace, details::EMPTY)
      , slots(memory::allocate({ reserve_size * sizeof(Slot) }))
    {}

    /// @brief Destructs a Map and its active elements
    ~Map()
      noexcept(std::is_nothrow_destructible_v<Key>
        && std::is_nothrow_destructible_v<Value>);

    /// @brief Clear all the active elements in the Map
    constexpr void clear()
      noexcept(std::is_nothrow_destructible_v<Key>
        && std::is_nothrow_destructible_v<Value>);

    /// @brief Returns the number of active elements in the Map
    /// @return The count of active elements
    constexpr size_t getSize() const noexcept { return size; }

    /// @brief Returns the capacity of the current allocation
    /// @return The capacity of the current allocation
    constexpr size_t getCapacity() const noexcept { return slots.getSize(); }

    /// @brief Check if the Map will reallocate on the next call of insert/insertOrAssign
    /// @return True if the Map will reallocate
    constexpr bool willReallocate() const noexcept;

    /// @brief Returns the load factor of the Map
    /// @return The load factor
    constexpr float getLoadFactor() const noexcept { return load_factor; }

    /// @brief Sets the load factor to 'nload_factor'.
    /// Precondition: nload_factor < 1.0f && nload_factor > 0.0f.
    /// @param nload_factor The new load factor
    constexpr void setLoadFactor(float nload_factor) noexcept;

    /// @brief Finds the key/value pair of key 'key'
    /// @param key The key to search for
    /// @return Pointer to the key/value pair if found, or null
    constexpr const Slot* find(traits::copy_if_trivial_t<const Key&> key) const noexcept;

    /// @brief Finds the key/value pair of key 'key'
    /// @param key The key to search for
    /// @return Pointer to the key/value pair if found, or null
    constexpr Slot* find(traits::copy_if_trivial_t<const Key&> key) noexcept;

    /// @brief Check if the Map contains a key/value pair of key 'key'
    /// @param key The key to check for
    /// @return True if the Map contains 'key' else false
    constexpr bool contains(traits::copy_if_trivial_t<const Key&> key) const noexcept;

    /// @brief Inserts a new value if 'key' does not already exist.
    /// Returns an InsertionResult SUCCESS (if the insertion was performed) or EXISTS (if the key already exists).
    /// The returned pointer is to the newly inserted key/value on SUCCESS.
    /// The returned pointer is to the existing key/value that matches 'key' on EXISTS.
    /// The returned pointer is never null.
    /// @param key The key of the value 'value'
    /// @param value The value to insert
    /// @return Pair of pointer to the inserted slot or the existent one, and SUCESS on insertion or EXISTS if the key already exists
    constexpr std::pair<Slot*, InsertionResult> insert(traits::copy_if_trivial_t<const Key&> key, traits::copy_if_trivial_t<const Value&> value)
      noexcept(std::is_nothrow_destructible_v<Key>
        && std::is_nothrow_destructible_v<Value>
        && std::is_nothrow_move_constructible_v<Key>
        && std::is_nothrow_move_constructible_v<Value>
        && std::is_nothrow_copy_constructible_v<Key>
        && std::is_nothrow_copy_constructible_v<Value>);

    /// @brief Insert a new value if 'key' does not already exist, else assigns 'value' to the existing value.
    /// Returns an InsertionResult SUCCESS (if the insertion was performed) or ASSIGNED (if the key already exists and was assigned).
    /// The returned pointer is to the newly inserted key/value on SUCCESS.
    /// The returned pointer is to the existing key/value that matches 'key' on ASSIGNED.
    /// The returned pointer is never null.
    /// @param key The key of the value 'value'
    /// @param value The value to insert or assign
    /// @return Pair of pointer to the inserted/assigned slot, and SUCESS on insertion or ASSIGN on assignment
    constexpr std::pair<Slot*, InsertionResult> insertOrAssign(traits::copy_if_trivial_t<const Key&> key, traits::copy_if_trivial_t<const Value&> value)
      noexcept(std::is_nothrow_destructible_v<Key>
        && std::is_nothrow_destructible_v<Value>
        && std::is_nothrow_move_constructible_v<Key>
        && std::is_nothrow_move_constructible_v<Value>
        && std::is_nothrow_copy_constructible_v<Key>
        && std::is_nothrow_copy_constructible_v<Value>
        && std::is_nothrow_copy_assignable_v<Value>);

    /// @brief Erases a key if it exists
    /// @param key The key whose key/value pair to erase
    /// @return True if the key existed and was erased, else false
    constexpr bool erase(traits::copy_if_trivial_t<const Key&> key)
      noexcept(std::is_nothrow_destructible_v<Key>
        && std::is_nothrow_destructible_v<Value>);

    /// @brief Calls 'find' on 'key'
    /// @param key The key to search for
    /// @return Pointer to the found slot or null if not found
    constexpr const Slot* operator[](traits::copy_if_trivial_t<const Key&> key) const noexcept
    {
      return find(key);
    }

    /// @brief Calls 'find' on 'key'
    /// @param key The key to search for
    /// @return Pointer to the found slot or null if not found
    constexpr Slot* operator[](traits::copy_if_trivial_t<const Key&> key) noexcept
    {
      return find(key);
    }

  private:
    /// @brief Finds a EMPTY/ACTIVE/DELETED slot matching 'key_hash'
    /// @param key_hash The hash of 'key', obtained through 'GetHash'.
    /// This function does not perform a hash of 'key' as usually the function
    /// that calls this function already possesses that hash.
    /// @param key The key to search for.
    /// This key will not be hashed by the function.
    /// @param prob The reference where to write the offset to the slot
    /// @param metadata The Vector of KeySentinel representing the state of 'blk'
    /// @param blk The array of slots
    /// @return True if the slot found is empty/deleted, false if the slot is already occupied
    static constexpr bool find_key(size_t key_hash, traits::copy_if_trivial_t<const Key&> key, size_t& prob,
      const Vector<details::KeySentinel>& metadata, memory::TypedBlock<Slot> blk) noexcept;

    /// @brief Doubles the Map's capacity, rehashing in the process
    constexpr void realloc_map()
      noexcept(std::is_nothrow_destructible_v<Key>
        && std::is_nothrow_destructible_v<Value>
        && std::is_nothrow_move_constructible_v<Key>
        && std::is_nothrow_move_constructible_v<Value>);
  };

  template<typename Key, typename Value>
  constexpr bool Map<Key, Value>::find_key(size_t key_hash, traits::copy_if_trivial_t<const Key&> key, size_t& prob, const Vector<details::KeySentinel>& metadata, memory::TypedBlock<Slot> blk) noexcept
  {
    assert(key_hash == GetHash(key));
    assert(metadata.getSize() == blk.getSize());
    size_t prob_index = key_hash % blk.getSize();
    for (;;)
    {
      if (auto sentinel = metadata[prob_index];
        details::is_sentinel_empty(sentinel) || details::is_sentinel_deleted(sentinel))
      {
        prob = prob_index;
        return true;
      }
      else if (details::is_sentinel_equal(sentinel, key_hash))
      {
        if (blk.getPtr()[prob_index].first == key)
        {
          prob = prob_index;
          return false;
        }
      }
      prob_index = details::advance_prob(prob_index, blk.getSize());
    }
  }

  template<typename Key, typename Value>
  constexpr void Map<Key, Value>::realloc_map() noexcept(std::is_nothrow_destructible_v<Key>&& std::is_nothrow_destructible_v<Value>&& std::is_nothrow_move_constructible_v<Key>&& std::is_nothrow_move_constructible_v<Value>)
  {
    //Grow by twice the capacity
    auto new_slot = memory::allocate({ (slots.getSize() + 16) * 2 * sizeof(Slot) });

    Vector<details::KeySentinel> new_metadata = { (slots.getSize() + 16) * 2, InPlace, details::EMPTY };
    for (size_t i = 0; i < sentinel_metadata.getSize(); i++)
    {
      auto sentinel = sentinel_metadata[i];
      if (details::is_sentinel_active(sentinel))
      {
        //find the key
        Slot* ptr = slots.getPtr() + i;
        const size_t key_hash = GetHash(ptr->first);
        size_t prob_index;
        //Rehash the key to get its new index in the new array
        if (find_key(key_hash, ptr->first, prob_index, sentinel_metadata, slots))
        {
          //Move destruct
          new(slots.getPtr() + prob_index) Slot(std::move(*ptr));
          ptr->~Slot();

          //Set the slot to ACTIVE
          new_metadata[prob_index] = details::create_active_sentinel(key_hash);
        }
      }
    }
    sentinel_metadata = std::move(new_metadata);
    if (slots)
      memory::deallocate(slots);
    slots = new_slot;
  }

  template<typename Key, typename Value>
  Map<Key, Value>::~Map() noexcept(std::is_nothrow_destructible_v<Key>&& std::is_nothrow_destructible_v<Value>)
  {
    clear();
    if (slots)
      memory::deallocate(slots);
  }

  template<typename Key, typename Value>
  constexpr void Map<Key, Value>::clear() noexcept(std::is_nothrow_destructible_v<Key>&& std::is_nothrow_destructible_v<Value>)
  {
    for (size_t i = 0; i < sentinel_metadata.getSize(); i++)
    {
      auto sentinel = sentinel_metadata[i];
      if (details::is_sentinel_active(sentinel))
        slots.getPtr()[i].~Slot(); //destroy active slots
    }
    size = 0;
  }

  template<typename Key, typename Value>
  constexpr bool Map<Key, Value>::willReallocate() const noexcept
  {
    return float(getSize() + 1) / getCapacity() > load_factor;
  }

  template<typename Key, typename Value>
  constexpr void Map<Key, Value>::setLoadFactor(float nload_factor) noexcept
  {
    assert(nload_factor < 1.0f && nload_factor > 0.0f && "Invalid load factor!");
    load_factor = nload_factor;
  }

  template<typename Key, typename Value>
  constexpr Map<Key, Value>::Slot* Map<Key, Value>::find(traits::copy_if_trivial_t<const Key&> key) noexcept
  {
    //No UB as the map is not const
    return const_cast<Slot*>(static_cast<const Map*>(this)->find(key));
  }

  template<typename Key, typename Value>
  constexpr const Map<Key, Value>::Slot* Map<Key, Value>::find(traits::copy_if_trivial_t<const Key&> key) const noexcept
  {
    const size_t key_hash = GetHash(key);
    size_t prob_index = key_hash % slots.getSize();
    for (;;)
    {
      if (auto sentinel = sentinel_metadata[prob_index];
        details::is_sentinel_empty(sentinel))
      {
        return nullptr; //not found
      }
      else if (details::is_sentinel_deleted(sentinel))
      {
        prob_index = details::advance_prob(prob_index, slots.getSize());
        continue;
      }
      else if (details::is_sentinel_equal(sentinel, key_hash))
      {
        if (slots.getPtr()[prob_index].first == key)
          return slots.getPtr() + prob_index;
      }
      prob_index = details::advance_prob(prob_index, slots.getSize());
    }
  }

  template<typename Key, typename Value>
  constexpr bool Map<Key, Value>::contains(traits::copy_if_trivial_t<const Key&> key) const noexcept
  {
    return find(key) != nullptr;
  }

  template<typename Key, typename Value>
  constexpr std::pair<Map<Key, Value>::Slot*, InsertionResult> Map<Key, Value>::insertOrAssign(traits::copy_if_trivial_t<const Key&> key, traits::copy_if_trivial_t<const Value&> value)
    noexcept(std::is_nothrow_destructible_v<Key>
      && std::is_nothrow_destructible_v<Value>
      && std::is_nothrow_move_constructible_v<Key>
      && std::is_nothrow_move_constructible_v<Value>
      && std::is_nothrow_copy_constructible_v<Key>
      && std::is_nothrow_copy_constructible_v<Value>
      && std::is_nothrow_copy_assignable_v<Value>)
  {
    if (willReallocate())
      realloc_map();

    const size_t key_hash = GetHash(key);
    size_t prob_index;
    if (find_key(key_hash, key, prob_index, sentinel_metadata, slots))
    {
      new(slots.getPtr() + prob_index) Slot(key, value);
      //Set the slot to ACTIVE
      sentinel_metadata[prob_index] = details::create_active_sentinel(key_hash);
      //Update size
      ++size;
      return { slots.getPtr() + prob_index, InsertionResult::SUCCESS };
    }
    else
    {
      slots.getPtr()[prob_index].second = value;
      return { slots.getPtr() + prob_index, InsertionResult::ASSIGNED };
    }
  }

  template<typename Key, typename Value>
  constexpr bool Map<Key, Value>::erase(traits::copy_if_trivial_t<const Key&> key) noexcept(std::is_nothrow_destructible_v<Key>&& std::is_nothrow_destructible_v<Value>)
  {
    if (Slot* ptr = find(key))
    {
      size_t index = (slots.getPtr() + slots.getSize()) - ptr;
      sentinel_metadata[index] = details::DELETED; //set the sentinel to deleted
      ptr->~Slot(); //destroy the key/value pair
      //Update size
      --size;
      return true;
    }
    else
      return false;
  }

  template<typename Key, typename Value>
  constexpr std::pair<Map<Key, Value>::Slot*, InsertionResult> Map<Key, Value>::insert(traits::copy_if_trivial_t<const Key&> key, traits::copy_if_trivial_t<const Value&> value)
    noexcept(std::is_nothrow_destructible_v<Key>
      && std::is_nothrow_destructible_v<Value>
      && std::is_nothrow_move_constructible_v<Key>
      && std::is_nothrow_move_constructible_v<Value>
      && std::is_nothrow_copy_constructible_v<Key>
      && std::is_nothrow_copy_constructible_v<Value>)
  {
    if (willReallocate())
      realloc_map();

    const size_t key_hash = GetHash(key);
    size_t prob_index;
    if (find_key(key_hash, key, prob_index, sentinel_metadata, slots))
    {
      new(slots.getPtr() + prob_index) Slot(key, value);
      //Set the slot to ACTIVE
      sentinel_metadata[prob_index] = details::create_active_sentinel(key_hash);
      //Update size
      ++size;
      return { slots.getPtr() + prob_index, InsertionResult::SUCCESS };
    }
    else
      return { slots.getPtr() + prob_index, InsertionResult::EXISTS };
  }
}

#endif //!HG_COLT_MULTIMAP