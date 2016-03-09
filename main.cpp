#include <iostream>
#include "DiskMultiMap.h"

using namespace std;

int main() {
    DiskMultiMap d;
    d.createNew("there.dat", 420);
    d.close();
    d.openExisting("there.dat");
    d.insert("HI FRIEND", "HI", "HELLO");
    d.close();
    DiskMultiMap e;
    e.openExisting("there.dat");
    auto iter = e.search("HI FRIEND");
    MultiMapTuple a = *iter;
    std::cout << a.key << ", " << a.value << ", " << a.context << std::endl;
    return 0;
}