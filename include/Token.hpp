#ifndef PASCAL_TOKEN_HPP
#define PASCAL_TOKEN_HPP

#include <memory>
#include <vector>
#include <string>

namespace Pascal
{
	enum class TokenType
	{
		NONE, UNINTIALIZED_TOKEN,

		// Single-character tokens
		OPEN_PAREN, CLOSE_PAREN,
		OPEN_BRACE, CLOSE_BRACE,
		COMMA, DOT, SEMICOLON, COLON,
		PLUS, MINUS, SLASH, STAR,

		COLON_EQUAL,

		IDENTIFIER,

		// Literals
		STRING_LITERAL,
		INT_LITERAL,
		FLOAT_LITERAL,

		// Keywords
		PROGRAM, BEGIN, END, PROCEDURE,
		VAR, CONST, IF, THEN, ELSE,

		EOF_TOKEN
	};

	class Token
	{
	public:
		TokenType type;
		std::string str;
		size_t pos;

		Token(TokenType type, std::string const& str, size_t pos)
			: type(type), str(str), pos(pos) 
		{ }

		Token()
			: type(TokenType::UNINTIALIZED_TOKEN), str(), pos(0)
		{ }
	};

	extern const Token nullToken;

	using TokenList = std::shared_ptr<std::vector<Token>>;
}

#endif // PASCAL_TOKEN_HPP