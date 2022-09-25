#ifndef PASCAL_SCANNER_HPP
#define PASCAL_SCANNER_HPP

#include <memory>
#include <string>
#include <vector>

#include <Token.hpp>

namespace Pascal
{
	class Scanner
	{
	public:
		Scanner(std::shared_ptr<std::string> source);

		TokenList scanTokens();

	private:
		std::shared_ptr<std::string> m_Source;
		TokenList m_Res;
		size_t start;
		size_t current;

		bool match(char ch);
		inline bool isAtEnd();
		inline char advance();
		inline char peek();

		void scanToken();
		inline void addToken(TokenType type);

		void identifier();
		void number();
	};
}

#endif // PASCAL_SCANNER_HPP