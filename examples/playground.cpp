#include <iostream>
#include <string>
#include <thread>
#include <set>

#include "fleece/Fleece.hh"
#include "fleece/Mutable.hh"
#include "CouchbaseLite.hh"

using namespace fleece;
using namespace cbl;

//////////////////////
// Replicator settings
const char *rep_url_ = "ws://localhost:4984/db";
const char *rep_username_ = "";
const char *rep_password_ = "";
//////////////////////

void statusChanged(CBLReplicator *r, const CBLReplicatorStatus &status) {
    std::cout << "--- PROGRESS: status=" << status.activity << ", fraction=" << status.progress.fractionComplete << ", err=" << status.error.domain << "/" << status.error.code << "\n";
}

void docProgress(CBLReplicator *r, bool isPush, unsigned numDocuments, const CBLReplicatedDocument* documents) {
    std::cout << "--- " << numDocuments << " docs " << (isPush ? "pushed" : "pulled") << ":";
    for (unsigned i = 0; i < numDocuments; ++i) {
        std::cout << " " << documents[i].ID;
    }
    std::cout << "\n";
}

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

    std::cout << "Saved documents." << std::endl;

    // Get element count for "Doc_1"
    std::cout << "Elements in the database: " << db.count() << std::endl;

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

    db.close();

    //
    // Replicator API:
    //

    Database db_Rep("DB_Rep", {"", kCBLDatabase_Create});

    CBLReplicatorConfiguration config = {};
    CBLReplicator *repl = nullptr;
    std::set<std::string> docsNotified;

    config.database = db_Rep.ref();
    config.replicatorType = kCBLReplicatorTypePull;
    config.endpoint = CBLEndpoint_NewWithURL(rep_url_);
    config.authenticator = CBLAuth_NewBasic(rep_password_, rep_username_);
   
    repl = CBLReplicator_New(&config, &error);

    auto ctoken = CBLReplicator_AddChangeListener(repl, [](void *context, CBLReplicator *r, const CBLReplicatorStatus *status) {
        statusChanged(r, *status);
    }, nullptr);

    auto dtoken = CBLReplicator_AddDocumentListener(repl, [](void *context, CBLReplicator *r, bool isPush, unsigned numDocuments, const CBLReplicatedDocument* documents) {
        docProgress(r, isPush, numDocuments, documents);
    }, nullptr);

    CBLReplicator_Start(repl);
    CBLReplicatorStatus status;
    while ((status = CBLReplicator_Status(repl)).activity != kCBLReplicatorStopped) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "Finished with activity = " << status.activity << ", error = (" << status.error.domain << "/" << status.error.code << ")\n";

    CBLListener_Remove(ctoken);
    CBLListener_Remove(dtoken);

    CBLReplicator_Release(repl);
    CBLAuth_Free(config.authenticator);
    CBLEndpoint_Free(config.endpoint);

    return 0;
}