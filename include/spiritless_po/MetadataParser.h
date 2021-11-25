/* MetadataParser.h

Copyright © 2019 OOTA, Masato

This program is distributed under the Boost Software License Version 1.0.
You can get the license file at “https://www.boost.org/LICENSE_1_0.txt”.
*/

#ifndef SRIRITLESS_PO_METADATA_PARSER_H_
#define SRIRITLESS_PO_METADATA_PARSER_H_

#include <string>
#include <unordered_map>

namespace spiritless_po {
    namespace MetadataParser {
        typedef std::unordered_map<std::string, std::string> MapT;

        inline MapT Parse(const std::string &metadataString)
        {
            MapT map;
            enum { KEY, SPACE, VALUE } stat = KEY;
            std::string key;
            std::string value;
            for (char c : metadataString) {
                if (stat == KEY) {
                    if (c == ':') {
                        stat = SPACE;
                    } else {
                        key += c;
                    }
                } else if ((stat == SPACE && c != ' ') || stat == VALUE) {
                    stat = VALUE;
                    if (c == '\n') {
                        stat = KEY;
                        map.emplace(key, value);
                        key.clear();
                        value.clear();
                    } else {
                        value += c;
                    }
                }
            }
            if (!key.empty()) {
                map.emplace(key, value);
            }
            return map;
        }
    } // namespace MetadataParser
} // namespace spiritless_po

#endif // SRIRITLESS_PO_METADATA_PARSER_H_
