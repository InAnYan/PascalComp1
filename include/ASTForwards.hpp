#ifndef PASCAL_ASTFORWARDS_HPP_DEFINED
#define PASCAL_ASTFORWARDS_HPP_DEFINED

namespace Pascal
{
    namespace AST
    {
        class Visitor;
        class NonConstVisitor;
        
        struct Node;
        struct ProgramNode;
        struct CompoundNode;
        struct VarDeclNode;
        struct TypeNode;
        struct ProcDeclNode;
        struct AssignmentNode;
        struct VarNode;
        struct IntLiteralNode;
        struct BinaryExprNode;
        struct UnaryExprNode;
        struct ExpressionNode;
        struct DeclarationNode;
        struct StmtNode;
        struct CallStmtNode;
        struct IfNode;
        struct FunctionDeclNode;
        struct FunctionCallNode;
    } // namespace AST
} // namespace Pascal

#endif // PASCAL_ASTFORWARDS_HPP_DEFINED
