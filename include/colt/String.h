#ifndef HG_COLT_STRING
#define HG_COLT_STRING

#include "details/char.h"
#include "Vector.h"
#include "Expected.h"
#include "Hash.h"

namespace colt
{
  namespace details
  {
    static constexpr size_t strlen(const char* str) noexcept
    {
      size_t ret = 0;
      while (str[ret] != '\0')
        ++ret;
      return ret;
    }
  }

  template<typename CharT = char>
  class StringViewOf
    : public ContiguousView<CharT>
  {
    static_assert(std::is_same_v<CharT, char>, "StringViewOf only supports char for now!");

    using View = ContiguousView<CharT>;

  public:
    /// @brief Constructs an empty StringViewOf
    constexpr StringViewOf() noexcept
      : View(nullptr, nullptr) {}
    /// @brief Range constructor
    /// @param begin The beginning of the view
    /// @param end The end of the view
    constexpr StringViewOf(const CharT* begin, const CharT* end) noexcept
      : View(begin, end) {}
    /// @brief Constructs a StringView over a NUL terminated string
    /// @param cstr The NUL terminated string to span over
    constexpr StringViewOf(const CharT* cstr) noexcept
      : View(cstr, details::strlen(cstr)) {}
    /// @brief Constructs a StringView over a NUL terminated string, including its NUL terminator
    /// @param cstr The NUL terminated string to span over
    /// @param  Tag object (WithNUL)
    constexpr StringViewOf(const CharT* cstr, traits::WithNULT) noexcept
      : View(cstr, details::strlen(cstr) + 1) {}
    /// @brief Copy constructor
    /// @param  The StringView to copy
    constexpr StringViewOf(const StringViewOf&) noexcept = default;
    /// @brief Move constructor
    /// @param  The StringView to move
    constexpr StringViewOf(StringViewOf&&) noexcept = default;

    constexpr StringViewOf& operator=(const StringViewOf&) noexcept = default;
    constexpr StringViewOf& operator=(StringViewOf&&) noexcept = default;

    /// @brief Pops all spaces from the beginning and the end of the StringView.
    /// The characters that are considered spaces are '\n', ' ', '\v', '\t'.
    constexpr void strip_spaces() noexcept;    
    
    /// @brief Conversion operator
    /// @return ContiguousView
    constexpr operator ContiguousView<CharT>() const noexcept;

    constexpr friend bool operator==(const StringViewOf& strv1, const StringViewOf& strv2) noexcept
    {
      if (strv1.get_size() != strv2.get_size())
        return false;
      for (size_t i = 0; i < strv1.get_size(); i++)
      {
        if (strv1[i] != strv2[i])
          return false;
      }
      return true;
    }

    constexpr friend bool operator!=(const StringViewOf& strv1, const StringViewOf& strv2) noexcept
    {
      return !(strv1 == strv2);
    }
  };

  /// @brief StringViewOf char
  using StringView = StringViewOf<char>;

  enum class StringError
  {
    EOF_HIT, INVALID_PATH, CANNOT_READ_ALL
  };

  /// @brief Contiguous array of characters.
  /// Possesses a small buffer of 16 characters.
  template<typename CharT = char>
  class StringOf
    : public SmallVector<CharT, 16>
  {
    static_assert(std::is_same_v<CharT, char>, "StringOf only supports char for now!");
    
    using Str = SmallVector<CharT, 16>;

  public:    

    /// @brief Default constructor
    constexpr StringOf() noexcept = default;
    /// @brief Copy constructor
    /// @param  The StringOf to copy
    constexpr StringOf(const StringOf&) noexcept = default;
    /// @brief Move constructor
    /// @param  The StringOf to move
    constexpr StringOf(StringOf&&) noexcept = default;
    /// @brief Constructs a StringOf from a StringView
    /// @param strv The StringView to copy
    explicit constexpr StringOf(StringViewOf<CharT> strv) noexcept;
    /// @brief Constructs a StringView over a NUL terminated string
    /// @param cstr The NUL terminated string to span over
    explicit constexpr StringOf(const CharT* cstr) noexcept;
    /// @brief Constructs a StringView over a NUL terminated string
    /// @param cstr The NUL terminated string to span over
    /// @param  Tag object (WithNUL)
    constexpr StringOf(const CharT* cstr, traits::WithNULT) noexcept;     
    /// @brief Destructor
    ~StringOf() noexcept = default;

    /// @brief NUL terminates the StringOf
    /// @return Pointer to the NUL terminated StringOf
    constexpr const CharT* c_str() noexcept;

    /// @brief Appends a character to the end of the StringOf
    /// @param chr The character to append
    constexpr void append(CharT chr) noexcept;
    /// @brief Appends a StringView to the end of the StringOf
    /// @param strv The view to append
    constexpr void append(StringViewOf<CharT> strv) noexcept;

    /// @brief Appends a character to the end of the StringOf
    /// @param strv The view to append
    /// @return Self
    constexpr StringOf& operator+=(StringViewOf<CharT> strv) noexcept;
    /// @brief Appends a character to the end of the StringOf
    /// @param chr The character to append
    /// @return Self
    constexpr StringOf& operator+=(CharT chr) noexcept;    

    /// @brief Get a line from a file (by default 'stdin').
    /// This returns a non NUL-terminated StringOf.
    /// The new-line is not included, but is consumed.
    /// @param from The FILE from which to read the characters
    /// @return StringOf over the line
    static Expected<StringOf, StringError> getLine(FILE* from = stdin) noexcept;
    /// /// @brief Get a line from a file (by default 'stdin'), and NUL terminates the StringOf.
    /// The new-line is not included, but is consumed.
    /// @param  Tag object (WithNUL)
    /// @param from The FILE from which to read the characters
    /// @return StringOf over the line 
    static Expected<StringOf, StringError> getLine(traits::WithNULT, FILE* from = stdin) noexcept;
    
    /// @brief Returns a StringOf containing the content of the file at path 'path'.
    /// This functions is faster than getFileContent for large file sizes, as it checks
    /// the size of the file to allocate once.
    /// Returns StringError::INVALID_PATH if the path is not valid or the OS was not able to open the file.
    /// Returns StringError::CANNOT_READ_ALL if a call to 'ftell/fseek' fails.
    /// @param path The file path
    /// @return StringOf containing the content of 'from' or StringError
    static Expected<StringOf, StringError> getFileContent(const char* path) noexcept;
    /// @brief Returns a StringOf containing the content of the file 'from'.
    /// Repeatedly calls 'fgetc' on from and appends to the StringOf.
    /// Can return a StringError::EOF_HIT if 'feof(from)' returns true before the first 'fgetc' call.
    /// @param from The file on which to repeatedly call 'fgetc'
    /// @return StringOf containing the content of 'from' or StringError::EOF_HIT
    static Expected<StringOf, StringError> getFileContent(FILE* from) noexcept;

    /// @brief Conversion operator
    /// @return StringViewOf
    constexpr operator StringViewOf<CharT>() const noexcept;    
    /// @brief Conversion operator
    /// @return ContiguousView
    constexpr operator ContiguousView<CharT>() const noexcept;

    friend constexpr bool operator==(const StringOf& strv1, const StringViewOf<CharT>& strv2) noexcept
    {
      if (strv1.get_size() != strv2.get_size())
        return false;
      for (size_t i = 0; i < strv1.get_size(); i++)
      {
        if (strv1[i] != strv2[i])
          return false;
      }
      return true;
    }

    friend constexpr bool operator!=(const StringOf& strv1, const StringViewOf<CharT>& strv2) noexcept
    {
      return !(strv1 == strv2);
    }

    friend constexpr bool operator==(const StringOf& strv1, const StringOf& strv2) noexcept
    {
      if (strv1.get_size() != strv2.get_size())
        return false;
      for (size_t i = 0; i < strv1.get_size(); i++)
      {
        if (strv1[i] != strv2[i])
          return false;
      }
      return true;
    }

    friend constexpr bool operator!=(const StringOf& strv1, const StringOf& strv2) noexcept
    {
      return !(strv1 == strv2);
    }
  };

  using String = StringOf<char>;
  
  template<typename CharT>
  constexpr StringOf<CharT>::StringOf(StringViewOf<CharT> strv) noexcept
    : Str(strv.get_size())
  {
    for (auto& i : strv)
      Str::push_back(i);
  }

  template<typename CharT>
  constexpr StringOf<CharT>::StringOf(const CharT* cstr) noexcept
    : Str()
  {
    size_t strl = details::strlen(cstr);
    Str::reserve(strl);
    for (size_t i = 0; i < strl; i++)
      Str::push_back(cstr[i]);
  }

  template<typename CharT>
  constexpr StringOf<CharT>::StringOf(const CharT* cstr, traits::WithNULT) noexcept
    : Str()
  {
    size_t strl = details::strlen(cstr) + 1;
    Str::reserve(strl);
    for (size_t i = 0; i < strl; i++)
      Str::push_back(cstr[i]);
  }
  
  template<typename CharT>
  constexpr const CharT* StringOf<CharT>::c_str() noexcept
  {
    if (Str::is_not_empty())
      if (Str::get_back() == '\0')
        return Str::get_data();
    append('\0');
    return Str::get_data();
  }
  
  template<typename CharT>
  constexpr void StringOf<CharT>::append(CharT chr) noexcept
  {
    Str::push_back(chr);
  }
  
  template<typename CharT>
  constexpr void StringOf<CharT>::append(StringViewOf<CharT> strv) noexcept
  {
    for (auto& i : strv)
      Str::push_back(i);
  }
  
  template<typename CharT>
  constexpr StringOf<CharT>& StringOf<CharT>::operator+=(StringViewOf<CharT> strv) noexcept
  {
    append(strv);
    return *this;
  }
  
  template<typename CharT>
  constexpr StringOf<CharT>& StringOf<CharT>::operator+=(CharT chr) noexcept
  {
    append(chr);
    return *this;
  }

  template<typename CharT>
  Expected<StringOf<CharT>, StringError> StringOf<CharT>::getLine(FILE* from) noexcept
  {
    StringOf<CharT> str;

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
  Expected<StringOf<CharT>, StringError> StringOf<CharT>::getLine(traits::WithNULT, FILE* from) noexcept
  {    
    if (auto estr = StringOf::getLine(from); estr.isError())
      return estr; //Propagate the error
    else
    {
      estr->append('\0');
      return estr;
    }
  }

  template<typename CharT>
  inline Expected<StringOf<CharT>, StringError> StringOf<CharT>::getFileContent(const char* path) noexcept
  {
    StringOf<CharT> content;
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
  inline Expected<StringOf<CharT>, StringError> StringOf<CharT>::getFileContent(FILE* from) noexcept
  {
    assert(from && "FILE* cannot be NULL!");
    
    if (std::feof(from))
      return { Error, StringError::EOF_HIT };

    StringOf<CharT> content;

    while (!std::feof(from))
      content.append(static_cast<char>(std::fgetc(from)));

    return content;
  }

  template<typename CharT>
  constexpr StringOf<CharT>::operator ContiguousView<CharT>() const noexcept
  {
    return { Str::begin(), Str::end() };
  }
  
  template<typename CharT>
  constexpr StringOf<CharT>::operator StringViewOf<CharT>() const noexcept
  {
    return { Str::begin(), Str::end() };
  }
  
  template<typename CharT>
  constexpr void StringViewOf<CharT>::strip_spaces() noexcept
  {
    while (View::is_not_empty())
      if (isSpace(*View::begin()))
        View::popFront();
      else
        break;

    while (View::is_not_empty())
      if (isSpace(*(View::begin() + View::get_size() - 1)))
        View::popFront();
      else
        break;
  }
  
  template<typename CharT>
  constexpr StringViewOf<CharT>::operator ContiguousView<CharT>() const noexcept
  {
    return { View::begin(), View::end() };
  }

  template<>
  std::size_t hash(const StringViewOf<char>& str) noexcept
  {
    auto size = str.get_size();
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
  std::size_t hash(const StringOf<char>& str) noexcept
  {
    auto size = str.get_size();
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
  std::ostream& operator<<(std::ostream& os, const StringViewOf<CharT>& var)
  {
    os.write(var.begin(), var.get_size());
    return os;
  }

  template<typename CharT>
  std::ostream& operator<<(std::ostream& os, const StringOf<CharT>& var)
  {
    os.write(var.begin(), var.get_size());
    return os;
  }

#endif

}

#ifdef COLT_USE_FMT

template<>
struct fmt::formatter<colt::StringView>
{
  template<typename ParseContext>
  constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

  template<typename FormatContext>
  auto format(const colt::StringView& str, FormatContext& ctx)
  {
    return fmt::format_to(ctx.out(), "{:.{}}", str.get_data(), str.get_size());
  }
};

template<>
struct fmt::formatter<colt::StringOf<char>>
{
  template<typename ParseContext>
  constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

  template<typename FormatContext>
  auto format(const colt::StringOf<char>& str, FormatContext& ctx)
  {
    return fmt::format_to(ctx.out(), "{:.{}}", str.get_data(), str.get_size());
  }
};

#endif

#endif //!HG_COLT_STRING