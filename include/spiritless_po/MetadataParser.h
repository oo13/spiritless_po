/* MetadataParser.h

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
			for(char c : metadataString)
			{
				if(stat == KEY)
				{
					if(c == ':')
						stat = SPACE;
					else
						key += c;
				}
				else if ((stat == SPACE && c != ' ') || stat == VALUE)
				{
					stat = VALUE;
					if(c == '\n')
					{
						stat = KEY;
						map.emplace(key, value);
						key.clear();
						value.clear();
					}
					else
						value += c;
				}
			}
			if(! key.empty())
				map.emplace(key, value);
			return map;
		}
	} // namespace MetadataParser
} // namespace spiritless_po

#endif // SRIRITLESS_PO_METADATA_PARSER_H_
