#ifndef PASCAL_CODEGEN_HPP
#define PASCAL_CODEGEN_HPP

#include <Visitor.hpp>

#include <array>
#include <map>
#include <string>

namespace Pascal
{
    class CodeGenVisitor : public AST::Visitor
    {
    public:
        CodeGenVisitor();
        ~CodeGenVisitor();
        
        void visitProgramNode(const AST::ProgramNode& node);
        void visitBlockNode(const AST::BlockNode& node);
        void visitCompoundNode(const AST::CompoundNode& node);
        void visitVarDeclNode(const AST::VarDeclNode& node);
        void visitTypeNode(const AST::TypeNode& node);
        void visitProcDeclNode(const AST::ProcDeclNode& node);
        void visitAssignmentNode(const AST::AssignmentNode& node);
        void visitVarNode(const AST::VarNode& node);
        void visitIntLiteralNode(const AST::IntLiteralNode& node);
        void visitLogicExprNode(const AST::LogicExprNode& node);
        void visitEqualityExprNode(const AST::EqualityExprNode& node);
        void visitComparisonExprNode(const AST::ComparisonExprNode& node);
        void visitBinaryExprNode(const AST::BinaryExprNode& node);
        void visitUnaryExprNode(const AST::UnaryExprNode& node);
        void visitProcCallNode(const AST::ProcCallNode& node);
        
    private:
		enum class LoadType
		{
			LOAD, ADD, SUBSTRACT
		};

		LoadType currentLoadType;
		bool assignTargetIsLong;

		inline std::string loadType()
		{
			switch (currentLoadType)
			{
			case LoadType::LOAD:
				return "ld";
			case LoadType::ADD:
				return "add";
			case LoadType::SUBSTRACT:
				return "sub";
			}
		}

		inline unsigned accHigh() { return 0x3; }
		inline unsigned accLow() { return 0x2; }
		inline unsigned loadHigh() { return 0x1; }
		inline unsigned loadLow() { return 0x0; }
		inline unsigned negReg() { return 0xD; }
		inline unsigned stackReg() { return 0xE; }
		inline unsigned flagReg() { return 0xF; }

		enum class VarType
		{
			INTEGER,
			LONG
		};

		typedef struct
		{
			VarType type;
			bool isReg;

			union
			{
				unsigned reg;
				unsigned addr;
			};

		} VarAttribs;

		typedef struct
		{
			bool used;
		} RegsAttribs;

		std::map<std::string, VarAttribs> vars;
		std::array<RegsAttribs, 16> regs;
    }; // class CodeGen
} // namespace Pascal

#endif // PASCAL_CODEGEN_HPP
