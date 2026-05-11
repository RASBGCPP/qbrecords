#include "stdafx.h"
#include "QBRecord.h"

void QBFindMatchingRecordsInThread(QBFindInfo& info)
{
    while (info.process != -1)
    {
        if (info.process == 1)
        {
            {
                if (info.columnName == "column0") {
                    std::copy_if((*info.records).begin(), (*info.records).end(), std::back_inserter(info.result),
                        [&](QBRecord rec) {
                            uint matchValue = std::stoul(info.matchString);
                            return matchValue == rec.column0;
                        });
                }
                else if (info.columnName == "column1") {
                    std::copy_if((*info.records).begin(), (*info.records).end(), std::back_inserter(info.result), [&](QBRecord rec) {
                        return rec.column1.find(info.matchString) != std::string::npos;
                        });
                }
                else if (info.columnName == "column2") {
                    std::copy_if((*info.records).begin(), (*info.records).end(), std::back_inserter(info.result), [&](QBRecord rec) {
                        long matchValue = std::stol(info.matchString);
                        return matchValue == rec.column2;
                        });
                }
                else if (info.columnName == "column3") {
                    std::copy_if((*info.records).begin(), (*info.records).end(), std::back_inserter(info.result), [&](QBRecord rec) {
                        return rec.column3.find(info.matchString) != std::string::npos;
                        });
                }
            }
            info.process = 0;
            info.finished.Set(true);
        }
    }
}
