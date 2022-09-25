#ifndef HG_COLT_STRING
#define HG_COLT_STRING

#include "details/char.h"
#include "Vector.h"
#include "Expected.h"
#include "Hash.h"

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
    
    /// @brief Returns a String containing the content of the file at path 'path'.
    /// This functions is faster than getFileContent for large file sizes, as it checks
    /// the size of the file to allocate once.
    /// Returns StringError::INVALID_PATH if the path is not valid or the OS was not able to open the file.
    /// Returns StringError::CANNOT_READ_ALL if a call to 'ftell/fseek' fails.
    /// @param path The file path
    /// @return String containing the content of 'from' or StringError
    static Expected<String, StringError> getFileContent(const char* path) noexcept;
    /// @brief Returns a String containing the content of the file 'from'.
    /// Repeatedly calls 'fgetc' on from and appends to the String.
    /// Can return a StringError::EOF_HIT if 'feof(from)' returns true before the first 'fgetc' call.
    /// @param from The file on which to repeatedly call 'fgetc'
    /// @return String containing the content of 'from' or StringError::EOF_HIT
    static Expected<String, StringError> getFileContent(FILE* from) noexcept;

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
    String<CharT> str;

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
      estr->append('\0');
      return estr;
    }
  }

  template<typename CharT>
  inline Expected<String<CharT>, StringError> String<CharT>::getFileContent(const char* path) noexcept
  {
    String<CharT> content;
    FILE* file = std::fopen(path, "rb");
    if (file == nullptr)
      return { Error, StringError::INVALID_PATH };

    if (std::fseek(file, 0, SEEK_END) != 0)
      return { Error, StringError::CANNOT_READ_ALL };

    auto byte_sz = std::ftell(file);
    if (byte_sz == -1)
      return { Error, StringError::CANNOT_READ_ALL };
    std::rewind(file);
    
    //Save enough memory for the whole file
    content.reserve(byte_sz);
    
    file = std::freopen(path, "rt", file);
    if (file == nullptr)
      return { Error, StringError::INVALID_PATH };
    
    while (!feof(file))
      content.append(static_cast<char>(std::fgetc(file)));
    std::fclose(file);

    return content;
  }

  template<typename CharT>
  inline Expected<String<CharT>, StringError> String<CharT>::getFileContent(FILE* from) noexcept
  {
    assert(from && "FILE* cannot be NULL!");
    
    if (std::feof(from))
      return { Error, StringError::EOF_HIT };

    String<CharT> content;

    while (!std::feof(from))
      content.append(static_cast<char>(std::fgetc(from)));

    return content;
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

  template<>
  std::size_t hash(const StringView<char>& str) noexcept
  {
    auto size = str.getSize();
    size = size > 64 ? 64 : size;

    uint64_t hash = 0xCBF29CE484222325;
    for (size_t i = 0; i < size; i++)
    {
      hash ^= (uint8_t)str[i];
      hash *= 0x100000001B3; //FNV prime
    }
    return hash;
  }

  template<>
  std::size_t hash(const String<char>& str) noexcept
  {
    auto size = str.getSize();
    size = size > 64 ? 64 : size;

    uint64_t hash = 0xCBF29CE484222325;
    for (size_t i = 0; i < size; i++)
    {
      hash ^= (uint8_t)str[i];
      hash *= 0x100000001B3; //FNV prime
    }
    return hash;
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