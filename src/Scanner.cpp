#include <Scanner.hpp>
#include <ReportsManager.hpp>
#include <ParserRules.hpp>

#include <sstream>
#include <map>

namespace Pascal
{
	namespace
	{
		std::map<std::string, TokenType> PascalKeywords = {
			{"program", TokenType::PROGRAM},
			{"procedure", TokenType::PROCEDURE},
			{"begin", TokenType::BEGIN},
			{"end", TokenType::END},
			{"var", TokenType::VAR},
			{"const", TokenType::CONST},
			{"if", TokenType::IF},
			{"else", TokenType::ELSE},
			{"then", TokenType::THEN},
		};
	}

	Scanner::Scanner(std::shared_ptr<std::string> source)
		: m_Source(source), 
		  m_Res(std::make_shared<std::vector<Token>>()),
		  start(0), current(0)
	{}

	TokenList Scanner::scanTokens()
	{
		while (!isAtEnd())
		{
			start = current;
			scanToken();
		}

		m_Res->push_back(Token(TokenType::EOF_TOKEN, "", m_Source->size() - 1));
		return m_Res;
	}

	inline bool Scanner::isAtEnd()
	{
		return current >= m_Source->size();
	}

	inline char Scanner::peek()
	{
		return (*m_Source)[current];
	}

	bool Scanner::match(char ch)
	{
		if (isAtEnd()) return false;
		if (peek() != ch) return false;

		current++;
		return true;
	}

	void Scanner::scanToken()
	{
		char ch = advance();
		switch (ch)
		{
		case '(': addToken(TokenType::OPEN_PAREN); break;
		case ')': addToken(TokenType::CLOSE_PAREN); break;
		case '{': addToken(TokenType::OPEN_BRACE); break;
		case '}': addToken(TokenType::CLOSE_BRACE); break;
		case '+':
			addToken(TokenType::PLUS);
			break;
		case '-':
			addToken(TokenType::MINUS);
			break;
		case '*':
			addToken(TokenType::STAR);
			break;
		case '.': addToken(TokenType::DOT); break;
		case ',': addToken(TokenType::COMMA); break;
		case ';': addToken(TokenType::SEMICOLON); break;
		case ':': addToken(match('=') ? TokenType::COLON_EQUAL : TokenType::COLON); break;
		case '/':
			if (match('/')) 
			{
				while (peek() != '\n' && !isAtEnd()) advance();
			}
			else if (match('*')) 
			{
				while (!isAtEnd()) 
				{
					if (match('*')) 
					{
						if (match('/')) 
						{
							break;
						}
						else 
						{
							advance();
						}
					}
					else
					{
						advance();
					}
				}
			}
			else 
			{
				addToken(TokenType::SLASH);
			}
			break;
			break;
		case ' ':
		case '\t':
		case '\r':
			break;
		case '\n':
			break;
		case '\"':
		{
			std::stringstream ss;
			while (!match('\"')) 
			{
				if (peek() == '\\') 
				{
					advance();
					if (peek() == '\\' || peek() == '\"') 
					{
						ss << peek();
					}
					else if (peek() == 'n') 
					{
						ss << '\n';
					}
					else {
						ReportsManager::ReportWarning(start, WarningType::UNKNOWN_ESCAPE_CHAR);
					}
					advance();
				}
				else if (isAtEnd()) 
				{
					ReportsManager::ReportError(start, ErrorType::UNTERMINATED_STRING);
					return;
				}
				else 
				{
					ss << peek();
					advance();
				}
			}
			(*m_Res).push_back(Token(TokenType::STRING_LITERAL, ss.str(), start));
			break;
		}
		default:
			if (Rules::isAlpha(ch) || ch == '_') 
			{
				identifier();
			}
			else if (Rules::isDigit(ch)) 
			{
				number();
			}
			else 
			{
				ReportsManager::ReportError(current, ErrorType::ILLEGAL_CHAR);
			}
			break;
		}
	}

	void Scanner::identifier()
	{
		while (Rules::isAlphaNum(peek()) || peek() == '_') advance();

		std::string str = m_Source->substr(start, current - start);

		if (PascalKeywords.count(str))
		{
			m_Res->push_back(Token(PascalKeywords[str], str, start));
		}
		else
		{
			m_Res->push_back(Token(TokenType::IDENTIFIER, str, start));
		}
	}

	void Scanner::number()
	{
		while (Rules::isDigit(peek())) advance();

		addToken(TokenType::INT_LITERAL);
	}

	inline void Scanner::addToken(TokenType type)
	{
		m_Res->push_back(Token(type, m_Source->substr(start, current - start), start));
	}

	inline char Scanner::advance()
	{
		current++;
		return (* m_Source)[current - 1];
	}
}