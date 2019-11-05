#include <iostream>
#include <string>
#include <thread>

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

void statusChanged(Replicator r, const CBLReplicatorStatus &status) {
    std::cout << "--- PROGRESS: status=" << status.activity << ", fraction=" << status.progress.fractionComplete << ", err=" << status.error.domain << "/" << status.error.code << "\n";
}

void docProgress(Replicator r, bool isPush, const std::vector<CBLReplicatedDocument, std::allocator<CBLReplicatedDocument>> documents) {
    std::cout << "--- " << documents.size() << " docs " << (isPush ? "pushed" : "pulled") << ":";
    for (unsigned i = 0; i < documents.size(); ++i) {
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
    Query query_1(db, kCBLN1QLLanguage, "SELECT _id");
    ResultSet results = query_1.execute();

    for(ResultSetIterator it = results.begin(); it != results.end(); ++it) {
        Result r = *it;
        slice value_sl = r.valueAtIndex(0).asString();
        std::cout << "Document ID: " << std::string(value_sl) << std::endl;
    }

    // Get all keys "Name" that match Jo% (The '%' symbols acts as a wildcard)
    std::string my_query = "SELECT Name WHERE Name LIKE 'Jo%'";
    Query query_2(db, kCBLN1QLLanguage, my_query.c_str());
    results = query_2.execute();

    for(ResultSetIterator it = results.begin(); it != results.end(); ++it) {
        Result r = *it;
        slice value_sl = r.valueAtIndex(0).asString();
        std::cout << "Name: " << std::string(value_sl) << std::endl;
    }

    // Get all keys "Name" that match "Age" between 25 and 35
    my_query = "SELECT Name WHERE Age BETWEEN 25 AND 35";
    Query query_3(db, kCBLN1QLLanguage, my_query.c_str());
    results = query_3.execute();

    for(ResultSetIterator it = results.begin(); it != results.end(); ++it) {
        Result r = *it;
        slice value_sl = r.valueAtIndex(0).asString();
        std::cout << "Name: " << std::string(value_sl) << std::endl;
    }

    // Close the database
    db.close();

    //
    // Replicator API:
    //

    // Open a database "DB_Rep"
    Database db_Rep("DB_Rep", {"", kCBLDatabase_Create});

    ReplicatorConfiguration config(db_Rep);

    // Set the endpoint URL to connect to
    config.endpoint.setURL(rep_url_);

    // Set the username and password for authentication
    config.authenticator.setBasic(rep_username_, rep_password_);

    // Set the replication type (push/pull/push and pull)
    config.replicatorType = kCBLReplicatorTypePull;

    // Set the channels to listen to
    // MutableArray ma = MutableArray::newArray();
    // ma.append("chan1");
    // config.channels = ma;

    // Set the document ID's to listen to
    // MutableArray ma2 = MutableArray::newArray();
    // ma2.append("Doc1_chan1");
    // config.documentIDs = ma2;

    Replicator replicator(config);

    // Connect a status changed callback function
    std::function<void(Replicator, const CBLReplicatorStatus &)> statusChangedCallback = statusChanged;
    auto ctoken = replicator.addChangeListener(statusChangedCallback);

    // Connect a document progress callback function
    std::function<void(Replicator, bool, const std::vector<CBLReplicatedDocument, std::allocator<CBLReplicatedDocument>>)> documentCallback = docProgress;
    auto dtoken = replicator.addDocumentListener(documentCallback);

    // Start replication
    replicator.start();

    while(replicator.status().activity != kCBLReplicatorStopped) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "Finished with activity = " << replicator.status().activity << ", error = (" << replicator.status().error.domain << "/" << replicator.status().error.code << ")\n";

    // Stop replication
    replicator.stop();

    // Close the database
    db_Rep.close();

    return 0;
}