#include <fstream>
#include <sstream>

#include <display/ds9/ds9parser.h>

namespace casa {
	namespace viewer {

		ds9parser::ds9parser( ) : trace_scanning(false), trace_parsing(false) { }

		bool ds9parser::parse_stream(ds9context &base, std::istream& in, const std::string& sname) {
			streamname = sname;

			ds9lex scanner(&in);
			scanner.set_debug(trace_scanning);
			lexer = &scanner;

			ds9parse parser( *this, &base );
			parser.set_debug_level(trace_parsing);
			return (parser.parse() == 0);
		}

		bool ds9parser::parse_file( ds9context &base, const std::string &filename) {
			std::ifstream in(filename.c_str());
			if (!in.good()) return false;
			return parse_stream(base, in, filename);
		}

		bool ds9parser::parse_string( ds9context &base, const std::string &input, const std::string& sname) {
			std::istringstream iss(input);
			return parse_stream(base, iss, sname);
		}

		void ds9parser::error(const class location& l, const std::string& m) {
			std::cerr << l << ": " << m << std::endl;
		}

		void ds9parser::error(const std::string& m) {
			std::cerr << m << std::endl;
		}
	}
}

