#ifndef HG_COLT_STRING
#define HG_COLT_STRING

#include <cctype>

#include "char.h"
#include "vector.h"

namespace colt
{
  template<typename CharT = char>
  class StringView
    : public ContiguousView<CharT>
  {
    static_assert(std::is_same_v<CharT, char>, "StringView only supports char for now!");

    using View = ContiguousView<CharT>;

  public:
    /// @brief Range constructor
    /// @param begin The beginning of the view
    /// @param end The end of the view
    constexpr StringView(const CharT* begin, const CharT* end) noexcept
      : View(begin, end) {}
    /// @brief Copy constructor
    /// @param  The StringView to copy
    constexpr StringView(const StringView&) noexcept = default;
    /// @brief Move constructor
    /// @param  The StringView to move
    constexpr StringView(StringView&&) noexcept = default;

    /// @brief Pops all spaces from the beginning and the end of the StringView
    constexpr void strip() noexcept;    
    
    /// @brief Conversion operator
    /// @return ContiguousView
    constexpr operator ContiguousView<CharT>() const noexcept;    
  };

  /// @brief Contiguous array of characters.
  /// Possesses a small buffer of 16 characters.
  template<typename CharT = char>
  class String
    : public SmallVector<CharT, 16>
  {
    static_assert(std::is_same_v<CharT, char>, "String only supports char for now!");
    
    using Str = SmallVector<CharT, 16>;    

  public:
    /// @brief Default constructor
    constexpr String() noexcept = default;
    /// @brief Copy constructor
    /// @param  The String to copy
    constexpr String(const String&) noexcept = default;
    /// @brief Move constructor
    /// @param  The String to move
    constexpr String(String&&) noexcept = default;
    /// @brief Destructor
    ~String() noexcept = default;

    /// @brief NUL terminates the String
    /// @return Pointer to the NUL terminated String
    constexpr const CharT* cStr() noexcept;

    /// @brief Appends a character to the end of the String
    /// @param chr The character to append
    constexpr void append(CharT chr) noexcept;
    /// @brief Appends a StringView to the end of the String
    /// @param strv The view to append
    constexpr void append(StringView<CharT> strv) noexcept;    

    /// @brief Appends a character to the end of the String
    /// @param strv The view to append
    /// @return Self
    constexpr String& operator+=(StringView<CharT> strv) noexcept;    
    /// @brief Appends a character to the end of the String
    /// @param chr The character to append
    /// @return Self
    constexpr String& operator+=(CharT chr) noexcept;    

    /// @brief Conversion operator
    /// @return StringView
    constexpr operator StringView<CharT>() const noexcept;    
    /// @brief Conversion operator
    /// @return ContiguousView
    constexpr operator ContiguousView<CharT>() const noexcept;    
  };
  template<typename CharT>
  constexpr const CharT* String<CharT>::cStr() noexcept
  {
    if (Str::isNotEmpty())
      if (Str::getBack() == '\0')
        return;
    append('\0');
  }
  
  template<typename CharT>
  constexpr void String<CharT>::append(CharT chr) noexcept
  {
    Str::pushBack(chr);
  }
  
  template<typename CharT>
  constexpr void String<CharT>::append(StringView<CharT> strv) noexcept
  {
    for (auto& i : strv)
      Str::pushBack(i);
  }
  
  template<typename CharT>
  constexpr String<CharT>& String<CharT>::operator+=(StringView<CharT> strv) noexcept
  {
    append(strv);
    return *this;
  }
  
  template<typename CharT>
  constexpr String<CharT>& String<CharT>::operator+=(CharT chr) noexcept
  {
    append(chr);
    return *this;
  }

  template<typename CharT>
  constexpr String<CharT>::operator ContiguousView<CharT>() const noexcept
  {
    return { Str::begin(), Str::end() };
  }
  
  template<typename CharT>
  constexpr String<CharT>::operator StringView<CharT>() const noexcept
  {
    return { Str::begin(), Str::end() };
  }
  
  template<typename CharT>
  constexpr void StringView<CharT>::strip() noexcept
  {
    while (View::isNotEmpty())
      if (isSpace(*View::begin()))
        View::popFront();

    while (View::isNotEmpty())
      if (isSpace(*(View::begin() + View::getSize() - 1)))
        View::popFront();
  }
  
  template<typename CharT>
  constexpr StringView<CharT>::operator ContiguousView<CharT>() const noexcept
  {
    return { View::begin(), View::end() };
  }
}

#endif //!HG_COLT_STRING