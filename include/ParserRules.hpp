#ifndef PASCAL_RULES_HPP
#define PASCAL_RULES_HPP

#include <cstdint>

namespace Pascal
{
	namespace Rules
	{
		bool isAlpha(char ch);
		bool isDigit(char ch);
		bool isAlphaNum(char ch);
		
		unsigned twosComplement(unsigned char num);
	}
}

#endif // PASCAL_RULES_HPP