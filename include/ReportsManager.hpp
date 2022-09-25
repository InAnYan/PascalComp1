#ifndef PASCAL_INTERNAL_HPP
#define PASCAL_INTERNAL_HPP

#include <memory>
#include <string>
#include <vector>

namespace Pascal
{
	enum class ErrorType
	{
		NONE = 0,
		ILLEGAL_LETTER,
		UNEXPECTED_WORD,
		EXPECTED,
		NAME_UNDEFINED,
		NAME_REDEFINITION,
		ILLEGAL_ASSIGNMENT,
		ILLEGAL_STATEMENT,
		CALLING_NON_PROCEDURE,
		CALLING_NON_FUNCTION,
		WRONG_ARGUMENTS_COUNT,
	    PROCEDURE_AS_FUNCTION,
		CANT_PARSE_LITERAL,
		DIVISION_BY_ZERO,
		UNTERMINATED_STRING,
		ILLEGAL_CHAR
	};

	enum class WarningType
	{
		NONE = 0,
	    UNINTIALIZED_VAR,
		UNUSED_VAR,
		UNKNOWN_ESCAPE_CHAR
	};

	typedef struct
	{
		std::string fileName;
		std::shared_ptr<const std::string> source;
	} ReportFile;

	class ReportsManager
	{
	public:
		static void Init(std::vector<std::string> const& args);
	
		static void SetCurrentFile(ReportFile const& file);

		static void PushInclude(ReportFile const& file);
		static ReportFile PopInclude();
	
		static void ReportError(size_t where, ErrorType type, bool noStop = true);
		static void ReportError(size_t where, ErrorType type,
								std::string const& additionalMsg, bool noStop = true);
		static void ReportError(size_t where, std::string const& msg, bool noStop = true);

		static void ReportWarning(size_t where, WarningType type);
		static void ReportWarning(size_t where, WarningType type, std::string const& additionalMsg);
		static void ReportWarning(size_t where, std::string const& msg, bool noStop = true);

		static void ReportNote(size_t where, std::string const& msg);

		static unsigned GetErrorsCount();
		static unsigned GetWarningsCount();
		
	private:
		static std::vector<ReportFile> includeStack;

		static ReportFile currentFile;
		
		static std::vector<ErrorType> disallowedErrors;
		static std::vector<WarningType> disallowedWarnings;

		static unsigned warningsCount;
		static unsigned errorsCount;

		static bool treatWarningsAsError;

		typedef struct
		{
			size_t where, startPos, endPos, column, lineNumber;
		} ErrorPos;

		static ErrorPos getErrorPos(size_t where);
		
		static std::string typeToString(ErrorType type);
		static std::string typeToString(WarningType type);

		enum class ReportType
		{
			WARNING,
			ERROR,
			NOTE
		};

		static void PrintReport(size_t where, ReportType type, std::string const& msg);
	};
	
	class StopExecution : std::exception
	{
	public:
		const char* what() const throw() { return "DON'T CALL Pascal::StopExecution::what()"; }
	};	
}

// Only foregrounds
// ANSI Escape sequences
namespace TermColor
{
	extern const std::string BrightRed;
	extern const std::string BrightGreen;
	extern const std::string BrightMagenta;
	extern const std::string BrightWhite;
	extern const std::string Reset;
}

#endif
