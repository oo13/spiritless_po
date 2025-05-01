/** PO text parser.
    \file PoParser.h
    \author OOTA, Masato
    \copyright Copyright © 2019, 2022 OOTA, Masato
    \par License Boost
    \parblock
      This program is distributed under the Boost Software License Version 1.0.
      You can get the license file at “https://www.boost.org/LICENSE_1_0.txt”.
    \endparblock
*/

#ifndef SPIRITLESS_PO_PO_PARSER_H_
#define SPIRITLESS_PO_PO_PARSER_H_

#include "Common.h"

#include <cassert>
#include <cstddef>
#include <locale>
#include <string>
#include <utility>
#include <vector>

namespace spiritless_po {
    /** This class is a parser for the text that contains the PO entries. */
    class PoParser {
    public:
        /** Type of a PO entry.

            - msgid and msgstr are uncertain when error is not empty.
            - msgstr.size() > 0 when error is empty.
            - msgstr[0] is an empty string if the entry is fuzzy.
        */
        struct PoEntryT {
            std::string msgid; /**< msgid or msgid_plural ( + CONTEXT_SEPARATOR + msgctxt if msgctxt exists.) */
            std::vector<std::string> msgstr; /**< msgstr, or msgstr[n] if the entry is for msgid_plural. */
            std::string error; /**< The messages that describe the error in the parsing. */
        };

        /** Parse the text that contains the PO entries.
            \tparam INP A type of an input iterator.
            \tparam Sentinel A type of a sentinel.
            \param [in] begin The beginning of the text to parse.
            \param [in] end The end of the text to parse.
            \return The result of the parsing.
        */
        template <typename INP, typename Sentinel>
        static std::vector<PoEntryT> GetEntries(INP &&begin, Sentinel &&end);

    private:
        // Reading position type.
        template <typename INP, typename Sentinel>
        class PositionT {
        public:
            PositionT(INP &it, Sentinel &end, std::size_t line = 1, std::size_t column = 1);

            bool IsEnd() const;
            bool IsNotEnd() const;
            char Get() const;
            void Next();
            std::size_t GetLine() const;
            std::size_t GetColumn() const;

        private:
            INP &curIt;
            Sentinel &endIt;
            std::size_t lineNumber;
            std::size_t columnNumber;
        };

        // Parse Error in PO file.
        template <typename INP, typename Sentinel>
        class PoParseError : public std::runtime_error {
        public:
            explicit PoParseError(const std::string &whatArg, const PositionT<INP, Sentinel> &it);
            explicit PoParseError(const char *whatArg, const PositionT<INP, Sentinel> &it);

            // Get the error location.
            const PositionT<INP, Sentinel> &GetLocation() const noexcept;

        private:
            PositionT<INP, Sentinel> loc;
        };

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
            FUZZY = 1 << 0
        };


        PoParser() = delete;
        ~PoParser() = delete;

        static PoParser::FlagT FlagOR(PoParser::FlagT a, PoParser::FlagT b);
        template <typename INP, typename Sentinel>
        static void SkipSpacesExceptNL(PositionT<INP, Sentinel> &it);
        template <typename INP, typename Sentinel>
        static void SkipUntilNL(PositionT<INP, Sentinel> &it);
        template <typename INP, typename Sentinel>
        static std::string GetToken(PositionT<INP, Sentinel> &it);
        template <typename INP, typename Sentinel>
        static std::size_t GetNumber(PositionT<INP, Sentinel> &it);
        template <typename INP, typename Sentinel>
        static std::size_t GetOctalNumber(PositionT<INP, Sentinel> &it);
        template <typename INP, typename Sentinel>
        static std::size_t GetHexadecimalNumber(PositionT<INP, Sentinel> &it);
        template <typename INP, typename Sentinel>
        static bool IsTextLine(PositionT<INP, Sentinel> &it);
        template <typename INP, typename Sentinel>
        static LineT DecisionTypeOfLine(PositionT<INP, Sentinel> &it);
        template <typename INP, typename Sentinel>
        static void ParseEmptyLine(PositionT<INP, Sentinel> &it);
        template <typename INP, typename Sentinel>
        static void ParseText(PositionT<INP, Sentinel> &it, std::string &s);
        template <typename INP, typename Sentinel>
        static PoParser::FlagT ParseFlagComment(PositionT<INP, Sentinel> &it);
        template <typename INP, typename Sentinel>
        static void ParseComment(PositionT<INP, Sentinel> &it);
        template <typename INP, typename Sentinel>
        static std::string ParseMsgdata(PositionT<INP, Sentinel> &it);
        template <typename INP, typename Sentinel>
        static std::pair<std::size_t, std::string> ParseMsgstrPlural(PositionT<INP, Sentinel> &it);
        template <typename INP, typename Sentinel>
        static PoEntryT ParseOneEntry(PositionT<INP, Sentinel> &it, LineT &previousLine);
    };



    inline PoParser::FlagT PoParser::FlagOR(PoParser::FlagT a, PoParser::FlagT b)
    {
        return static_cast<PoParser::FlagT>(a | b);
    }


    template <typename INP, typename Sentinel>
    PoParser::PositionT<INP, Sentinel>::PositionT(INP &it, Sentinel &end, std::size_t line, std::size_t column)
        : curIt(it), endIt(end), lineNumber(line), columnNumber(column)
    {
    }

    template <typename INP, typename Sentinel>
    bool PoParser::PositionT<INP, Sentinel>::IsEnd() const
    {
        return curIt == endIt;
    }

    template <typename INP, typename Sentinel>
    bool PoParser::PositionT<INP, Sentinel>::IsNotEnd() const
    {
        return curIt != endIt;
    }

    template <typename INP, typename Sentinel>
    char PoParser::PositionT<INP, Sentinel>::Get() const
    {
        return IsEnd() ? '\0' : *curIt;
    }

    template <typename INP, typename Sentinel>
    void PoParser::PositionT<INP, Sentinel>::Next()
    {
        if (IsNotEnd()) {
            if (Get() == '\n') {
                ++lineNumber;
                columnNumber = 0;
            }
            ++curIt;
            ++columnNumber;
        }
    }

    template <typename INP, typename Sentinel>
    std::size_t PoParser::PositionT<INP, Sentinel>::GetLine() const
    {
        return lineNumber;
    }

    template <typename INP, typename Sentinel>
    std::size_t PoParser::PositionT<INP, Sentinel>::GetColumn() const
    {
        return columnNumber;
    }

    template <typename INP, typename Sentinel>
    PoParser::PoParseError<INP, Sentinel>::PoParseError(const std::string &whatArg, const PositionT<INP, Sentinel> &it)
        : std::runtime_error(whatArg), loc(it)
    {
    }

    template <typename INP, typename Sentinel>
    PoParser::PoParseError<INP, Sentinel>::PoParseError(const char *whatArg, const PositionT<INP, Sentinel> &it)
        : std::runtime_error(whatArg), loc(it)
    {
    }

    // Get the error location.
    template <typename INP, typename Sentinel>
    const PoParser::PositionT<INP, Sentinel> &PoParser::PoParseError<INP, Sentinel>::GetLocation() const noexcept
    {
        return loc;
    }

    // Skip spaces except NL. (Utility function)
    template <typename INP, typename Sentinel>
    void PoParser::SkipSpacesExceptNL(PositionT<INP, Sentinel> &it)
    {
        for (;;) {
            const char c = it.Get();
            if (c != '\n' && std::isspace(c, std::locale::classic())) {
                it.Next();
            } else {
                break;
            }
        }
    }

    // Skip until NL. (Utility function)
    template <typename INP, typename Sentinel>
    void PoParser::SkipUntilNL(PositionT<INP, Sentinel> &it)
    {
        while (it.IsNotEnd() && it.Get() != '\n') {
            it.Next();
        }
    }

    // get a token. (Utility function)
    template <typename INP, typename Sentinel>
    std::string PoParser::GetToken(PositionT<INP, Sentinel> &it)
    {
        std::string s;
        for (;;) {
            const char c = it.Get();
            // '-' is a valid character of flags.
            if (std::isalpha(c, std::locale::classic()) || c == '_' || c == '-') {
                s += c;
                it.Next();
            } else {
                break;
            }
        }
        return s;
    }

    // get a number. (Utility function)
    template <typename INP, typename Sentinel>
    std::size_t PoParser::GetNumber(PositionT<INP, Sentinel> &it)
    {
        std::string s;
        for (;;) {
            const char c = it.Get();
            if (std::isdigit(c, std::locale::classic())) {
                s += c;
                it.Next();
            } else {
                break;
            }
        }
        if (s.empty()) {
            throw PoParseError<INP, Sentinel>("'0'..'9' is expected.", it);
        }
        return std::stoi(s);
    }

    // get a octal number. (Utility function)
    template <typename INP, typename Sentinel>
    std::size_t PoParser::GetOctalNumber(PositionT<INP, Sentinel> &it)
    {
        std::string s;
        for (size_t i=0; i<3; ++i) {
            const char c = it.Get();
            if (std::isdigit(c, std::locale::classic()) && c != '8' && c != '9') {
                s += c;
                it.Next();
            } else {
                break;
            }
        }
        if (s.empty()) {
            throw PoParseError<INP, Sentinel>("'0'..'7' is expected.", it);
        }
        return std::stoi(s, nullptr, 8);
    }

    // get a hexadecimal number. (Utility function)
    template <typename INP, typename Sentinel>
    std::size_t PoParser::GetHexadecimalNumber(PositionT<INP, Sentinel> &it)
    {
        std::string s;
        for (;;) {
            const char c = it.Get();
            if (std::isxdigit(c, std::locale::classic())) {
                s += c;
                it.Next();
            } else {
                break;
            }
        }
        if (s.empty()) {
            throw PoParseError<INP, Sentinel>("[0-9A-Fa-f] is expected.", it);
        }
        return std::stoi(s, nullptr, 16);
    }

    // Check if this line is a TEXT.
    // Pre position: Start of a line.
    // Post position: Start of a line (except spaces).
    template <typename INP, typename Sentinel>
    bool PoParser::IsTextLine(PositionT<INP, Sentinel> &it)
    {
        SkipSpacesExceptNL(it);
        return it.Get() == '"';
    }

    // Decision the type of a line.
    // Pre position: Start of a line.
    template <typename INP, typename Sentinel>
    PoParser::LineT PoParser::DecisionTypeOfLine(PositionT<INP, Sentinel> &it)
    {
        SkipSpacesExceptNL(it);
        if (it.IsEnd()) {
            return LineT::END;
        }
        const char c = it.Get();
        if (c == '\n') {
            return LineT::EMPTY;
        } else if (c == '"') {
            return LineT::TEXT;
        } else if (c == '#') {
            it.Next();
            if (it.Get() == ',') {
                it.Next();
                return LineT::FLAG_COMMENT;
            } else {
                return LineT::COMMENT;
            }
        } else if (c == 'm') {
            const std::string s = GetToken(it);
            if (s == "msgctxt") {
                return LineT::MSGCTXT;
            } else if (s == "msgid") {
                return LineT::MSGID;
            } else if (s == "msgid_plural") {
                return LineT::MSGID_PLURAL;
            } else if (s == "msgstr") {
                if (it.Get() == '[') {
                    it.Next();
                    return LineT::MSGSTR_PLURAL;
                } else {
                    return LineT::MSGSTR;
                }
            }
        }
        return LineT::UNKNOWN;
    }

    // Skip an empty line.
    // Pre position: The end of a line.
    // Post position: The next line.
    template <typename INP, typename Sentinel>
    void PoParser::ParseEmptyLine(PositionT<INP, Sentinel> &it)
    {
        SkipUntilNL(it);
        it.Next();
    }

    // Pick out a content of the text.
    // Pre position: The first double quotation mark.
    // Post position: The next line.
    template <typename INP, typename Sentinel>
    void PoParser::ParseText(PositionT<INP, Sentinel> &it, std::string &s)
    {
        if (it.Get() != '"') {
            throw PoParseError<INP, Sentinel>("'\"' is expected.", it);
        }
        it.Next();
        for (;;) {
            const char c = it.Get();
            it.Next();
            if (c == '\\') {
                std::size_t val = 0;
                const char c2 = it.Get();
                switch (c2) {
                case 'a':
                    s += '\a';
                    it.Next();
                    break;
                case 'b':
                    s += '\b';
                    it.Next();
                    break;
                case 'f':
                    s += '\f';
                    it.Next();
                    break;
                case 'n':
                    s += '\n';
                    it.Next();
                    break;
                case 'r':
                    s += '\r';
                    it.Next();
                    break;
                case 't':
                    s += '\t';
                    it.Next();
                    break;
                case 'v':
                    s += '\v';
                    it.Next();
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
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
                    throw PoParseError<INP, Sentinel>("This text has no terminator.", it);
                default:
                    s += c2;
                    it.Next();
                    break;
                }
            } else if (c == '"') {
                SkipSpacesExceptNL(it);
                if (it.Get() != '\n' && !it.IsEnd()) {
                    throw PoParseError<INP, Sentinel>("Unexpected character is found.", it);
                }
                it.Next();
                return;
            } else if (it.IsEnd()) {
                throw PoParseError<INP, Sentinel>("This text has no terminator.", it);
            } else {
                s += c;
            }
        }
    }

    // Pick out a flag of the line.
    // Pre position: The next character of ','.
    // Post position: The next line.
    template <typename INP, typename Sentinel>
    PoParser::FlagT PoParser::ParseFlagComment(PositionT<INP, Sentinel> &it)
    {
        FlagT flag = NONE;
        while (it.Get() != '\n' && it.IsNotEnd()) {
            SkipSpacesExceptNL(it);
            std::string s = GetToken(it);
            if (s == "fuzzy") {
                flag = FlagOR(flag, FUZZY);
            }
            SkipSpacesExceptNL(it);
            const char c = it.Get();
            if (c == ',') {
                it.Next();
            } else if (c != '\n' && c != '\0') {
                throw PoParseError<INP, Sentinel>("Unexpected character is found.", it);
            }
        }
        it.Next();
        return flag;
    }

    // Skip a comment line.
    // Pre position: The next character of '#'.
    // Post position: The next line.
    template <typename INP, typename Sentinel>
    void PoParser::ParseComment(PositionT<INP, Sentinel> &it)
    {
        SkipUntilNL(it);
        it.Next();
    }

    // Pick out a message text.
    // Pre position: The next character of a keyword.
    // Post position: The next line of the last text line.
    template <typename INP, typename Sentinel>
    std::string PoParser::ParseMsgdata(PositionT<INP, Sentinel> &it)
    {
        SkipSpacesExceptNL(it);
        std::string s;
        ParseText(it, s);
        while (IsTextLine(it)) {
            ParseText(it, s);
        }
        return s;
    }

    // Pick out a msgstr[n] text.
    // Pre position: The next character of a keyword.
    // Post position: The next line of the last text line.
    template <typename INP, typename Sentinel>
    std::pair<std::size_t, std::string> PoParser::ParseMsgstrPlural(PositionT<INP, Sentinel> &it)
    {
        SkipSpacesExceptNL(it);
        const std::size_t idx = GetNumber(it);
        SkipSpacesExceptNL(it);
        if (it.Get() != ']') {
            throw PoParseError<INP, Sentinel>("']' is expected.", it);
        }
        it.Next();
        SkipSpacesExceptNL(it);
        std::string s;
        ParseText(it, s);
        while (IsTextLine(it)) {
            ParseText(it, s);
        }
        return std::make_pair(idx, s);
    }

    // Parse one PO entry.
    // Pre position: The result of DecisionTypeOfLine() for the first line.
    // Post position: The result of DecisionTypeOfLine() for next entry.
    // Return: previousLine: The line of a type for the next line.
    // Return: one PO entry data. it's empty if previousLine == END.
    // Note: previousLine must be LineT::START if there is no previous lines.
    template <typename INP, typename Sentinel>
    PoParser::PoEntryT PoParser::ParseOneEntry(PositionT<INP, Sentinel> &it, LineT &previousLine)
    {
        LineT stat = previousLine;
        PoEntryT out;
        try {
            FlagT flag = NONE;
            if (stat == LineT::START) {
                stat = DecisionTypeOfLine(it);
            }
            while (stat == LineT::EMPTY || stat == LineT::COMMENT || stat == LineT::FLAG_COMMENT) {
                if (stat == LineT::EMPTY) {
                    ParseEmptyLine(it);
                    flag = NONE;
                } else if (stat == LineT::COMMENT) {
                    ParseEmptyLine(it);
                } else {
                    flag = FlagOR(flag, ParseFlagComment(it));
                }
                stat = DecisionTypeOfLine(it);
            }
            if (stat == LineT::UNKNOWN) {
                throw PoParseError<INP, Sentinel>("An unknown keyword is found.", it);
            }
            if (it.IsEnd()) {
                previousLine = LineT::END;
                return out;
            }
            if (stat == LineT::MSGCTXT) {
                out.msgid = ParseMsgdata(it);
                out.msgid += CONTEXT_SEPARATOR;
                stat = DecisionTypeOfLine(it);
            }
            if (stat != LineT::MSGID) {
                throw PoParseError<INP, Sentinel>("'msgid' is expected.", it);
            } else {
                out.msgid += ParseMsgdata(it);
                stat = DecisionTypeOfLine(it);
            }
            if (stat == LineT::MSGID_PLURAL) {
                ParseMsgdata(it);
                for (;;) {
                    stat = DecisionTypeOfLine(it);
                    if (stat == LineT::MSGSTR_PLURAL) {
                        const auto saveIt = it;
                        const auto p = ParseMsgstrPlural(it);
                        if (p.first != out.msgstr.size()) {
                            throw PoParseError<INP, Sentinel>("Invalid plural index in msgstr[n].", saveIt);
                        }
                        out.msgstr.push_back(p.second);
                    } else {
                        break;
                    }
                }
                if (out.msgstr.empty()) {
                    throw PoParseError<INP, Sentinel>("'msgstr[n]' is expected.", it);
                }
            } else if (stat != LineT::MSGSTR) {
                throw PoParseError<INP, Sentinel>("'msgstr' is expected.", it);
            } else {
                out.msgstr.push_back(ParseMsgdata(it));
                stat = DecisionTypeOfLine(it);
            }
            if (flag & FUZZY) {
                out.msgstr[0].clear();
            }
        } catch (PoParseError<INP, Sentinel> &e) {
            const auto &loc = e.GetLocation();
            out.error = std::to_string(loc.GetLine()) + ',' + std::to_string(loc.GetColumn()) + ": " + e.what();

            do {
                SkipUntilNL(it);
                it.Next();
                stat = DecisionTypeOfLine(it);
            } while (stat != LineT::EMPTY && stat != LineT::COMMENT && stat != LineT::FLAG_COMMENT && stat != LineT::MSGCTXT && stat != LineT::MSGID && stat != LineT::END && stat != LineT::UNKNOWN);
        }
        previousLine = stat;
        return out;
    }

    // Parse all PO entries.
    template <typename INP, typename Sentinel>
    std::vector<PoParser::PoEntryT> PoParser::GetEntries(INP &&begin, Sentinel &&end)
    {
        std::vector<PoEntryT> entries;
        PositionT<INP, Sentinel> pos(begin, end);
        LineT typeOfLine = LineT::START;
        while (pos.IsNotEnd()) {
            PoEntryT value = ParseOneEntry(pos, typeOfLine);
            if (!value.error.empty() || !value.msgstr.empty()) {
                entries.push_back(std::move(value));
            }
            if (typeOfLine == LineT::END) {
                break;
            }
        }
        return entries;
    }
} // namespace spiritless_po

#endif // SPIRITLESS_PO_PO_PARSER_H_
