#include <ParserRules.hpp>

namespace Pascal
{
	namespace Rules
	{
		bool isAlpha(char ch)
		{
			return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
		}

		bool isDigit(char ch)
		{
			return ch >= '0' && ch <= '9';
		}

		bool isAlphaNum(char ch)
		{
			return isAlpha(ch) || isDigit(ch);
		}

		unsigned twosComplement(unsigned char num)
		{
			return (unsigned char)(~num + 1);
		}
	}
}