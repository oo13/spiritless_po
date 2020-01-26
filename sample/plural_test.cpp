/* sample/plural_test.cpp

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
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include "spiritless_po/PluralParser.h"

using namespace std;
using namespace spiritless_po;
using namespace spiritless_po::PluralParser;

vector<string> test_table{
" n==1 || n%10==1 ? 0 : 1",
"(n != 0)",
"(n != 1)",
"(n > 1)",
"(n%10!=1 || n%100==11)",
"(n%100==1 ? 0 : n%100==2 ? 1 : n%100==3 || n%100==4 ? 2 : 3)",
"(n%10==1 && n%100!=11 ? 0 : n != 0 ? 1 : 2)",
"(n%10==1 && n%100!=11 ? 0 : n%10>=2 && (n%100<10 || n%100>=20) ? 1 : 2)",
"(n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2)",
"(n==0 ? 0 : n==1 ? 1 : 2)",
"(n==0 ? 0 : n==1 ? 1 : n==2 ? 2 : n%100>=3 && n%100<=10 ? 3 : n%100>=11 ? 4 : 5)",
"(n==1 ? 0 : (n==0 || (n%100 > 0 && n%100 < 20)) ? 1 : 2)",
"(n==1 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2)",
"(n==1 ? 0 : n==0 || ( n%100>1 && n%100<11) ? 1 : (n%100>10 && n%100<20 ) ? 2 : 3)",
"(n==1 || n==11) ? 0 : (n==2 || n==12) ? 1 : (n > 2 && n < 20) ? 2 : 3",
"(n==1) ? 0 : (n==2) ? 1 : (n != 8 && n != 11) ? 2 : 3",
"(n==1) ? 0 : (n==2) ? 1 : (n == 3) ? 2 : 3",
"(n==1) ? 0 : (n>=2 && n<=4) ? 1 : 2",
"(n==1) ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2",
"0",
"n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2",
"n==1 ? 0 : n==2 ? 1 : (n>2 && n<7) ? 2 :(n>6 && n<11) ? 3 : 4",
	"n+3",
	"n*3+4",
	"n*n+4*n-1",
	"!n ? 3 : n/3+8",
	"!! !! ! !n ? 3 : n/3+8",
	"n==0.0 ? 0 : 1",
};


int main()
{
	for (auto &s : test_table) {
		cout << s << endl;
		auto it = s.cbegin();
		try {
			auto f = ParseExpression(it, s.end());
			cout << "0 -> " << f(0) << endl;
			cout << "1 -> " << f(1) << endl;
			cout << "2 -> " << f(2) << endl;
			cout << "3 -> " << f(3) << endl;
			cout << "4 -> " << f(4) << endl;
			cout << "5 -> " << f(5) << endl;
			cout << "6 -> " << f(6) << endl;
			cout << "7 -> " << f(7) << endl;
			cout << "8 -> " << f(8) << endl;
			cout << "9 -> " << f(9) << endl;
			cout << "10 -> " << f(10) << endl;
			cout << "11 -> " << f(11) << endl;
			cout << "12 -> " << f(12) << endl;
			cout << "13 -> " << f(13) << endl;
			cout << "14 -> " << f(14) << endl;
			cout << "15 -> " << f(15) << endl;
			cout << "19 -> " << f(19) << endl;
			cout << "20 -> " << f(20) << endl;
			cout << "21 -> " << f(21) << endl;
			cout << "22 -> " << f(22) << endl;
			cout << "23 -> " << f(23) << endl;
			cout << "24 -> " << f(24) << endl;
			cout << "25 -> " << f(25) << endl;
			cout << "100 -> " << f(100) << endl;
			cout << "101 -> " << f(101) << endl;
			cout << "102 -> " << f(102) << endl;
			cout << "103 -> " << f(103) << endl;
			cout << "104 -> " << f(104) << endl;
			cout << "111 -> " << f(111) << endl;
			cout << "113 -> " << f(113) << endl;
			cout << "123 -> " << f(123) << endl;
		} catch (ExpressionError &e) {
			cerr << e.what() << endl;
			auto it = e.Where();
			if (it == s.end()) {
				cerr << s << "][" << endl;
			} else {
				auto pos = std::distance(s.cbegin(), e.Where());
				for (size_t ii=0; ii<s.length(); ii++) {
					if (static_cast<size_t>(pos) == ii) {
						cerr << ']' << s[ii] << '[';
					} else {
						cerr << s[ii];
					}
				}
				cerr << endl;
			}
		}
		string np = "nplurals=1; plural=n;";
		try {
			auto r = Parse(np);
			cout << r.first << ": " << r.second(1) << endl;
		} catch (ExpressionError &e) {
			cerr << e.what() << endl;
			auto it = e.Where();
			if (it == np.end()) {
				cerr << s << "][" << endl;
			} else {
				auto pos = std::distance(np.cbegin(), e.Where());
				for (size_t ii=0; ii<np.length(); ii++) {
					if (static_cast<size_t>(pos) == ii) {
						cerr << ']' << np[ii] << '[';
					} else {
						cerr << np[ii];
					}
				}
				cerr << endl;
			}
		}
	}
    return 0;
}
