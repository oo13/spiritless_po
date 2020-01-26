/* Common.h

Copyright (c) 2019 OOTA, Masato

spiritless_po is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

spiritless_po is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
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
