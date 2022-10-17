#ifndef HG_COLT_SET
#define HG_COLT_SET

#include "colt/details/linear_probing.h"
#include "colt/Hash.h"
#include "colt/List.h"

namespace colt
{
  template<typename T, size_t obj_per_node = 256>
  /// @brief An ordered container without duplicates that guarantees iterator validity for its lifetime.
  /// This StableSet is implemented using an internal hash table and a doubly linked list.
  /// The doubly linked list is a 'FlatList', of 'obj_per_node' equal to 'obj_per_node', which
  /// preserves the insertion order, and iterator validity.
  /// @tparam T The type to store
  class StableSet
  {
    static_assert(traits::is_hashable_v<T>, "'T' should be hashable!");
    static_assert(traits::is_equal_comparable_v<T>, "'T' should implement operator==!");
    
    using Slot = std::pair<size_t, T*>;

    /// @brief Contains meta-data information about the slots of the map
    Vector<details::KeySentinel> sentinel_metadata = {};
    /// @brief Memory block of the pointers
    memory::TypedBlock<Slot> slots = {};
    /// @brief The list containing the objects
    FlatList<T, obj_per_node> list;
    /// @brief The load factor before reallocation
    float load_factor = 0.70f;

  public:

    /// @brief Constructor
    /// @param load_factor The load factor (> 0.0f && < 1.0f)
    constexpr StableSet(float load_factor = 0.70f) noexcept;

    /// @brief Constructor, which reserves 'reserve_size' capacity for objects
    /// @param reserve_size The capacity to reserve
    /// @param load_factor The load factor (> 0.0f && < 1.0f)
    constexpr StableSet(size_t reserve_size, float load_factor = 0.70f) noexcept;      

    /// @brief Destructor of the StableSet
    ~StableSet()
      noexcept(std::is_nothrow_destructible_v<T>);    

    /// @brief Returns the number of active unique elements in the StableSet
    /// @return The count of active unique elements
    constexpr size_t get_size() const noexcept { return list.get_size(); }
    
    /// @brief Returns the capacity of the internal hash map used by the StableSet.
    /// @return The capacity of the internal map of the StableSet
    constexpr size_t get_capacity() const noexcept { return slots.get_size(); }

    /// @brief Check if the StableSet is empty
    /// @return True if empty
    constexpr bool is_empty() const noexcept { return list.get_size() == 0; }
    /// @brief Check if the StableSet is not empty
    /// @return True if not empty
    constexpr bool is_not_empty() const noexcept { return list.get_size() != 0; }

    /// @brief Returns const iterators to the beginning of the StableSet
    /// @return Iterator to the beginning of the set
    constexpr auto begin() const noexcept -> decltype(list.begin()) { return list.begin(); }
    /// @brief Returns const iterators to the end of the StableSet
    /// @return Iterator to the end of the set
    constexpr auto end() const noexcept -> decltype(list.begin()) { return list.end(); }

    /// @brief Return the nth value inserted (with n being index)
    /// @param index The insertion number
    /// @return The nth value inserted (with n being index)
    constexpr traits::copy_if_trivial_t<const T&> operator[](size_t index) const noexcept;    

    /// @brief Check if the internal hash map used by the StableSet will rehash on the next insertion
    /// @return True if the next insert will rehash the internal map of the StableSet
    constexpr bool will_reallocate() const noexcept;    

    /// @brief Returns the load factor of the Map
    /// @return The load factor
    constexpr float get_load_factor() const noexcept { return load_factor; }
    /// @brief Sets the load factor to 'nload_factor'.
    /// Precondition: nload_factor < 1.0f && nload_factor > 0.0f.
    /// @param nload_factor The new load factor
    constexpr void set_load_factor(float nload_factor) noexcept;    

    /// @brief Returns a const reference to the internal list used by the StableSet
    /// @return Const reference to the list
    constexpr const FlatList<T, obj_per_node>& get_internal_list() const noexcept { return list; }

    /// @brief Inserts a new value if it does not already exist.
    /// Returns an InsertionResult SUCCESS (if the insertion was performed) or EXISTS (if the key already exists).
    /// The returned pointer is to the newly inserted value on SUCCESS.
    /// The returned pointer is to the existing value on EXISTS.
    /// The returned pointer is never null.
    /// @param key The value to insert
    /// @return Pair of pointer to the inserted slot or the existent one, and SUCESS on insertion or EXISTS if the key already exists
    constexpr std::pair<T*, InsertionResult> insert(traits::copy_if_trivial_t<const T&> key)
      noexcept(std::is_nothrow_copy_constructible_v<T>);

    template<typename T_ = T, typename = std::enable_if_t<!std::is_trivial_v<T_>>>
    /// @brief Inserts a new value if it does not already exist.
    /// Returns an InsertionResult SUCCESS (if the insertion was performed) or EXISTS (if the key already exists).
    /// The returned pointer is to the newly inserted value on SUCCESS.
    /// The returned pointer is to the existing value on EXISTS.
    /// The returned pointer is never null.
    /// @tparam T_ SFINAE helper
    /// @tparam  SFINAE helper
    /// @param key The value to insert
    /// @return Pair of pointer to the inserted slot or the existent one, and SUCESS on insertion or EXISTS if the key already exists
    constexpr std::pair<T*, InsertionResult> insert(T&& key)
      noexcept(std::is_nothrow_move_constructible_v<T>);
    
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
    static constexpr bool find_key(size_t key_hash, traits::copy_if_trivial_t<const T&> key, size_t& prob,
      const Vector<details::KeySentinel>& metadata, memory::TypedBlock<Slot> blk) noexcept;    

    /// @brief Augments the capacity of the internal hash map, rehashing in the process
    /// @param new_capacity The new capacity of the map
    constexpr void realloc_map(size_t new_capacity) noexcept;    
  };
  
  template<typename T, size_t obj_per_node>
  constexpr StableSet<T, obj_per_node>::StableSet(float load_factor) noexcept
    : load_factor(load_factor)
  {
    assert(0.0f < load_factor && load_factor < 1.0f && "Invalid load factor!");
  }

  template<typename T, size_t obj_per_node>
  constexpr StableSet<T, obj_per_node>::StableSet(size_t reserve_size, float load_factor) noexcept
    : sentinel_metadata(reserve_size, InPlace, details::EMPTY)
    , slots(memory::allocate({ reserve_size * sizeof(Slot) }))
    , list(reserve_size / obj_per_node)
    , load_factor(load_factor)
  {
    assert(0.0f < load_factor && load_factor < 1.0f && "Invalid load factor!");
  }

  template<typename T, size_t obj_per_node>
  StableSet<T, obj_per_node>::~StableSet()
    noexcept(std::is_nothrow_destructible_v<T>)
  {
    for (size_t i = 0; i < sentinel_metadata.get_size(); i++)
    {
      if (details::is_sentinel_active(sentinel_metadata[i]))
        slots.get_ptr()[i].~Slot(); //destroy active slots
    }
    memory::deallocate(slots);
  }
  
  template<typename T, size_t obj_per_node>
  constexpr traits::copy_if_trivial_t<const T&> StableSet<T, obj_per_node>::operator[](size_t index) const noexcept
  {
    assert(index < list.get_size() && "Invalid index!");
    return list[index];
  }

  template<typename T, size_t obj_per_node>
  constexpr bool StableSet<T, obj_per_node>::will_reallocate() const noexcept
  {
    return float(get_size() + 1) > load_factor * get_capacity();
  }
  
  template<typename T, size_t obj_per_node>
  constexpr void StableSet<T, obj_per_node>::set_load_factor(float nload_factor) noexcept
  {
    assert(nload_factor < 1.0f && nload_factor > 0.0f && "Invalid load factor!");
    load_factor = nload_factor;
  }
  
  template<typename T, size_t obj_per_node>
  constexpr std::pair<T*, InsertionResult> StableSet<T, obj_per_node>::insert(traits::copy_if_trivial_t<const T&> key) noexcept(std::is_nothrow_copy_constructible_v<T>)
  {
    if (will_reallocate())
      realloc_map(get_capacity() + 16);

    const size_t key_hash = GetHash(key);
    size_t prob_index;
    if (find_key(key_hash, key, prob_index, sentinel_metadata, slots))
    {
      list.push_back(key);
      T* to_ret = &list[list.get_size() - 1]; // always safe as push_backed the value
      new(slots.get_ptr() + prob_index) Slot(key_hash, to_ret);
      //Set the slot to ACTIVE
      sentinel_metadata[prob_index] = details::create_active_sentinel(key_hash);
      return { to_ret, InsertionResult::SUCCESS };
    }
    else
      return { slots.get_ptr()[prob_index].second, InsertionResult::EXISTS };
  }

  template<typename T, size_t obj_per_node>
  template<typename T_, typename>
  constexpr std::pair<T*, InsertionResult> StableSet<T, obj_per_node>::insert(T&& key) noexcept(std::is_nothrow_move_constructible_v<T>)
  {
    if (will_reallocate())
      realloc_map(get_capacity() + 16);

    const size_t key_hash = GetHash(key);
    size_t prob_index;
    if (find_key(key_hash, key, prob_index, sentinel_metadata, slots))
    {
      list.push_back(std::move(key));
      T* to_ret = &list[list.get_size() - 1]; // always safe as push_backed the value
      new(slots.get_ptr() + prob_index) Slot(key_hash, to_ret);
      //Set the slot to ACTIVE
      sentinel_metadata[prob_index] = details::create_active_sentinel(key_hash);
      return { to_ret, InsertionResult::SUCCESS };
    }
    else
      return { slots.get_ptr()[prob_index].second, InsertionResult::EXISTS };
  }
  
  template<typename T, size_t obj_per_node>
  constexpr bool StableSet<T, obj_per_node>::find_key(size_t key_hash, traits::copy_if_trivial_t<const T&> key, size_t& prob, const Vector<details::KeySentinel>& metadata, memory::TypedBlock<Slot> blk) noexcept
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
        if (*(blk.get_ptr()[prob_index].second) == key)
        {
          prob = prob_index;
          return false;
        }
      }
      prob_index = details::advance_prob(prob_index, blk.get_size());
    }
  }
  
  template<typename T, size_t obj_per_node>
  constexpr void StableSet<T, obj_per_node>::realloc_map(size_t new_capacity) noexcept
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
        if (find_key(key_hash, *ptr->second, prob_index, sentinel_metadata, slots))
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

#ifdef COLT_USE_IOSTREAMS

  template<typename T, size_t size>
  static std::ostream& operator<<(std::ostream& os, const StableSet<T, size>& var)
  {
    os << var.get_internal_list();
    return os;
  }

#endif
}

#endif //!HG_COLT_SET