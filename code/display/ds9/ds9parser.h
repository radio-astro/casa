#ifndef DISPLAY_DS9_PARSER_H_
#define DISPLAY_DS9_PARSER_H_

#include <string>
#include <vector>
#include <display/ds9/ds9lex.h>

namespace casa {
	namespace viewer {

		class ds9parser {
		public:
			ds9parser( );

			/// enable debug output in the flex scanner
			bool trace_scanning;

			/// enable debug output in the bison parser
			bool trace_parsing;

			/// stream name (file or input stream) used for error messages.
			std::string streamname;

			// input stream, stream name for error messages, return true if successfully parsed
			bool parse_stream(ds9context &base, std::istream& in, const std::string& sname = "stream input");

			bool parse_string(ds9context &base, const std::string& input, const std::string& sname = "string stream");

			bool parse_file(ds9context &base, const std::string& filename);

			// To demonstrate pure handling of parse errors, instead of
			// simply dumping them on the standard error output, we will pass
			// them to the driver using the following two member functions.

			/** Error handling with associated line number. This can be modified to
			  * output the error e.g. to a dialog box. */
			void error(const class location& l, const std::string& m);

			/** General error handling. This can be modified to output the error
			  * e.g. to a dialog box. */
			void error(const std::string& m);
			ds9lex* lexer;

		};

	}
}

#endif
