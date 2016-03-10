#include <iostream>
#include <assert.h>
#include "DiskMultiMap.h"

using namespace std;

int main() {
    DiskMultiMap d;
    assert(d.createNew("there.dat", 420));
    d.close();
    assert(d.openExisting("there.dat"));
    std::string hif("key"), hi("HI"), hello("HELLO");
    assert(d.insert(hif, hi, hello));
    d.close();
    DiskMultiMap e;
    assert(e.openExisting("there.dat"));
    auto iter = e.search(hif);
    MultiMapTuple a = *iter;
    assert(iter.isValid());
    std::cout << a.key << ", " << a.value << ", " << a.context << std::endl;
    e.insert(hif, hi, hello);
    for (auto i = e.search(hif); i.isValid(); ++i) {
        std::cout << (*iter).key << ", " << (*iter).value << ", " << (*iter).context << std::endl;
    }
    std::cout << std::hash<std::string>()(std::string("key")) << std::endl;
    return 0;
}