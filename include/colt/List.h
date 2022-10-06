#ifndef HG_COLT_LIST
#define HG_COLT_LIST

#include <colt/Vector.h>

namespace colt
{
  template<typename T, size_t obj_per_node = 16>
  /// @brief A doubly linked list, that contains multiple object per nodes.
  /// @tparam T The type to store
  class FlatList
  {
    /// @brief Doubly linked Node
    struct Node
    {
      /// @brief Owning pointer (to free) pointing to the next node
      Node* after = nullptr;
      /// @brief Pointer to the node before
      Node* before = nullptr;
      /// @brief The data
      StaticVector<T, obj_per_node> data;
    };

    template<typename Node_t>
    class Iterator
    {
      size_t node_index;
      Node_t* current_node;

    public:
      constexpr Iterator(Node_t* node, size_t index = 0) noexcept
        : node_index(index), current_node(node) { assert(index < obj_per_node); }

      constexpr Iterator& operator++() noexcept
      {
        if (node_index + 1 == obj_per_node)
        {
          current_node = current_node->after;
          node_index = 0;
        }
        else
          ++node_index;
        return *this;
      }

      constexpr Iterator operator++(int) noexcept
      {
        auto copy = *this;
        ++(*this);
        return copy;
      }

      constexpr Iterator& operator--() noexcept
      {
        if (node_index - 1 == 0)
        {
          current_node = current_node->before;
          node_index = obj_per_node - 1;
        }
        else
          --node_index;
        return *this;
      }

      constexpr Iterator operator--(int) noexcept
      {
        auto copy = *this;
        --(*this);
        return copy;
      }

      constexpr T* operator->() noexcept { return current_node->data.get_data() + node_index; }
      constexpr const T* operator->() const noexcept { return current_node->data.get_data() + node_index; }
      constexpr T& operator*() noexcept { return current_node->data[node_index]; }
      constexpr const T& operator*() const noexcept { return current_node->data[node_index]; }

      friend constexpr bool operator==(const Iterator& i1, const Iterator& i2) noexcept
      {
        return i1.node_index == i2.node_index && i1.current_node == i2.current_node;
      }

      friend constexpr bool operator!=(const Iterator& i1, const Iterator& i2) noexcept
      {
        return !(i1 == i2);
      }
    };

    /// @brief Owning pointer to the head of the list, never null
    Node* head = memory::new_t<Node>().get_ptr();
    /// @brief Pointer to the tail of the list
    Node* tail = head;
    /// @brief Pointer to the last active node of the list
    Node* last_active_node = head;
    /// @brief Count of active elements
    size_t size = 0;

  public:
    /// @brief Constructs an empty FlatList.
    /// This will always preallocate one Node.
    constexpr FlatList() noexcept {}

    /// @brief Constructs an empty FlatList, reserving node_reserve_count + 1.
    /// Each Node can contain up to 'obj_per_node'.
    /// @param node_reserve_count The count of nodes to preallocate
    constexpr FlatList(size_t node_reserve_count) noexcept;    

    /// @brief Destructor, frees any used resources
    ~FlatList()
      noexcept(std::is_nothrow_destructible_v<T>);    

    /// @brief Clears the FlatList from all its item.
    /// This does not modify the capacity of the list.
    constexpr void clear()
      noexcept(std::is_nothrow_destructible_v<T>);    

    /// @brief Check if the FlatList is empty.
    /// Equivalent to 'get_size() == 0'.
    /// @return True if the list is empty
    constexpr bool is_empty() const noexcept { return size == 0; }
    /// @brief Check if the FlatList is not empty.
    /// Equivalent to 'get_size() != 0'.
    /// @return True if the list is not empty
    constexpr bool is_not_empty() const noexcept { return size != 0; }

    /// @brief Returns the size of the FlatList
    /// @return The count of active objects in the list
    constexpr size_t get_size() const noexcept { return size; }

    /// @brief Returns the first object in the list.
    /// Precondition: is_not_empty().
    /// @return The first object in the list
    constexpr traits::copy_if_trivial_t<const T&> get_front() const noexcept;
    /// @brief Returns the first object in the list.
    /// Precondition: is_not_empty().
    /// @return The first object in the list
    constexpr T& get_front() noexcept;    

    /// @brief Returns the last object in the list.
    /// Precondition: is_not_empty().
    /// @return The first object in the list
    constexpr traits::copy_if_trivial_t<const T&> get_back() const noexcept;
    /// @brief Returns the last object in the list.
    /// Precondition: is_not_empty().
    /// @return The first object in the list
    constexpr T& get_back() noexcept;

    constexpr Iterator<Node> begin() noexcept { return { head, 0 }; }
    constexpr Iterator<const Node> begin() const noexcept { return { head, 0 }; }

    constexpr Iterator<Node> end() noexcept
    {
      if (last_active_node->data.is_full())
        return { last_active_node->after, 0 };
      return { last_active_node, last_active_node->data.get_size() };
    }
    constexpr Iterator<const Node> end() const noexcept
    {
      if (last_active_node->data.is_full())
        return { last_active_node->after, 0 };
      return { last_active_node, last_active_node->data.get_size() };
    }

    /// @brief Appends an item to the end of the list
    /// @param to_copy The item to append
    constexpr void push_back(traits::copy_if_trivial_t<const T&> to_copy)
      noexcept(std::is_nothrow_copy_constructible_v<T>);

    template<typename T_ = T, typename = std::enable_if_t<!std::is_trivial_v<T_>>>
    /// @brief Appends an item to the end of the list
    /// @tparam T_ SFINAE helper
    /// @tparam  SFINAE helper
    /// @param to_move The item to append
    constexpr void push_back(T&& to_move)
      noexcept(std::is_nothrow_move_constructible_v<T>);    

    template<typename... Args>
    /// @brief Appends an item to the end of the list, constructing it in place
    /// @tparam ...Args The parameter pack
    /// @param  Tag struct (InPlace)
    /// @param ...args The argument pack
    constexpr void push_back(traits::InPlaceT, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<T, Args...>);    

    /// @brief Returns the object at index 'index' from the FlatList.
    /// Precondition: index < get_size().
    /// @param index The index
    /// @return The object at index 'index'
    constexpr traits::copy_if_trivial_t<const T&> operator[](size_t index) const noexcept;    
    /// @brief Returns the object at index 'index' from the FlatList.
    /// Precondition: index < get_size().
    /// @param index The index
    /// @return The object at index 'index'
    constexpr T& operator[](size_t index) noexcept;    

  private:
    /// @brief Creates an empty node
    /// @return Node allocated through the colt global allocator
    constexpr Node* create_node() noexcept { return memory::new_t<Node>().get_ptr(); }

    /// @brief Creates and appends a node to the end of the list
    constexpr void create_and_append_node() noexcept;    
    
    /// @brief Advances the last_active_node to the next node, appending a new node if needed
    constexpr void advance_active_node() noexcept;    
  };
  
  template<typename T, size_t obj_per_node>
  constexpr FlatList<T, obj_per_node>::FlatList(size_t node_reserve_count) noexcept
  {
    for (size_t i = 0; i < node_reserve_count; i++)
      create_and_append_node();
  }

  template<typename T, size_t obj_per_node>
  FlatList<T, obj_per_node>::~FlatList() noexcept(std::is_nothrow_destructible_v<T>)
  {
    clear();
    //Free the head node, which is always allocated
    memory::delete_t(memory::TypedBlock<Node>{ head, sizeof(Node) });
  }

  template<typename T, size_t obj_per_node>
  constexpr void FlatList<T, obj_per_node>::clear() noexcept(std::is_nothrow_destructible_v<T>)
  {
    while (tail != head)
    {
      auto before = tail->before; //store the tail's preceding 
      memory::delete_t(memory::TypedBlock<Node>{ tail, sizeof(Node) });
      tail = before;
    }
    head->data.clear();
    last_active_node = head;
  }

  template<typename T, size_t obj_per_node>
  constexpr traits::copy_if_trivial_t<const T&> FlatList<T, obj_per_node>::get_front() const noexcept
  {
    assert(is_not_empty() && "List was empty!");
    return head->data.get_front();
  }

  template<typename T, size_t obj_per_node>
  constexpr T& FlatList<T, obj_per_node>::get_front() noexcept
  {
    assert(is_not_empty() && "List was empty!");
    return head->data.get_front();
  }

  template<typename T, size_t obj_per_node>
  constexpr traits::copy_if_trivial_t<const T&> FlatList<T, obj_per_node>::get_back() const noexcept
  {
    assert(is_not_empty() && "List was empty!");
    return last_active_node->data.get_back();
  }

  template<typename T, size_t obj_per_node>
  constexpr T& FlatList<T, obj_per_node>::get_back() noexcept
  {
    assert(is_not_empty() && "List was empty!");
    return last_active_node->data.get_back();
  }

  template<typename T, size_t obj_per_node>
  constexpr void FlatList<T, obj_per_node>::push_back(traits::copy_if_trivial_t<const T&> to_copy) noexcept(std::is_nothrow_copy_constructible_v<T>)
  {
    if (last_active_node->data.get_size() == last_active_node->data.get_capacity())
      advance_active_node();
    last_active_node->data.push_back(to_copy);
    ++size;
  }

  template<typename T, size_t obj_per_node>
  constexpr traits::copy_if_trivial_t<const T&> FlatList<T, obj_per_node>::operator[](size_t index) const noexcept
  {
    assert(index < size && "Invalid index for list!");
    //Compiler should optimize this into a single instruction
    size_t div = index / obj_per_node;
    size_t rem = index % obj_per_node;

    auto head_c = head;
    while (div-- != 0)
      head_c = head_c->after;
    return head_c->data[rem];
  }

  template<typename T, size_t obj_per_node>
  constexpr T& FlatList<T, obj_per_node>::operator[](size_t index) noexcept
  {
    assert(index < size && "Invalid index for list!");
    //Compiler should optimize this into a single instruction
    size_t div = index / obj_per_node;
    size_t rem = index % obj_per_node;

    auto head_c = head;
    while (div-- != 0)
      head_c = head_c->after;
    return head_c->data[rem];
  }

  template<typename T, size_t obj_per_node>
  constexpr void FlatList<T, obj_per_node>::create_and_append_node() noexcept
  {
    auto before = tail; //copy pointer
    tail = create_node(); //set the tail to the new Node
    tail->before = before; //set the tail before to the old tail
    before->after = tail; //set the old tail's after to the new node
  }

  template<typename T, size_t obj_per_node>
  constexpr void FlatList<T, obj_per_node>::advance_active_node() noexcept
  {
    if (last_active_node == tail)
    {
      create_and_append_node();
      last_active_node = tail;
    }
    else
      last_active_node = last_active_node->after;
  }

  template<typename T, size_t obj_per_node>
  template<typename T_, typename>
  constexpr void FlatList<T, obj_per_node>::push_back(T&& to_move) noexcept(std::is_nothrow_move_constructible_v<T>)
  {
    if (last_active_node->data.get_size() == last_active_node->data.get_capacity())
      advance_active_node();
    last_active_node->data.push_back(std::move(to_move));
    ++size;
  }

  template<typename T, size_t obj_per_node>
  template<typename ...Args>
  constexpr void FlatList<T, obj_per_node>::push_back(traits::InPlaceT, Args && ...args) noexcept(std::is_nothrow_constructible_v<T, Args ...>)
  {
    if (last_active_node->data.get_size() == last_active_node->data.get_capacity())
      advance_active_node();
    last_active_node->data.push_back(InPlace, std::forward<Args>(args)...);
    ++size;
  }

#ifdef COLT_USE_IOSTREAMS

  template<typename T, size_t size>
  static std::ostream& operator<<(std::ostream& os, const FlatList<T, size>& var)
  {
    os << '[';
    if (var.is_not_empty())
      os << var.get_front();
    for (size_t i = 1; i < var.get_size(); i++)
      os << ", " << var[i];
    os << ']';
    return os;
  }
#endif
}

#endif //!HG_COLT_LIST