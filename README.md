# QBRecords

QBRecords is a class for manipulating data (QBRecord).



NB the original function QBFindMatchingRecords is renamed QBFindMatchingRecordsOrig



All times from profiler are achieved with the release version of the project

profiler: 0.0542863  time of QBFindMatchingRecordsOrig(original implementation)

profiler: 0.0339569  time of simple optimization QBFindMatchingRecordsSimpleOptimization

profiler: 0.0062284  time of QBRecords class(using chunks and threads)



main contains:

1.populateData (1000000 records are used for profiling)

2.running and profiling the original QBFindMatchingRecordsOrig



3.running and profiling the simple optimization QBFindMatchingRecordsSimpleOptimization

4.checking that the returned records are equal to records returned by QBFindMatchingRecordsOrig



5.creating QBRecords class

6.Insert data to it (the time is not included in profiler time)

7.running and profiling QBRecords::Select function



8.checking that the returned records are equal to records returned by QBFindMatchingRecordsOrig

9.checking that the data in QBRecords are equal to QBFindMatchingRecordsOrig



10.tests-deleting records: 3 fixed id are deleted in data and in QBRecords and then checked for correctness





