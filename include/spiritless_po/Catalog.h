/* Catalog.h

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

#ifndef SRIRITLESS_PO_CATALOG_H_
#define SRIRITLESS_PO_CATALOG_H_

#include "Common.h"
#include "MetadataParser.h"
#include "PluralParser.h"
#include "PoParser.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iosfwd>
#include <iterator>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace spiritless_po {
	// Catalog class
	class Catalog {
	public:
		Catalog();
		explicit Catalog(std::istream &is);
		
		Catalog(const Catalog &a) = default;
		Catalog(Catalog &&a) = default;
		Catalog &operator=(const Catalog &a) = default;
		Catalog &operator=(Catalog &&a) = default;
		
		void Clear();
		
		// Add another text range contents.
		// This function will not change any existed values including metadata.
		// An entry will not be added if its msgstr[0] is empty.
		// It returns true if no error is existed.
		template<class INP>
		bool Add(INP &it, const INP &end);
		
		// Add another istream contents.
		// This function will not change any existed values including metadata.
		// An entry will not be added if its msgstr[0] is empty.
		// It returns true when no error is existed.
		bool Add(std::istream &is);
		
		// Add another Catalog contents.
		// This function will not change any existed values including metadata.
		// It returns true when no error is existed.
		void Merge(const Catalog &a);
		
		// Clear error.
		void ClearError();
		
		// Get error information.
		const std::vector<std::string> &GetError() const;
		
		// Get the translated text.
		const std::string gettext(const std::string &msgid) const;
		const std::string ngettext(const std::string &msgid, const std::string &msgidPlural,
			unsigned long int n) const;
		const std::string pgettext(const std::string &msgctxt, const std::string &msgid) const;
		const std::string npgettext(const std::string &msgctxt, const std::string &msgid,
			const std::string &msgidPlural, unsigned long int n) const;
		
		
		// Type of index.
		struct IndexDataT {
			std::size_t stringTableIndex;
			std::size_t totalPlurals;
		};
		
	private:
		MetadataParser::MapT metadata;
		std::unordered_map<std::string, IndexDataT> index;
		std::vector<std::string> stringTable;
		PluralParser::FunctionType pluralFunction;
		std::size_t maxPlurals;
		std::vector<std::string> errors;
		
	public:
		// Maintenance functions
		// Metadata Map.
		const MetadataParser::MapT &GetMetadata() const;
		// Index Map.
		const std::unordered_map<std::string, IndexDataT> &GetIndex() const;
		// String Table.
		const std::vector<std::string> &GetStringTable() const;
	};
	
	
	
	inline Catalog::Catalog()
		: metadata(), index(), stringTable(), pluralFunction([](int n){ return 0; }),
			maxPlurals(0), errors()
	{
	}
	
	inline Catalog::Catalog(std::istream &is)
		: Catalog()
	{
		Add(is);
	}
	
	inline void Catalog::Clear()
	{
		*this = Catalog();
	}
	
	template<class INP>
	bool Catalog::Add(INP &begin, const INP &end)
	{
		std::vector<CatalogEntryT> newEntries(PoParser::GetEntries(begin, end));
		for (const auto &it : newEntries)
		{
			if(!it.error.empty())
				errors.push_back(std::move(it.error));
			else if(!it.msgstr[0].empty())
			{
				if(!it.msgid.empty())
				{
					IndexDataT idx;
					idx.stringTableIndex = stringTable.size();
					idx.totalPlurals = it.msgstr.size();
					stringTable.insert(stringTable.end(), it.msgstr.begin(), it.msgstr.end());
					index.emplace(it.msgid, idx);
				}
				else if(metadata.empty())
				{
					metadata = MetadataParser::Parse(it.msgstr[0]);
					const auto plural = metadata.find("Plural-Forms");
					if(plural != metadata.end())
					{
						const auto pluralText = plural->second;
						try
						{
							const auto pluralData = PluralParser::Parse(pluralText);
							if(pluralData.first > 0)
								maxPlurals = pluralData.first - 1;
							pluralFunction = pluralData.second;
						}
						catch (PluralParser::ExpressionError &e)
						{
							const size_t col = std::distance(pluralText.cbegin(), e.Where());
							errors.emplace_back("Column#" + std::to_string(col + 1)
								+ " in plural expressoin: " + e.what());
						}
					}
				}
			}
		}
		return errors.empty();
	}
	
	inline bool Catalog::Add(std::istream &is)
	{
		std::istreambuf_iterator<char> begin(is);
		std::istreambuf_iterator<char> end;
		return Add(begin, end);
	}
	
	inline void Catalog::Merge(const Catalog &a)
	{
		const auto &it = index.find("");
		if(it == index.end())
		{
			// This had no metadata.
			pluralFunction = a.pluralFunction;
			maxPlurals = a.maxPlurals;
		}
		const std::size_t origSize = stringTable.size();
		stringTable.insert(stringTable.end(), a.stringTable.begin(), a.stringTable.end());
		for(const auto &idx : a.index)
		{
			auto modifyedIdx = idx.second;
			modifyedIdx.stringTableIndex += origSize;
			index.emplace(idx.first, modifyedIdx);
		}
	}
	
	inline void Catalog::ClearError()
	{
		errors.clear();
	}
	
	inline const std::vector<std::string> &Catalog::GetError() const
	{
		return errors;
	}
	
	inline const std::string Catalog::gettext(const std::string &msgid) const
	{
		const auto &it = index.find(msgid);
		if(it != index.end())
			return stringTable[it->second.stringTableIndex];
		else
			return msgid;
	}
	
	inline const std::string Catalog::ngettext(const std::string &msgid, const std::string &msgidPlural,
		unsigned long int n) const
	{
		const auto &it = index.find(msgid);
		if(it != index.end())
		{
			std::size_t nIdx = pluralFunction(n);
			if(nIdx >= it->second.totalPlurals)
				nIdx = 0;
			return stringTable[it->second.stringTableIndex + nIdx];
		}
		else
		{
			if(n == 1)
				return msgid;
			else
				return msgidPlural;
		}
	}
	
	inline const std::string Catalog::pgettext(const std::string &msgctxt, const std::string &msgid) const
	{
		std::string s(msgctxt);
		s += CONTEXT_SEPARATOR;
		s += msgid;
		const auto &it = index.find(s);
		if(it != index.end())
			return stringTable[it->second.stringTableIndex];
		else
			return msgid;
	}
	
	inline const std::string Catalog::npgettext(const std::string &msgctxt, const std::string &msgid,
		const std::string &msgidPlural, unsigned long int n) const
	{
		std::string s(msgctxt);
		s += CONTEXT_SEPARATOR;
		s += msgid;
		const auto &it = index.find(s);
		if(it != index.end())
		{
			std::size_t nIdx = pluralFunction(n);
			if(nIdx >= it->second.totalPlurals)
				nIdx = 0;
			return stringTable[it->second.stringTableIndex + nIdx];
		}
		else
		{
			if(n == 1)
				return msgid;
			else
				return msgidPlural;
		}
	}
	
	inline const MetadataParser::MapT &Catalog::GetMetadata() const
	{
		return metadata;
	}
	
	inline const std::unordered_map<std::string, Catalog::IndexDataT> &Catalog::GetIndex() const
	{
		return index;
	}
	
	inline const std::vector<std::string> &Catalog::GetStringTable() const
	{
		return stringTable;
	}
} // namespace spiritless_po

#endif // SRIRITLESS_PO_CATALOG_H_
