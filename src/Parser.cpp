#include <pscpch.hpp>
#include <Parser.hpp>
#include <ReportsManager.hpp>

namespace Pascal
{
	class ParserError : std::exception
	{
	public:
		ParserError(const char* msg)
			: m_Msg(msg) {}

		const char* what() const throw()
		{ return m_Msg; }

	private:
		const char* m_Msg;
	};

	Parser::Parser(TokenList tokens)
		: m_Tokens(tokens), m_ParserPos(0)
	{ }
	
	std::unique_ptr<AST::ProgramNode> Parser::parseProgram()
	{
		Token name;
		std::vector<std::unique_ptr<AST::DeclarationNode>> decls;
		std::unique_ptr<AST::CompoundNode> compound;
		
		try
		{
			require(TokenType::PROGRAM, "Expected 'program' keyword");
			name = require(TokenType::IDENTIFIER, "Expected program name");
			require(TokenType::SEMICOLON, "Expcted ';' after program statement");
			
			while (!matching(TokenType::BEGIN))
			{
				try
				{
					decls.push_back(parseDeclaration());
				}
				catch (ParserError const& e)
				{
					ReportsManager::ReportError(peek().pos, e.what());
					synchronise();
				}
			}

			compound = parseCompound();

			require(TokenType::DOT, "Expected '.' at the end of the program");
		}
		catch (ParserError const& e)
		{
			ReportsManager::ReportError(peek().pos, e.what());
			// TODO: What to do?
		}

		return std::make_unique<AST::ProgramNode>(
			name, std::move(decls), std::move(compound)
		);
	}
	
	std::unique_ptr<AST::DeclarationNode> Parser::parseDeclaration()
	{
		if (matching(TokenType::VAR)) return parseVarDecl();
		if (matching(TokenType::CONST)) return parseVarDecl();
		else if (matching(TokenType::PROCEDURE)) return parseProcDecl();
		else
		{
			throw ParserError("Expected declaration");
		}
	}
	
	std::unique_ptr<AST::VarDeclNode> Parser::parseVarDecl()
	{
		Token t_Name;
		std::unique_ptr<AST::TypeNode> t_Type;

		bool isConst = previous().type == TokenType::CONST;

		t_Name = require(TokenType::IDENTIFIER, "Expected variable name");
		require(TokenType::COLON, "Expected ':' in variable declaration");
		t_Type = parseType();
		require(TokenType::SEMICOLON, "Expected ';' after variable declaration");

		return std::make_unique<AST::VarDeclNode>(
			std::move(t_Name),
			std::move(t_Type),
			isConst
		);
	}

	std::unique_ptr<AST::VarDeclNode> Parser::parseParamDecl()
	{
		Token t_Name;
		std::unique_ptr<AST::TypeNode> t_Type;

		// require(TokenType::VAR);
		t_Name = require(TokenType::IDENTIFIER, "Expected parameter name");
		require(TokenType::COLON, "Expected ':' in parameter declaration");
		t_Type = parseType();

		return std::make_unique<AST::VarDeclNode>(
			std::move(t_Name),
			std::move(t_Type),
			false
		);
	}
	
	std::unique_ptr<AST::TypeNode> Parser::parseType()
	{
		Token t_Token;
		
		t_Token = require(TokenType::IDENTIFIER, "Expected type specifier");
		
		return std::make_unique<AST::TypeNode>(
			std::move(t_Token)
		);
	}
	
	std::unique_ptr<AST::ProcDeclNode> Parser::parseProcDecl()
	{
		Token name; 
		std::vector<std::unique_ptr<AST::VarDeclNode>> params;
		std::vector<std::unique_ptr<AST::VarDeclNode>> decls;
		std::unique_ptr<AST::CompoundNode> compound;

		name = require(TokenType::IDENTIFIER, "Expected procedure name");
		if (matching(TokenType::OPEN_PAREN))
		{
			if (check(TokenType::IDENTIFIER))
			{
				do
				{
					params.push_back(parseParamDecl());
				} while (matching(TokenType::SEMICOLON));
			}
			require(TokenType::CLOSE_PAREN, "Expected ')' after procedure parameters");
		}
		require(TokenType::SEMICOLON, "Expected ';' in procedure declaration");
		
		while (matching(TokenType::VAR, TokenType::CONST)) 
			decls.push_back(parseVarDecl());

		require(TokenType::BEGIN, "Expected procedure body ('begin')");
		compound = parseCompound();
		
		return std::make_unique<AST::ProcDeclNode>(
			name, std::move(params), std::move(decls), std::move(compound)
		);
	}
	
	std::unique_ptr<AST::CompoundNode> Parser::parseCompound()
	{
		std::vector<std::unique_ptr<AST::StmtNode>> t_Stmts;
		
		while (!isAtEnd() && !check(TokenType::END))
		{
			try
			{
				t_Stmts.push_back(parseStmt());
			}
			catch (ParserError& e)
			{
				ReportsManager::ReportError(peek().pos, e.what());
				synchronise();
			}
		}
		require(TokenType::END, "Expected 'end' keyword");
		
		return std::make_unique<AST::CompoundNode>(
			std::move(t_Stmts)
		);
	}
	
	std::unique_ptr<AST::StmtNode> Parser::parseStmt()
	{
		if (matching(TokenType::BEGIN)) return parseCompound();
		else if (peek(0).type == TokenType::IDENTIFIER && peek(1).type == TokenType::COLON_EQUAL)
			return parseAssignment();
		else if (peek(0).type == TokenType::IDENTIFIER
			&& (peek(1).type == TokenType::SEMICOLON || peek(1).type == TokenType::OPEN_PAREN))
			return parseProcCall();
		else if (matching(TokenType::IF)) return parseIf();
		else
		{
			throw ParserError("Unrecognized statement");
		}
	}
	
	std::unique_ptr<AST::IfNode> Parser::parseIf()
	{
		auto condition = parseExpression();
		require(TokenType::THEN, "Expected 'then' keyword");
		auto thenArm = parseStmt();
		std::unique_ptr<AST::StmtNode> elseArm = nullptr;
		if (matching(TokenType::ELSE))
		{
			if (matching(TokenType::IF)) elseArm = parseIf();
			else elseArm = parseStmt();
		}

		return std::make_unique<AST::IfNode>(
			std::move(condition),
			std::move(thenArm),
			std::move(elseArm)
		);
	}

	std::unique_ptr<AST::AssignmentNode> Parser::parseAssignment()
	{
		std::unique_ptr<AST::VarNode> t_Var;
		std::unique_ptr<AST::ExpressionNode> t_Expr;
		
		t_Var = std::make_unique<AST::VarNode>(require(TokenType::IDENTIFIER, "Expected assignment target"));
		require(TokenType::COLON_EQUAL, "Expected ':=' in assignment statement");
		t_Expr = parseExpression();
		require(TokenType::SEMICOLON, "Expected ';' after assignemnt statement");
		
		return std::make_unique<AST::AssignmentNode>(
			std::move(t_Var), 
			std::move(t_Expr)
		);
	}
	
	std::unique_ptr<AST::CallStmtNode> Parser::parseProcCall()
	{
		Token t_Name;
		std::vector<std::unique_ptr<AST::ExpressionNode>> t_Args;
		
		t_Name = require(TokenType::IDENTIFIER, "Expected procedure name");
		if (matching(TokenType::OPEN_PAREN))
		{
			do
			{
				t_Args.push_back(parseExpression());
			} while (matching(TokenType::COMMA));
			require(TokenType::CLOSE_PAREN, "Expected ')' after procedure arguments");
		}
		require(TokenType::SEMICOLON, "Expected ';' after procedure call statement");
		
		return std::make_unique<AST::CallStmtNode>(
			std::move(t_Name), 
			std::move(t_Args)
		);
	}
	
	std::unique_ptr<AST::ExpressionNode> Parser::parseExpression()
	{
		std::unique_ptr<AST::ExpressionNode> left = parseFactor();
		
		while (matching(TokenType::PLUS, TokenType::MINUS))
		{
			Token op = previous();
			std::unique_ptr<AST::ExpressionNode> right = parseFactor();
			left = std::make_unique<AST::BinaryExprNode>(std::move(left), op, std::move(right));
		}
		
		return left;
	}
	
	std::unique_ptr<AST::ExpressionNode> Parser::parseFactor()
	{
		std::unique_ptr<AST::ExpressionNode> left = parseUnary();
		
		while (matching(TokenType::PLUS, TokenType::MINUS))
		{
			Token op = previous();
			std::unique_ptr<AST::ExpressionNode> right = parseUnary();
			left = std::make_unique<AST::BinaryExprNode>(std::move(left), op, std::move(right));
		}
		
		return left;
	}
	
	std::unique_ptr<AST::ExpressionNode> Parser::parseUnary()
	{
		if (matching(TokenType::MINUS, TokenType::PLUS))
			return std::make_unique<AST::UnaryExprNode>(previous(), parsePrimary());
		else if (matching(TokenType::OPEN_PAREN))
		{
			auto expr = parseExpression();
			require(TokenType::CLOSE_PAREN, "Unbalanced parenthesis");
			return std::move(expr);
		}
		else
			return parsePrimary();
	}
	
	std::unique_ptr<AST::ExpressionNode> Parser::parsePrimary()
	{
		if (matching(TokenType::INT_LITERAL)) return std::make_unique<AST::IntLiteralNode>(previous());
		else if (matching(TokenType::IDENTIFIER)) return std::make_unique<AST::VarNode>(previous());
		else
		{
			throw ParserError("Expected expression");
		}
	}
	
	Token Parser::match(TokenType type)
	{
		if (check(type))
		{
			advance();
			return previous();
		}
		else return nullToken;
	}
	
	bool Parser::check(TokenType type)
	{
		return peek().type == type;
	}
	
	Token Parser::require(TokenType type, const char* errorMsg)
	{
		Token res = this->match(type);

		if (res.type == TokenType::NONE)
		{
			throw ParserError(errorMsg);
		}

		return res;
	}
	
	Token Parser::peek(size_t offset)
	{
		return (*m_Tokens)[m_ParserPos + offset];
	}

	Token Parser::previous()
	{
		return (*m_Tokens)[m_ParserPos - 1];
	}

	bool Parser::isAtEnd()
	{
		return peek().type == TokenType::EOF_TOKEN;
	}

	void Parser::advance()
	{
		if (!isAtEnd()) m_ParserPos++;
	}
	
	void Parser::synchronise()
	{
		advance();
		while (!isAtEnd())
		{
			if (previous().type == TokenType::SEMICOLON) return;

			switch (peek().type)
			{
			case TokenType::PROCEDURE:
			case TokenType::VAR:
			case TokenType::BEGIN:
				return;

			default:
				break;
			}

			advance();
		}
	}
} // namespace Pascal

