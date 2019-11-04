#include <iostream>
#include <string>
#include <thread>

#include "fleece/Fleece.hh"
#include "fleece/Mutable.hh"
#include "CouchbaseLite.hh"

using namespace fleece;
using namespace cbl;

int main() {

    //
    // Database/Document API:
    //

    // Open a database "DB_1"
    // Database location will be the current location
    Database db("DB_1", {"", kCBLDatabase_Create});

    // Verify that database is valid
    if(!db.valid()) {
        std::cout << "\nInvalid database!" << std::endl;
        return 1;
    }

    // Create a document "Doc_1"
    MutableDocument doc_1("Doc_1");

    // Create a document "Doc_2"
    MutableDocument doc_2("Doc_2");

    // Add key "Name" with value "John" to "Doc_1"
    // Add key "Age" with value "20" to "Doc_1"
    doc_1["Name"] = "John";
    doc_1["Age"] = 20;

    // Add key "Name" with value "Mary" to "Doc_2"
    // Add key "Age" with value "30" to "Doc_2"
    doc_2["Name"] = "Mary";
    doc_2["Age"] = 30;

    // Save document "Doc_1"
    db.saveDocument(doc_1);

    // Save document "Doc_2"
    db.saveDocument(doc_2);

    std::cout << "\nSaved documents." << std::endl;

    // Get key count for "Doc_1"
    std::cout << "\nElements in the database: " << db.count() << std::endl;

    // Read back the contents of "Doc_1"
    Document d = db.getMutableDocument("Doc_1");
    Dict read_dict = d.properties();

    // Get contents of document in JSON format
    std::cout << "Contents of document in JSON format: " << read_dict.toJSONString() << std::endl;

    // Get value of key "Name"
    slice read_slice = read_dict["Name"].asString();
    std::cout << "Value of \"Name\": " << std::string(read_slice) << std::endl;

    //
    // N1QL query API:
    //

    // Get all Document ID's in the current database
    CBLError error;
    int errPos;
    CBLQuery *query = CBLQuery_New(db.ref(), kCBLN1QLLanguage, "SELECT _id", &errPos, &error);
    CBLResultSet *results = CBLQuery_Execute(query, &error);

    while (CBLResultSet_Next(results)) {
        FLValue value = CBLResultSet_ValueAtIndex(results, 0);
        slice value_sl = FLValue_AsString(value);
        std::cout << "Document ID: " << std::string(value_sl) << std::endl;
    }

    // Get all keys "Name" that match Jo% (The '%' symbols acts as a wildcard)
    std::string my_query = "SELECT Name WHERE Name LIKE 'Jo%'";
    query = CBLQuery_New(db.ref(), kCBLN1QLLanguage, my_query.c_str(), &errPos, &error);
    results = CBLQuery_Execute(query, &error);

    while (CBLResultSet_Next(results)) {
        FLValue value = CBLResultSet_ValueAtIndex(results, 0);
        slice value_sl = FLValue_AsString(value);
        std::cout << "Name: " << std::string(value_sl) << std::endl;
    }

    // Get all keys "Name" that match "Age" between 25 and 35
    my_query = "SELECT Name WHERE Age BETWEEN 25 AND 35";
    query = CBLQuery_New(db.ref(), kCBLN1QLLanguage, my_query.c_str(), &errPos, &error);
    results = CBLQuery_Execute(query, &error);

    while (CBLResultSet_Next(results)) {
        FLValue value = CBLResultSet_ValueAtIndex(results, 0);
        slice value_sl = FLValue_AsString(value);
        std::cout << "Name: " << std::string(value_sl) << std::endl;
    }

    return 0;

}