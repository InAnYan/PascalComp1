#ifndef PASCAL_SEMANTIC_ANALYZER
#define PASCAL_SEMANTIC_ANALYZER

#include <Visitor.hpp>
#include <Environment.hpp>
#include <CodeGenVisitor.hpp>

#include <memory>

namespace Pascal
{
	class SemanticAnalyzer : public AST::Visitor
	{
	public:
		SemanticAnalyzer();
		~SemanticAnalyzer();

        void visitProgramNode(const AST::ProgramNode& node);
        void visitCompoundNode(const AST::CompoundNode& node);
        void visitVarDeclNode(const AST::VarDeclNode& node);
        void visitTypeNode(const AST::TypeNode& node);
        void visitProcDeclNode(const AST::ProcDeclNode& node);
        void visitAssignmentNode(const AST::AssignmentNode& node);
        void visitVarNode(const AST::VarNode& node);
        void visitIntLiteralNode(const AST::IntLiteralNode& node);
        void visitBinaryExprNode(const AST::BinaryExprNode& node);
        void visitUnaryExprNode(const AST::UnaryExprNode& node);
        void visitProcCallNode(const AST::CallStmtNode& node);
        void visitFunctionDeclNode(const AST::FunctionDeclNode& node);
        void visitIfNode(const AST::IfNode& node);
        void visitFunctionCall(const AST::FunctionCallNode& node);

    private:
        std::shared_ptr<Environment<SymAttribs>> currentScope;
        std::shared_ptr<Environment<SymAttribs>> globalScope;

        SymType lastType;
        SymAttribs lastSym;
	};
}

#endif // PASCAL_SEMANTIC_ANALYZER