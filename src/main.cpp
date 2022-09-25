#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <fstream>
#include <sstream>

#include <ReportsManager.hpp>
#include <Scanner.hpp>
#include <Parser.hpp>
#include <AST.hpp>

#include <UndeclRedefinitionVisitor.hpp>
#include <UsedInitializedVisitor.hpp>
//#include <CodeGenVisitor.hpp>
#include <SemanticAnalyzer.hpp>

void job(std::string const& inFileName, std::string const& outFileName, std::shared_ptr<std::string> prg);

int main(int argc, char** argv)
{
	std::vector<std::string> args;
	for (int i = 1; i < argc; i++)
	{
		args.push_back(argv[i]);
	}

	if (args.empty())
	{
		std::cout << TermColor::BrightRed << "error" << TermColor::BrightWhite <<
			": no files specified" << TermColor::Reset << std::endl;
		return 1;
	}

	if (std::find(args.begin(), args.end(), "-h") != args.end()
		|| std::find(args.begin(), args.end(), "--help") != args.end())
	{
		std::cout << "TODO: Usage";
		return 0;
	}

	Pascal::ReportsManager::Init(args);

	std::string inFileName;
	for (auto const& arg : args)
	{
		if (arg.rfind("-", 0) != 0)
		{
			inFileName = arg;
			break;
		}
	}

	std::string outFileName = "";

	for (auto it = args.begin(); it != args.end(); ++it)
	{
		if (*it == "-o")
		{
			++it;
			if (it == args.end())
			{
				std::cout << "error: expected output file name" << std::endl;
				return 2;
			}

			outFileName = *it;
			break;
		}
	}

	if (outFileName == "")
	{
		outFileName = "out.asm";
	}

	std::ifstream fin(inFileName);
	if (!fin.is_open())
	{
		std::cout << TermColor::BrightRed << "error" << TermColor::BrightWhite <<
			": can't open file \"" << args[0] << "\"" << TermColor::Reset << std::endl;
		return 2;
	}

	std::shared_ptr<std::string> prg = std::make_shared<std::string>();

	{
		std::stringstream ss;
		ss << fin.rdbuf();
		*prg = ss.str();
	}

	job(inFileName, outFileName, prg);

	if (Pascal::ReportsManager::GetWarningsCount() > 0)
	{
		if (Pascal::ReportsManager::GetErrorsCount() > 0)
			std::cout << "Generated " << Pascal::ReportsManager::GetWarningsCount() <<
			" warnings and " << Pascal::ReportsManager::GetErrorsCount() << " errors." << std::endl;
		else
			std::cout << "Generated " << Pascal::ReportsManager::GetWarningsCount() << " warnings." << std::endl;
	}
	else if (Pascal::ReportsManager::GetErrorsCount() > 0)
		std::cout << "Generated " << Pascal::ReportsManager::GetErrorsCount() << " errors." << std::endl;

	return 0;
}

void job(std::string const& inFileName, std::string const& outFileName, std::shared_ptr<std::string> prg)
{
	std::unique_ptr<Pascal::AST::ProgramNode> tree;

	Pascal::ReportsManager::SetCurrentFile({ inFileName, prg });

	try
	{
		{
			Pascal::Scanner scanner(prg);
			Pascal::TokenList tokens = scanner.scanTokens();
			Pascal::Parser parser(tokens);
			tree = parser.parseProgram();
		}

		if (Pascal::ReportsManager::GetErrorsCount() != 0)
			return;

		Pascal::UndeclRedefinitionVisitor undeclPass;
		tree->accept(&undeclPass);

		if (Pascal::ReportsManager::GetErrorsCount() != 0)
			return;

		Pascal::SemanticAnalyzer semanticAnalyzer;
		tree->accept(&semanticAnalyzer);

		if (Pascal::ReportsManager::GetErrorsCount() != 0)
			return;
	
		Pascal::UsedInitializedVisitor usedPass;
		tree->accept(&usedPass);

		if (Pascal::ReportsManager::GetErrorsCount() == 0)
		{
			//Pascal::CodeGenVisitor code(outFileName);
			//tree->accept(&code);
		}
	}
	catch (Pascal::StopExecution const& e)
	{
		return;
	}
}