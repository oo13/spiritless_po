/** Metadata parser.
    \file MetadataParser.h
    \author OOTA, Masato
    \copyright Copyright © 2019, 2022 OOTA, Masato
    \par License Boost
    \parblock
      This program is distributed under the Boost Software License Version 1.0.
      You can get the license file at “https://www.boost.org/LICENSE_1_0.txt”.
    \endparblock
*/

#ifndef SRIRITLESS_PO_METADATA_PARSER_H_
#define SRIRITLESS_PO_METADATA_PARSER_H_

#include <string>
#include <unordered_map>

namespace spiritless_po {
    namespace MetadataParser {
        /** The type of the metadata.

            map[key] == value
        */
        typedef std::unordered_map<std::string, std::string> MapT;

        /** Parse a metadata.
            \param [in] metadataString The source text of the metadata.
            \return The map of the metadata.
        */
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
