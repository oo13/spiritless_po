/* ParseError.h

Copyright © 2019 OOTA, Masato

This program is distributed under the Boost Software License Version 1.0.
You can get the license file at “https://www.boost.org/LICENSE_1_0.txt”.

*/

#ifndef SRIRITLESS_PO_PARSE_ERROR_H_
#define SRIRITLESS_PO_PARSE_ERROR_H_

#include <stdexcept>
#include <string>

namespace spiritless_po {
	// Parse Error Exception.
	class ParseError : public std::runtime_error {
	public:
		explicit ParseError(const std::string &whatArg)
			: std::runtime_error(whatArg)
		{}
		explicit ParseError(const char *whatArg)
			: std::runtime_error(whatArg)
		{}
	};
} // namespace spiritless_po

#endif // SRIRITLESS_PO_PLURAL_PARSER_H_
