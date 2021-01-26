/* PoParer.h

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

#ifndef SRIRITLESS_PO_PO_PARSER_H_
#define SRIRITLESS_PO_PO_PARSER_H_

#include "Common.h"

#include <cassert>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace spiritless_po {
	namespace PoParser {
		// Type of a line.
		enum class LineT {
			START,
			EMPTY,
			COMMENT,
			FLAG_COMMENT,
			MSGCTXT,
			MSGID,
			MSGID_PLURAL,
			MSGSTR,
			MSGSTR_PLURAL,
			TEXT,
			END,
			UNKNOWN
		};
		
		// Type of a flag.
		enum FlagT {
			NONE = 0,
			FUZZY = 1<<0
		};
		
		inline FlagT operator|=(FlagT &a, FlagT b)
		{
			return a = static_cast<FlagT>(a | b);
		}
		
		
		
		// Reading position type.
		template<class INP>
		class PositionT {
		public:
			PositionT(const INP &it, const INP &end, std::size_t line = 1, std::size_t column = 1);
			
			bool IsEnd() const;
			bool IsNotEnd() const;
			char Get() const;
			void Next();
			std::size_t GetLine() const;
			std::size_t GetColumn() const;
			
		private:
			INP curIt;
			INP endIt;
			std::size_t lineNumber;
			std::size_t columnNumber;
		};
		
		
		// Parse Error in PO file.
		template<class INP>
		class PoParseError : public std::runtime_error {
		public:
			explicit PoParseError(const std::string &whatArg, const PositionT<INP> &it);
			explicit PoParseError(const char *whatArg, const PositionT<INP> &it);
			
			// Get the error location.
			const PositionT<INP> &GetLocation() const noexcept;
			
		private:
			PositionT<INP> loc;
		};
		
		
		
		template<class INP>
		PositionT<INP>::PositionT(const INP &it, const INP &end, std::size_t line, std::size_t column)
			: curIt(it), endIt(end), lineNumber(line), columnNumber(column)
		{
		}
		
		template<class INP>
		bool PositionT<INP>::IsEnd() const
		{
			return curIt == endIt;
		}
		
		template<class INP>
		bool PositionT<INP>::IsNotEnd() const
		{
			return curIt != endIt;
		}
		
		template<class INP>
		char PositionT<INP>::Get() const
		{
			return IsEnd() ? '\0' : *curIt;
		}
		
		template<class INP>
		void PositionT<INP>::Next()
		{
			if(IsNotEnd())
			{
				if(Get() == '\n')
				{
					++lineNumber;
					columnNumber = 0;
				}
				++curIt;
				++columnNumber;
			}
		}
		
		template<class INP>
		std::size_t PositionT<INP>::GetLine() const
		{
			return lineNumber;
		}
		
		template<class INP>
		std::size_t PositionT<INP>::GetColumn() const
		{
			return columnNumber;
		}
		
		
		template<class INP>
		PoParseError<INP>::PoParseError(const std::string &whatArg, const PositionT<INP> &it)
			: std::runtime_error(whatArg), loc(it)
		{
		}
		
		template<class INP>
		PoParseError<INP>::PoParseError(const char *whatArg, const PositionT<INP> &it)
			: std::runtime_error(whatArg), loc(it)
		{
		}
		
		// Get the error location.
		template<class INP>
		const PositionT<INP> &PoParseError<INP>::GetLocation() const noexcept
		{
			return loc;
		}
		
		
		
		// Skip spaces except NL. (Utility function)
		template<class INP>
		void SkipSpacesExceptNL(PositionT<INP> &it)
		{
			for(;;)
			{
				const char c = it.Get();
				if(c != '\n' && std::isspace(static_cast<unsigned char>(c)))
					it.Next();
				else
					break;
			}
		}
		
		// Skip until NL. (Utility function)
		template<class INP>
		void SkipUntilNL(PositionT<INP> &it)
		{
			while(it.IsNotEnd() && it.Get() != '\n')
				it.Next();
		}
		
		// get a token. (Utility function)
		template<class INP>
		std::string GetToken(PositionT<INP> &it)
		{
			std::string s;
			for(;;)
			{
				const char c = it.Get();
				// '-' is a valid character of flags.
				if(std::isalpha(static_cast<unsigned char>(c)) || c == '_' || c == '-')
				{
					s += c;
					it.Next();
				}
				else
					break;
			}
			return s;
		}
		
		// get a number. (Utility function)
		template<class INP>
		std::size_t GetNumber(PositionT<INP> &it)
		{
			std::string s;
			for(;;)
			{
				const char c = it.Get();
				if(std::isdigit(static_cast<unsigned char>(c)))
				{
					s += c;
					it.Next();
				}
				else
					break;
			}
			return std::stoi(s);
		}
		
		// get a octal number. (Utility function)
		template<class INP>
		std::size_t GetOctalNumber(PositionT<INP> &it)
		{
			std::string s;
			for(;;)
			{
				const char c = it.Get();
				if(std::isdigit(static_cast<unsigned char>(c)) && c != '8' && c != '9')
				{
					s += c;
					it.Next();
				}
				else
					break;
			}
			return std::stoi(s, nullptr, 8);
		}
		
		// get a hexadecimal number. (Utility function)
		template<class INP>
		std::size_t GetHexadecimalNumber(PositionT<INP> &it)
		{
			std::string s;
			for(;;)
			{
				const char c = it.Get();
				if(std::isxdigit(static_cast<unsigned char>(c)))
				{
					s += c;
					it.Next();
				}
				else
					break;
			}
			return std::stoi(s, nullptr, 16);
		}
		
		// Check if this line is a TEXT.
		// Pre position: Start of a line.
		// Post position: Start of a line (except spaces).
		template<class INP>
		bool IsTextLine(PositionT<INP> &it)
		{
			SkipSpacesExceptNL(it);
			return it.Get() == '"';
		}
		
		// Decision the type of a line.
		// Pre position: Start of a line.
		template<class INP>
		LineT DecisionTypeOfLine(PositionT<INP> &it)
		{
			SkipSpacesExceptNL(it);
			const char c = it.Get();
			if(c == '\n')
				return LineT::EMPTY;
			else if(c == '"')
				return LineT::TEXT;
			else if (c == '#')
			{
				it.Next();
				if(it.Get() == ',')
				{
					it.Next();
					return LineT::FLAG_COMMENT;
				}
				else
					return LineT::COMMENT;
			}
			else if(c == 'm')
			{
				const std::string s = GetToken(it);
				if(s == "msgctxt")
					return LineT::MSGCTXT;
				else if(s == "msgid")
					return LineT::MSGID;
				else if(s == "msgid_plural")
					return LineT::MSGID_PLURAL;
				else if(s == "msgstr")
				{
					if(it.Get() == '[')
					{
						it.Next();
						return LineT::MSGSTR_PLURAL;
					}
					else
						return LineT::MSGSTR;
				}
			}
			return LineT::UNKNOWN;
		}
		
		// Skip an empty line.
		// Pre position: The end of a line.
		// Post position: The next line.
		template<class INP>
		void ParseEmptyLine(PositionT<INP> &it)
		{
			SkipUntilNL(it);
			it.Next();
		}
		
		// Pick out a content of the text.
		// Pre position: The first double quotation mark.
		// Post position: The next line.
		template<class INP>
		std::string ParseText(PositionT<INP> &it)
		{
			assert(it.Get() == '"');
			it.Next();
			std::string s;
			for(;;)
			{
				const char c = it.Get();
				it.Next();
				std::size_t val = 0;
				if(c == '\\')
				{
					const char c2 = it.Get();
					it.Next();
					switch(c2)
					{
					case 'a':
						s += '\a';
						break;
					case 'b':
						s += '\b';
						break;
					case 'f':
						s += '\f';
						break;
					case 'n':
						s += '\n';
						break;
					case 'r':
						s += '\r';
						break;
					case 't':
						s += '\t';
						break;
					case 'v':
						s += '\v';
						break;
					case '0':
						val = GetOctalNumber(it);
						s += static_cast<char>(val);
						break;
					case 'x':
						it.Next();
						val = GetHexadecimalNumber(it);
						s += static_cast<char>(val);
						break;
					case '\0':
					case '\n':
						throw PoParseError<INP>("This text has no terminator.", it);
					default:
						s += c2;
						break;
					}
				}
				else if(c == '"')
				{
					SkipSpacesExceptNL(it);
					if(it.Get() != '\n' && !it.IsEnd())
						throw PoParseError<INP>("Unexpected character is found.", it);
					it.Next();
					return s;
				}
				else
					s += c;
			}
			assert(0);
			return s;
		}
		
		// Pick out a flag of the line.
		// Pre position: The next character of ','.
		// Post position: The next line.
		template<class INP>
		FlagT ParseFlagComment(PositionT<INP> &it)
		{
			FlagT flag = NONE;
			while(it.Get() != '\n' && it.IsNotEnd())
			{
				SkipSpacesExceptNL(it);
				std::string s = GetToken(it);
				if(s == "fuzzy")
					flag |= FUZZY;
				SkipSpacesExceptNL(it);
				const char c = it.Get();
				if(c == ',')
					it.Next();
				else if(c != '\n' && c != '\0')
					throw PoParseError<INP>("Unexpected character is found.", it);
			}
			it.Next();
			return flag;
		}
		
		// Skip a comment line.
		// Pre position: The next character of '#'.
		// Post position: The next line.
		template<class INP>
		void ParseComment(PositionT<INP> &it)
		{
			SkipUntilNL(it);
			it.Next();
		}
		
		// Pick out a message text.
		// Pre position: The next character of a keyword.
		// Post position: The next line of the last text line.
		template<class INP>
		std::string ParseMsgdata(PositionT<INP> &it)
		{
			SkipSpacesExceptNL(it);
			std::string s(ParseText(it));
			while (IsTextLine(it))
				s += ParseText(it);
			return s;
		}
		
		// Pick out a msgstr[n] text.
		// Pre position: The next character of a keyword.
		// Post position: The next line of the last text line.
		template<class INP>
		std::pair<std::size_t, std::string> ParseMsgstrPlural(PositionT<INP> &it)
		{
			SkipSpacesExceptNL(it);
			const std::size_t idx = GetNumber(it);
			SkipSpacesExceptNL(it);
			if(it.Get() != ']')
				throw PoParseError<INP>("']' is expected.", it);
			it.Next();
			SkipSpacesExceptNL(it);
			std::string s(ParseText(it));
			while(IsTextLine(it))
				s += ParseText(it);
			return std::make_pair(idx, s);
		}
		
		// Parse one catalog entry.
		// Pre position: The result of DecisionTypeOfLine() for the first line.
		// Post position: The result of DecisionTypeOfLine() for next entry.
		// Return: previousLine: The line of a type for the next line.
		// Return: one catalog entry data. it's empty if previousLine == END.
		// Note: previousLine must be LineT::START if there is no previous lines.
		template<class INP>
		CatalogEntryT ParseOneEntry(PositionT<INP> &it, LineT &previousLine)
		{
			LineT stat = previousLine;
			CatalogEntryT out;
			try {
				FlagT flag = NONE;
				if(stat == LineT::START)
					stat = DecisionTypeOfLine(it);
				while(stat == LineT::EMPTY || stat == LineT::COMMENT || stat == LineT::FLAG_COMMENT)
				{
					if (stat == LineT::EMPTY)
					{
						ParseEmptyLine(it);
						flag = NONE;
					}
					else if (stat == LineT::COMMENT)
						ParseEmptyLine(it);
					else
						flag |= ParseFlagComment(it);
					stat = DecisionTypeOfLine(it);
				}
				if(it.IsEnd())
				{
					previousLine = LineT::END;
					return out;
				}
				if(stat == LineT::MSGCTXT)
				{
					out.msgid = ParseMsgdata(it);
					out.msgid += CONTEXT_SEPARATOR;
					stat = DecisionTypeOfLine(it);
				}
				if(stat != LineT::MSGID)
					throw PoParseError<INP>("'msgid' is expected.", it);
				else
				{
					out.msgid += ParseMsgdata(it);
					stat = DecisionTypeOfLine(it);
				}
				if(stat == LineT::MSGID_PLURAL)
				{
					ParseMsgdata(it);
					for(;;)
					{
						stat = DecisionTypeOfLine(it);
						if(stat == LineT::MSGSTR_PLURAL)
						{
							const auto saveIt = it;
							const auto p = ParseMsgstrPlural(it);
							if(p.first != out.msgstr.size())
								throw PoParseError<INP>("Invalid plural index in msgstr[n].", saveIt);
							out.msgstr.push_back(p.second);
						}
						else
							break;
					}
					if(out.msgstr.empty())
						throw PoParseError<INP>("'msgstr[n]' is expected.", it);
				}
				else
					if(stat != LineT::MSGSTR)
						throw PoParseError<INP>("'msgstr' is expected.", it);
					else
					{
						out.msgstr.push_back(ParseMsgdata(it));
						stat = DecisionTypeOfLine(it);
					}
				if(flag & FUZZY)
					out.msgstr[0].clear();
			}
			catch(PoParseError<INP> &e)
			{
				const auto &loc = e.GetLocation();
				out.error = std::to_string(loc.GetLine()) + ',' + std::to_string(loc.GetColumn()) + ": " + e.what();
				
				do
				{
					SkipUntilNL(it);
					it.Next();
					stat = DecisionTypeOfLine(it);
				}
				while(stat != LineT::EMPTY && stat != LineT::COMMENT && stat != LineT::FLAG_COMMENT && stat != LineT::MSGCTXT && stat != LineT::MSGID && stat != LineT::UNKNOWN);
			}
			previousLine = stat;
			return out;
		}
		
		// Parse all catalog entries.
		template<class INP>
		std::vector<CatalogEntryT> GetEntries(INP &begin, const INP &end)
		{
			std::vector<CatalogEntryT> entries;
			PositionT<INP> pos(begin, end);
			LineT typeOfLine = LineT::START;
			while(pos.IsNotEnd())
			{
				const CatalogEntryT value = ParseOneEntry(pos, typeOfLine);
				if(typeOfLine == LineT::END)
					break;
				entries.push_back(std::move(value));
			}
			return entries;
		}
	} // namespace PoParser
} // namespace spiritless_po

#endif // SRIRITLESS_PO_PO_PARSER_H_
