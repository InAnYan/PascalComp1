#include <pscpch.hpp>
#include <ReportsManager.hpp>

#include <iostream>
#include <algorithm>

namespace Pascal
{
	std::vector<ReportFile> ReportsManager::includeStack;

    ReportFile ReportsManager::currentFile;
	
	std::vector<ErrorType> ReportsManager::disallowedErrors;
	std::vector<WarningType> ReportsManager::disallowedWarnings;

	unsigned ReportsManager::errorsCount = 0;
	unsigned ReportsManager::warningsCount = 0;
	
	bool ReportsManager::treatWarningsAsError;

	std::string tabTransform(std::string const& work)
	{
		std::stringstream ss;
		for (char ch : work)
		{
			if (ch == '\t')
			{
				ss << "    ";
			}
			else
			{
				ss << ch;
			}
		}
		return ss.str();
	}
	
    void ReportsManager::Init(const std::vector<std::string> &args)
	{
		if (find(args.begin(), args.end(), "-Wall") != args.end())
		{
			disallowedWarnings.clear();
		}

		if (find(args.begin(), args.end(), "-Eall") != args.end())
		{
		    disallowedErrors.clear();
		}

		if (find(args.begin(), args.end(), "-Werror") != args.end())
		{
		    treatWarningsAsError = true;
		}
		else
		{
			treatWarningsAsError = false;
		}
	}

	void ReportsManager::SetCurrentFile(ReportFile const& file)
	{
		currentFile = file;
	}

	void ReportsManager::PushInclude(ReportFile const& file)
	{
		includeStack.push_back(file);
	}

    ReportFile ReportsManager::PopInclude()
	{
		ReportFile temp = includeStack.front();
		includeStack.pop_back();
		return temp;
	}

	void ReportsManager::PrintReport(size_t where, ReportType type, std::string const& msg)
	{
		if (!includeStack.empty())
		{
			for (auto const& e : includeStack)
			{
				std::cout << "In file included from \"" << e.fileName << "\":" << std::endl;
			}
		}
		
		ErrorPos pos = getErrorPos(where);
		
		std::cout << TermColor::BrightWhite << currentFile.fileName << ":" <<
			pos.lineNumber << ":" << pos.column << " ";
		
		switch (type)
		{
		case ReportType::ERROR:
			std::cout << TermColor::BrightRed << "error";
			break;
		case ReportType::WARNING:
			std::cout << TermColor::BrightMagenta << "warning";
			break;
		case ReportType::NOTE:
			std::cout << "note";
			break;
		}
		
		std::cout  << TermColor::BrightWhite << ": " << msg << std::endl;

		std::string prefix = " " + std::to_string(pos.lineNumber) + " | ";
		
		std::cout << prefix << TermColor::Reset <<
			tabTransform(currentFile.source->substr(pos.startPos, pos.endPos - pos.startPos + 1)) << std::endl;
		std::cout << std::string(pos.column + prefix.size(), ' ') << TermColor::BrightGreen << "^";

		for (size_t i = pos.where + 1; isalnum((*currentFile.source)[i]) ||
				 (*currentFile.source)[i] == '.' || (*currentFile.source)[i] == '_'; i++)
		{
			std::cout << "~";
		}
		std::cout << TermColor::Reset << std::endl;
	}
	
	void ReportsManager::ReportError(size_t where, const std::string &msg, bool noStop)
	{
		errorsCount++;

		PrintReport(where, ReportType::ERROR, msg);
		
		if (!noStop)
			throw StopExecution();
	}

	void ReportsManager::ReportError(size_t where, ErrorType type, bool noStop)
	{
		if (find(disallowedErrors.begin(), disallowedErrors.end(), type) != disallowedErrors.end())
		{
			return;
		}

		ReportError(where, typeToString(type), noStop);
	}

	void ReportsManager::ReportError(size_t where, ErrorType type,
									 std::string const& additionalMsg, bool noStop)
	{
		if (find(disallowedErrors.begin(), disallowedErrors.end(), type) != disallowedErrors.end())
		{
			return;
		}

		ReportError(where, typeToString(type) + additionalMsg, noStop);
	}
	
	void ReportsManager::ReportWarning(size_t where, const std::string &msg, bool noStop)
	{
		warningsCount++;

		PrintReport(where, ((treatWarningsAsError) ? (ReportType::ERROR) : (ReportType::WARNING)), msg);
		
		if (!noStop)
			throw StopExecution();
	}

	void ReportsManager::ReportWarning(size_t where, WarningType type)
	{
		if (find(disallowedWarnings.begin(), disallowedWarnings.end(), type) != disallowedWarnings.end())
		{
			return;
		}

		ReportWarning(where, typeToString(type));
	}

	void ReportsManager::ReportWarning(size_t where, WarningType type, std::string const& additionalMsg)
	{
		if (find(disallowedWarnings.begin(), disallowedWarnings.end(), type) != disallowedWarnings.end())
		{
			return;
		}

		ReportWarning(where, typeToString(type) + additionalMsg);
	}

	ReportsManager::ErrorPos ReportsManager::getErrorPos(size_t where)
	{
		ErrorPos res;
		res.where = where;
	
		for (res.endPos = where; res.endPos < currentFile.source->size(); res.endPos++)
		{
			if ((*currentFile.source)[res.endPos] == '\n')
			{
				res.endPos--;
				break;
			}
		}

		for (res.startPos = where; res.startPos != 0; res.startPos--)
		{
			if ((*currentFile.source)[res.startPos] == '\n')
			{
				res.startPos++;
				break;
			}
		}

		res.lineNumber = 0;
		for (size_t i = res.endPos; i != static_cast<size_t>(-1); i--)
		{
			if ((*currentFile.source)[i] == '\n')
				res.lineNumber++;
		}
		
	    size_t tabAdjust = 0;
		for (size_t i = res.startPos; i <= res.where; i++)
		{
			if ((*currentFile.source)[i] == '\t')
			{
				tabAdjust += 3;
			}
		}
		res.column = res.where - res.startPos + tabAdjust;

		//printf("\n[%zu:%zu - %zu] \'%c\'\n", res.startPos, res.endPos, res.where, (*currentFile)[res.where]);

		res.lineNumber++;
		return res;
	}

	std::string ReportsManager::typeToString(ErrorType type)
	{
		switch (type)
		{
		case ErrorType::ILLEGAL_LETTER:
			return "unknown or illegal symbol";
		case ErrorType::UNEXPECTED_WORD:
			return "unexpected word";
		case ErrorType::EXPECTED:
			return "expected ";
		case ErrorType::NAME_UNDEFINED:
			return "undefined identifier";
		case ErrorType::NAME_REDEFINITION:
			return "name redefinition";
		case ErrorType::ILLEGAL_ASSIGNMENT:
			return "attempt to assign a value to non-variable object";
		case ErrorType::ILLEGAL_STATEMENT:
			return "can't recognize statement";
		case ErrorType::CALLING_NON_PROCEDURE:
			return "attempt to call non-callable object";
		case ErrorType::CALLING_NON_FUNCTION:
			return "attempt to call non-function object";
		case ErrorType::PROCEDURE_AS_FUNCTION:
			return "required function, not procedure";
		case ErrorType::WRONG_ARGUMENTS_COUNT:
			return "wrong arguments count";
		case ErrorType::CANT_PARSE_LITERAL:
			return "can't parse literal";
		case ErrorType::DIVISION_BY_ZERO:
			return "division by zero";
		case ErrorType::NONE:
			return "NONE ERROR";
		}
	}

	std::string ReportsManager::typeToString(WarningType type)
	{
		switch (type)
		{
		case WarningType::UNINTIALIZED_VAR:
			return "using unintialized variable";
		case WarningType::UNUSED_VAR:
			return "unused variable";
		case WarningType::NONE:
			return "NONE WARNING";
		}
	}

	unsigned ReportsManager::GetErrorsCount()
	{
		return errorsCount;
	}

	unsigned ReportsManager::GetWarningsCount()
	{
		return warningsCount;
	}

	void ReportsManager::ReportNote(size_t where, const std::string &msg)
	{
		PrintReport(where, ReportType::NOTE, msg);
	}
}

namespace TermColor
{
	const std::string BrightRed     = "\x1b[91m";
	const std::string BrightGreen   = "\x1b[92m";
	const std::string BrightMagenta = "\x1b[95m";
	const std::string BrightWhite   = "\x1b[97m";
	const std::string Reset         = "\x1b[0m";
}
