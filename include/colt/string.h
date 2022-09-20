#ifndef HG_COLT_STRING
#define HG_COLT_STRING

#include "char.h"
#include "vector.h"
#include "expected.h"

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
    /// @brief Constructs a StringView over a NUL terminated string
    /// @param cstr The NUL terminated string to span over
    constexpr StringView(const CharT* cstr) noexcept
      : View(cstr, std::strlen(cstr)) {}
    /// @brief Constructs a StringView over a NUL terminated string, including its NUL terminator
    /// @param cstr The NUL terminated string to span over
    /// @param  Tag object (WithNUL)
    constexpr StringView(const CharT* cstr, traits::WithNULT) noexcept
      : View(cstr, std::strlen(cstr) + 1) {}
    /// @brief Copy constructor
    /// @param  The StringView to copy
    constexpr StringView(const StringView&) noexcept = default;
    /// @brief Move constructor
    /// @param  The StringView to move
    constexpr StringView(StringView&&) noexcept = default;

    /// @brief Pops all spaces from the beginning and the end of the StringView.
    /// The characters that are considered spaces are '\n', ' ', '\v', '\t'.
    constexpr void stripSpaces() noexcept;    
    
    /// @brief Conversion operator
    /// @return ContiguousView
    constexpr operator ContiguousView<CharT>() const noexcept;    
  };

  enum class StringError
  {
    EOF_HIT, INVALID_PATH, CANNOT_READ_ALL
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
    /// @brief Constructs a String from a StringView
    /// @param strv The StringView to copy
    constexpr String(StringView<CharT> strv) noexcept;
    /// @brief Constructs a StringView over a NUL terminated string
    /// @param cstr The NUL terminated string to span over
    constexpr String(const CharT* cstr) noexcept;
    /// @brief Constructs a StringView over a NUL terminated string
    /// @param cstr The NUL terminated string to span over
    /// @param  Tag object (WithNUL)
    constexpr String(const CharT* cstr, traits::WithNULT) noexcept;     
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

    /// @brief Get a line from a file (by default 'stdin').
    /// This returns a non NUL-terminated String.
    /// The new-line is not included, but is consumed.
    /// @param from The FILE from which to read the characters
    /// @return String over the line
    static Expected<String, StringError> getLine(FILE* from = stdin) noexcept;
    /// /// @brief Get a line from a file (by default 'stdin'), and NUL terminates the String.
    /// The new-line is not included, but is consumed.
    /// @param  Tag object (WithNUL)
    /// @param from The FILE from which to read the characters
    /// @return String over the line 
    static Expected<String, StringError> getLine(traits::WithNULT, FILE* from = stdin) noexcept;
    
    //static Expected<String, StringError> getFileContent(StringView<char> path) noexcept;

    /// @brief Conversion operator
    /// @return StringView
    constexpr operator StringView<CharT>() const noexcept;    
    /// @brief Conversion operator
    /// @return ContiguousView
    constexpr operator ContiguousView<CharT>() const noexcept;    
  };
  
  template<typename CharT>
  constexpr String<CharT>::String(StringView<CharT> strv) noexcept
    : Str(strv.getSize())
  {
    for (auto& i : strv)
      Str::pushBack(i);
  }

  template<typename CharT>
  constexpr String<CharT>::String(const CharT* cstr) noexcept
    : Str()
  {
    size_t strl = std::strlen(cstr);
    Str::reserve(strl);
    for (size_t i = 0; i < strl; i++)
      Str::pushBack(cstr[i]);
  }

  template<typename CharT>
  constexpr String<CharT>::String(const CharT* cstr, traits::WithNULT) noexcept
    : Str()
  {
    size_t strl = std::strlen(cstr) + 1;
    Str::reserve(strl);
    for (size_t i = 0; i < strl; i++)
      Str::pushBack(cstr[i]);
  }
  
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
  Expected<String<CharT>, StringError> String<CharT>::getLine(FILE* from) noexcept
  {
    String str;

    for (;;)
    {
      auto gchar = static_cast<char>(std::fgetc(stdin));
      if (gchar != '\n' && gchar != EOF)
        str.append(gchar);
      else
        break;
    }
    if (std::feof(from))
      return { Error, StringError::EOF_HIT };
    return str;
  }

  template<typename CharT>
  Expected<String<CharT>, StringError> String<CharT>::getLine(traits::WithNULT, FILE* from) noexcept
  {    
    if (auto estr = String::getLine(from); estr.isError())
      return estr; //Propagate the error
    else
    {
      estr.append('\0');
      return estr;
    }
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
  constexpr void StringView<CharT>::stripSpaces() noexcept
  {
    while (View::isNotEmpty())
      if (isSpace(*View::begin()))
        View::popFront();
      else
        break;

    while (View::isNotEmpty())
      if (isSpace(*(View::begin() + View::getSize() - 1)))
        View::popFront();
      else
        break;
  }
  
  template<typename CharT>
  constexpr StringView<CharT>::operator ContiguousView<CharT>() const noexcept
  {
    return { View::begin(), View::end() };
  }

#ifdef COLT_USE_IOSTREAMS

  template<typename CharT>
  std::ostream& operator<<(std::ostream& os, const StringView<CharT>& var)
  {
    os.write(var.begin(), var.getSize());
    return os;
  }

  template<typename CharT>
  std::ostream& operator<<(std::ostream& os, const String<CharT>& var)
  {
    os.write(var.begin(), var.getSize());
    return os;
  }

#endif
}

#endif //!HG_COLT_STRING