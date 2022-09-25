#include <UndeclRedefinitionVisitor.hpp>
#include <AST.hpp>
#include <ReportsManager.hpp>

namespace Pascal
{
	UndeclRedefinitionVisitor::UndeclRedefinitionVisitor()
		: currentScope(std::make_shared<Environment<int>>()),
		  globalScope(std::make_shared<Environment<int>>())
	{
		globalScope->defineBuiltins(-1);
	}
	
	UndeclRedefinitionVisitor::~UndeclRedefinitionVisitor()
	{
		
	}

	void UndeclRedefinitionVisitor::check(const std::unique_ptr<AST::Node>& node)
	{
		node->accept(this);
	}

	void UndeclRedefinitionVisitor::visitProgramNode(const AST::ProgramNode& node)
	{
		currentScope = globalScope;
		
		for (auto const& decl : node.decls)
			decl->accept(this);

		node.compound->accept(this);
	}
	
	void UndeclRedefinitionVisitor::visitCompoundNode(const AST::CompoundNode& node)
	{
		for (auto const& stmt : node.stmts)
			stmt->accept(this);
	}
	
	void UndeclRedefinitionVisitor::visitVarDeclNode(const AST::VarDeclNode& node)
	{
		if (currentScope->has(node.name.str))
		{
			int previousPos = currentScope->lookup(node.name.str);
			ReportsManager::ReportError(node.name.pos, ErrorType::NAME_REDEFINITION);
			if (previousPos != -1) ReportsManager::ReportNote(previousPos, "previous declared here");
		}
		else
		{
			currentScope->define(node.name.str, node.name.pos);
		}

		node.type->accept(this);
	}
	
	void UndeclRedefinitionVisitor::visitTypeNode(const AST::TypeNode& node)
	{
		if (!currentScope->hasAndAncestors(node.token.str))
		{
			ReportsManager::ReportError(node.token.pos, ErrorType::NAME_UNDEFINED);
		}
	}
	
	void UndeclRedefinitionVisitor::visitProcDeclNode(const AST::ProcDeclNode& node)
	{
		if (currentScope->has(node.name.str))
		{
			int previousPos = currentScope->lookup(node.name.str);
			ReportsManager::ReportError(node.name.pos, ErrorType::NAME_REDEFINITION);
			ReportsManager::ReportNote(previousPos, "previous declared here");
		}
		else
		{
			currentScope->define(node.name.str, node.name.pos);
		}

		auto oldScope = currentScope;
		currentScope = std::make_shared<Environment<int>>(currentScope);

		for (auto const& e : node.params)
			e->accept(this);

		for (auto const& e : node.decls)
			e->accept(this);

		node.compound->accept(this);

		currentScope = oldScope;
	}
	
	void UndeclRedefinitionVisitor::visitAssignmentNode(const AST::AssignmentNode& node)
	{
		node.var->accept(this);
		node.expr->accept(this);
	}
	
	void UndeclRedefinitionVisitor::visitVarNode(const AST::VarNode& node)
	{
		if (!currentScope->hasAndAncestors(node.token.str))
		{
			ReportsManager::ReportError(node.token.pos, ErrorType::NAME_UNDEFINED);
		}
	}
	
	void UndeclRedefinitionVisitor::visitIntLiteralNode(const AST::IntLiteralNode& node)
	{
		
	}
		
	void UndeclRedefinitionVisitor::visitUnaryExprNode(const AST::UnaryExprNode& node)
	{
		node.expr->accept(this);
	}
	
	void UndeclRedefinitionVisitor::visitProcCallNode(const AST::CallStmtNode& node)
	{
		if (!currentScope->hasAndAncestors(node.name.str))
		{
			ReportsManager::ReportError(node.name.pos, ErrorType::NAME_UNDEFINED);
		}

		for (auto const& e : node.args)
			e->accept(this);
	}
	
	void UndeclRedefinitionVisitor::visitBinaryExprNode(const AST::BinaryExprNode& node)
	{
		node.left->accept(this);
		node.right->accept(this);
	}

	void UndeclRedefinitionVisitor::visitFunctionDeclNode(const AST::FunctionDeclNode& node)
	{
		if (currentScope->has(node.name.str))
		{
			int previousPos = currentScope->lookup(node.name.str);
			ReportsManager::ReportError(node.name.pos, ErrorType::NAME_REDEFINITION);
			ReportsManager::ReportNote(previousPos, "previous declared here");
		}
		else
		{
			currentScope->define(node.name.str, node.name.pos);
		}

		auto oldScope = currentScope;
		currentScope = std::make_shared<Environment<int>>(currentScope);

		for (auto const& e : node.params)
			e->accept(this);

		for (auto const& e : node.decls)
			e->accept(this);

		node.compound->accept(this);

		currentScope = oldScope;
	}

	void UndeclRedefinitionVisitor::visitIfNode(const AST::IfNode& node)
	{
		node.condition->accept(this);
		node.thenArm->accept(this);

		if (node.elseArm != nullptr) node.elseArm->accept(this);
	}

	void UndeclRedefinitionVisitor::visitFunctionCall(const AST::FunctionCallNode& node)
	{
		if (!currentScope->hasAndAncestors(node.name.str))
		{
			ReportsManager::ReportError(node.name.pos, ErrorType::NAME_UNDEFINED);
		}

		for (auto const& e : node.args)
			e->accept(this);
	}

} // namespace Pascal
