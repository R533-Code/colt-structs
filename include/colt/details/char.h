/** @file colt_char.h
* Contains helpers function for char handling.
*/

#ifndef HG_COLT_CHAR
#define HG_COLT_CHAR

#include "../View.h"

/// @brief Contains character usage helpers
namespace colt
{
	/// @brief Checks if a character is a space, newline, tab...
	/// @param chr The char to check for
	/// @return True if the character is a space
	bool isSpace(char chr) noexcept
	{
		return static_cast<unsigned char>(chr) == ' '
			|| static_cast<unsigned char>(chr) == '\n'
			|| static_cast<unsigned char>(chr) == '\t'
			|| static_cast<unsigned char>(chr) == '\v'
			|| static_cast<unsigned char>(chr) == '\f'
			|| static_cast<unsigned char>(chr) == '\r';
	}

	/// @brief Checks if a character is a digit
	/// @param chr The char to check for
	/// @return True if the character is a digit
	bool isDigit(char chr) noexcept
	{
		return static_cast<unsigned char>(chr) > 47 && static_cast<unsigned char>(chr) < 58;
	}

	/// @brief Checks if a character is alpha [a-zA-Z]
	/// @param chr The char to check for
	/// @return True if the character is alpha
	bool isAlpha(char chr) noexcept
	{
		return (static_cast<unsigned char>(chr) >  64 && static_cast<unsigned char>(chr) < 91)
			|| (static_cast<unsigned char>(chr) > 96 && static_cast<unsigned char>(chr) < 123);
	}

	/// @brief Checks if a character is alpha numerical
	/// @param chr The char to check for
	/// @return True if the character is alpha numerical
	bool isAlnum(char chr) noexcept
	{
		return isDigit(chr) || isAlpha(chr);
	}
	
	/// @brief Checks if a character is a control character
	/// @param chr The char to check for
	/// @return True if the character is a control character
	bool isControl(char chr) noexcept
	{
		return static_cast<unsigned char>(chr) < ' ';
	}

	/// @brief Checks if a string view does not contain invalid characters for a name.
	/// Does not check if the file exits or not, merely verifies that no illegal characters
	/// are contained in the name.
	/// @param name The string view to check for
	/// @return True if the name is valid
	bool isValidFileName(ContiguousView<char> name) noexcept
	{
		if (name.is_empty())
			return false;

#ifdef _WIN32
		for (const auto& chr : name)
		{
			if (isControl(chr)
				|| chr == ':'
				|| chr == '"'
				|| chr == '|'
				|| chr == '?'
				|| chr == '*'
				|| chr == '<'
				|| chr == '>')
				return false;
		}
		if (name.get_back() == ' ' || name.get_back() == '.')
			return false;
		return true;
#else 
		for (const auto& chr : name)
		{
			if (isControl(chr))
				return false;
		}
		return true;
#endif
	}
}

#endif //!HG_COLT_CHAR