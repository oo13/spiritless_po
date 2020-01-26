/* sample/load.cpp

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
#include <fstream>
#include <iostream>
#include "spiritless_po/spiritless_po.h"

using namespace std;

int main(int argc, char* argv[])
{
	if (argc <= 1) {
		cerr << "This program needs one filename." << endl;
		return 1;
	}
	
	spiritless_po::Catalog catalog;
	for (size_t ii=0; ii<static_cast<size_t>(argc)-1; ii++) {
		ifstream f(argv[ii+1]);
		catalog.ClearError();
		if (!catalog.Add(f)) {
			for (const auto &s : catalog.GetError()) {
				cerr << argv[ii+1] << ": " << s << endl;
			}
		}
	}
	
	cout << "apples" << ": " << catalog.pgettext("commodity", "apples") << endl;
	for (size_t ii=0; ii<30; ii++) {
		cout << ii << ": aa" << ": " << catalog.ngettext("aa", "aas", ii) << endl;
	}
	
	auto index = catalog.GetIndex();
	cout << "Number of msgid: " << index.size() << endl;
	return 0;
}
