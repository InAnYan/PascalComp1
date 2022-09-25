#ifndef PASCAL_AST_HPP_DEFINED
#define PASCAL_AST_HPP_DEFINED

#include <memory>

#include <ASTForwards.hpp>
#include <Visitor.hpp>
#include <NonConstVisitor.hpp>
#include <Token.hpp>

namespace Pascal
{
	namespace AST
	{
		struct Node
		{
			virtual ~Node() {}
			virtual void accept(Visitor* visitor) const = 0;
			virtual void accept(NonConstVisitor* visitor) = 0;
		};
		
		struct ProgramNode : public Node
		{
			ProgramNode(Token name, std::vector<std::unique_ptr<DeclarationNode>> decls, std::unique_ptr<CompoundNode> compound)
				: name(name), decls(std::move(decls)), compound(std::move(compound))
			{ }
			
			void accept(Visitor* visitor) const
			{ visitor->visitProgramNode(*this); }
			
			void accept(NonConstVisitor* visitor)
			{ visitor->visitProgramNode(*this); }
			
			Token name;
			std::vector<std::unique_ptr<DeclarationNode>> decls;
			std::unique_ptr<CompoundNode> compound;
		};
		
		struct StmtNode : public Node
		{
		};

		struct DeclarationNode : public Node
		{
		};

		struct ExpressionNode : public Node
		{
		};

		struct CompoundNode : public StmtNode
		{
			CompoundNode(std::vector<std::unique_ptr<StmtNode>> stmts)
				: stmts(std::move(stmts))
			{ }
			
			void accept(Visitor* visitor) const
			{ visitor->visitCompoundNode(*this); }
			
			void accept(NonConstVisitor* visitor)
			{ visitor->visitCompoundNode(*this); }
			
			std::vector<std::unique_ptr<StmtNode>> stmts;
		};
		
		struct VarDeclNode : public DeclarationNode
		{
			VarDeclNode(Token name, std::unique_ptr<TypeNode> type, bool isConst)
				: name(name), type(std::move(type)), isConst(isConst)
			{ }
			
			void accept(Visitor* visitor) const
			{ visitor->visitVarDeclNode(*this); }
			
			void accept(NonConstVisitor* visitor)
			{ visitor->visitVarDeclNode(*this); }
			
			Token name;
			std::unique_ptr<TypeNode> type;
			bool isConst;
		};
		
		struct TypeNode : public Node
		{
			TypeNode(Token token)
				: token(token)
			{ }
			
			void accept(Visitor* visitor) const
			{ visitor->visitTypeNode(*this); }
			
			void accept(NonConstVisitor* visitor)
			{ visitor->visitTypeNode(*this); }
			
			Token token;
		};
		
		struct ProcDeclNode : public DeclarationNode
		{
			ProcDeclNode(Token name, std::vector<std::unique_ptr<VarDeclNode>> params, 
				std::vector<std::unique_ptr<VarDeclNode>> decls, std::unique_ptr<CompoundNode> compound)
				: name(name), params(std::move(params)), decls(std::move(decls)), compound(std::move(compound))
			{ }
			
			void accept(Visitor* visitor) const
			{ visitor->visitProcDeclNode(*this); }
			
			void accept(NonConstVisitor* visitor)
			{ visitor->visitProcDeclNode(*this); }

			Token name;
			std::vector<std::unique_ptr<VarDeclNode>> params;
			std::vector<std::unique_ptr<VarDeclNode>> decls;
			std::unique_ptr<CompoundNode> compound;
		};
		
		struct AssignmentNode : public StmtNode
		{
			AssignmentNode(std::unique_ptr<VarNode> var, std::unique_ptr<ExpressionNode> expr)
				: var(std::move(var)), expr(std::move(expr))
			{ }
			
			void accept(Visitor* visitor) const
			{ visitor->visitAssignmentNode(*this); }
			
			void accept(NonConstVisitor* visitor)
			{ visitor->visitAssignmentNode(*this); }
		
			std::unique_ptr<VarNode> var;
			std::unique_ptr<ExpressionNode> expr;
		};
		
		struct VarNode : public ExpressionNode
		{
			VarNode(Token token)
				: token(token)
			{ }
			
			void accept(Visitor* visitor) const
			{ visitor->visitVarNode(*this); }
			
			void accept(NonConstVisitor* visitor)
			{ visitor->visitVarNode(*this); }

			Token token;
		};
		
		struct IntLiteralNode : public ExpressionNode
		{
			IntLiteralNode(Token token)
				: token(token)
			{ }
			
			void accept(Visitor* visitor) const
			{ visitor->visitIntLiteralNode(*this); }
			
			void accept(NonConstVisitor* visitor)
			{ visitor->visitIntLiteralNode(*this); }

			Token token;
		};
		
		struct BinaryExprNode : public ExpressionNode
		{
			BinaryExprNode(std::unique_ptr<ExpressionNode> left, Token op, std::unique_ptr<ExpressionNode> right)
				: left(std::move(left)), op(op), right(std::move(right))
			{ }
			
			void accept(Visitor* visitor) const
			{ visitor->visitBinaryExprNode(*this); }
			
			void accept(NonConstVisitor* visitor)
			{ visitor->visitBinaryExprNode(*this); }

			std::unique_ptr<ExpressionNode> left;
			std::unique_ptr<ExpressionNode> right;
			Token op;
		};
		
		struct UnaryExprNode : public ExpressionNode
		{
			UnaryExprNode(Token op, std::unique_ptr<ExpressionNode> expr)
				: op(op), expr(std::move(expr))
			{ }
			
			void accept(Visitor* visitor) const
			{ visitor->visitUnaryExprNode(*this); }
			
			void accept(NonConstVisitor* visitor)
			{ visitor->visitUnaryExprNode(*this); }
			
			Token op;
			std::unique_ptr<ExpressionNode> expr;
		};

		struct CallStmtNode : public StmtNode
		{
			CallStmtNode(Token name, std::vector<std::unique_ptr<ExpressionNode>> args)
				: name(name), args(std::move(args))
			{}

			virtual void accept(Visitor* visitor) const
			{ visitor->visitProcCallNode(*this); }
			virtual void accept(NonConstVisitor* visitor)
			{ visitor->visitProcCallNode(*this); }

			std::vector<std::unique_ptr<ExpressionNode>> args;
			Token name;
		};

		struct FunctionDeclNode : public DeclarationNode
		{
			FunctionDeclNode(Token name, std::vector<std::unique_ptr<VarDeclNode>> params,
				std::vector<std::unique_ptr<VarDeclNode>> decls, std::unique_ptr<CompoundNode> compound)
				: name(name), params(std::move(params)), decls(std::move(decls)), compound(std::move(compound))
			{ }

			void accept(Visitor* visitor) const
			{
				visitor->visitFunctionDeclNode(*this);
			}

			void accept(NonConstVisitor* visitor)
			{
				visitor->visitFunctionDeclNode(*this);
			}

			Token name;
			std::vector<std::unique_ptr<VarDeclNode>> params;
			std::vector<std::unique_ptr<VarDeclNode>> decls;
			std::unique_ptr<CompoundNode> compound;
		};

		struct FunctionCallNode : ExpressionNode
		{
			FunctionCallNode(Token name, std::vector<std::unique_ptr<VarDeclNode>> args)
				: name(name), args(std::move(args))
			{ }

			void accept(Visitor* visitor) const
			{
				visitor->visitFunctionCall(*this);
			}

			void accept(NonConstVisitor* visitor)
			{
				visitor->visitFunctionCall(*this);
			}

			Token name;
			std::vector<std::unique_ptr<VarDeclNode>> args;
		};

		struct IfNode : StmtNode
		{
			IfNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<StmtNode> thenArm, std::unique_ptr<StmtNode> elseArm)
				: condition(std::move(condition)), thenArm(std::move(thenArm)), elseArm(std::move(elseArm))
			{ }

			void accept(Visitor* visitor) const
			{
				visitor->visitIfNode(*this);
			}

			void accept(NonConstVisitor* visitor)
			{
				visitor->visitIfNode(*this);
			}

			std::unique_ptr<ExpressionNode> condition;
			std::unique_ptr<StmtNode> thenArm;
			std::unique_ptr<StmtNode> elseArm;
		};
		
	} // namespace AST
	
} // namespace Pascal

#endif // PASCAL_AST_HPP_DEFINED
