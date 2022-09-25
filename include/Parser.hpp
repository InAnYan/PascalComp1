#ifndef PASCAL_PARSER_HPP_DEFINED
#define PASCAL_PARSER_HPP_DEFINED

#include <memory>

#include <Token.hpp>
#include <AST.hpp>

namespace Pascal
{
    class Parser
    {
    public:
        Parser(TokenList tokens);
        std::unique_ptr<AST::ProgramNode> parseProgram();

    private:
        std::unique_ptr<AST::DeclarationNode> parseDeclaration();
        std::unique_ptr<AST::VarDeclNode> parseVarDecl();
        std::unique_ptr<AST::VarDeclNode> parseParamDecl();
        std::unique_ptr<AST::TypeNode> parseType();
        std::unique_ptr<AST::ProcDeclNode> parseProcDecl();
        std::unique_ptr<AST::CompoundNode> parseCompound();
        std::unique_ptr<AST::StmtNode> parseStmt();
        std::unique_ptr<AST::AssignmentNode> parseAssignment();
        std::unique_ptr<AST::CallStmtNode> parseProcCall();
        std::unique_ptr<AST::IfNode> parseIf();
        std::unique_ptr<AST::ExpressionNode> parseExpression();
        std::unique_ptr<AST::ExpressionNode> parseFactor();
        std::unique_ptr<AST::ExpressionNode> parseUnary();
        std::unique_ptr<AST::ExpressionNode> parsePrimary();

        TokenList m_Tokens;
        size_t m_ParserPos;

        Token require(TokenType type, const char* errorMsg);
        Token match(TokenType type);
        template <typename... T>
        bool matching(T... types)
        {
            std::vector<TokenType> typesVec = { types... };
            for (TokenType t : typesVec)
            {
                if (match(t).type != TokenType::NONE) return true;
            }
            return false;
        }
        bool check(TokenType type);

        void synchronise();

        Token peek(size_t offset = 0);
        Token previous();

        bool isAtEnd();

        void advance();
    }; // Parser
} // namespace Pascal

#endif // PASCAL_PARSER_HPP_DEFINED

