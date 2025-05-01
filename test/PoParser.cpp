/*
  Copyright © 2022, 2024 OOTA, Masato
  License: CC-BY-SA-3.0
  See https://creativecommons.org/licenses/by-sa/3.0/legalcode for license details.
*/
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>

#include "spiritless_po/PoParser.h"

using namespace std;
using namespace spiritless_po;


bool equal(const PoParser::PoEntryT &a, const PoParser::PoEntryT &b)
{
    return a.msgid == b.msgid && a.msgstr == b.msgstr && a.error == b.error;
}

PoParser::PoEntryT create(const string &msgid, const vector<string> &msgstr, const string &error)
{
    PoParser::PoEntryT a;
    a.msgid = msgid;
    a.msgstr = msgstr;
    a.error = error;
    return a;
}

void dump_PO_entry(const vector<PoParser::PoEntryT> &entries)
{
    size_t n = 0;
    for (auto &ent : entries) {
        cout << n << ":\n";
        auto pos = ent.msgid.find("\04");
        if (pos == ent.msgid.npos) {
            cout << "  msgid: \"" << ent.msgid << "\"\n";
        } else {
            cout << "  msgctxt: \"" << ent.msgid.substr(0, pos) << "\"\n";
            cout << "  msgid: \"" << ent.msgid.substr(pos + 1) << "\"\n";
        }
        size_t pn = 0;
        for (auto &s : ent.msgstr) {
            cout << "  msgstr[" << pn << "]: \"" << s << "\"\n";
            ++pn;
        }
        cout << "  error: \"" << ent.error << "\"\n";
        ++n;
    }
}


const string test_data = R"(# translator-comments
#. extracted-comment
#: references
#, flags
#| msgid previous-untranslated-string
msgid ""
msgstr "Project-Id-Version: test-data\n"

msgid "apples"
msgstr "APPLES"

msgid "bananas"
msgstr "BANANAS"

msgid "corn"
msgid_plural "corns"
msgstr[0] "CORN#0"
msgstr[1] "CORN#1"
msgstr[2] "CORN#2"
msgstr[3] "CORN#3"

msgctxt "food"
msgid "eggs"
msgstr "EGGS"

msgctxt "food"
msgid "garlic"
msgid_plural "garlics"
msgstr[0] "GARLIC#0"
msgstr[1] "GARLIC#1"
msgstr[2] "GARLIC#2"
msgstr[3] "GARLIC#3"
msgstr[4] "GARLIC#4"

msgid "apples"
msgstr "Apples"
)";

TEST_CASE( "Normal PO Entries", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data.begin(), test_data.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 7 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("", { "Project-Id-Version: test-data\n" }, "")) );
        REQUIRE( equal(entries[1], create("apples", { "APPLES" }, "")) );
        REQUIRE( equal(entries[2], create("bananas", { "BANANAS" }, "")) );
        REQUIRE( equal(entries[3], create("corn", { "CORN#0", "CORN#1", "CORN#2", "CORN#3" }, "")) );
        REQUIRE( equal(entries[4], create("food\04eggs", { "EGGS" }, "")) );
        REQUIRE( equal(entries[5], create("food\04garlic", { "GARLIC#0", "GARLIC#1", "GARLIC#2", "GARLIC#3", "GARLIC#4" }, "")) );
        REQUIRE( equal(entries[6], create("apples", { "Apples" }, "")) );
    }
    SECTION( "no errors" ) {
        REQUIRE( entries[0].error.size() == 0 );
        REQUIRE( entries[1].error.size() == 0 );
        REQUIRE( entries[2].error.size() == 0 );
        REQUIRE( entries[3].error.size() == 0 );
        REQUIRE( entries[4].error.size() == 0 );
        REQUIRE( entries[5].error.size() == 0 );
        REQUIRE( entries[6].error.size() == 0 );
    }
}


const string test_data_fuzzy = R"(
# fuzzy
msgid "apples"
msgstr "APPLES"

# comment
#, fuzzy
# comment
msgid "bananas"
msgstr "BANANAS"

# comment
#, fuzzy, c-format
# comment
msgid "corn"
msgid_plural "corns"
msgstr[0] "CORN#0"
msgstr[1] "CORN#1"
msgstr[2] "CORN#2"
msgstr[3] "CORN#3"

# comment
#, c-format, fuzzy
# comment
msgctxt "food"
msgid "eggs"
msgstr "EGGS"
)";

TEST_CASE( "Fuzzy PO Entries", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_fuzzy.begin(), test_data_fuzzy.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 4 );
    }
    SECTION( "fuzzy msgstr is empty" ) {
        REQUIRE( equal(entries[0], create("apples", { "APPLES" }, "")) );
        REQUIRE( equal(entries[1], create("bananas", { "" }, "")) );
        REQUIRE( equal(entries[2], create("corn", { "", "CORN#1", "CORN#2", "CORN#3" }, "")) );
        REQUIRE( equal(entries[3], create("food\04eggs", { "" }, "")) );
    }
    SECTION( "no errors" ) {
        REQUIRE( entries[0].error.size() == 0 );
        REQUIRE( entries[1].error.size() == 0 );
        REQUIRE( entries[2].error.size() == 0 );
        REQUIRE( entries[3].error.size() == 0 );
    }
}


const string test_data_comment = R"(
# comment

msgid "apples"
msgstr "APPLES"

# comment

#, c-format
msgid "bananas"
msgstr "BANANAS"

# comment
msgid "corn"
msgid_plural "corns"
msgstr[0] "CORN#0"
msgstr[1] "CORN#1"
msgstr[2] "CORN#2"
msgstr[3] "CORN#3"

#, fuzzy


msgctxt "food"
msgid "eggs"
msgstr "EGGS"

msgctxt "food"
msgid "garlic"
msgid_plural "garlics"
msgstr[0] "GARLIC#0"
msgstr[1] "GARLIC#1"
msgstr[2] "GARLIC#2"
msgstr[3] "GARLIC#3"
msgstr[4] "GARLIC#4"
# comment

#~ msgid "aaa"
)";

TEST_CASE( "Comment in PO Entries", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_comment.begin(), test_data_comment.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 5 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("apples", { "APPLES" }, "")) );
        REQUIRE( equal(entries[1], create("bananas", { "BANANAS" }, "")) );
        REQUIRE( equal(entries[2], create("corn", { "CORN#0", "CORN#1", "CORN#2", "CORN#3" }, "")) );
        REQUIRE( equal(entries[3], create("food\04eggs", { "EGGS" }, "")) );
        REQUIRE( equal(entries[4], create("food\04garlic", { "GARLIC#0", "GARLIC#1", "GARLIC#2", "GARLIC#3", "GARLIC#4" }, "")) );
    }
    SECTION( "no errors" ) {
        REQUIRE( entries[0].error.size() == 0 );
        REQUIRE( entries[1].error.size() == 0 );
        REQUIRE( entries[2].error.size() == 0 );
        REQUIRE( entries[3].error.size() == 0 );
        REQUIRE( entries[4].error.size() == 0 );
    }
}


const string test_data_empty = R"(
# empty
#, empty
# empty
)";

TEST_CASE( "Empty stream in PO Entries", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_empty.begin(), test_data_empty.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 0 );
    }
}


const string test_data_errors_1 = R"(
msgstr "APPLES"

msgid_plural "corns"

msgstr[0] "CORNS#0"
)";

TEST_CASE( "Errors in PO Entries (1/2)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_errors_1.begin(), test_data_errors_1.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 3 );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
        REQUIRE( entries[1].error.size() > 0 );
        REQUIRE( entries[2].error.size() > 0 );
    }
}


const string test_data_errors_2 = R"(
msgid "apples"
msgctxt "food"
msgstr "APPLES"

msgid "bananas"
msgstr[0] "BANANAS"

msgid_plural "corns"
msgid "corn"
msgstr[0] "CORNS#0"

msgid "hops"

msgctxt "food"
msgstr "Apples"

msgid "garlic"
msgid_plural "garlics"
msgstr "GARLIC#0"

msgid "eggs")";

TEST_CASE( "Errors in PO Entries (2/2)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_errors_2.begin(), test_data_errors_2.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 8 );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
        REQUIRE( entries[1].error.size() > 0 );
        REQUIRE( entries[2].error.size() > 0 );
        REQUIRE( entries[3].error.size() > 0 );
        REQUIRE( entries[4].error.size() > 0 );
        REQUIRE( entries[5].error.size() > 0 );
        REQUIRE( entries[6].error.size() > 0 );
        REQUIRE( entries[7].error.size() > 0 );
    }
}


const string test_data_multi_line_string = R"(
msgid "apples"
msgstr ""
"APPLES"

msgid "bananas"
msgstr "BANANAS\n"
"BANANAS\n"
"BANANAS\n"
"BANANAS\n"
"BANANAS\n"

"BANANAS"

msgid ""
"corn"
msgid_plural "corns\n"
"corns"
msgstr[0] ""
"CORNS#0"
msgstr[1] "CORNS#1\n"
"CORNS#1"

msgid ""
"garlics\n"
"garlics\n"
"garlics"
msgstr ""
"GARLICS\n"
"GARLICS"

msgctxt ""
"food"
msgid "eggs"
msgstr "EGGS"

msgctxt ""
"food\n"
"food"
msgid "hops"
msgstr "HOPS"
)";

TEST_CASE( "Multi line strings in PO Entries", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_multi_line_string.begin(), test_data_multi_line_string.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 7 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("apples", { "APPLES" }, "")) );
        REQUIRE( equal(entries[1], create("bananas", { "BANANAS\nBANANAS\nBANANAS\nBANANAS\nBANANAS\n" }, "")) );
        REQUIRE( equal(entries[3], create("corn", { "CORNS#0", "CORNS#1\nCORNS#1" }, "")) );
        REQUIRE( equal(entries[4], create("garlics\ngarlics\ngarlics", { "GARLICS\nGARLICS" }, "")) );
        REQUIRE( equal(entries[5], create("food\04eggs", { "EGGS" }, "")) );
        REQUIRE( equal(entries[6], create("food\nfood\04hops", { "HOPS" }, "")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() == 0 );
        REQUIRE( entries[1].error.size() == 0 );
        REQUIRE( entries[2].error.size() > 0 );
        REQUIRE( entries[3].error.size() == 0 );
        REQUIRE( entries[4].error.size() == 0 );
        REQUIRE( entries[5].error.size() == 0 );
        REQUIRE( entries[6].error.size() == 0 );
    }
}


const string test_data_escape_sequence = R"(
msgid "apple"
msgstr "a\\b\ac\bd\fe\ng\rh\ti\vj\zk\033l\1111m\xABCDEFG"
)";

TEST_CASE( "Escape Sequence", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_escape_sequence.begin(), test_data_escape_sequence.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("apple", { "a\\b\ac\bd\fe\ng\rh\ti\vjzk\x1bl\x49" "1m\xefG" }, "")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() == 0 );
    }
}


const string test_data_no_quote_at_beginning_of_text_1 = R"(
msgid xapples"
msgstr "APPLES"

msgid "bananas"
msgstr "BANANAS"
)";

TEST_CASE( "No quote at beginning of text (1/2)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_no_quote_at_beginning_of_text_1.begin(), test_data_no_quote_at_beginning_of_text_1.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 2 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[1], create("bananas", { "BANANAS" }, "")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
        REQUIRE( entries[1].error.size() == 0 );
    }
}


const string test_data_no_quote_at_beginning_of_text_2 = R"(
msgid ""
xapples"
msgstr "APPLES"

msgid "bananas"
msgstr "BANANAS"
)";

TEST_CASE( "No quote at beginning of text (2/2)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_no_quote_at_beginning_of_text_2.begin(), test_data_no_quote_at_beginning_of_text_2.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 2 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[1], create("bananas", { "BANANAS" }, "")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
        REQUIRE( entries[1].error.size() == 0 );
    }
}


const string test_data_eof_1 = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
msgstr[0] "str[0]\t"
"str[0]"
)";
TEST_CASE( "EOF in PO Entries (1/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_1.begin(), test_data_eof_1.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("ctxt\140ctxt\04id\x40id", { "str[0]\tstr[0]" }, "")) );
    }
    SECTION( "no errors" ) {
        REQUIRE( entries[0].error.size() == 0 );
    }
}


const string test_data_eof_2 = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
msgstr[0] "str[0]\t"
"str[0]")";
TEST_CASE( "EOF in PO Entries (2/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_2.begin(), test_data_eof_2.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("ctxt\140ctxt\04id\x40id", { "str[0]\tstr[0]" }, "")) );
    }
    SECTION( "no errors" ) {
        REQUIRE( entries[0].error.size() == 0 );
    }
}


const string test_data_eof_3 = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
msgstr[0] "str[0]\t"
"str[0])";
TEST_CASE( "EOF in PO Entries (3/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_3.begin(), test_data_eof_3.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("ctxt\140ctxt\04id\x40id", {}, "12,8: This text has no terminator.")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
    }
}


const string test_data_eof_4 = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
msgstr[0] "str[0]\t"
")";
TEST_CASE( "EOF in PO Entries (4/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_4.begin(), test_data_eof_4.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("ctxt\140ctxt\04id\x40id", {}, "12,2: This text has no terminator.")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
    }
}


const string test_data_eof_5 = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
msgstr[0] "str[0]\t)";
TEST_CASE( "EOF in PO Entries (5/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_5.begin(), test_data_eof_5.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("ctxt\140ctxt\04id\x40id", {}, "11,20: This text has no terminator.")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
    }
}


const string test_data_eof_6 = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
msgstr[0] "str[0]\)";
TEST_CASE( "EOF in PO Entries (6/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_6.begin(), test_data_eof_6.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("ctxt\140ctxt\04id\x40id", {}, "11,19: This text has no terminator.")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
    }
}


const string test_data_eof_7 = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
msgstr[0] )";
TEST_CASE( "EOF in PO Entries (7/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_7.begin(), test_data_eof_7.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("ctxt\140ctxt\04id\x40id", {}, "11,11: '\"' is expected.")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
    }
}


const string test_data_eof_8 = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
msgstr[0)";
TEST_CASE( "EOF in PO Entries (8/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_8.begin(), test_data_eof_8.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("ctxt\140ctxt\04id\x40id", {}, "11,9: ']' is expected.")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
    }
}


const string test_data_eof_9 = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
msgstr[)";
TEST_CASE( "EOF in PO Entries (9/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_9.begin(), test_data_eof_9.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("ctxt\140ctxt\04id\x40id", {}, "11,8: '0'..'9' is expected.")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
    }
}


const string test_data_eof_10 = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
msgstr)";
TEST_CASE( "EOF in PO Entries (10/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_10.begin(), test_data_eof_10.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("ctxt\140ctxt\04id\x40id", {}, "11,7: 'msgstr[n]' is expected.")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
    }
}


const string test_data_eof_11 = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
ms)";
TEST_CASE( "EOF in PO Entries (11/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_11.begin(), test_data_eof_11.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("ctxt\140ctxt\04id\x40id", {}, "11,3: 'msgstr[n]' is expected.")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
    }
}


const string test_data_eof_12 = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
m)";
TEST_CASE( "EOF in PO Entries (12/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_12.begin(), test_data_eof_12.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("ctxt\140ctxt\04id\x40id", {}, "11,2: 'msgstr[n]' is expected.")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
    }
}


const string test_data_eof_13 = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural")";
TEST_CASE( "EOF in PO Entries (13/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_13.begin(), test_data_eof_13.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("ctxt\140ctxt\04id\x40id", {}, "9,25: 'msgstr[n]' is expected.")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
    }
}


const string test_data_eof_14 = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural)";
TEST_CASE( "EOF in PO Entries (14/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_14.begin(), test_data_eof_14.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("ctxt\140ctxt\04id\x40id", {}, "9,24: This text has no terminator.")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
    }
}


const string test_data_eof_15 = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plura)";
TEST_CASE( "EOF in PO Entries (15/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_15.begin(), test_data_eof_15.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("ctxt\140ctxt\04id\x40id", {}, "9,12: 'msgstr' is expected.")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
    }
}


const string test_data_eof_16 = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgstr "str)";
TEST_CASE( "EOF in PO Entries (16/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_16.begin(), test_data_eof_16.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("ctxt\140ctxt\04id\x40id", {}, "9,12: This text has no terminator.")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
    }
}


const string test_data_eof_17 = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgst)";
TEST_CASE( "EOF in PO Entries (17/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_17.begin(), test_data_eof_17.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("ctxt\140ctxt\04id\x40id", {}, "9,6: 'msgstr' is expected.")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
    }
}


const string test_data_eof_18 = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40)";
TEST_CASE( "EOF in PO Entries (18/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_18.begin(), test_data_eof_18.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("ctxt\140ctxt\04", {}, "7,14: This text has no terminator.")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
    }
}


const string test_data_eof_19 = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x)";
TEST_CASE( "EOF in PO Entries (19/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_19.begin(), test_data_eof_19.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("ctxt\140ctxt\04", {}, "7,12: [0-9A-Fa-f] is expected.")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
    }
}


const string test_data_eof_20 = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgi)";
TEST_CASE( "EOF in PO Entries (20/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_20.begin(), test_data_eof_20.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("ctxt\140ctxt\04", {}, "7,5: 'msgid' is expected.")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
    }
}


const string test_data_eof_21 = R"(

# comment
#, flag
msgctxt "ctxt\140)";
TEST_CASE( "EOF in PO Entries (21/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_21.begin(), test_data_eof_21.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("", {}, "5,18: This text has no terminator.")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
    }
}


const string test_data_eof_22 = R"(

# comment
#, flag
msgctxt "ctxt\)";
TEST_CASE( "EOF in PO Entries (22/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_22.begin(), test_data_eof_22.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("", {}, "5,15: This text has no terminator.")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
    }
}


const string test_data_eof_23 = R"(

# comment
#, flag
msgctx)";
TEST_CASE( "EOF in PO Entries (23/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_23.begin(), test_data_eof_23.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("", {}, "5,7: An unknown keyword is found.")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
    }
}


const string test_data_eof_24 = R"(

# comment
#, flag)";
TEST_CASE( "EOF in PO Entries (24/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_24.begin(), test_data_eof_24.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 0 );
    }
}


const string test_data_eof_25 = R"(

# comment
#,)";
TEST_CASE( "EOF in PO Entries (25/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_25.begin(), test_data_eof_25.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 0 );
    }
}


const string test_data_eof_26 = R"(

# comment)";
TEST_CASE( "EOF in PO Entries (26/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_26.begin(), test_data_eof_26.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 0 );
    }
}


const string test_data_eof_27 = R"(

#)";
TEST_CASE( "EOF in PO Entries (27/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_27.begin(), test_data_eof_27.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 0 );
    }
}


const string test_data_eof_28 = R"(
)";
TEST_CASE( "EOF in PO Entries (28/28)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_28.begin(), test_data_eof_28.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 0 );
    }
}


const string test_data_eof_in_error_recovering = R"(
msgstr "APPLES"
msgstr "BANANAS"
)";
TEST_CASE( "EOF in error recovering", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_in_error_recovering.begin(), test_data_eof_in_error_recovering.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("", {}, "2,7: 'msgid' is expected.")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
    }
}
