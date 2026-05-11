#pragma once
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <assert.h>
#include <chrono>
#include <iostream>
#include <ratio>
#include <thread>
#include <atomic>
#include <mutex>

#include "SignalWait.h"

//#include <windows.h>


#define INVALID_NUM  -1
#define INVALID_IND  -1

typedef unsigned int uint;

/**
    Represents a Record Object
*/
struct QBRecord
{
    uint column0; // unique id column
    std::string column1;
    long column2;
    std::string column3;

    bool operator== (const QBRecord& t)const
    {
        return column0 == t.column0 && column1 == t.column1 && column2 == t.column2 && column3 == t.column3;
    }

    bool operator!= (const QBRecord& t)const
    {
        return !operator==(t);
    }

    bool operator < (const QBRecord& t)const
    {
        return column0 < t.column0;
    }
};
/**
Represents a Record Collections
*/
typedef std::vector<QBRecord> QBRecordCollection;


struct QBPosition
{
    int chunkNum;
    int indInChunk;
    QBPosition()
    {
        chunkNum = INVALID_NUM;
        indInChunk = INVALID_IND;
    }
};

struct QBFindInfo
{
    std::atomic_int    process;
    SignalWait         finished;
    QBRecordCollection result;
    const QBRecordCollection const* records;
    std::string columnName;
    std::string matchString;
    QBFindInfo()
    {
        process = 0;
        records = nullptr;
    }
};

void QBFindMatchingRecordsInThread(QBFindInfo& info);

template<int maxNumThreads>
class QBRecords
{
protected:
    size_t mChunkSize;
    std::vector<QBRecordCollection> mChunks;
    std::map<uint, QBPosition> mIdToPos;

    //helpers, next variables are not data connected to records, they are needed to manage threads
    std::vector<std::thread> mThreads;//helper
    mutable QBFindInfo mThreadInfo[maxNumThreads];//helper - mutable because it is changed in Select, because of the treads

    void addEmptyChunk();
public:
    QBRecords(size_t chunkSize);
    ~QBRecords();
    void Insert(const QBRecord&);
    void Insert(const QBRecordCollection&);
    void Update(const QBRecord&);
    bool Delete(uint id);
    void Select(QBRecordCollection& records, const std::string& columnName, const std::string& matchString)const;
    void Sort(QBRecordCollection& records)const;
};

//template members of QBRecords
template<int maxNumThreads>
void QBRecords<maxNumThreads>::addEmptyChunk()
{
    QBRecordCollection chunk;
    chunk.reserve(mChunkSize);
    mChunks.push_back(chunk);
}

template<int maxNumThreads>
QBRecords<maxNumThreads>::QBRecords(size_t chunkSz)
{
    mChunkSize = chunkSz;
    addEmptyChunk(); //first chunk

    for (int i = 0; i < maxNumThreads; i++)
    {
        mThreadInfo[i].process = 0;
        mThreads.emplace_back(std::thread(QBFindMatchingRecordsInThread, std::ref(mThreadInfo[i])));
        std::thread& th = mThreads[mThreads.size() - 1];
        //SetThreadPriority(th.native_handle(), THREAD_PRIORITY_ABOVE_NORMAL);//THREAD_PRIORITY_NORMAL,THREAD_PRIORITY_HIGHEST
    }
}
template<int maxNumThreads>
QBRecords<maxNumThreads>::~QBRecords()
{
    for (int i = 0; i < maxNumThreads; i++)
    {
        mThreadInfo[i].process = -1;
        std::thread& th = mThreads[i];
        th.join();
    }
}

template<int maxNumThreads>
void QBRecords<maxNumThreads>::Insert(const QBRecord& r)
{
    int lastChunk = mChunks.size() - 1;
    std::vector<QBRecord>& chunk = mChunks[lastChunk];
    assert(chunk.size() <= mChunkSize - 1);

    QBPosition pos;
    pos.chunkNum = lastChunk;
    pos.indInChunk = chunk.size();
    chunk.push_back(r);
    mIdToPos.insert(std::pair<uint, QBPosition>(r.column0, pos));//r.column0 is the id of the record

    if (chunk.size() >= mChunkSize)
    {
        addEmptyChunk();
    }
}

template<int maxNumThreads>
void QBRecords<maxNumThreads>::Insert(const QBRecordCollection& rs)
{
    for (int i = 0; i < rs.size(); i++)
    {
        Insert(rs[i]);
    }
}

template<int maxNumThreads>
void QBRecords<maxNumThreads>::Update(const QBRecord& r)
{
    //r.column0 is the id of the record
    QBPosition pos = mIdToPos.find(r.column0);
    std::vector<QBRecord>& chunk = mChunks[pos.chunkNum];
    chunk[pos.indInChunk].column1 = r.column1;
    chunk[pos.indInChunk].column2 = r.column2;
    chunk[pos.indInChunk].column3 = r.column3;
}

template<int maxNumThreads>
bool QBRecords<maxNumThreads>::Delete(uint id)
{
    auto pos = mIdToPos.find(id);
    if (pos == mIdToPos.end())
        return false;

    QBPosition posDel = pos->second;

    std::vector<QBRecord>& chunk = mChunks[posDel.chunkNum];
    //move one element from last chunk to this chunk!!!
    int lastChunkInd = mChunks.size() - 1;
    std::vector<QBRecord>& lastChunk = mChunks[lastChunkInd];

    assert(0 < lastChunk.size() && lastChunk.size() < mChunkSize - 1);

    int lastRecordInd = lastChunk.size() - 1;
    QBRecord lastRecord = lastChunk[lastRecordInd];
    auto lastPos = mIdToPos.find(lastRecord.column0);

    if (lastPos == mIdToPos.end())
    {
        assert(false);//should not happen
        return false;
    }


    QBPosition posLastRecord = lastPos->second;

    //copy last value to the empty position
    chunk[posDel.indInChunk] = lastRecord;

    //update size of lastChunk
    if (lastRecordInd > 0)
    {
        mChunks[lastChunkInd].resize(lastRecordInd);//the last element is deleted
    }
    else if (mChunks.size() > 1)
    {
        //delete the last chunk...
        mChunks.resize(mChunks.size() - 1);
    }

    //update mIdToPos of moved record (lastRecord)
    mIdToPos[lastRecord.column0] = posDel;

    auto testPos = mIdToPos.find(lastRecord.column0);
    if (testPos == mIdToPos.end())
    {
        assert(false);//should not happen
        return false;
    }

    QBPosition posTestRecord = testPos->second;
    assert(posTestRecord.chunkNum == posDel.chunkNum && posTestRecord.indInChunk == posDel.indInChunk);

    return true;
}



template<int maxNumThreads>
void QBRecords<maxNumThreads>::Select(QBRecordCollection& selection, const std::string& columnName, const std::string& matchString)const
{
    int c = 0;
    while (c < mChunks.size())
    {
        int usedThreads = 0;
        for (int i = 0; i < maxNumThreads; i++)
        {
            if (c >= mChunks.size())
            {
                break;
            }
            //mThreadInfo(helper not data) if mutable and can be changed in const function!
            mThreadInfo[i].records = &mChunks[c++];

            mThreadInfo[i].columnName = columnName;
            mThreadInfo[i].matchString = matchString;
            mThreadInfo[i].result.clear();

            mThreadInfo[i].finished.Set(false);
            mThreadInfo[i].process = 1;//resume thread
            usedThreads++;
        }

        //wait all threads are finished
        for (int i = 0; i < usedThreads; i++)
        {
            mThreadInfo[i].finished.WaitUntilTrue();
            mThreadInfo[i].process = 0;
        }

        //collect all results in selection
        for (int i = 0; i < usedThreads; i++)
        {
            if (mThreadInfo[i].result.size() > 0)
            {
                std::copy(mThreadInfo[i].result.begin(), mThreadInfo[i].result.end(), std::back_inserter(selection));
            }
        }
    }

}

template<int maxNumThreads>
void QBRecords<maxNumThreads>::Sort(QBRecordCollection& records)const
{
    //TODO: every chunk sort in thread and then merge from mChunks.size() sorted arrays!
    records.clear();
    for (int i = 0; i < mChunks.size(); i++)
    {
        const QBRecordCollection& chunk = mChunks[i];

        QBRecordCollection  sortChunk;
        std::copy(chunk.begin(), chunk.end(), std::back_inserter(sortChunk));
        std::sort(sortChunk.begin(), sortChunk.end(), [](const QBRecord& a, const QBRecord& b) { return a.column0 < b.column0; });
        if (records.size() > 0)
        {
            QBRecordCollection  mergeChunk(records.size() + sortChunk.size());
            std::merge(records.begin(), records.end(), sortChunk.begin(), sortChunk.end(), mergeChunk.begin());
            records.clear();
            std::copy(mergeChunk.begin(), mergeChunk.end(), std::back_inserter(records));
        }
        else
        {
            //TODO: move copy of first sorted chunk to records outside of the loop
            std::copy(sortChunk.begin(), sortChunk.end(), std::back_inserter(records));
        }

    }
}

