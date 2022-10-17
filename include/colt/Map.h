#ifndef HG_COLT_MAP
#define HG_COLT_MAP

#include <utility>
#include <iterator>

#include "colt/details/linear_probing.h"
#include "Hash.h"
#include "Vector.h"

namespace colt
{
  template<typename Key, typename Value>
  /// @brief A unordered associative container that contains key/value pairs with unique keys.
  /// @tparam Key The Key that can be hashed through colt::hash or std::hash
  /// @tparam Value The Value that is accessed through the Key
  class Map
  {
    static_assert(!traits::is_tag_v<Key> && traits::is_tag_v<Value>, "Cannot use tag struct as typename!");
    static_assert(traits::is_hashable_v<Key>, "Key of a Map should be hashable!");
    static_assert(traits::is_equal_comparable_v<Key>, "Key of a Map should implement operator==!");

  public:
    using Slot = typename std::pair<const Key, Value>;

  private:
    /// @brief Contains meta-data information about the slots of the map
    Vector<details::KeySentinel> sentinel_metadata = {};
    /// @brief Memory block of the key/value pair
    memory::TypedBlock<Slot> slots = {};
    /// @brief The count of active objects in the container
    size_t size = 0;
    /// @brief The load factor before reallocation
    float load_factor = 0.70f;

    template<typename SlotT>
    /// @brief Map Iterator
    /// @tparam SlotT The Slot type of the Map (for const qualifiers)
    struct MapIterator
    {
      /// @brief Forward Iterator
      using iterator_category = std::forward_iterator_tag;
      /// @brief Value type of the iterator
      using value_type = SlotT;
      /// @brief Pointer type of the iterator
      using pointer = SlotT*;
      /// @brief Reference type of the iterator
      using reference = Slot&;

    private:
      /// @brief Pointer to the current active slot or end()
      SlotT* slot_ptr;
      /// @brief Pointer to the map from which the iterator was constructed
      traits::match_const_t<SlotT, Map>* map_ptr;

    public:
      /// @brief Constructor of MapIterator
      /// @param slot The active slot or end()
      /// @param map_ptr The pointer to the map
      constexpr MapIterator(SlotT* slot, traits::match_const_t<SlotT, Map>* map_ptr) noexcept
        : slot_ptr(slot), map_ptr(map_ptr) {}

      /// @brief Returns a pointer to the current slot pointer to
      /// @return Current slot or end()
      constexpr SlotT* operator->() noexcept { return slot_ptr; }
      /// @brief Returns a pointer to the current slot pointer to
      /// @return Current slot or end()
      constexpr const SlotT* operator->() const noexcept { return slot_ptr; }
      /// @brief Returns a reference to the current slot pointer to
      /// @return Current slot or end()
      constexpr SlotT& operator*() noexcept { return *slot_ptr; }
      /// @brief Returns a reference to the current slot pointer to
      /// @return Current slot or end()
      constexpr const SlotT& operator*() const noexcept { return *slot_ptr; }

      /// @brief Increments the current iterator to the next active slot or end()
      /// @return Self
      constexpr MapIterator& operator++() noexcept;

      /// @brief Post increment operator
      /// @param  Post increment
      /// @return Current iterator
      constexpr MapIterator operator++(int) noexcept;

      /// @brief Check if two MapIterator are equal
      /// @param a First MapIterator
      /// @param b Second MapIterator
      /// @return True if equal
      friend constexpr bool operator==(const MapIterator& a, const MapIterator& b) noexcept { return a.slot_ptr == b.slot_ptr; }
      /// @brief Check if two MapIterator are not equal
      /// @param a First MapIterator
      /// @param b Second MapIterator
      /// @return True if not equal
      friend constexpr bool operator!=(const MapIterator& a, const MapIterator& b) noexcept { return a.slot_ptr != b.slot_ptr; }
    };

  public:
    /// @brief Constructs an empty Map, of load factor 0.7
    constexpr Map(float load_factor = 0.70f) noexcept;

    /// @brief Constructs a Map of load factor 0.7, reserving memory for 'reserve_size' objects
    /// @param reserve_size The count of object to reserve for
    constexpr Map(size_t reserve_size, float load_factor = 0.70f) noexcept;

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
    constexpr size_t get_size() const noexcept { return size; }

    /// @brief Returns the capacity of the current allocation
    /// @return The capacity of the current allocation
    constexpr size_t get_capacity() const noexcept { return slots.get_size(); }
    
    /// @brief Returns a MapIterator to the first active slot in the Map, or end() if no slots are active
    /// @return MapIterator to the first active slot or end()
    constexpr MapIterator<Slot> begin() noexcept;
    /// @brief Returns a MapIterator past the end of the Map
    /// @return MapIterator that should not be dereferenced
    constexpr MapIterator<Slot> end() noexcept;
    /// @brief Returns a MapIterator to the first active slot in the Map, or end() if no slots are active
    /// @return MapIterator to the first active slot or end()
    constexpr MapIterator<const Slot> begin() const noexcept;
    /// @brief Returns a MapIterator past the end of the Map
    /// @return MapIterator that should not be dereferenced
    constexpr MapIterator<const Slot> end() const noexcept;

    /// @brief Check if the Map is empty
    /// @return True if the Map is empty
    constexpr bool is_empty() const noexcept { return size == 0; }
    /// @brief Check if the Map is not empty
    /// @return True if the Map is not empty
    constexpr bool is_not_empty() const noexcept { return size != 0; }

    /// @brief Check if the Map will reallocate on the next call of insert/insertOrAssign
    /// @return True if the Map will reallocate
    constexpr bool will_reallocate() const noexcept;

    /// @brief Returns the load factor of the Map
    /// @return The load factor
    constexpr float get_load_factor() const noexcept { return load_factor; }

    /// @brief Sets the load factor to 'nload_factor'.
    /// Precondition: nload_factor < 1.0f && nload_factor > 0.0f.
    /// @param nload_factor The new load factor
    constexpr void set_load_factor(float nload_factor) noexcept;

    /// @brief Finds the key/value pair of key 'key'
    /// @param key The key to search for
    /// @return Pointer to the key/value pair if found, or null
    constexpr const Slot* find(traits::copy_if_trivial_t<const Key&> key) const noexcept;

    /// @brief Finds the key/value pair of key 'key'
    /// @param key The key to search for
    /// @return Pointer to the key/value pair if found, or null
    constexpr Slot* find(traits::copy_if_trivial_t<const Key&> key) noexcept;

    /// @brief Check if the Map contains a key/value pair of key 'key'.
    /// Prefer using 'find' if the value which is being checked for will be used.
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
    constexpr std::pair<Slot*, InsertionResult> insert_or_assign(traits::copy_if_trivial_t<const Key&> key, traits::copy_if_trivial_t<const Value&> value)
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

    constexpr void reserve(size_t by_more)
      noexcept(std::is_nothrow_destructible_v<Key>
        && std::is_nothrow_destructible_v<Value>
        && std::is_nothrow_move_constructible_v<Key>
        && std::is_nothrow_move_constructible_v<Value>);

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

    /// @brief Augments the capacity of the Map, rehashing in the process
    /// @param new_capacity The new capacity of the map
    constexpr void realloc_map(size_t new_capacity)
      noexcept(std::is_nothrow_destructible_v<Key>
        && std::is_nothrow_destructible_v<Value>
        && std::is_nothrow_move_constructible_v<Key>
        && std::is_nothrow_move_constructible_v<Value>);
  };

  template<typename Key, typename Value>
  constexpr bool Map<Key, Value>::find_key(size_t key_hash, traits::copy_if_trivial_t<const Key&> key, size_t& prob, const Vector<details::KeySentinel>& metadata, memory::TypedBlock<Slot> blk) noexcept
  {
    assert(key_hash == GetHash(key));
    assert(metadata.get_size() == blk.get_size());
    size_t prob_index = key_hash % blk.get_size();
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
        if (blk.get_ptr()[prob_index].first == key)
        {
          prob = prob_index;
          return false;
        }
      }
      prob_index = details::advance_prob(prob_index, blk.get_size());
    }
  }

  template<typename Key, typename Value>
  constexpr void Map<Key, Value>::realloc_map(size_t new_capacity) noexcept(std::is_nothrow_destructible_v<Key>&& std::is_nothrow_destructible_v<Value>&& std::is_nothrow_move_constructible_v<Key>&& std::is_nothrow_move_constructible_v<Value>)
  {
    auto new_slot = memory::allocate({ new_capacity * sizeof(Slot) });

    Vector<details::KeySentinel> new_metadata = { new_capacity, InPlace, details::EMPTY };
    for (size_t i = 0; i < sentinel_metadata.get_size(); i++)
    {
      auto sentinel = sentinel_metadata[i];
      if (details::is_sentinel_active(sentinel))
      {
        //find the key
        Slot* ptr = slots.get_ptr() + i;
        const size_t key_hash = GetHash(ptr->first);
        size_t prob_index;
        //Rehash the key to get its new index in the new array
        if (find_key(key_hash, ptr->first, prob_index, sentinel_metadata, slots))
        {
          //Move destruct
          new(slots.get_ptr() + prob_index) Slot(std::move(*ptr));
          ptr->~Slot();

          //Set the slot to ACTIVE
          new_metadata[prob_index] = details::create_active_sentinel(key_hash);
        }
      }
    }
    sentinel_metadata = std::move(new_metadata);
    memory::deallocate(slots);
    slots = new_slot;
  }

  template<typename Key, typename Value>
  constexpr Map<Key, Value>::Map(float load_factor) noexcept
    : load_factor(load_factor)
  {
    assert(0.0f < load_factor && load_factor < 1.0f && "Invalid load factor!");
  }

  template<typename Key, typename Value>
  constexpr Map<Key, Value>::Map(size_t reserve_size, float load_factor) noexcept
    : sentinel_metadata(reserve_size, InPlace, details::EMPTY)
    , slots(memory::allocate({ reserve_size * sizeof(Slot) }))
    , load_factor(load_factor)
  {
    assert(0.0f < load_factor && load_factor < 1.0f && "Invalid load factor!");
  }

  template<typename Key, typename Value>
  Map<Key, Value>::~Map() noexcept(std::is_nothrow_destructible_v<Key>&& std::is_nothrow_destructible_v<Value>)
  {
    clear();
    memory::deallocate(slots);
  }

  template<typename Key, typename Value>
  constexpr void Map<Key, Value>::clear() noexcept(std::is_nothrow_destructible_v<Key>&& std::is_nothrow_destructible_v<Value>)
  {
    for (size_t i = 0; i < sentinel_metadata.get_size(); i++)
    {
      if (details::is_sentinel_active(sentinel_metadata[i]))
        slots.get_ptr()[i].~Slot(); //destroy active slots
    }
    size = 0;
  }

  template<typename Key, typename Value>
  constexpr Map<Key, Value>::MapIterator<std::pair<const Key, Value>> Map<Key, Value>::begin() noexcept
  {
    for (size_t i = 0; i < sentinel_metadata.get_size(); i++)
    {
      if (details::is_sentinel_active(sentinel_metadata[i]))
        return { slots.get_ptr() + i, this };
    }
    return end();
  }

  template<typename Key, typename Value>
  constexpr Map<Key, Value>::MapIterator<std::pair<const Key, Value>> Map<Key, Value>::end() noexcept
  {
    return { slots.get_ptr() + slots.get_size(), this };
  }

  template<typename Key, typename Value>
  constexpr Map<Key, Value>::MapIterator<const std::pair<const Key, Value>> Map<Key, Value>::begin() const noexcept
  {
    for (size_t i = 0; i < sentinel_metadata.get_size(); i++)
    {
      if (details::is_sentinel_active(sentinel_metadata[i]))
        return { slots.get_ptr() + i, this };
    }
    return end();
  }

  template<typename Key, typename Value>
  constexpr Map<Key, Value>::MapIterator<const std::pair<const Key, Value>> Map<Key, Value>::end() const noexcept
  {
    return { slots.get_ptr() + slots.get_size(), this };
  }

  template<typename Key, typename Value>
  constexpr bool Map<Key, Value>::will_reallocate() const noexcept
  {
    return float(get_size() + 1) > load_factor * get_capacity();
  }

  template<typename Key, typename Value>
  constexpr void Map<Key, Value>::set_load_factor(float nload_factor) noexcept
  {
    assert(nload_factor < 1.0f && nload_factor > 0.0f && "Invalid load factor!");
    load_factor = nload_factor;
  }

  template<typename Key, typename Value>
  constexpr typename Map<Key, Value>::Slot* Map<Key, Value>::find(traits::copy_if_trivial_t<const Key&> key) noexcept
  {
    //No UB as the map is not const
    return const_cast<Slot*>(static_cast<const Map*>(this)->find(key));
  }

  template<typename Key, typename Value>
  constexpr const std::pair<const Key, Value>* Map<Key, Value>::find(traits::copy_if_trivial_t<const Key&> key) const noexcept
  {
    const size_t key_hash = GetHash(key);
    size_t prob_index = key_hash % slots.get_size();
    for (;;)
    {
      if (auto sentinel = sentinel_metadata[prob_index];
        details::is_sentinel_empty(sentinel))
      {
        return nullptr; //not found
      }
      else if (details::is_sentinel_deleted(sentinel))
      {
        prob_index = details::advance_prob(prob_index, slots.get_size());
        continue;
      }
      else if (details::is_sentinel_equal(sentinel, key_hash))
      {
        if (slots.get_ptr()[prob_index].first == key)
          return slots.get_ptr() + prob_index;
      }
      prob_index = details::advance_prob(prob_index, slots.get_size());
    }
  }

  template<typename Key, typename Value>
  constexpr bool Map<Key, Value>::contains(traits::copy_if_trivial_t<const Key&> key) const noexcept
  {
    return find(key) != nullptr;
  }

  template<typename Key, typename Value>
  constexpr std::pair<typename Map<Key, Value>::Slot*, InsertionResult> Map<Key, Value>::insert_or_assign(traits::copy_if_trivial_t<const Key&> key, traits::copy_if_trivial_t<const Value&> value)
    noexcept(std::is_nothrow_destructible_v<Key>
      && std::is_nothrow_destructible_v<Value>
      && std::is_nothrow_move_constructible_v<Key>
      && std::is_nothrow_move_constructible_v<Value>
      && std::is_nothrow_copy_constructible_v<Key>
      && std::is_nothrow_copy_constructible_v<Value>
      && std::is_nothrow_copy_assignable_v<Value>)
  {
    if (will_reallocate())
      realloc_map(get_capacity() + 16);

    const size_t key_hash = GetHash(key);
    size_t prob_index;
    if (find_key(key_hash, key, prob_index, sentinel_metadata, slots))
    {
      new(slots.get_ptr() + prob_index) Slot(key, value);
      //Set the slot to ACTIVE
      sentinel_metadata[prob_index] = details::create_active_sentinel(key_hash);
      //Update size
      ++size;
      return { slots.get_ptr() + prob_index, InsertionResult::SUCCESS };
    }
    else
    {
      slots.get_ptr()[prob_index].second = value;
      return { slots.get_ptr() + prob_index, InsertionResult::ASSIGNED };
    }
  }

  template<typename Key, typename Value>
  constexpr bool Map<Key, Value>::erase(traits::copy_if_trivial_t<const Key&> key) noexcept(std::is_nothrow_destructible_v<Key>&& std::is_nothrow_destructible_v<Value>)
  {
    if (Slot* ptr = find(key))
    {
      size_t index = (slots.get_ptr() + slots.get_size()) - ptr;
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
  constexpr void Map<Key, Value>::reserve(size_t by_more) noexcept(std::is_nothrow_destructible_v<Key>&& std::is_nothrow_destructible_v<Value>&& std::is_nothrow_move_constructible_v<Key>&& std::is_nothrow_move_constructible_v<Value>)
  {
    realloc_map(by_more);
  }

  template<typename Key, typename Value>
  constexpr std::pair<typename Map<Key, Value>::Slot*, InsertionResult> Map<Key, Value>::insert(traits::copy_if_trivial_t<const Key&> key, traits::copy_if_trivial_t<const Value&> value)
    noexcept(std::is_nothrow_destructible_v<Key>
      && std::is_nothrow_destructible_v<Value>
      && std::is_nothrow_move_constructible_v<Key>
      && std::is_nothrow_move_constructible_v<Value>
      && std::is_nothrow_copy_constructible_v<Key>
      && std::is_nothrow_copy_constructible_v<Value>)
  {
    if (will_reallocate())
      realloc_map(get_capacity() + 16);

    const size_t key_hash = GetHash(key);
    size_t prob_index;
    if (find_key(key_hash, key, prob_index, sentinel_metadata, slots))
    {
      new(slots.get_ptr() + prob_index) Slot(key, value);
      //Set the slot to ACTIVE
      sentinel_metadata[prob_index] = details::create_active_sentinel(key_hash);
      //Update size
      ++size;
      return { slots.get_ptr() + prob_index, InsertionResult::SUCCESS };
    }
    else
      return { slots.get_ptr() + prob_index, InsertionResult::EXISTS };
  }

  template<typename Key, typename Value>
  template<typename SlotT>
  constexpr Map<Key, Value>::MapIterator<SlotT>& Map<Key, Value>::MapIterator<SlotT>::operator++() noexcept
  {
    size_t index = slot_ptr - map_ptr->slots.get_ptr() + 1;
    for (size_t i = index; i < map_ptr->sentinel_metadata.get_size(); i++)
    {
      if (details::is_sentinel_active(map_ptr->sentinel_metadata[i]))
      {
        slot_ptr = map_ptr->slots.get_ptr() + i;
        return *this;
      }
    }
    //Set to end()
    slot_ptr = map_ptr->slots.get_ptr() + map_ptr->slots.get_size();
    return *this;
  }

  template<typename Key, typename Value>
  template<typename SlotT>
  constexpr Map<Key, Value>::MapIterator<SlotT> Map<Key, Value>::MapIterator<SlotT>::operator++(int) noexcept
  {
    MapIterator to_ret = *this; //copy
    ++(*this); //increment
    return to_ret;
  }

#ifdef COLT_USE_IOSTREAMS

  template<typename Key, typename Value>
  static std::ostream& operator<<(std::ostream& os, const Map<Key, Value>& var) noexcept
  {
    static_assert(traits::is_coutable_v<Key>, "Key of Map should implement operator<<(std::ostream&)!");
    static_assert(traits::is_coutable_v<Value>, "Value of Map should implement operator<<(std::ostream&)!");

    auto begin_it = var.begin();
    os << '[';
    if (begin_it != var.end())
    {
      os << "{ " << begin_it->first << ": " << begin_it->second << " }";
      ++begin_it;
    }
    while (begin_it != var.end())
    {
      os << ", { " << begin_it->first << ": " << begin_it->second << " }";
      ++begin_it;
    }
    os << ']';
    return os;
  }

#endif
}

#endif //!HG_COLT_MULTIMAP