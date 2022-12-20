#ifndef HG_COLT_LINEAR_PROBING
#define HG_COLT_LINEAR_PROBING

#include "common.h"

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
      return (hash & 0b01111111) == (key & 0b01111111);
    }

    /// @brief Increments a probing index, faster than a modulo operation.
    /// Equivalent to a faster 'prob + 1 % slots.get_size()', but all increment
    /// done one a probe should pass by this functions.
    /// @param prob The probing index to increment
    /// @return The incremented index
    constexpr size_t advance_prob(size_t prob, size_t mod) noexcept
    {
      //If this asserts then the optimization should be checked
      assert((prob + 1) % mod == ((prob + 1) * (prob + 1 != mod)));
      return  (prob + 1) * (prob + 1 != mod);
    }
  }

  /// @brief Represents the result of an insert/insert_or_assign operation
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
}

#endif //!HG_COLT_LINEAR_PROBING