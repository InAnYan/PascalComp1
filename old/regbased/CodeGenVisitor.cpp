#include <CodeGenVisitor.hpp>
#include <AST.hpp>
#include <ReportsManager.hpp>

#include <iostream>

#define COMPILE_FOR_LITTLE_ENDIAN

using std::cout;
using std::endl;

namespace Pascal
{
    CodeGenVisitor::CodeGenVisitor()
    {
        for (int i = 0; i < 16; i++)
            regs[i] = { false };

        regs[0].used = true;
        regs[1].used = true;
        regs[2].used = true;
        regs[3].used = true;
        regs[0xD].used = true;
        regs[0xE].used = true;
        regs[0xF].used = true;
        currentLoadType = LoadType::LOAD;
        assignTargetIsLong = false;
    }
    
    CodeGenVisitor::~CodeGenVisitor()
    {
        
    }
    
    void CodeGenVisitor::visitProgramNode(const AST::ProgramNode& node)
    {
        cout << ";; " << node.name().str << endl;
        cout << ";; Attemp to program a compiler for CHIP-8" << endl;
        cout << ";; v0 - used only for retrieving value from memory" << endl;
        cout << ";; v1 - used only for retrieving value from memory" << endl;
        cout << ";; v2 - acumulator high half, used while evaluating expressions" << endl;
        cout << ";; v3 - acumulator low half, used while evaluating expressions" << endl;
        cout << ";; v4 - free" << endl;
        cout << ";; v5 - free" << endl;
        cout << ";; v6 - free" << endl;
        cout << ";; v7 - free" << endl;
        cout << ";; v8 - free" << endl;
        cout << ";; v9 - free" << endl;
        cout << ";; vA - free" << endl;
        cout << ";; vB - free" << endl;
        cout << ";; vC - free" << endl;
        cout << ";; vD - used for negation" << endl;
        cout << ";; vE - stack pointer" << endl;
        cout << ";; vF - used only for ariphmetic operations flag holding" << endl;
        cout << endl;

        for (int i = 0; i < 16; i++)
        {
            if (i == 0xD)
                cout << "ld v" << i << ", 0xFF" << endl;
            else
                cout << "ld v" << i << ", 0" << endl;
        }
        cout << "ld I, 0" << endl << endl;

        node.block().accept(this);

        cout << endl;
        cout << "BCD_ZONE_LOW:" << endl;
        cout << "    db 0" << endl;
        cout << "    db 0" << endl;
        cout << "    db 0" << endl;
        cout << endl;
        cout << "BCD_ZONE_HIGH:" << endl;
        cout << "    db 0" << endl;
        cout << "    db 0" << endl;
        cout << "    db 0" << endl;
        cout << endl;
        cout << "STACK_ZONE:" << endl;
        //		for (int i = 0; i < 64; i++) // TODO: More?
        cout << "    db 0" << endl;
        cout << endl << ";; End!" << endl;
    }
    
    void CodeGenVisitor::visitBlockNode(const AST::BlockNode& node)
    {
        for (auto const& decl : node.decls())
            decl->accept(this);

        node.compound().accept(this);
    }
    
    void CodeGenVisitor::visitCompoundNode(const AST::CompoundNode& node)
    {
        for (auto const& stmt : node.stmts())
            stmt->accept(this);
    }
    
    void CodeGenVisitor::visitVarDeclNode(const AST::VarDeclNode& node)
    {
        for (int i = 0; i < 15; i++)
        {
            if (!regs[i].used)
            {
                VarAttribs var;

                if (node.type().token().str == "integer")
                    var.type = VarType::INTEGER;
                else if (node.type().token().str == "long")
                {
                    if (i + 1 >= 15 || regs[i + 1].used)
                    {
                        continue;
                    }
                    regs[i + 1].used = true;
                }
                
                regs[i].used = true;
                
                var.isReg = true;
                var.reg = i;

                vars[node.name().str] = var;
                
                cout << ";; allocation of variable '" << node.name().str << "' successfull" << endl;
                return;
            }
        }

        ReportsManager::ReportError(node.name().pos,
            "Could not allocate memory for value (WRAM unsupported)", false);
    }
    
    void CodeGenVisitor::visitTypeNode(const AST::TypeNode& node)
    {
        
    }
    
    void CodeGenVisitor::visitProcDeclNode(const AST::ProcDeclNode& node)
    {
        ReportsManager::ReportError(node.name().pos, "Procedures unsupported");
    }
    
    void CodeGenVisitor::visitAssignmentNode(const AST::AssignmentNode& node)
    {
        VarAttribs var = vars[node.var().token().str];

        assignTargetIsLong = var.type == VarType::LONG;

        currentLoadType = LoadType::LOAD;

        node.expr().accept(this);

        if (var.isReg)
        {
            cout << "ld v" << var.reg << ", v" << accLow() << endl;
            if (var.type == VarType::LONG)
            {
                cout << "ld v" << var.reg + 1 << ", v" << accHigh() << endl;
            }
        }
        else
        {
            // TODO:
        }

        cout << endl;
    }
    
    void CodeGenVisitor::visitVarNode(const AST::VarNode& node)
    {
        VarAttribs var = vars[node.token().str];
        if (var.isReg)
        {
            cout << loadType() << " v" << accLow() << ", v" << var.reg << endl;
            if (assignTargetIsLong)
            {
                if (var.type == VarType::LONG)
                    cout << loadType() << " v" << accHigh() << ", v" << var.reg + 1 << endl;
                else if (currentLoadType == LoadType::LOAD)
                    cout << loadType() << " v" << accHigh() << ", 0" << endl;
            }
        }
        else
        {
            // TODO:
        }

        cout << endl;
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

        char lowPart, highPart;

#if defined(COMPILE_FOR_BIG_ENDIAN)
#error ???        
#elif defined(COMPILE_FOR_LITTLE_ENDIAN)
        lowPart = (literal & 0xFF);
        highPart = ((literal >> 8) & 0xFF);
        if (assignTargetIsLong)
            cout << 
            (currentLoadType == LoadType::SUBSTRACT ? "add" : loadType())
            << " v" << accHigh() << ", " << 
            (currentLoadType == LoadType::SUBSTRACT ? (~highPart + 1) : highPart)
            << endl;
        cout << 
            (currentLoadType == LoadType::SUBSTRACT ? "add" : loadType())
            << " v" << accLow() << ", " << 
            (int)(unsigned char)(currentLoadType == LoadType::SUBSTRACT ? (~lowPart + 1) : lowPart)
            << endl;
#else
#error No endianess defined
#endif
        cout << endl;
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
        // 1. Evaluate left
        // 2. If right is not var or literal, then save to stack, evaluate right, and then perfom final operation
        // 3. Else save accumulator to stack, evalluate right, and 
        //    preform binary operation with saved acum and current acum
        // ....

        node.left().accept(this);

        if (dynamic_cast<const AST::BinaryExprNode*>(&node.right()) != nullptr
            || dynamic_cast<const AST::UnaryExprNode*>(&node.right()) != nullptr)
        {
            // save accum to stack
            // set load type to load
            // evaluate right
            // load accum
            // perform operation with loaded value and accum

            cout << "ld I, [STACK_ZONE]" << endl;
            cout << "add I, v" << stackReg() << endl;
            cout << "add v" << stackReg() <<
                (assignTargetIsLong ? ", 2" : ", 1")
                << endl;
            cout << "ld v" << loadLow() << ", v" << accLow() << endl;
            if (assignTargetIsLong)
                cout << "ld v" << loadHigh() << ", v" << accHigh() << endl;

            cout << "ld [I], v" << (assignTargetIsLong ? loadHigh() : loadLow()) << endl;
            cout << endl;

            LoadType oldLoad = currentLoadType;
            currentLoadType = LoadType::LOAD;

            node.right().accept(this);

            currentLoadType = oldLoad;

            cout << "add v" << stackReg() <<
                (assignTargetIsLong ? ", 0xFE ; -2" : ", 0xFF ; -1")
                << endl;

            cout << "ld v" << (assignTargetIsLong ? loadHigh() : loadLow()) << ", [I]" << endl;

            switch (node.op().type)
            {
            case TokenType::PLUS:
                cout << "add v" << loadLow() << ", v" << accLow() << endl;
                if (assignTargetIsLong)
                    cout << "add v" << loadHigh() << ", v" << accHigh() << endl;
                break;
            case TokenType::MINUS:
                cout << "sub v" << loadLow() << ", v" << accLow() << endl;
                if (assignTargetIsLong)
                    cout << "sub v" << loadHigh() << ", v" << accHigh() << endl;
                break;

            default:
                ReportsManager::ReportError(node.op().pos, "Operation unimplemented");
                break;
            }

            cout << "ld v" << accLow() << ", v" << loadLow() << endl;
            if (assignTargetIsLong)
                cout << "ld v" << accHigh() << ", v" << loadHigh() << endl;
            cout << endl;
        }
        else
        {
            // probably it is literal or variable
            // change load type
            // evaluate right

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

    }
    
    void CodeGenVisitor::visitUnaryExprNode(const AST::UnaryExprNode& node)
    {
        node.expr().accept(this);

        if (node.op().type == TokenType::PLUS)
            return;
        else if (node.op().type == TokenType::MINUS)
        {
            cout << "xor v" << accLow() << ", v" << negReg() << endl;
            if (assignTargetIsLong)
            {
                cout << "xor v" << accHigh() << ", v" << negReg() << endl;
            }
            // Carry is set only when adding register and register
            if (assignTargetIsLong)
            {
                cout << "ld v" << negReg() << ", 1" << endl;
                cout << "add v" << accLow() << ", v" << negReg() << endl;
                cout << "add v" << accHigh() << ", v" << flagReg() << endl;
                cout << "ld v" << negReg() << ", 0xFF" << endl;
            }
            else
            {
                cout << "add v" << accLow() << ", 1" << endl;
            }
        }
    }
    
    void CodeGenVisitor::visitProcCallNode(const AST::ProcCallNode& node)
    {
        std::string const& name = node.name().str;
        if (name == "break")
        {
            cout << "break" << endl;
        }
        else if (name == "cls")
        {
            cout << "cls" << endl;
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

            cout << "ld I, [BCD_ZONE_LOW]" << endl;
            cout << "ld B, v" << accLow() << endl;
            cout << "ld I, [BCD_ZONE_HIGH]" << endl;
            cout << "ld B, v" << accHigh() << endl;
        }
        else if (name == "debug_print_bcd")
        {
            if (node.args().size() != 0)
            {
                ReportsManager::ReportError(node.name().pos, ErrorType::WRONG_ARGUMENTS_COUNT);
                return;
            }

            /*
            cout << "ld v" << negReg() << ", 0" << endl;
            cout << "ld I, [BCD_ZONE_HIGH]" << endl;
            cout << "ld v2, [I]" << endl;

            cout << "ld F, v0" << endl;
            cout << "drw v" << negReg() << ", v" << negReg() << ", 5" << endl;

            cout << "add v" << negReg() << ", 9" << endl;
            cout << "ld v0, 0" << endl;

            cout << "ld F, v1" << endl;
            cout << "drw v0, v" << negReg() << ", 5" << endl;

            cout << "add v" << negReg() << ", 9" << endl;

            cout << "ld F, v2" << endl;
            cout << "drw v0, v" << negReg() << ", 5" << endl;
            */

            cout << "ld v" << negReg() << ", 0" << endl;
            cout << "ld I, [BCD_ZONE_LOW]" << endl;
            cout << "ld v2, [I]" << endl;

            cout << "ld F, v0" << endl;
            cout << "drw v" << negReg() << ", v" << negReg() << ", 5" << endl;
            cout << "ld v0, 6" << endl;

            cout << "ld F, v1" << endl;
            cout << "drw v0, v" << negReg() << ", 5" << endl;

            cout << "add v0, 6" << endl;

            cout << "ld F, v2" << endl;
            cout << "drw v0, v" << negReg() << ", 5" << endl;

            cout << "ld v" << negReg() << ", 0xFF" << endl;
        }
    }
    
} // namespace Pascal
