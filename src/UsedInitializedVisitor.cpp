#include <UsedInitializedVisitor.hpp>
#include <AST.hpp>
#include <ReportsManager.hpp>

namespace Pascal
{
    UsedInitializedVisitor::UsedInitializedVisitor()
        : currentScope(std::make_shared<Environment<Attribs>>()),
          globalScope(std::make_shared<Environment<Attribs>>())
    {
        globalScope->defineBuiltins({true, true, static_cast<size_t>(-1)});
    }
    
    UsedInitializedVisitor::~UsedInitializedVisitor()
    {
        
    }

    void UsedInitializedVisitor::check(const std::unique_ptr<AST::Node>& node)
    {
        node->accept(this);
    }
    
    void UsedInitializedVisitor::visitProgramNode(const AST::ProgramNode& node)
    {
        currentScope = globalScope;
       
        for (auto const& decl : node.decls)
            decl->accept(this);

        node.compound->accept(this);
    }
    
    void UsedInitializedVisitor::visitCompoundNode(const AST::CompoundNode& node)
    {
        for (auto const& stmt : node.stmts)
            stmt->accept(this);

        for (auto& e : currentScope->getMap())
        {
            if (!e.second.used)
            {
                ReportsManager::ReportWarning(e.second.pos, WarningType::UNUSED_VAR);
            }
        }
    }
    
    void UsedInitializedVisitor::visitVarDeclNode(const AST::VarDeclNode& node)
    {
        currentScope->define(node.name.str, { false, false, node.name.pos });
    }
    
    void UsedInitializedVisitor::visitTypeNode(const AST::TypeNode& node)
    {
        
    }
    
    void UsedInitializedVisitor::visitProcDeclNode(const AST::ProcDeclNode& node)
    {
        currentScope->define(node.name.str, { false, true, node.name.pos });
        
        auto oldScope = currentScope;
        currentScope = std::make_shared<Environment<Attribs>>(globalScope);

        for (auto const& param : node.params)
            param->accept(this);

        for (auto const& decl : node.decls)
            decl->accept(this);

        node.compound->accept(this);

        currentScope = oldScope;
    }
    
    void UsedInitializedVisitor::visitAssignmentNode(const AST::AssignmentNode& node)
    {
        node.expr->accept(this);
        Attribs& attrs = currentScope->lookupAndAncestors(node.var->token.str);
        attrs.initialized = true;
        attrs.used = true;
    }
    
    void UsedInitializedVisitor::visitVarNode(const AST::VarNode& node)
    {
        Attribs& attrs = currentScope->lookupAndAncestors(node.token.str);
        attrs.used = true;

        if (!attrs.initialized)
        {
            ReportsManager::ReportWarning(node.token.pos, WarningType::UNINTIALIZED_VAR);
        }
    }
    
    void UsedInitializedVisitor::visitIntLiteralNode(const AST::IntLiteralNode& node)
    {
        
    }
    
    void UsedInitializedVisitor::visitProcCallNode(const AST::CallStmtNode& node)
    {
        Attribs& attrs = currentScope->lookupAndAncestors(node.name.str);
        attrs.used = true;
        
        for (auto const& arg : node.args)
            arg->accept(this);
    }

    void UsedInitializedVisitor::visitFunctionDeclNode(const AST::FunctionDeclNode& node)
    {
        currentScope->define(node.name.str, { false, true, node.name.pos });

        auto oldScope = currentScope;
        currentScope = std::make_shared<Environment<Attribs>>(globalScope);

        for (auto const& param : node.params)
            param->accept(this);

        for (auto const& decl : node.decls)
            decl->accept(this);

        node.compound->accept(this);

        currentScope = oldScope;
    }

    void UsedInitializedVisitor::visitIfNode(const AST::IfNode& node)
    {
        node.condition->accept(this);
        node.thenArm->accept(this);

        if (node.elseArm != nullptr) node.elseArm->accept(this);
    }

    void UsedInitializedVisitor::visitFunctionCall(const AST::FunctionCallNode& node)
    {
        Attribs& attrs = currentScope->lookupAndAncestors(node.name.str);
        attrs.used = true;

        for (auto const& arg : node.args)
            arg->accept(this);
    }
    
} // namespace Pascal
