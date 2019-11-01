#include <iostream>

// #include "CBLTest.hh"
// #include "CBLPrivate.h"
#include "fleece/Fleece.hh"
#include "fleece/Mutable.hh"
#include <string>
#include <thread>
#include "CouchbaseLite.hh"
using namespace std;
using namespace fleece;


int main() {
    std::cout << "Hello world." << std::endl;

    // CBLDocument* doc = CBLDocument_New("foo");

    cbl::Database db("abc", {"/tmp", kCBLDatabase_Create});

    return 0;

}