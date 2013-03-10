/*
 * InternalDatabaseWriter.h
 * SQLite 3 database manager
 *
 *  Created on: 9 mai 2013
 *      Author: oc
 */

#ifndef _INTERNAL_DATABASE_WRITER_H_
#define _INTERNAL_DATABASE_WRITER_H_

#include "globals.h"
#include "IEC61162/IEC-61162.h"
#include <sqlite3.h>
#include <cstdarg>
#include <csignal>
#include <ctime>

#define NO_TRANSACTION_IN_PROGRESS  0xFFFFFFFF
#define MODULE_FLAG FLAG_IDB_WRITER

// the enum is outside the class because else the compiler (g++ (Debian 4.4.5-8) 4.4.5)
// refuses the operators:
// InternalDatabase.h:28: error: postfix ‘InternalDatabase::SQL_Statement& InternalDatabase::operator++(InternalDatabase::SQL_Statement&)’ must take ‘int’ as its argument
// InternalDatabase.h:41: error: ‘InternalDatabase::SQL_Statement InternalDatabase::operator++(InternalDatabase::SQL_Statement&, int)’ must take either zero or one argument

enum SQL_Statement {
     insert_into_travels_positions
    ,insert_update_vessels
    ,insert_into_SAR_aircrafts_positions
    ,insert_into_travels
    ,insert_update_into_vessels_19
    ,insert_update_name_into_vessels
    ,update_vessels_24B
    ,last_sql_statement
};

// prefix (++e)
inline enum SQL_Statement& operator++(enum SQL_Statement &e) {
    switch(e) {
        case insert_into_travels_positions:
            e = insert_update_vessels;
            break;
        case insert_update_vessels:
            e = insert_into_SAR_aircrafts_positions;
            break;
        case insert_into_SAR_aircrafts_positions:
            e = insert_into_travels;
            break;
        case insert_into_travels:
            e= insert_update_into_vessels_19;
            break;
        case insert_update_into_vessels_19:
            e = insert_update_name_into_vessels;
            break;
        case insert_update_name_into_vessels:
            e = update_vessels_24B;
            break;
        default:
            e = last_sql_statement;
            break;
    }
  return e;
}

// postfix (e++)
inline enum SQL_Statement operator++(enum SQL_Statement &e, int n) {
    enum SQL_Statement rVal = e;
    ++e;
    return rVal;
}

class InternalDatabaseWriter: public sigc::trackable {
    sqlite3 *database;
    sqlite3_stmt *SQLStatements[last_sql_statement];
    int openDatabase(const char *fileName,bool create = true);
    int BuildPreparedStatements();

    // transactions management (used for performances purposes)
    friend class InternalDatabase_tests;
    bool transactionsEnabled;

    timer_t transactionTimeOutCommit;
    int initializeTransactionTimeOutTimer();
    int resetTransactionTimeOutTimer();
    friend void onTransactionCommitTimeout (union sigval signalValue);

    unsigned int numberOfOperationsInCurrentTransaction;
    int manageTransactionStart();
    int manageTransactionEnd(bool commit = true);

    int commitTransaction(bool commit = true) {
        int error(EXIT_SUCCESS);
        char *errorMsg = NULL;
        if (commit) {
            error = sqlite3_exec(database,"COMMIT TRANSACTION",NULL,NULL,&errorMsg);
            if (SQLITE_OK == error) {
                numberOfOperationsInCurrentTransaction = NO_TRANSACTION_IN_PROGRESS;
            } else {
                ERROR_MSG("Error during last transaction commit %d (%s), number of ops = %u",error,errorMsg,numberOfOperationsInCurrentTransaction);
                sqlite3_free(errorMsg);
            }
        } else {
            error = sqlite3_exec(database,"ROLLBACK TRANSACTION",NULL,NULL,&errorMsg);
            if (SQLITE_OK == error) {
                numberOfOperationsInCurrentTransaction = NO_TRANSACTION_IN_PROGRESS;
            } else {
                ERROR_MSG("Error during last transaction commit %d (%s), number of ops = %u",error,errorMsg,numberOfOperationsInCurrentTransaction);
                sqlite3_free(errorMsg);
            }
        }
        return error;
    }

    bool isTransactionInProgress() {
        return (numberOfOperationsInCurrentTransaction != NO_TRANSACTION_IN_PROGRESS);
    }

    class TransactionsManager {
        InternalDatabaseWriter *dbWriter;
    public:
        TransactionsManager(InternalDatabaseWriter *writer)
            :dbWriter(writer) {
            dbWriter->manageTransactionStart();
        }
        ~TransactionsManager() {
            dbWriter->manageTransactionEnd();
        }
    };

    // database data writer methods
    int setVesselData(IEC61162::AISMsg5 msg);
    int setVesselData(IEC61162::AISMsg19 msg);
    int setTravelsData(IEC61162::AISMsg5 msg);
    int setTravelsData(IEC61162::AISMsg19 msg);
    int fillRefCountries();
    int fillRefTypeOfElectronicPositionFixingDevice();
    int fillReferencesTables();
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

    InternalDatabaseWriter();
    ~InternalDatabaseWriter();

    void enableTransactions(bool enable = true) {
        transactionsEnabled = enable;
    }

    void onMessage1(IEC61162::AISMsg1 msg,void *param); // Position report
    void onMessage2(IEC61162::AISMsg2 msg,void *param); // Position report
    void onMessage3(IEC61162::AISMsg3 msg,void *param); // Position report
    void onMessage4(IEC61162::AISMsg4 msg,void *param); // Base station report (TODO ?)
    void onMessage5(IEC61162::AISMsg5 msg,void *param); // Static and voyage related data
    void onMessage6(IEC61162::AISMsg6 msg,void *param); // Binary addressed message (TODO ?)
    void onMessage7(IEC61162::AISMsg7 msg,void *param); // Binary acknowledgement (TODO ?)
    void onMessage8(IEC61162::AISMsg8 msg,void *param); // Binary broadcast message (TODO ?)
    void onMessage9(IEC61162::AISMsg9 msg,void *param); // Standard SAR aircraft position report (TODO)
    void onMessage10(IEC61162::AISMsg10 msg,void *param); // UTC and date inquiry (TODO ?)
    void onMessage11(IEC61162::AISMsg11 msg,void *param); // UTC/date response (TODO ?)
    void onMessage12(IEC61162::AISMsg12 msg,void *param); // Addressed safety related message (TODO ?)
    void onMessage13(IEC61162::AISMsg13 msg,void *param); // Safety related acknowledge (TODO ?)
    void onMessage14(IEC61162::AISMsg14 msg,void *param); // Safety related broadcast message (TODO ?)
    void onMessage15(IEC61162::AISMsg15 msg,void *param); // Interrogation (TODO ?)
    //void onMessage16(IEC61162::AISMsg16 msg,void *param); // Assigned mode command (TODO ?)
    //void onMessage17(IEC61162::AISMsg17 msg,void *param); // GNSS broadcast binary message (TODO ?)
    void onMessage18(IEC61162::AISMsg18 msg,void *param); // Standard Class B equipment position report
    void onMessage19(IEC61162::AISMsg19 msg,void *param); // Extended Class B equipment position report
    //void onMessage20(IEC61162::AISMsg20 msg,void *param); // Data link management message (TODO ?)
    //void onMessage21(IEC61162::AISMsg21 msg,void *param); // Aids-to-navigation report (AtoN) (TODO ?)
    //void onMessage22(IEC61162::AISMsg22 msg,void *param); // Channel management (TODO ?)
    //void onMessage23(IEC61162::AISMsg23 msg,void *param); // Group Assignment Command (TODO ?)
    void onMessage24A(IEC61162::AISMsg24A msg,void *param); // Static data report
    void onMessage24B(IEC61162::AISMsg24B msg,void *param); // Static data report
    //void onMessage25(IEC61162::AISMsg25 msg,void *param); // Single slot binary message (TODO ?)
    //void onMessage26(IEC61162::AISMsg26 msg,void *param); // Multiple slot binary message with communications state (TODO ?)
    void onMessage27(IEC61162::AISMsg27 msg,void *param); // Long-range AIS broadcast message

    void onTimeOut(bool commit = true) {
        const int error = commitTransaction(commit);
        DEBUG_MSG("Time Out Commit (error = %d)",error);
    }
};

inline std::ostream& operator<< (std::ostream &o,const InternalDatabaseWriter::exception &ex) {
  return o << "InternalDatabaseWriter::Exception : " << ex.what() << " code = " << ex.code();
}

#undef MODULE_FLAG
#endif /* _INTERNAL_DATABASE_WRITER_H_ */
