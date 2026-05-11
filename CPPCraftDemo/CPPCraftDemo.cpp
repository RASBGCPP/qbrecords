#include "stdafx.h"
#include "QBRecord.h"


/**
    Return records that contains a string in the StringValue field
    records - the initial set of records to filter
    matchString - the string to search for
*/
QBRecordCollection QBFindMatchingRecordsOrig(const QBRecordCollection& records, const std::string& columnName, const std::string& matchString)
{
    QBRecordCollection result;
    std::copy_if(records.begin(), records.end(), std::back_inserter(result), [&](QBRecord rec){
        if (columnName == "column0") {
            uint matchValue = std::stoul(matchString);
            return matchValue == rec.column0;
        } else if (columnName == "column1") {
            return rec.column1.find(matchString) != std::string::npos;
        } else if (columnName == "column2") {
            long matchValue = std::stol(matchString);
            return matchValue == rec.column2;
        } else if (columnName == "column3") {
            return rec.column3.find(matchString) != std::string::npos;
        } else {
            return false;
        }
    });
    return result;
}

QBRecordCollection QBFindMatchingRecordsSimpleOptimization(const QBRecordCollection& records, const std::string& columnName, const std::string& matchString)
{
    QBRecordCollection result;
    if (columnName == "column0") {
        std::copy_if(records.begin(), records.end(), std::back_inserter(result), [&](QBRecord rec) {

            uint matchValue = std::stoul(matchString);
            return matchValue == rec.column0;
            });
    }
    else if (columnName == "column1") {
        std::copy_if(records.begin(), records.end(), std::back_inserter(result), [&](QBRecord rec) {
            return rec.column1.find(matchString) != std::string::npos;
            });
    }
    else if (columnName == "column2") {
        std::copy_if(records.begin(), records.end(), std::back_inserter(result), [&](QBRecord rec) {
            long matchValue = std::stol(matchString);
            return matchValue == rec.column2;
            });
    }
    else if (columnName == "column3") {
        std::copy_if(records.begin(), records.end(), std::back_inserter(result), [&](QBRecord rec) {
            return rec.column3.find(matchString) != std::string::npos;
            });
    }
    return result;
}

/**
    Utility to populate a record collection
    prefix - prefix for the string value for every record
    numRecords - number of records to populate in the collection
*/
QBRecordCollection populateDummyData(const std::string& prefix, int numRecords)
    {
    QBRecordCollection data;
    data.reserve(numRecords);
    for (int i = numRecords-1; i >=0 ; i--)//TODO:for (uint i = 0; i < numRecords; i++)
        {
        QBRecord rec = { i, prefix + std::to_string(i), i % 100, std::to_string(i) + prefix };
        data.emplace_back(rec);
        }
    return data;
    }


void sort(QBRecordCollection& chunk)
{
    std::sort(chunk.begin(), chunk.end(), [](const QBRecord& a, const QBRecord& b) { return a.column0 < b.column0; });
}

bool equal(const QBRecordCollection& r1, const QBRecordCollection& r2)
{
    if (r1.size() != r2.size() || r1.size() == 0)
        return false;
    for (int i = 0; i < r1.size(); i++)
    {
        if (r1[i] != r2[i])
            return false;
    }
    return true;
}


bool test_delete(QBRecords<8>& qbrecords, QBRecordCollection&  data, uint id)
{
    QBRecordCollection sorted;

    qbrecords.Delete(id);
    qbrecords.Sort(sorted);

    
    for (int i = 0; i < data.size(); i++)
    {
        if (data[i].column0 == id)
        {
            data.erase(data.begin() + i);
            break;
        }

    }
    sort(data);

    return equal(data, sorted);
}

int main(int argc, _TCHAR* argv[])
{
    using namespace std::chrono;
    // populate a bunch of data
    auto data = populateDummyData("testdata", 1000000);//1000, 1000000

    //--------------------------------------------------------------------------------
    // ORIGINAL QBFindMatchingRecordsOrig
    //--------------------------------------------------------------------------------
    // Find a record that contains and measure the perf
    auto startTimer = steady_clock::now();
    auto filteredSet = QBFindMatchingRecordsOrig(data, "column1", "testdata500");
    auto filteredSet2 = QBFindMatchingRecordsOrig(data, "column2", "24");

    std::cout << "profiler: " << double((steady_clock::now() - startTimer).count()) * steady_clock::period::num / steady_clock::period::den << std::endl;
    //================================================================================


    
    //--------------------------------------------------------------------------------
    // SimpleOptimization of QBFindMatchingRecordsOrig (QBFindMatchingRecordsSimpleOptimization)
    //--------------------------------------------------------------------------------
    // Find a record that contains and measure the perf
    startTimer = steady_clock::now();
    auto filteredSet3 = QBFindMatchingRecordsSimpleOptimization(data, "column1", "testdata500");
    auto filteredSet4 = QBFindMatchingRecordsSimpleOptimization(data, "column2", "24");

    std::cout << "profiler: " << double((steady_clock::now() - startTimer).count()) * steady_clock::period::num / steady_clock::period::den << std::endl;

   
    //make sure that QBFindMatchingRecordsSimpleOptimization is working as QBFindMatchingRecordsOrig
    if (!std::equal(filteredSet.begin(), filteredSet.begin() + filteredSet.size(), filteredSet3.begin()))
        std::cout << "Error in QBFindMatchingRecordsSimpleOptimization" << std::endl;
    else
        std::cout << "QBFindMatchingRecordsSimpleOptimization OK" << std::endl;

    if (!std::equal(filteredSet2.begin(), filteredSet2.begin() + filteredSet2.size(), filteredSet4.begin()))
        std::cout << "Error in QBFindMatchingRecordsSimpleOptimization" << std::endl;
    else
        std::cout << "QBFindMatchingRecordsSimpleOptimization OK" << std::endl;
    //================================================================================
    


    //--------------------------------------------------------------------------------
    // QBRecords class -  selecting records using chunks and threads
    //--------------------------------------------------------------------------------
    QBRecords<8> qbrecords(4096);  //QBRecords<8> qbr(8192); 
    qbrecords.Insert(data);

    QBRecordCollection  selection1;
    QBRecordCollection  selection2;
    startTimer = steady_clock::now();
    qbrecords.Select(selection1, "column1", "testdata500");
    qbrecords.Select(selection2, "column2", "24");
    std::cout << "profiler: " << double((steady_clock::now() - startTimer).count()) * steady_clock::period::num / steady_clock::period::den << std::endl;

    //make sure that QBRecords::select is working as QBFindMatchingRecords
    if (!std::equal(filteredSet.begin(), filteredSet.begin() + filteredSet.size(), selection1.begin()))
        std::cout << "Error in QBRecords" << std::endl;
    else
       std::cout << "QBRecords OK" << std::endl;

    if (!std::equal(filteredSet2.begin(), filteredSet2.begin() + filteredSet2.size(), selection2.begin()))
        std::cout << "Error in QBRecords" << std::endl;
    else
        std::cout << "QBRecords OK" << std::endl;

    //
    QBRecordCollection sorted;
    qbrecords.Sort(sorted);
    sort(data);
    if (equal(data, sorted))
    {
        std::cout << "Sort OK" << std::endl;
    }
    else
    {
        std::cout << "Sort NOK" << std::endl;
    }

    //test delete a record
    if (test_delete(qbrecords, data, 99))
    {
        std::cout << "Del 99 OK" << std::endl;
    }
    else
    {
        std::cout << "Del 99 NOK" << std::endl;
    }
    if (test_delete(qbrecords, data, 409700))
    {
        std::cout << "Del 409700 OK" << std::endl;
    }
    else
    {
        std::cout << "Del 409700 NOK" << std::endl;
    }
    if (test_delete(qbrecords, data, 999500))
    {
        std::cout << "Del 999500 OK" << std::endl;
    }
    else
    {
        std::cout << "Del 999500 NOK" << std::endl;
    }
    
    QBRecordCollection  test;
    qbrecords.Select(test, "column0", "99");
    assert(test.empty());
    qbrecords.Select(test, "column0", "409700");
    assert(test.empty());
    qbrecords.Select(test, "column0", "999500");
    assert(test.empty());
    
    //================================================================================


	return 0;
}

