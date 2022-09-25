#include <CodeGenVisitor.hpp>
#include <AST.hpp>
#include <ReportsManager.hpp>
#include <ParserRules.hpp>

#include <iostream>
#include <iomanip>

#define COMPILE_FOR_LITTLE_ENDIAN

using std::cout;
using std::endl;

namespace Pascal
{
	CodeGenVisitor::CodeGenVisitor(std::string const& path)
		: fout(path, std::ios::out),
		currentEnvironment(std::make_shared<Environment<SymAttribs>>()),
		currentStack(std::make_shared<StackEnvironment>())
	{
		if (!fout)
		{
			cout << "error: couldn't open file '" << path << "'" << endl;
			// TODO: Better
		}

		for (int i = 0; i < 16; i++)
			regs[i] = { false };

		regs[0].used = true;
		regs[1].used = true;
		regs[2].used = true;
		regs[3].used = true;
		regs[0xC].used = true;
		regs[0xD].used = true;
		regs[0xE].used = true;
		regs[0xF].used = true;
		currentLoadType = LoadType::LOAD;
		assignTargetIsLong = false;
	}
	
	CodeGenVisitor::~CodeGenVisitor()
	{
		fout.close();
	}
	
	void CodeGenVisitor::visitProgramNode(const AST::ProgramNode& node)
	{
		fout << ";; " << node.name().str << endl << std::hex << std::showbase;
		fout << ";; Attemp to program a compiler for CHIP-8" << endl;
		fout << ";; v0 - used only for retrieving value from memory (low)" << endl;
		fout << ";; v1 - used only for retrieving value from memory (high)" << endl;
		fout << ";; v2 - acumulator low half, used while evaluating expressions" << endl;
		fout << ";; v3 - acumulator high half, used while evaluating expressions" << endl;
		fout << ";; v4 - free" << endl;
		fout << ";; v5 - free" << endl;
		fout << ";; v6 - free" << endl;
		fout << ";; v7 - free" << endl;
		fout << ";; v8 - free" << endl;
		fout << ";; v9 - free" << endl;
		fout << ";; vA - free" << endl;
		fout << ";; vB - free" << endl;
		fout << ";; vC - various purposes (negation)" << endl;
		fout << ";; vD - bp" << endl;
		fout << ";; vE - stack pointer" << endl;
		fout << ";; vF - used only for ariphmetic operations flag holding" << endl;
		fout << endl;

		for (int i = 0; i < 16; i++)
		{
			if (i == 0xC)
				fout << "ld v" << i << ", 0xFF" << endl;
			else
				fout << "ld v" << i << ", 0" << endl;
		}
		fout << "ld I, 0" << endl;
		fout << "jp [__start__main]" << endl << endl;
		
		curBlockName = "__start__main";

		doProgramBlock(node.block());

		// ret?
		fout << "ret" << endl;
		fout << endl;
		
		fout << ";; global vars" << endl;
		for (auto const& e : currentStack->getVector())
		{
			fout << e.name << ": " << endl;
			fout << "    dw 0" << endl;
		}

		fout << endl;
		fout << "BCD_ZONE_LOW:" << endl;
		fout << "    dw 0" << endl;
		fout << "    dw 0" << endl;
		fout << endl;
		fout << "BCD_ZONE_HIGH:" << endl;
		fout << "    dw 0" << endl;
		fout << "    dw 0" << endl;
		fout << endl;
		fout << "STACK_ZONE:" << endl;
		//		for (int i = 0; i < 128; i++)
		fout << "    dw 0" << endl;
		fout << endl << ";; End!" << endl;
	}
	
	void CodeGenVisitor::doProgramBlock(const AST::BlockNode& node)
	{
		programBlock = true;
		for (auto const& decl : node.decls())
			decl->accept(this);
		programBlock = false;

		fout << curBlockName << ":" << endl;

		node.compound().accept(this);

		fout << endl;
		fout << "ret" << endl;
	}

	void CodeGenVisitor::visitBlockNode(const AST::BlockNode& node)
	{
		for (auto const& decl : node.decls())
			decl->accept(this);


		fout << curBlockName << ":" << endl;

		// push bp
		fout << "ld I, [STACK_ZONE]      ; block start" << endl;
		fout << "add I, v" << spReg() << endl;
		fout << "ld v0, v" << bpReg() << endl;
		fout << "ld [I], v0" << endl;
		fout << "add v" << spReg() << ", 1" << endl;

		// mov bp, sp
		fout << "ld v" << bpReg() << ", v" << spReg() << endl;

		// add sp, size
		fout << "add v" << spReg() << ", " << currentStack->size() << endl;

		fout << endl;

		node.compound().accept(this);

		fout << endl;

		// mov sp, bp
		fout << "ld v" << spReg() << ", v" << bpReg() << "           ; block end" << endl;
		// pop bp
		fout << "add v" << spReg() << ", 0xFF" << endl;
		fout << "ld I, [STACK_ZONE]" << endl;
		fout << "add I, v" << spReg() << endl;
		fout << "ld v0, [I]" << endl;
		fout << "ld v" << bpReg() << ", v0" << endl;

		// ret?
		//fout << "ret" << endl;
	}
	
	void CodeGenVisitor::visitCompoundNode(const AST::CompoundNode& node)
	{
		for (auto const& stmt : node.stmts())
			stmt->accept(this);
	}
	
	void CodeGenVisitor::visitVarDeclNode(const AST::VarDeclNode& node)
	{
		// TODO: Make better
		// Probably because there are only two type, it is appropriate way
		if (programBlock) currentStack->push(node.name().str, node.type().token().str == "long" ? 2 : 1);

		SymAttribs attrs;
		attrs.asVar.isReg = false;
		attrs.asVar.isGlobal = programBlock;
		attrs.type = node.type().token().str == "long" ? SymType::LONG : SymType::INTEGER;
		currentEnvironment->define(node.name().str, attrs);
	}
	
	void CodeGenVisitor::visitTypeNode(const AST::TypeNode& node)
	{
		if (node.token().str == "long") curParam = SymType::LONG;
		else curParam = SymType::INTEGER;
	}
	
	void CodeGenVisitor::visitProcDeclNode(const AST::ProcDeclNode& node)
	{
		SymAttribs attrs;
		attrs.type = SymType::PROCEDURE;
		attrs.asProc.arity = node.params().size();
		attrs.asProc.paramTypes = new SymType[node.params().size()];
		for (unsigned i = 0; i < node.params().size(); i++)
		{
			node.params()[i]->type().accept(this);
			attrs.asProc.paramTypes[i] = curParam;
		}

		currentEnvironment->define(node.name().str, attrs);

		std::string oldBlock = curBlockName;
		curBlockName = node.name().str;

		auto oldEnvironment = currentEnvironment;
		auto oldStack = currentStack;

		currentEnvironment = std::make_shared<Environment<SymAttribs>>(currentEnvironment);
		currentStack = std::make_shared<StackEnvironment>(currentStack);

		for (auto const& e : node.params())
			e->accept(this);

		node.block().accept(this);

		fout << "ret" << endl << endl;

		curBlockName = oldBlock;
		currentEnvironment = currentEnvironment->getEnclosing();
		currentStack = currentStack->getEnclosing();
	}
	
	void CodeGenVisitor::visitAssignmentNode(const AST::AssignmentNode& node)
	{
		SymAttribs attrs = currentEnvironment->lookupAndAncestors(node.var().token().str);
		assignTargetIsLong = attrs.type == SymType::LONG;

		node.expr().accept(this);

		if (attrs.asVar.isGlobal)
			assignGlobalVariable(node.var());
		else
			assignStackVariable(node.var());
	}

	void CodeGenVisitor::assignStackVariable(const AST::VarNode& node)
	{
		fout << "ld I, [STACK_ZONE]      ; assigning variable" << endl;
		unsigned char gotOffset = currentStack->offset(node.token().str);
		//if (gotOffset < 0) gotOffset = Rules::twosComplement(gotOffset);
		fout << "add v" << bpReg() << ", " << (int)gotOffset << endl;
		fout << "add I, v" << bpReg() << endl;
		fout << "ld v" << loadLow() << ", v" << accLow() << endl;
		if (assignTargetIsLong)
			fout << "ld v" << loadHigh() << ", v" << accHigh() << endl;
		fout << "ld [I], v" << (assignTargetIsLong ? 1 : 0) << endl;
		fout << "add v" << bpReg() << ", " << Rules::twosComplement(gotOffset) << endl;
		fout << endl;
	}

	void CodeGenVisitor::assignGlobalVariable(const AST::VarNode& node)
	{
		fout << "ld I, [" << node.token().str << "]      ; assigning variable" << endl;
		fout << "ld v" << loadLow() << ", v" << accLow() << endl;
		if (assignTargetIsLong)
			fout << "ld v" << loadHigh() << ", v" << accHigh() << endl;
		fout << "ld [I], v" << (assignTargetIsLong ? 1 : 0) << endl;
		fout << endl;
	}
	
	void CodeGenVisitor::visitVarNode(const AST::VarNode& node)
	{
		SymAttribs varType = currentEnvironment->lookupAndAncestors(node.token().str);
		if (varType.asVar.isGlobal)
			getGlobalVariable(node);
		else
			getStackVariable(node);
	}

	void CodeGenVisitor::getStackVariable(const AST::VarNode& node)
	{
		fout << "ld I, [STACK_ZONE]      ; getting variable '" << node.token().str << "'" << endl;
		unsigned char gotOffset = currentStack->offset(node.token().str);
		//if (gotOffset < 0) gotOffset = Rules::twosComplement(gotOffset);
		fout << "add v" << bpReg() << ", " << (int)gotOffset << endl;
		fout << "add I, v" << bpReg() << endl;

		SymType varType = currentEnvironment->lookupAndAncestors(node.token().str).type;

		fout << "ld " << " v" <<
			(varType == SymType::LONG && assignTargetIsLong ? loadHigh() : loadLow())
			<< ", [I]" << endl;
		if (varType != SymType::LONG && assignTargetIsLong)
			fout << "ld v" << loadHigh() << ", 0" << endl;

		fout << loadType() << " v" << accLow() << ", v" << loadLow() << endl;
		if (assignTargetIsLong)
		{
			if (currentLoadType == LoadType::ADD) // TODO: Substract?
				fout << "add v" << accHigh() << ", v" << flagReg() << endl;
			fout << loadType() << " v" << accHigh() << ", v" << loadHigh() << endl;
		}

		fout << "add v" << bpReg() << ", " << Rules::twosComplement(gotOffset) << endl;
		fout << endl;
	}

	void CodeGenVisitor::getGlobalVariable(const AST::VarNode& node)
	{
		fout << "ld I, [" << node.token().str << "]     ; loading global var" << endl;
		
		SymType varType = currentEnvironment->lookupAndAncestors(node.token().str).type;

		if (varType == SymType::LONG && assignTargetIsLong)
			fout << "ld v1, [I]" << endl;
		else
			fout << "ld v0, [I]" << endl;

		fout << loadType() << " v" << accLow() << ", v" << loadLow() << endl;
		if (assignTargetIsLong)
		{
			if (currentLoadType == LoadType::ADD) // TODO: Substract?
				fout << "add v" << accHigh() << ", v" << flagReg() << endl;
			fout << loadType() << " v" << accHigh() << ", v" << loadHigh() << endl;
		}
		fout << endl;
	}
	
	void CodeGenVisitor::visitIntLiteralNode(const AST::IntLiteralNode& node)
	{
		uint32_t literal;

		try
		{
			literal = std::stoi(node.token().str);
		}
		catch (...)
		{
			ReportsManager::ReportError(node.token().pos, ErrorType::CANT_PARSE_LITERAL);
			return;
		}

		// TODO: Check if too big

		unsigned char lowPart, highPart;

#if defined(COMPILE_FOR_BIG_ENDIAN)
#error ???        
#elif defined(COMPILE_FOR_LITTLE_ENDIAN)
		lowPart = (literal & 0xFF);
		highPart = ((literal >> 8) & 0xFF);
#else
#error No endianess defined
#endif

		if (currentLoadType == LoadType::LOAD)
		{
			fout << "ld v" << accLow() << ", " << (int)lowPart << "     ; literal" << endl;
			if (assignTargetIsLong)
				fout << "ld v" << accHigh() << ", " << (int)highPart << endl;
		}
		else if (!assignTargetIsLong)
		{
			fout << loadType() << " v" << accLow() << ", " << (int)lowPart << "     ; literal" << endl;
		}
		else
		{
			fout << "ld v" << loadLow() << ", " << (int)lowPart << "     ; literal" << endl;

			fout << loadType() << " v" << accLow() << ", v" << loadLow() << endl;
			fout << "add v" << accHigh() << ", v" << flagReg() << endl;
			fout << loadType() << " v" << accHigh() << ", v" << (int)highPart << endl;
		}
	}
	
	void CodeGenVisitor::visitLogicExprNode(const AST::LogicExprNode& node)
	{
		
	}
	
	void CodeGenVisitor::visitEqualityExprNode(const AST::EqualityExprNode& node)
	{
		
	}
	
	void CodeGenVisitor::visitComparisonExprNode(const AST::ComparisonExprNode& node)
	{
		
	}

	void CodeGenVisitor::visitBinaryExprNode(const AST::BinaryExprNode& node)
	{
		node.left().accept(this);

		if (dynamic_cast<const AST::VarNode*>(&node.right()) != nullptr
			|| dynamic_cast<const AST::IntLiteralNode*>(&node.right()) != nullptr)
		{
			LoadType oldLoad = currentLoadType;

			switch (node.op().type)
			{
			case TokenType::PLUS:
				currentLoadType = LoadType::ADD;
				break;
			case TokenType::MINUS:
				currentLoadType = LoadType::SUBSTRACT;
				break;

			default:
				ReportsManager::ReportError(node.op().pos, "Operation unimplemented");
				break;
			}

			node.right().accept(this);
			
			currentLoadType = oldLoad;
		}
		else if (dynamic_cast<const AST::BinaryExprNode*>(&node.right()) != nullptr
				 || dynamic_cast<const AST::UnaryExprNode*>(&node.right()) != nullptr)
		{
			fout << "ld v" << loadLow() << ", v" << accLow() << endl;
			if (assignTargetIsLong)
				fout << "ld v" << loadHigh() << ", v" << accHigh() << endl;
			fout << "ld I, [STACK_ZONE]" << endl;
			fout << "add I, v" << spReg() << endl;
			fout << "ld [I], v" << (assignTargetIsLong ? 1 : 0) << endl;
			fout << "add v" << spReg() << ", " << (assignTargetIsLong ? 2 : 1) << endl;

			LoadType oldLoad = currentLoadType;
			currentLoadType = LoadType::LOAD;

			node.right().accept(this);

			currentLoadType = oldLoad;

			fout << "ld I, [STACK_ZONE]" << endl;
			fout << "add v" << spReg() << ", " << Rules::twosComplement(assignTargetIsLong ? 2 : 1) << endl;
			fout << "add I, v" << spReg() << endl;
			fout << "ld v" << (assignTargetIsLong ? 1 : 0) << ", [I]" << endl;
			
			switch (node.op().type)
			{
			case TokenType::PLUS:
				fout << "add v" << loadLow() << ", v" << accLow() << endl;
				if (assignTargetIsLong)
				{
					fout << "add v" << loadHigh() << ", v" << flagReg() << endl;
					fout << "add v" << loadHigh() << ", v" << accHigh() << endl;
				}
				break;
			case TokenType::MINUS:
				// TODO: Minus ???
				// vf = 1 if NOT BORROW (vx > vy then 1 otherwise 0)
				fout << "sub v" << loadLow() << ", v" << accLow() << endl;
				if (assignTargetIsLong)
				{
					//fout << "sub v" << loadHigh() << ", v" << flagReg() << endl;
					fout << "sub v" << loadHigh() << ", v" << accHigh() << endl;
				}
				break;

			default:
				ReportsManager::ReportError(node.op().pos, "Operation unimplemented");
				break;
			}

			fout << "ld v" << accLow() << ", v" << loadLow() << endl;
			if (assignTargetIsLong)
				fout << "ld v" << accHigh() << ", " << loadHigh() << endl;

			fout << endl;
		}
		else
		{
			assert(false && "Unreachable");
		}
	}
	
	void CodeGenVisitor::visitUnaryExprNode(const AST::UnaryExprNode& node)
	{
		node.expr().accept(this);

		if (node.op().type == TokenType::PLUS)
			return;
		else if (node.op().type == TokenType::MINUS)
		{
			fout << "xor v" << accLow() << ", v" << negReg() << endl;
			if (assignTargetIsLong)
			{
				fout << "xor v" << accHigh() << ", v" << negReg() << endl;
			}
			// Carry is set only when adding register and register
			if (assignTargetIsLong)
			{
				fout << "ld v" << negReg() << ", 1" << endl;
				fout << "add v" << accLow() << ", v" << negReg() << endl;
				fout << "add v" << accHigh() << ", v" << flagReg() << endl;
				fout << "ld v" << negReg() << ", 0xFF" << endl;
			}
			else
			{
				fout << "add v" << accLow() << ", 1" << endl;
			}
		}
	}
	
	void CodeGenVisitor::visitProcCallNode(const AST::CallStmtNode& node)
	{
		std::string const& name = node.name().str;
		if (name == "break")
		{
			fout << "break" << endl;
		}
		else if (name == "cls")
		{
			fout << "cls" << endl;
		}
		else if (name == "make_bcd")
		{
			if (node.args().size() != 1)
			{
				ReportsManager::ReportError(node.name().pos, ErrorType::WRONG_ARGUMENTS_COUNT);
				return;
			}

			assignTargetIsLong = true; // ???
			node.args()[0]->accept(this);

			fout << "ld I, [BCD_ZONE_LOW]" << endl;
			fout << "ld B, v" << accLow() << endl;
			fout << "ld I, [BCD_ZONE_HIGH]" << endl;
			fout << "ld B, v" << accHigh() << endl;
		}
		else if (name == "debug_print_bcd")
		{
			if (node.args().size() != 0)
			{
				ReportsManager::ReportError(node.name().pos, ErrorType::WRONG_ARGUMENTS_COUNT);
				return;
			}

			fout << "ld v" << negReg() << ", 0" << endl;
			fout << "ld I, [BCD_ZONE_LOW]" << endl;
			fout << "ld v2, [I]" << endl;

			fout << "ld F, v0" << endl;
			fout << "drw v" << negReg() << ", v" << negReg() << ", 5" << endl;
			fout << "ld v0, 6" << endl;

			fout << "ld F, v1" << endl;
			fout << "drw v0, v" << negReg() << ", 5" << endl;

			fout << "add v0, 6" << endl;

			fout << "ld F, v2" << endl;
			fout << "drw v0, v" << negReg() << ", 5" << endl;

			fout << "ld v" << negReg() << ", 0xFF" << endl;
		}
		else if (name == "debug_print_bcd_high")
		{
			if (node.args().size() != 0)
			{
				ReportsManager::ReportError(node.name().pos, ErrorType::WRONG_ARGUMENTS_COUNT);
				return;
			}

			fout << "ld v" << negReg() << ", 0" << endl;
			fout << "ld I, [BCD_ZONE_HIGH]" << endl;
			fout << "ld v2, [I]" << endl;

			fout << "ld F, v0" << endl;
			fout << "drw v" << negReg() << ", v" << negReg() << ", 5" << endl;
			fout << "ld v0, 6" << endl;

			fout << "ld F, v1" << endl;
			fout << "drw v0, v" << negReg() << ", 5" << endl;

			fout << "add v0, 6" << endl;

			fout << "ld F, v2" << endl;
			fout << "drw v0, v" << negReg() << ", 5" << endl;

			fout << "ld v" << negReg() << ", 0xFF" << endl;
		}
		else
		{
			SymAttribs proc = currentEnvironment->lookupAndAncestors(node.name().str);
			if (node.args().size() != proc.asProc.arity)
			{
				ReportsManager::ReportError(node.name().pos, ErrorType::WRONG_ARGUMENTS_COUNT);
				return;
			}

			SymType* curParam = proc.asProc.paramTypes;
			unsigned sperr = 1;
			if (node.args().size() != 0) fout << "add v" << spReg() << ", 1" << endl; // because bp will be saved
			for (auto const& e : node.args())
			{
				// TODO: Args type check
				assignTargetIsLong = *curParam == SymType::LONG;
				e->accept(this);
				
				fout << "ld I, [STACK_ZONE]" << endl;
				fout << "add I, v" << spReg() << endl;

				fout << "ld v" << loadLow() << ", v" << accLow() << endl;
				if (*curParam == SymType::LONG)
					fout << "ld v" << loadHigh() << ", v" << accHigh() << endl;

				fout << "ld [I], v" << (*curParam == SymType::LONG ? 1 : 0) << endl;

				fout << "add v" << spReg() << ", " << (*curParam == SymType::LONG ? 2 : 1) << endl;
				sperr += (*curParam == SymType::LONG ? 2 : 1);

				curParam++;
			}
			// restore sp
			if (node.args().size() != 0) fout << "add v" << spReg() << ", " << Rules::twosComplement(sperr) << endl;

			fout << "call [" << node.name().str << "]" << endl << endl;
		}
	}
	
} // namespace Pascal
