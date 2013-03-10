/*
 * Configuration.h
 *
 *  Created on: 6 mai 2013
 *      Author: oc
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include "globals.h"
#include "tools.h"
#include <string.h>
#include <vector>
#include <ctime>

#define DATABASE_DEFAULT_NAME   "iec61162.dat"
#define MODULE_FLAG FLAG_INIT
#define NO_QUERY_LIMIT          (0xFFFFFFFF)

class Configuration {
    std::string source;
    std::string database;
    time_t databaseContentTimeOut;
    unsigned int transactionSize;
    time_t  transactionMaxTime;
    unsigned int queryResultsLimit;
public:

    Configuration()
        :databaseContentTimeOut(15*60)
        ,transactionSize(30)
        ,transactionMaxTime(30)
        ,queryResultsLimit(NO_QUERY_LIMIT) {
        database = TO_STRING(LOCALSTATEDIR) "/" DATABASE_DEFAULT_NAME;
    }

    ~Configuration() {
    }

    int init(int argc, char *argv[]);

    const char *getSource() {
        DEBUG_VAR(source.c_str(),"%s");
        return source.c_str();
    }

    const char *getDatabase() {
        DEBUG_VAR(database.c_str(),"%s");
        return database.c_str();
    }

    time_t getDatabaseContentTimeOut() {
        return databaseContentTimeOut;
    }

    unsigned int getTransactionSize() {
        return transactionSize;
    }

    time_t getTransactionMaxTime() {
        DEBUG_VAR(transactionMaxTime,"%u sec");
        return transactionMaxTime;
    }

    const char *getDatabaseRefCountriesFile() {
        return TO_STRING(DATADIR)"/countries.csv";
    }

    const char *getDatabaseRefTypeOfElectronicPositionFixingDeviceFile() {
        return TO_STRING(DATADIR)"/Type_of_electronic_position_fixing_device.csv";
    }

    unsigned int getQueryResultsLimit() const {
        return queryResultsLimit;
    }

    void setQueryResultsLimit(unsigned int newLimit = NO_QUERY_LIMIT) {
        queryResultsLimit = newLimit;
    }
};

extern Configuration theConfiguration;

#undef MODULE_FLAG

#endif /* CONFIGURATION_H_ */
