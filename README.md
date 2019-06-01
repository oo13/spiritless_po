# spiritless_po

**spiritless_po** is a kind of gettext library in C++11 and inspired by [spirit-po](https://github.com/cbeck88/spirit-po).

The only feature of this library is that it does not depend on Boost library.

I don't intend to be compatible with spirit-po.

Example:
```c++
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
	
	cout << "Apple" << ": " << catalog.gettext("Apple") << endl;
	for (size_t ii=0; ii<30; ii++) {
		cout << ii << ": Bean" << ": " << catalog.ngettext("Bean", "Beans", ii) << endl;
	}
	
	auto index = textDomain.GetIndex();
	cout << "Number of msgid: " << index.size() << endl;
	return 0;
}
```
