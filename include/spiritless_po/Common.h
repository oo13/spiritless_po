/* Common.h

Copyright © 2019 OOTA, Masato

This program is distributed under the Boost Software License Version 1.0.
You can get the license file at “https://www.boost.org/LICENSE_1_0.txt”.
*/

#ifndef SRIRITLESS_PO_COMMON_H_
#define SRIRITLESS_PO_COMMON_H_

#include <string>
#include <vector>

namespace spiritless_po {
	// Context Separator. (Compatible with GNU Gettext)
	const char CONTEXT_SEPARATOR = '\x04';
	
	// Type of Result data.
	// msgid and msgstr is undefined when error is not empty.
	// msgstr.size() > 0 when error is empty.
	// msgstr[0] is an empty string if the entry is fuzzy.
	struct CatalogEntryT {
		std::string msgid;
		std::vector<std::string> msgstr;
		std::string error;
	};
} // namespace spiritless_po

#endif // SRIRITLESS_PO_COMMON_H_
