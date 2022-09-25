#ifndef PASCAL_USEDINITIALIZED_HPP
#define PASCAL_USEDINITIALIZED_HPP

#include <Visitor.hpp>
#include <Environment.hpp>
#include <memory>

namespace Pascal
{
    class UsedInitializedVisitor : public AST::Visitor
    {
    public:
        UsedInitializedVisitor();
        ~UsedInitializedVisitor();
        
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
        
        struct Attribs
        {
            bool used = false;
            bool initialized = false;
            size_t pos;
        };

        std::shared_ptr<Environment<Attribs>> currentScope;
        std::shared_ptr<Environment<Attribs>> globalScope;

        void check(const std::unique_ptr<AST::Node>& node);
    }; // class UsedInitialized
} // namespace Pascal

#endif // PASCAL_USEDINITIALIZED_HPP
