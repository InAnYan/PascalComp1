#ifndef PASCAL_NONCONSTVISITOR_HPP_DEFINED
#define PASCAL_NONCONSTVISITOR_HPP_DEFINED

#include <ASTForwards.hpp>

namespace Pascal
{
    namespace AST
    {
        class NonConstVisitor
        {
        public:
            virtual ~NonConstVisitor() {}
            
            virtual void visitProgramNode(ProgramNode& node) = 0;
            virtual void visitCompoundNode(CompoundNode& node) = 0;
            virtual void visitVarDeclNode(VarDeclNode& node) = 0;
            virtual void visitTypeNode(TypeNode& node) = 0;
            virtual void visitProcDeclNode(ProcDeclNode& node) = 0;
            virtual void visitAssignmentNode(AssignmentNode& node) = 0;
            virtual void visitVarNode(VarNode& node) = 0;
            virtual void visitIntLiteralNode(IntLiteralNode& node) = 0;
            virtual void visitBinaryExprNode(BinaryExprNode& node) = 0;
            virtual void visitUnaryExprNode(UnaryExprNode& node) = 0;
            virtual void visitProcCallNode(AST::CallStmtNode& node) = 0;
            virtual void visitFunctionDeclNode(AST::FunctionDeclNode& node) = 0;
            virtual void visitIfNode(AST::IfNode& node) = 0;
            virtual void visitFunctionCall(AST::FunctionCallNode& node) = 0;
        }; // class NonConstVisitor
    } // namespace AST
} // namespace Pascal

#endif // PASCAL_NONCONSTVISITOR_HPP_DEFINED
