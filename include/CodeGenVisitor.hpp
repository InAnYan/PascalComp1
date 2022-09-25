#ifndef PASCAL_CODEGEN_HPP
#define PASCAL_CODEGEN_HPP

#include <Visitor.hpp>

#include <Environment.hpp>
#include <StackEnvironment.hpp>

#include <array>
#include <map>
#include <string>
#include <fstream>

namespace Pascal
{
	enum class SymType
	{
		INTEGER,
		LONG,
		PROCEDURE
	};

	// TODO: Create better representation
	typedef struct
	{
		SymType type;
		
		union
		{
			struct
			{
				bool isReg;
				bool isGlobal;
				bool isConst;

				union
				{
					unsigned reg;
					unsigned addr;
				};
			} asVar;

			struct
			{
				unsigned arity;
				// TODO: Never freed...
				SymType* paramTypes;
			} asProc;
		};
	} SymAttribs;

    class CodeGenVisitor : public AST::Visitor
    {
    public:
        CodeGenVisitor(std::string const& path);
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
        void visitProcCallNode(const AST::CallStmtNode& node);
        
    private:
		std::ofstream fout;

		enum class LoadType
		{
			LOAD, ADD, SUBSTRACT
		};

		LoadType currentLoadType;
		bool assignTargetIsLong;
		bool programBlock;

		void doProgramBlock(const AST::BlockNode& node);

		void getStackVariable(const AST::VarNode& node);
		void getGlobalVariable(const AST::VarNode& node);

		void assignStackVariable(const AST::VarNode& node);
		void assignGlobalVariable(const AST::VarNode& node);

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
		inline unsigned negReg() { return 0xC; }
		inline unsigned spReg() { return 0xE; }
		inline unsigned bpReg() { return 0xD; }
		inline unsigned flagReg() { return 0xF; }

		typedef struct
		{
			bool used;
		} RegsAttribs;

		std::shared_ptr<Environment<SymAttribs>> currentEnvironment;
		std::shared_ptr<StackEnvironment> currentStack;

		std::array<RegsAttribs, 16> regs;

		std::string curBlockName;

		SymType curParam;
    }; // class CodeGen
} // namespace Pascal

#endif // PASCAL_CODEGEN_HPP
