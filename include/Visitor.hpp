#ifndef PASCAL_VISITOR_HPP_DEFINED
#define PASCAL_VISITOR_HPP_DEFINED

#include <ASTForwards.hpp>

namespace Pascal
{
    namespace AST
    {
        class Visitor
        {
        public:
            virtual ~Visitor() {}
            
            virtual void visitProgramNode(const AST::ProgramNode& node) = 0;
            virtual void visitCompoundNode(const AST::CompoundNode& node) = 0;
            virtual void visitVarDeclNode(const AST::VarDeclNode& node) = 0;
            virtual void visitTypeNode(const AST::TypeNode& node) = 0;
            virtual void visitProcDeclNode(const AST::ProcDeclNode& node) = 0;
            virtual void visitAssignmentNode(const AST::AssignmentNode& node) = 0;
            virtual void visitVarNode(const AST::VarNode& node) = 0;
            virtual void visitIntLiteralNode(const AST::IntLiteralNode& node) = 0;
            virtual void visitBinaryExprNode(const AST::BinaryExprNode& node) = 0;
            virtual void visitUnaryExprNode(const AST::UnaryExprNode& node) = 0;
            virtual void visitProcCallNode(const AST::CallStmtNode& node) = 0;
            virtual void visitFunctionDeclNode(const AST::FunctionDeclNode& node) = 0;
            virtual void visitIfNode(const AST::IfNode& node) = 0;
            virtual void visitFunctionCall(const AST::FunctionCallNode& node) = 0;
        }; // class Visitor
    } // namespace AST
} // namespace Pascal

#endif // PASCAL_VISITOR_HPP_DEFINED
