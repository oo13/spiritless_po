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
