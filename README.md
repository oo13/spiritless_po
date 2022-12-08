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

int main(int argc, char *argv[])
{
    if (argc <= 1) {
        cerr << "This program needs one filename." << endl;
        return 1;
    }

    spiritless_po::Catalog catalog;
    for (size_t i = 0; i < static_cast<size_t>(argc) - 1; i++) {
        ifstream f(argv[i + 1]);
        catalog.ClearError();
        if (!catalog.Add(f)) {
            for (const auto &s : catalog.GetError()) {
                cerr << argv[i + 1] << ": " << s << endl;
            }
        }
    }

    cout << "Apple"
         << ": " << catalog.gettext("Apple") << endl;
    for (size_t i = 0; i < 30; i++) {
        cout << i << ": Bean"
             << ": " << catalog.ngettext("Bean", "Beans", i) << endl;
    }

    auto index = catalog.GetIndex();
    cout << "Number of msgid: " << index.size() << endl;
    return 0;
}
```

# Unit Test
This library includes some unit test codes. If you want to run it, the following programs are needed:

- Catch2 (Tested in version 2.13.10)
- cmake  (Tested in Version 3.24.3)

```
% cd spiritless_po/test
% cmake -DCMAKE_BUILD_TYPE=Release -B build .
% cd build
% make
% ./test_spiritless_po
% ./test_spiritless_po '[!benchmark]' ; # For benchmark
```
