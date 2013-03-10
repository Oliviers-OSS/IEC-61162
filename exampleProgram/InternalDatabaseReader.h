/*
 * InternalDatabaseReader.h
 *
 *  Created on: 8 juil. 2013
 *      Author: oc
 */

#ifndef _INTERNAL_DATABASE_READER_H_
#define _INTERNAL_DATABASE_READER_H_

#include <sqlite3.h>
#include "String.h"
#include "CorbaClientAdaptor.h"

class InternalDatabaseReader {
    sqlite3 *database;
    sqlite3_stmt *SQLQueryStatement;
    int openDatabase(const char *fileName);
    String currentBaseQuery;
    int buildPublisherView(const char *baseQuery,const unsigned int newLimit = 0);
    int buildSQLQueryStatement();
    FastMutex mutex; // object of this class are not reentrant
public:
    class exception : public std::exception {
        int error;
        std::string msg;
    public:
        exception(int code,const std::string &message) throw()
            :error(code),msg(message) {
        }
        exception(int code,const char *format,...);
        virtual ~exception() throw() {
        }
        virtual const char* what() const throw() {
            return msg.c_str();
        }
        virtual int code() const throw() {
            return error;
        }
    };

    InternalDatabaseReader();
    ~InternalDatabaseReader();

    void setQueryResultsLimit(unsigned int newLimit) {
        MutexMgr mutexMgr(mutex);
        buildPublisherView((const char *)currentBaseQuery,newLimit);
        theConfiguration.setQueryResultsLimit(newLimit);
    }

    unsigned int getQueryResultsLimit() {
        return theConfiguration.getQueryResultsLimit();
    }

    const char *getFilter() {
        return (const char *)currentBaseQuery;
    }

    void setFilter(const char *newQuery) {
        MutexMgr mutexMgr(mutex);
        buildPublisherView(newQuery);
    }

    int getQueryResults(CorbaClientAdaptor &adaptor);
};

inline std::ostream& operator<< (std::ostream &o,const InternalDatabaseReader::exception &ex) {
  return o << "InternalDatabaseReader::Exception : " << ex.what() << " code = " << ex.code();
}

#endif /* _INTERNAL_DATABASE_READER_H_ */
