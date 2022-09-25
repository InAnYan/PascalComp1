#include <SemanticAnalyzer.hpp>

#include <SemanticAnalyzer.hpp>
#include <AST.hpp>
#include <ReportsManager.hpp>
#include <map>

namespace Pascal
{
    SemanticAnalyzer::SemanticAnalyzer()
        : currentScope(std::make_shared<Environment<SymAttribs>>()),
          globalScope(std::make_shared<Environment<SymAttribs>>())
    {
        globalScope->defineBuiltins({}); // TODO: ??
    }

    SemanticAnalyzer::~SemanticAnalyzer()
    {

    }

    void SemanticAnalyzer::visitProgramNode(const AST::ProgramNode& node)
    {
        currentScope = globalScope;

        for (auto const& decl : node.decls)
            decl->accept(this);

        node.compound->accept(this);
    }

    void SemanticAnalyzer::visitCompoundNode(const AST::CompoundNode& node)
    {
        for (auto const& stmt : node.stmts)
            stmt->accept(this);
    }

    void SemanticAnalyzer::visitVarDeclNode(const AST::VarDeclNode& node)
    {
        SymAttribs attrs;
        node.type->accept(this);
        attrs.type = lastType;
        attrs.asVar.isGlobal = (currentScope == globalScope);
        attrs.asVar.isConst = node.isConst;

        currentScope->define(node.name.str, attrs);

        lastSym = attrs;
    }

    std::map<std::string, SymType> PascalTypes =
    {
        {"integer", SymType::INTEGER},
        {"long", SymType::LONG}
    };

    void SemanticAnalyzer::visitTypeNode(const AST::TypeNode& node)
    {
        lastType = PascalTypes[node.token.str];
    }

    void SemanticAnalyzer::visitProcDeclNode(const AST::ProcDeclNode& node)
    {
        SymAttribs attrs;
        attrs.type = SymType::PROCEDURE;
        attrs.asProc.arity = node.params.size();
        attrs.asProc.paramTypes = new SymType[node.params.size()];
        for (unsigned i = 0; i < node.params.size(); i++)
        {
            node.params[i]->type->accept(this);
            attrs.asProc.paramTypes[i] = lastType;
        }

        currentScope->define(node.name.str, attrs);

        auto oldScope = currentScope;
        currentScope = std::make_shared<Environment<SymAttribs>>(globalScope);

        for (auto const& decl : node.decls)
            decl->accept(this);

        node.compound->accept(this);

        currentScope = oldScope;
    }

    void SemanticAnalyzer::visitAssignmentNode(const AST::AssignmentNode& node)
    {
        node.var->accept(this);
        if (lastSym.asVar.isConst)
        {
            ReportsManager::ReportError(node.var->token.pos, "attempt to assign constant variable");
        }

        node.expr->accept(this);
    }

    void SemanticAnalyzer::visitVarNode(const AST::VarNode& node)
    {
        lastSym = currentScope->lookupAndAncestors(node.token.str);
    }

    void SemanticAnalyzer::visitIntLiteralNode(const AST::IntLiteralNode& node)
    {
        uint32_t literal;

    }

    void SemanticAnalyzer::visitBinaryExprNode(const AST::BinaryExprNode& node)
    {

    }

    void SemanticAnalyzer::visitUnaryExprNode(const AST::UnaryExprNode& node)
    {

    }

    void SemanticAnalyzer::visitProcCallNode(const AST::CallStmtNode& node)
    {

    }

} // namespace Pascal
