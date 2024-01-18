/** class Catalog
    \file Catalog.h
    \author OOTA, Masato
    \copyright Copyright © 2019, 2022, 2024 OOTA, Masato
    \par License Boost
    \parblock
      This program is distributed under the Boost Software License Version 1.0.
      You can get the license file at “https://www.boost.org/LICENSE_1_0.txt”.
    \endparblock
*/

#ifndef SPIRITLESS_PO_CATALOG_H_
#define SPIRITLESS_PO_CATALOG_H_

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
    /** Class Catalog handles a catalog that contains original and translated messages, which come from PO format streams.

        You need only to use this class and not directly to use other classes under include/spiritless_po/.

        This class has some features:
        - An instance of Catalog handles only one textdomain and only one language, doesn't handle multiple textdomains and multiple languages.
        - Catalog can read the messages from multiple PO files, instead of a single MO file. You can add new messages to a single catalog any number of times.
        - Catalog doesn't care the locale.
        - Catalog doesn't handle the character encoding.

        If you would use multiple textdomains and/or multiple languages, you need to use multiple instances of this class.
    */
    class Catalog {
    public:
        /** Create an empty catalog. */
        Catalog() noexcept;

        /** Create and Add(begin, end).
            \tparam INP A type of an input iterator.
            \param [in] begin An input iterator pointing to the beginning of the range.
            \param [in] end An input iterator pointing to the end of the range.
        */
        template <class INP>
        explicit Catalog(INP begin, INP end);

        /** Create and Add(is).
            \param [in] is An input stream that contains PO entries.
        */
        explicit Catalog(std::istream &is);

        /** This class is copyable.
            \param [in] a The source.
        */
        Catalog(const Catalog &a) = default;

        /** This class is movable.
            \param [in] a The source.
        */
        Catalog(Catalog &&a) = default;

        /** This class is destructible. */
        ~Catalog() noexcept = default;

        /** This class is assignable.
            \param [in] a The source.
        */
        Catalog &operator=(const Catalog &a) = default;

        /** This class is move assignable.
            \param [in] a The source.
        */
        Catalog &operator=(Catalog &&a) = default;

        /** Clear all information and create an empty catalog. */
        void Clear() noexcept;

        /** Add PO entries.
            \tparam INP A type of an input iterator.
            \param [in] begin An input iterator pointing to the beginning of the range.
            \param [in] end An input iterator pointing to the end of the range.
            \return true if no error is existed.
            \note This function doesn't change any existed entries, that is a translated text (msgstr) that corresponds to an existed original text (msgid), and also metadata if it's already existed.
            \note An entry isn't added if the msgstr (including msgstr[0]) is empty.
        */
        template <class INP>
        bool Add(INP begin, INP end);

        /** Add some PO entries.
            \param [in] is An input stream that contains PO entries.
            \return true if no error is existed.
            \note This function doesn't change any existed entries, that is a translated text (msgstr) that corresponds to an existed original text (msgid), and also metadata if it's already existed.
            \note An entry isn't added if the msgstr (including msgstr[0]) is empty.
        */
        bool Add(std::istream &is);

        /** Add another catalog contents.
            \param [in] a A catalog to add the entries.
            \note This function doesn't change any existed entries, that is a translated text (msgstr) that corresponds to an existed original text (msgid), and also metadata if it's already existed.
        */
        void Merge(const Catalog &a);

        /** Clear the error information. */
        void ClearError() noexcept;

        /** Get the error information generated by Add() after ClearError() is called.
            \return The strings that describe the errors.
            \note The size of the result is 0 if no error is occurred.
        */
        const std::vector<std::string> &GetError() const noexcept;

        /** Get the translated text.
            \param [in] msgid The original text.
            \return The translated text if exists. If not, returns msgid. (It's a reference to the same object.)
        */
        const std::string &gettext(const std::string &msgid) const;

        /** Get the translated text.
            \param [in] msgid The original text.
            \param [in] msgidPlural The plural form of the original text.
            \param [in] n The value relating to the text.
            \return The translated text if exists. If not, returns msgid (if n == 1) or msgidPlural (if n != 1). (It's a reference to the same object.)
        */
        const std::string &ngettext(const std::string &msgid, const std::string &msgidPlural,
            unsigned long int n) const;

        /** Get the translated text.
            \param [in] msgctxt The context of the text.
            \param [in] msgid The original text.
            \return The translated text if exists. If not, returns msgid. (It's the reference to the same object.)
        */
        const std::string &pgettext(const std::string &msgctxt, const std::string &msgid) const;

        /** Get the translated text.
            \param [in] msgctxt The context of the text.
            \param [in] msgid The original text.
            \param [in] msgidPlural The plural form of the original text.
            \param [in] n The value relating to the text.
            \return The translated text if exists. If not, returns msgid (if n == 1) or msgidPlural (if n != 1). (It's a reference to the same object.)
        */
        const std::string &npgettext(const std::string &msgctxt, const std::string &msgid,
            const std::string &msgidPlural, unsigned long int n) const;


        /** Type of the string index.

            - stringTable[indexData[msgctxt + CONTEXT_SEPARATOR + msgid].stringTableIndex] == msgstr
            - stringTable[indexData[msgctxt + CONTEXT_SEPARATOR + msgid].stringTableIndex + n] == msgstr[n]
            - The maximum n is totalPlurals - 1.
            \attention This type is public to use for debugging and managing.
        */
        struct IndexDataT {
            std::size_t stringTableIndex; /**< The index of the StringTable. */
            std::size_t totalPlurals; /**< The number of the strings, including the plural forms, corresponding a msgid. */
        };

        /** Type of the statistics.

            The statistics of the messages added by Add() and Merge().

            For each entry added by Add() and Merge():
            1. `totalCount`++
            2. if msgstr[0] == "" then (* untranslated entry *) break
            3. if ID == "" then `metadataCount`++; break
            4. if msgstr[0] already exists in the catalog then `discardedCount`++; break
            5. `translatedCount`++

            "ID" means msgid, or msgctxt + CONTEXT_SEPARATOR + msgid if msgctxt != "".

            Add(a normal PO file) is to report `metadataCount` == 1 and `discardedCount` == 0.

            \note `totalCount` counts the empty ID, unlike "msgfmt --statistics".
            \note Only first metadata is used when `metadataCount` is more than one.
            \note `discardedCount` doesn't count the discarded metadata.
            \attention Merge() report no untranslated entry, because the catalog doesn't keep the untranslated entry.
        */
        struct StatisticsT {
            std::size_t totalCount; /**< The count of the entry. */
            std::size_t metadataCount; /**< The count of the metadata entry. */
            std::size_t translatedCount; /**< The count of the translated entry. */
            std::size_t discardedCount; /**< The count of the discarded entry. */
        };

    private:
        MetadataParser::MapT metadata;
        std::unordered_map<std::string, IndexDataT> index;
        std::vector<std::string> stringTable;
        PluralParser::FunctionType pluralFunction;
        std::size_t maxPlurals;
        std::vector<std::string> errors;
        StatisticsT statistics;

    public:
        // Debugging and managing functions

        /** Get the metadata.
            \return The map of the metadata.
            \attention This function is public to use for debugging and managing.
         */
        const MetadataParser::MapT &GetMetadata() const noexcept;

        /** Get the index map.
            \return The map of the string index.
            \attention This function is public to use for debugging and managing.
            \note The size of the index map is the number of the translatable msgid.
         */
        const std::unordered_map<std::string, IndexDataT> &GetIndex() const noexcept;

        /** Get the string table.
            \return The string table.
            \attention This function is public to use for debugging and managing.
         */
        const std::vector<std::string> &GetStringTable() const noexcept;

        /** Get the statistics of the messages added by Add().
            \return The statistics.
            \attention This function is public to use for debugging and managing.
         */
        const StatisticsT &GetStatistics() const noexcept;

        /** Clear the statistics of the messages added by Add().
            \attention This function is public to use for debugging and managing.
         */
        void ClearStatistics() noexcept;
    };

    inline Catalog::Catalog() noexcept
        : metadata(), index(), stringTable(), pluralFunction(),
          maxPlurals(0), errors()
    {
        ClearStatistics();
    }

    template <class INP>
    inline Catalog::Catalog(const INP begin, const INP end)
        : Catalog()
    {
        Add(begin, end);
    }

    inline Catalog::Catalog(std::istream &is)
        : Catalog()
    {
        Add(is);
    }

    inline void Catalog::Clear() noexcept
    {
        *this = Catalog();
    }

    template <class INP>
    bool Catalog::Add(const INP begin, const INP end)
    {
        std::vector<PoParser::PoEntryT> newEntries(PoParser::GetEntries(begin, end));
        statistics.totalCount += newEntries.size();
        for (auto &it : newEntries) {
            if (!it.error.empty()) {
                errors.push_back(std::move(it.error));
            } else if (!it.msgstr[0].empty()) {
                if (!it.msgid.empty()) {
                    if (index.find(it.msgid) == index.end()) {
                        statistics.translatedCount++;
                        IndexDataT idx;
                        idx.stringTableIndex = stringTable.size();
                        idx.totalPlurals = it.msgstr.size();
                        stringTable.insert(stringTable.end(), std::make_move_iterator(it.msgstr.begin()), std::make_move_iterator(it.msgstr.end()));
                        index.emplace(it.msgid, idx);
                    } else {
                        statistics.discardedCount++;
                    }
                } else {
                    statistics.metadataCount++;
                    if (metadata.empty()) {
                        metadata = MetadataParser::Parse(it.msgstr[0]);
                        const auto plural = metadata.find("Plural-Forms");
                        if (plural != metadata.end()) {
                            const auto pluralText = plural->second;
                            try {
                                const auto pluralData = PluralParser::Parse(pluralText);
                                if (pluralData.first > 0) {
                                    maxPlurals = pluralData.first - 1;
                                }
                                pluralFunction = pluralData.second;
                            } catch (PluralParser::ExpressionError &e) {
                                const size_t col = std::distance(pluralText.cbegin(), e.Where());
                                errors.emplace_back("Column#" + std::to_string(col + 1)
                                    + " in plural expression: " + e.what());
                            }
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
        if (!a.metadata.empty()) {
            statistics.metadataCount++;
            statistics.totalCount++;
            if (metadata.empty()) {
                metadata = a.metadata;
                maxPlurals = a.maxPlurals;
                pluralFunction = a.pluralFunction;
            }
        }
        statistics.totalCount += a.index.size();
        for (const auto &src : a.index) {
            if (index.find(src.first) == index.end()) {
                statistics.translatedCount++;
                auto srcIndexData = src.second;
                auto srcIndex = srcIndexData.stringTableIndex;
                auto srcTotal = srcIndexData.totalPlurals;
                auto srcStringIt = a.stringTable.begin() + srcIndex;
                IndexDataT idx;
                idx.stringTableIndex = stringTable.size();
                idx.totalPlurals = srcTotal;
                stringTable.insert(stringTable.end(), srcStringIt, srcStringIt + srcTotal);
                index.emplace(src.first, idx);
            } else {
                statistics.discardedCount++;
            }
        }
        errors.insert(errors.end(), a.errors.begin(), a.errors.end());
    }

    inline void Catalog::ClearError() noexcept
    {
        errors.clear();
    }

    inline const std::vector<std::string> &Catalog::GetError() const noexcept
    {
        return errors;
    }

    inline const std::string &Catalog::gettext(const std::string &msgid) const
    {
        const auto &it = index.find(msgid);
        if (it != index.end()) {
            return stringTable[it->second.stringTableIndex];
        } else {
            return msgid;
        }
    }

    inline const std::string &Catalog::ngettext(const std::string &msgid, const std::string &msgidPlural,
        unsigned long int n) const
    {
        const auto &it = index.find(msgid);
        if (it != index.end()) {
            std::size_t nIdx = pluralFunction(n);
            if (nIdx >= it->second.totalPlurals) {
                nIdx = 0;
            }
            return stringTable[it->second.stringTableIndex + nIdx];
        } else {
            if (n == 1) {
                return msgid;
            } else {
                return msgidPlural;
            }
        }
    }

    inline const std::string &Catalog::pgettext(const std::string &msgctxt, const std::string &msgid) const
    {
        std::string s(msgctxt);
        s += CONTEXT_SEPARATOR;
        s += msgid;
        const auto &it = index.find(s);
        if (it != index.end()) {
            return stringTable[it->second.stringTableIndex];
        } else {
            return msgid;
        }
    }

    inline const std::string &Catalog::npgettext(const std::string &msgctxt, const std::string &msgid,
        const std::string &msgidPlural, unsigned long int n) const
    {
        std::string s(msgctxt);
        s += CONTEXT_SEPARATOR;
        s += msgid;
        const auto &it = index.find(s);
        if (it != index.end()) {
            std::size_t nIdx = pluralFunction(n);
            if (nIdx >= it->second.totalPlurals) {
                nIdx = 0;
            }
            return stringTable[it->second.stringTableIndex + nIdx];
        } else {
            if (n == 1) {
                return msgid;
            } else {
                return msgidPlural;
            }
        }
    }

    inline const MetadataParser::MapT &Catalog::GetMetadata() const noexcept
    {
        return metadata;
    }

    inline const std::unordered_map<std::string, Catalog::IndexDataT> &Catalog::GetIndex() const noexcept
    {
        return index;
    }

    inline const std::vector<std::string> &Catalog::GetStringTable() const noexcept
    {
        return stringTable;
    }

    inline const Catalog::StatisticsT &Catalog::GetStatistics() const noexcept
    {
        return statistics;
    }

    inline void Catalog::ClearStatistics() noexcept
    {
        statistics = Catalog::StatisticsT{};
    }
} // namespace spiritless_po

#endif // SPIRITLESS_PO_CATALOG_H_
