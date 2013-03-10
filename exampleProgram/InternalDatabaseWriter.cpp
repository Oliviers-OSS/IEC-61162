/*
 * InternalDatabase.cpp
 *
 *  Created on: 9 mai 2013
 *      Author: oc
 */

#include "globals.h"
#include "InternalDatabaseWriter.h"
#include "tools.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cmath>

#define MODULE_FLAG FLAG_IDB_WRITER
#include "DelimitedFile.h"

static inline const char* toString(enum SQL_Statement &e) {
    const char *string = "";
#define DECLARE_ENUM(e) case e: string=TO_STRING(e); break
    switch(e) {
        DECLARE_ENUM(insert_into_travels_positions);
        DECLARE_ENUM(insert_update_vessels);
        DECLARE_ENUM(insert_into_SAR_aircrafts_positions);
        DECLARE_ENUM(insert_into_travels);
        DECLARE_ENUM(insert_update_into_vessels_19);
        DECLARE_ENUM(insert_update_name_into_vessels);
        DECLARE_ENUM(update_vessels_24B);
    }
#undef DECLARE_ENUM

    return string;
 }

InternalDatabaseWriter::exception::exception(int code,const char *format,...):error(code) {
    va_list optional_arguments;
    va_start(optional_arguments, format);
    vsprintf(msg,format,optional_arguments);
    va_end(optional_arguments);
}

inline int InternalDatabaseWriter::openDatabase(const char *fileName,bool create /*= true*/) {
    int error(EXIT_SUCCESS);
    error =  sqlite3_open(fileName,&database);
    if (0 == error) {
        char *errorMsg = NULL;
        if (create) {
            const char *SQLQueries[] = {
                    "CREATE TABLE _version (\"major\" INTEGER NOT NULL,\"minor\" INTEGER,\"comment\" TEXT);"
                    ,"CREATE TABLE travels (\"MMSI\" INTEGER NOT NULL,\"destination\" TEXT,\"ETA\" INTEGER,\"date\" DATETIME, PRIMARY KEY(MMSI,destination,ETA));"
                    ,"CREATE TABLE \"SAR_Aircrafts_Positions\" (\"MMSI\" INTEGER NOT NULL,\"altitude\" INTEGER,\"speedOverGround\" REAL,\"positionAccuracy\" INTEGER,\"longitude\" REAL,\"latitude\" REAL,\"courseOverGround\" REAL,\"date\" DATETIME NOT NULL);"
                    ,"CREATE TABLE Ref_Countries (id INTEGER PRIMARY KEY NOT NULL,Country TEXT);"
                    ,"CREATE TABLE vessels (\"MMSI\" INTEGER PRIMARY KEY NOT NULL,\"IMONumber\" INTEGER,\"callSign\" TEXT,\"name\" TEXT,\"country\" INTEGER REFERENCES RefCountries (id),\"type\" INTEGER,\"dimension_A\" INTEGER,\"dimension_B\" INTEGER,\"dimension_C\" INTEGER,\"dimension_D\" INTEGER,\"draught\" REAL,\"typeOfPositionDevice\" INTEGER,\"CreationTime\" DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,\"lastUpdateTime\" DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP);"
                    ,"CREATE TABLE travels_positions (\"MMSI\" INTEGER NOT NULL,\"latitude\" REAL,\"longitude\" REAL,\"navigationStatus\" INTEGER,\"rateOfTurn\" REAL,\"speedOverGround\" REAL,\"positionAccuracy\" INTEGER,\"courseOverGround\" REAL,\"trueHeading\" REAL,\"specialManoeuvreIndicator\" INTEGER,\"date\" DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP);"
                    ,"CREATE TABLE aid_to_navigation_reports (\"MMSI\" INTEGER NOT NULL,\"type\" INTEGER,\"name\" TEXT,\"positionAccuracy\" INTEGER,\"longitude\" REAL,\"latitude\" REAL,\"dimension_A\" INTEGER,\"dimension_B\" INTEGER,\"dimension_C\" INTEGER,\"dimension_D\" INTEGER,\"typeOfElectoniqueDevice\" INTEGER,\"date\" INTEGER NOT NULL DEFAULT CURRENT_TIMESTAMP,\"offPositionReport\" INTEGER,\"Virtual_AtoN\" INTEGER);"
                    ,"CREATE TABLE Ref_Type_of_electronic_position_fixing_device (id INTEGER PRIMARY KEY NOT NULL,DeviceType TEXT);"
                    ,"CREATE INDEX \"Vessels_Positions\" on travels_positions (MMSI ASC);"
                    ,"CREATE INDEX Vessels_Positions_date on travels_positions (date ASC);"
            };
            register unsigned int nbQueries(sizeof(SQLQueries)/sizeof(SQLQueries[0]));
            register const char **SQLQuery = SQLQueries;
            error = SQLITE_OK;
            while ((nbQueries >0) && (SQLITE_OK == error)) {
                error = sqlite3_exec(database,*SQLQuery,NULL,NULL,&errorMsg);
                if (likely(SQLITE_OK == error)) {
                    SQLQuery++;
                    nbQueries--;
                } else {
                    std::string exceptionMsg;
                    ERROR_MSG("error during database creation,%s error %d (%s)",*SQLQuery,error,errorMsg);
                    sprintf(exceptionMsg,"error during database creation during request %s (%s)",*SQLQuery,errorMsg);
                    sqlite3_free(errorMsg);
                    throw exception(error,exceptionMsg);
                }
            } // while ((nbQueries >0) && (SQLITE_OK == error))
        } //(create)

        // configuration
        if (SQLITE_OK == error) {
            error = sqlite3_exec(database, "PRAGMA synchronous = OFF", NULL, NULL, &errorMsg);
            if (unlikely(error != SQLITE_OK)) {
                //std::string exceptionMsg;
                WARNING_MSG("error setting asynchronous mode %d (%s)",error,errorMsg);
                //sprintf(exceptionMsg,"error during database creation: error setting asynchronous mode (%s)",errorMsg);
                sqlite3_free(errorMsg);
                //throw exception(error,exceptionMsg);
            }
            error = sqlite3_exec(database, "PRAGMA journal_mode = " SQLITE_JOURNAL_MODE, NULL, NULL, &errorMsg);
            if (unlikely(error != SQLITE_OK)) {
                //std::string exceptionMsg;
                WARNING_MSG("error setting journal mode to " SQLITE_JOURNAL_MODE " %d (%s)",error,errorMsg);
                //sprintf(exceptionMsg,"error during database creation: error setting journal mode to memory (%s)",errorMsg);
                sqlite3_free(errorMsg);
                //throw exception(error,exceptionMsg);
            }
            error = sqlite3_exec(database, "PRAGMA foreign_key = ON", NULL, NULL, &errorMsg);
            if (unlikely(error != SQLITE_OK)) {
                //std::string exceptionMsg;
                WARNING_MSG("error setting foreign key on %d (%s)",error,errorMsg);
                //sprintf(exceptionMsg,"error during database creation: error setting foreign key on (%s)",errorMsg);
                sqlite3_free(errorMsg);
                //throw exception(error,exceptionMsg);
            }
        }
    } else {
        std::string exceptionMsg;
        ERROR_MSG("error opening database %s, sqlite3_open error %d",fileName,error);
        sprintf(exceptionMsg,"error opening database %s, sqlite3_open error %d",fileName,error);
        throw exception(error,exceptionMsg);
    }
    return error;
}

int InternalDatabaseWriter::BuildPreparedStatements() {
    int error(EXIT_SUCCESS);
    const char *tail = NULL;
    const char *SQLQueries[] = {
             "insert into travels_positions (MMSI,latitude,longitude,navigationStatus,rateOfTurn,speedOverGround,positionAccuracy,courseOverGround,trueHeading,specialManoeuvreIndicator,date) values (@Id,@Lat,@Long,@NavStatus,@ROT,@SOG,@PosAcc,@COG,@TrueHeading,@SMI,datetime())" // insert_into_travels_positions, Msg 1,2,3,18,19,27
            ,"INSERT OR REPLACE INTO vessels (MMSI, IMONumber,callSign,name,country,type,dimension_A,dimension_B,dimension_C,dimension_D,draught,typeOfPositionDevice,CreationTime) VALUES (@Id,@IMO,@CallSign,@Name,@country,@Type,@DimA,@DimB,@DimC,@DimD,@Draught,@TypePosDev,COALESCE((SELECT CreationTime FROM vessels WHERE MMSI=@Id),datetime()))" // insert_update_vessels, Msg 5
            ,"insert into SAR_Aircrafts_Positions (MMSI, altitude, speedOverGround, positionAccuracy, longitude, latitude, courseOverGround, date) values (@Id,@Alt,@SOG,@PosAcc,@Long,@Lat,@COG,datetime())" // insert_into_SAR_aircrafts_positions
            ,"insert into travels (MMSI,destination,ETA,date) values(@Id,@Dest,@ETA,datetime())" // insert_into_travels
            ,"INSERT OR REPLACE INTO vessels (MMSI,name,type,dimension_A,dimension_B,dimension_C,dimension_D,typeOfPositionDevice,CreationTime) values(@Id,@name,@Type,@DimA,@DimB,@DimC,@DimD,@typePoseDevice,COALESCE((SELECT CreationTime FROM vessels WHERE MMSI=@Id),datetime()))" // insert_update_into_vessels_19 Msg19
            ,"INSERT OR REPLACE INTO vessels (MMSI, name,CreationTime) VALUES (@Id,@Name,COALESCE((SELECT CreationTime FROM vessels WHERE MMSI=@Id),datetime()))" // insert_update_name_into_vessels,  Msg 24A
            ,"INSERT OR REPLACE INTO vessels (MMSI,type,callSign,dimension_A,dimension_B,dimension_C,dimension_D,CreationTime) values(@Id,@Type,@CallSign,@DimA,@DimB,@DimC,@DimD,COALESCE((SELECT CreationTime FROM vessels WHERE MMSI=@Id),datetime()))" // update_vessels_24B
            ,""
    };
    SQL_Statement currentStatement = insert_into_travels_positions;
    register const char **SQLQueryCursor = SQLQueries;
    register const char *SQLQuery = *SQLQueryCursor;
    error = SQLITE_OK;
    while ((SQLQuery[0] != '\0') && (SQLITE_OK == error)) {
        error = sqlite3_prepare_v2(database,SQLQuery,strlen(SQLQuery),&SQLStatements[currentStatement],&tail);
        if (SQLITE_OK == error) {
            SQLQueryCursor++;
            SQLQuery = *SQLQueryCursor;
            ++currentStatement;
        } else {
            std::string exceptionMsg;
            ERROR_MSG("SQL prepare request %s error %d at %s",SQLQuery,error,tail);
            sprintf(exceptionMsg,"SQL prepare request %s error %d at %s",SQLQuery,error,tail);
            throw exception(error,exceptionMsg);
        }
    } //while ((SQLQuery[0] != '\0') && (SQLITE_OK == error))
    return error;
}

inline int InternalDatabaseWriter::fillRefCountries() {
    int error(EXIT_SUCCESS);
    const char *SQLQuery = "insert into Ref_Countries(id,Country) values(@id,@country)";
    const char *tail = NULL;
    sqlite3_stmt *insertSQLStatement = NULL;
    error = sqlite3_prepare_v2(database,SQLQuery,strlen(SQLQuery),&insertSQLStatement,&tail);
    if (SQLITE_OK == error) {
        const char *filename = theConfiguration.getDatabaseRefCountriesFile();
        try {
            DelimitedFile<char,4096> file(filename);
            char line[255];
            ssize_t n = file.getLineEx(line,sizeof(line),"\n","");
            while ((n > 0) && (SQLITE_OK == error)) {
                char *separator = strchr(line,';');
                if (separator) {
                    *separator = '\0';
                    char *country = separator+1;
                    if ('"' == *country) {
                        country++;
                        char *eol = line + n -1;
                        if ('"' == *eol) {
                            *eol = '\0';
                        }
                    }
                    const int id = atoi(line);
                    error = sqlite3_bind_int(insertSQLStatement,1,id);
                    if (SQLITE_OK == error) {
                        const size_t n = strlen(country);
                        error = sqlite3_bind_text(insertSQLStatement, 2, country, n, SQLITE_TRANSIENT);
                        if (SQLITE_OK == error) {
                            error = sqlite3_step(insertSQLStatement);
                            if (SQLITE_DONE == error) {
                                error = SQLITE_OK;
                            } else {
                                ERROR_MSG("sqlite3_step insert (%d,%s) into Ref_Countries error %d (%s)",id,country,error,sqlite3_errmsg(database));
                            }
                            sqlite3_clear_bindings(insertSQLStatement); /* Clear bindings */
                            sqlite3_reset(insertSQLStatement);          /* Reset VDBE */
                        } else {
                            ERROR_MSG("sqlite3_bind_text error %d",error);
                        }
                    } else {
                        ERROR_MSG("sqlite3_bind_int error %d",error);
                    }
                } else {
                    WARNING_MSG("invalid line %s in file %s",line,filename);
                }
                n = file.getLineEx(line,sizeof(line),"\n","");
            } //while ((n > 0) && (SQLITE_OK == error))
            if (SQLITE_OK == error) {
               error = EXIT_SUCCESS;
            }
        } //try
        catch(DelimitedFile<char,4096>::exception &e) {
            error = e.code();
            ERROR_MSG("Error %s (%d) with file %s",e.what(),e.code(),filename);
        }
        sqlite3_finalize(insertSQLStatement);
    } else {
      ERROR_MSG("SQL prepare request %s error %d at %s",SQLQuery,error,tail);
    }
    return error;
}

inline int InternalDatabaseWriter::fillRefTypeOfElectronicPositionFixingDevice() {
    int error(EXIT_SUCCESS);
    const char *SQLQuery = "insert into Ref_Type_of_electronic_position_fixing_device(id,DeviceType) values(@id,@DeviceType)";
    const char *tail = NULL;
    sqlite3_stmt *insertSQLStatement = NULL;
    error = sqlite3_prepare_v2(database,SQLQuery,strlen(SQLQuery),&insertSQLStatement,&tail);
    if (SQLITE_OK == error) {
        const char *filename = theConfiguration.getDatabaseRefTypeOfElectronicPositionFixingDeviceFile();
        try {
            DelimitedFile<char,4096> file(filename);
            char line[255];
            ssize_t n = file.getLineEx(line,sizeof(line),"\n","");
            while ((n > 0) && (SQLITE_OK == error)) {
                char *separator = strchr(line,';');
                if (separator) {
                    *separator = '\0';
                    char *deviceType = separator+1;
                    if ('"' == *deviceType) {
                        deviceType++;
                        char *eol = line + n -1;
                        if ('"' == *eol) {
                            *eol = '\0';
                        }
                    }
                    const int id = atoi(line);
                    error = sqlite3_bind_int(insertSQLStatement,1,id);
                    if (SQLITE_OK == error) {
                        const size_t n = strlen(deviceType);
                        error = sqlite3_bind_text(insertSQLStatement, 2, deviceType, n, SQLITE_TRANSIENT);
                        if (SQLITE_OK == error) {
                            error = sqlite3_step(insertSQLStatement);
                            if (SQLITE_DONE == error) {
                                error = SQLITE_OK;
                            } else {
                                ERROR_MSG("sqlite3_step insert (%d,%s) into Ref_Type_of_electronic_position_fixing_device error %d (%s)",id,deviceType,error,sqlite3_errmsg(database));
                            }
                            sqlite3_clear_bindings(insertSQLStatement); /* Clear bindings */
                            sqlite3_reset(insertSQLStatement);          /* Reset VDBE */
                        } else {
                            ERROR_MSG("sqlite3_bind_text error %d",error);
                        }
                    } else {
                        ERROR_MSG("sqlite3_bind_int error %d",error);
                    }
                } else {
                    WARNING_MSG("invalid line %s in file %s",line,filename);
                }
                n = file.getLineEx(line,sizeof(line),"\n","");
            } //while ((n > 0) && (SQLITE_OK == error))
            if (SQLITE_OK == error) {
               error = EXIT_SUCCESS;
            }
        } //try
        catch(DelimitedFile<char,4096>::exception &e) {
            error = e.code();
            ERROR_MSG("Error %s (%d) with file %s",e.what(),e.code(),filename);
        }
        sqlite3_finalize(insertSQLStatement);
    } else {
      ERROR_MSG("SQL prepare request %s error %d at %s",SQLQuery,error,tail);
    }
    return error;
}

inline int InternalDatabaseWriter::fillReferencesTables() {
    int error;
    char *errorMsg = NULL;
    error = sqlite3_exec(database,"BEGIN TRANSACTION",NULL,NULL,&errorMsg);
    bool transactionInProgress = (SQLITE_OK == error);
    if (!transactionInProgress) {
        WARNING_MSG("error starting the transaction to initialize the content of the database: %s",errorMsg);
    }
    error = fillRefCountries();
    if (EXIT_SUCCESS == error) {
        error = fillRefTypeOfElectronicPositionFixingDevice();
    }
    if (transactionInProgress) {
        if (EXIT_SUCCESS == error) {
            error = sqlite3_exec(database,"COMMIT TRANSACTION",NULL,NULL,&errorMsg);
        } else {
            error = sqlite3_exec(database,"ROLLBACK TRANSACTION",NULL,NULL,&errorMsg);
        }
    }
    return error;
}

InternalDatabaseWriter::InternalDatabaseWriter()
:database(NULL)
,transactionsEnabled(true)
,transactionTimeOutCommit(0)
,numberOfOperationsInCurrentTransaction(NO_TRANSACTION_IN_PROGRESS) {
    int error(EXIT_SUCCESS);
    const char *filename = theConfiguration.getDatabase();
    bool buildDatabase(true);

    try {
        enum SQL_Statement s;
        for(s=insert_into_travels_positions;s != last_sql_statement; s++) {
            SQLStatements[s] = 0;
        }

        // must build or reused a previous database ?
        struct stat databaseState;
        if (stat(filename,&databaseState) == 0) {
            // check it's time of last access
            const time_t currentTime(time(NULL));
            buildDatabase = ((currentTime - databaseState.st_atime) >= theConfiguration.getDatabaseContentTimeOut());
            DEBUG_VAR_BOOL(buildDatabase);
        } else {
            error = errno;
            if (error != ENOENT) {
                INFO_MSG("stat %s error %d (%m)",filename,error);
            }
        }

        if (buildDatabase) {
            if (unlink(filename) != 0) {
                error = errno;
                if (error != ENOENT) {
                    INFO_MSG("unlink %s error %d (%m)",filename,error);
                }
            } else {
                INFO_MSG("file %s deleted",filename);
            }
        }

        error = openDatabase(filename,buildDatabase);
        if (EXIT_SUCCESS == error) {
            error = BuildPreparedStatements();
            if (EXIT_SUCCESS == error) {
                if (buildDatabase) {
                    fillReferencesTables();
                }
            }
        } // error already printed
    } // try
    catch(...) {
        // free all allocated resources before forwarding the exception
        if (database != NULL) {
            enum SQL_Statement s;
            for(s=insert_into_travels_positions;s != last_sql_statement; s++) {
                if (SQLStatements[s] != 0) {
                    sqlite3_finalize(SQLStatements[s]);
                }
                SQLStatements[s] = 0;
            }
            sqlite3_close(database);
            database = NULL;
        }
        throw;
    }
}

InternalDatabaseWriter::~InternalDatabaseWriter() {
    if (database != NULL) {
        if (isTransactionInProgress()) {
            char *errorMsg = NULL;
            const int error = sqlite3_exec(database,"END TRANSACTION",NULL,NULL,&errorMsg);
            if (error != SQLITE_OK) {
                WARNING_MSG("Error during last transaction commit %d (%s), nb op = %u",error,errorMsg,numberOfOperationsInCurrentTransaction);
                sqlite3_free(errorMsg);
            }
        }
        enum SQL_Statement s;
        for(s=insert_into_travels_positions;s != last_sql_statement; s++) {
            sqlite3_finalize(SQLStatements[s]);
            SQLStatements[s] = 0;
        }
        sqlite3_close(database);
        database = NULL;
    }

    if (transactionTimeOutCommit != 0) {
        if (timer_delete(transactionTimeOutCommit)) {
            const int error(errno);
            ERROR_MSG("timer_delete 0x%X error %d (%m)",transactionTimeOutCommit,error);
        }
    }
}

void onTransactionCommitTimeout (union sigval signalValue);

inline int InternalDatabaseWriter::resetTransactionTimeOutTimer() {
    int error(EXIT_SUCCESS);
    struct itimerspec timerInterval;
    timerInterval.it_value.tv_sec = theConfiguration.getTransactionMaxTime();
    timerInterval.it_value.tv_nsec = 0;
    timerInterval.it_interval.tv_sec = theConfiguration.getTransactionMaxTime();
    timerInterval.it_interval.tv_nsec = 0;

    if (timer_settime(transactionTimeOutCommit,0,&timerInterval,NULL)) {
        error = errno;
        ERROR_MSG("timer_settime error %d (%m)\n",error);
    } else {
        DEBUG_MSG("transactionTimeOutCommit timer reset\n");
    }
    return error;
}

inline int InternalDatabaseWriter::initializeTransactionTimeOutTimer() {
    int error(EXIT_SUCCESS);

    struct sigevent signalEvent;
    pthread_attr_t onTransactionCommitTimeoutThreadAttr;

    signalEvent.sigev_notify = SIGEV_THREAD;
    signalEvent.sigev_signo = 0 ; // ignored
    signalEvent.sigev_value.sival_ptr = this;
    signalEvent.sigev_notify_function = onTransactionCommitTimeout;
    error = pthread_attr_init(&onTransactionCommitTimeoutThreadAttr);
    if (EXIT_SUCCESS == error) {
        error = pthread_attr_setdetachstate(&onTransactionCommitTimeoutThreadAttr, PTHREAD_CREATE_DETACHED);
        if (EXIT_SUCCESS == error) {
            signalEvent.sigev_notify_attributes = &onTransactionCommitTimeoutThreadAttr;
        } else {
            WARNING_MSG("pthread_attr_setdetachstate error %d\n",error);
            signalEvent.sigev_notify_attributes = 0;
        }
    } else {
        WARNING_MSG("pthread_attr_init error %d\n",error);
        signalEvent.sigev_notify_attributes = 0;
    }

    if (!timer_create(CLOCK_MONOTONIC,&signalEvent,&transactionTimeOutCommit)) {
        error = resetTransactionTimeOutTimer();
    } else {
        error = errno;
        WARNING_MSG("timer_create CLOCK_MONOTONIC transactionTimeOutCommit error %d (%m)\n",error);
    }

    error = pthread_attr_destroy(&onTransactionCommitTimeoutThreadAttr);
    if (error != EXIT_SUCCESS) {
        WARNING_MSG("pthread_attr_destroy error %d\n",error);
    }

    return error;
}

inline int InternalDatabaseWriter::manageTransactionStart() {
    int error(EXIT_SUCCESS);

#ifndef NO_AUTO_TIMEOUT_COMMIT
    if (transactionTimeOutCommit != 0) {
        resetTransactionTimeOutTimer();
    } else {
        initializeTransactionTimeOutTimer();
    }
#endif //NO_AUTO_TIMEOUT_COMMIT

    if (unlikely(NO_TRANSACTION_IN_PROGRESS == numberOfOperationsInCurrentTransaction)) {
        char *errorMsg = NULL;
        error = sqlite3_exec(database,"BEGIN TRANSACTION",NULL,NULL,&errorMsg);
        if (SQLITE_OK == error) {
            numberOfOperationsInCurrentTransaction = 0;
        } else {
            ERROR_MSG("Error during transaction start up %d (%s)",error,errorMsg);
            sqlite3_free(errorMsg);
        }
    }
    return error;
}

inline int InternalDatabaseWriter::manageTransactionEnd(bool commit /*= true*/) {
    int error(EXIT_SUCCESS);
    if (isTransactionInProgress()) {
        if (unlikely(numberOfOperationsInCurrentTransaction >= theConfiguration.getTransactionSize())) {
            error = commitTransaction(commit);
        } else {
            numberOfOperationsInCurrentTransaction++;
            DEBUG_VAR(numberOfOperationsInCurrentTransaction,"%u");
        }
    }
    return error;
}

// Warning this function is called in a signal context
// cf. signal(7) chap. Async-signal-safe functions for details
void onTransactionCommitTimeout(union sigval signalValue) {
    DEBUG_MSG("%s",__PRETTY_FUNCTION__);
    InternalDatabaseWriter *dbWriter = static_cast<InternalDatabaseWriter *>(signalValue.sival_ptr);
    if (dbWriter->isTransactionInProgress()) {
        dbWriter->commitTransaction();
    }
}

void InternalDatabaseWriter::onMessage1(IEC61162::AISMsg1 msg,void *param) {
    int error(EXIT_SUCCESS);
    TransactionsManager transactionsMgr(this);
    // Position reports
    // insert into travels_positions (MMSI,latitude,longitude,navigationStatus,rateOfTurn,speedOverGround,positionAccuracy,courseOverGround,trueHeading,specialManoeuvreIndicator,date)
    // values (@Id,@Lat,@Long,@NavStatus,@ROT,@SOG,@PosAcc,@COG,@TrueHeading,@SMI,datetime())
    sqlite3_stmt *SQLStatement = SQLStatements[insert_into_travels_positions];
    error = sqlite3_bind_int(SQLStatement,1,(unsigned int)msg->get_userID());
    if (SQLITE_OK == error) {
        if (msg->has_latitude()) {
            error = sqlite3_bind_double(SQLStatement, 2, msg->get_latitude());
        } else {
            error = sqlite3_bind_null(SQLStatement, 2);
        }
        if (SQLITE_OK == error) {
            if (msg->has_longitude()) {
                error = sqlite3_bind_double(SQLStatement, 3, msg->get_longitude());
            } else {
                error = sqlite3_bind_null(SQLStatement, 3);
            }
            if (SQLITE_OK == error) {
                if (msg->has_navigationStatus()) {
                    error = sqlite3_bind_int(SQLStatement, 4, msg->get_navigationStatus());
                } else {
                    error = sqlite3_bind_null(SQLStatement, 4);
                }
                if (SQLITE_OK == error) {
                    if (msg->has_rateOfTurn()) {
                        error = sqlite3_bind_double(SQLStatement, 5,msg->get_rateOfTurn());
                    } else {
                        error = sqlite3_bind_null(SQLStatement, 5);
                    }
                    if (SQLITE_OK == error) {
                        if (msg->has_speedOverGround()) {
                            error = sqlite3_bind_double(SQLStatement, 6,msg->get_speedOverGround());
                        } else {
                            error = sqlite3_bind_null(SQLStatement, 6);
                        }
                        if (SQLITE_OK == error) {
                            if (msg->has_positionAccuracy()) {
                                error = sqlite3_bind_int(SQLStatement, 7, msg->get_positionAccuracy());
                            } else {
                                error = sqlite3_bind_null(SQLStatement, 7);
                            }
                            if (SQLITE_OK == error) {
                                if (msg->has_courseOverGround()) {
                                    error = sqlite3_bind_double(SQLStatement, 8,msg->get_courseOverGround());
                                } else {
                                    error = sqlite3_bind_null(SQLStatement, 8);
                                }
                                if (SQLITE_OK == error) {
                                    if (msg->has_trueHeading()) {
                                        error = sqlite3_bind_int(SQLStatement, 9 ,msg->get_trueHeading());
                                    } else {
                                        error = sqlite3_bind_null(SQLStatement, 9);
                                    }
                                    if (SQLITE_OK == error) {
                                        if (msg->has_specialManoeuvreIndicator()) {
                                            error = sqlite3_bind_int(SQLStatement, 9 ,msg->get_specialManoeuvreIndicator());
                                        } else {
                                            error = sqlite3_bind_null(SQLStatement, 10);
                                        }
                                        if (SQLITE_OK == error) {
                                            error = sqlite3_step(SQLStatement);
                                            if (SQLITE_DONE == error) {
                                                error = EXIT_SUCCESS;
                                            } else {
                                                ERROR_MSG("sqlite3_step insert_into_travels_positions error %d (%s)",error,sqlite3_errmsg(database));
                                            }
                                            sqlite3_clear_bindings(SQLStatement); /* Clear bindings */
                                            sqlite3_reset(SQLStatement);          /* Reset VDBE */
                                        } else {
                                            ERROR_MSG("sqlite3_bind_int specialManoeuvreIndicator error %d: %s",error,sqlite3_errmsg(database));
                                        }
                                    } else {
                                        ERROR_MSG("sqlite3_bind_int trueHeading error %d: %s",error,sqlite3_errmsg(database));
                                    }
                                } else {
                                    ERROR_MSG("sqlite3_bind_double courseOverGround error %d: %s",error,sqlite3_errmsg(database));
                                }
                            } else {
                                ERROR_MSG("sqlite3_bind_int positionAccuracy error %d: %s",error,sqlite3_errmsg(database));
                            }
                        } else {
                            ERROR_MSG("sqlite3_bind_double speedOverGround error %d: %s",error,sqlite3_errmsg(database));
                        }
                    } else {
                        ERROR_MSG("sqlite3_bind_double rateOfTurn error %d: %s",error,sqlite3_errmsg(database));
                    }
                } else {
                    ERROR_MSG("sqlite3_bind_int navigationStatus error %d: %s",error,sqlite3_errmsg(database));
                }
            } else {
                ERROR_MSG("sqlite3_bind_double longitude error %d: %s",error,sqlite3_errmsg(database));
            }
        } else {
            ERROR_MSG("sqlite3_bind_double latitude error %d: %s",error,sqlite3_errmsg(database));
        }
    } else {
        ERROR_MSG("sqlite3_bind_int MMSI error %d: %s",error,sqlite3_errmsg(database));
    }
}

void InternalDatabaseWriter::onMessage2(IEC61162::AISMsg2 msg,void *param) {
    int error(EXIT_SUCCESS);
    TransactionsManager transactionsMgr(this);
    // Position reports
    // insert into travels_positions (MMSI,latitude,longitude,navigationStatus,rateOfTurn,speedOverGround,positionAccuracy,courseOverGround,trueHeading,specialManoeuvreIndicator,date)
    // values (@Id,@Lat,@Long,@NavStatus,@ROT,@SOG,@PosAcc,@COG,@TrueHeading,@SMI,datetime())
    sqlite3_stmt *SQLStatement = SQLStatements[insert_into_travels_positions];
    error = sqlite3_bind_int(SQLStatement,1,(unsigned int)msg->get_userID());
    if (SQLITE_OK == error) {
        if (msg->has_latitude()) {
            error = sqlite3_bind_double(SQLStatement, 2, msg->get_latitude());
        } else {
            error = sqlite3_bind_null(SQLStatement, 2);
        }
        if (SQLITE_OK == error) {
            if (msg->has_longitude()) {
                error = sqlite3_bind_double(SQLStatement, 3, msg->get_longitude());
            } else {
                error = sqlite3_bind_null(SQLStatement, 3);
            }
            if (SQLITE_OK == error) {
                if (msg->has_navigationStatus()) {
                    error = sqlite3_bind_int(SQLStatement, 4, msg->get_navigationStatus());
                } else {
                    error = sqlite3_bind_null(SQLStatement, 4);
                }
                if (SQLITE_OK == error) {
                    if (msg->has_rateOfTurn()) {
                        error = sqlite3_bind_double(SQLStatement, 5,msg->get_rateOfTurn());
                    } else {
                        error = sqlite3_bind_null(SQLStatement, 5);
                    }
                    if (SQLITE_OK == error) {
                        if (msg->has_speedOverGround()) {
                            error = sqlite3_bind_double(SQLStatement, 6,msg->get_speedOverGround());
                        } else {
                            error = sqlite3_bind_null(SQLStatement, 6);
                        }
                        if (SQLITE_OK == error) {
                            if (msg->has_positionAccuracy()) {
                                error = sqlite3_bind_int(SQLStatement, 7, msg->get_positionAccuracy());
                            } else {
                                error = sqlite3_bind_null(SQLStatement, 7);
                            }
                            if (SQLITE_OK == error) {
                                if (msg->has_courseOverGround()) {
                                    error = sqlite3_bind_double(SQLStatement, 8,msg->get_courseOverGround());
                                } else {
                                    error = sqlite3_bind_null(SQLStatement, 8);
                                }
                                if (SQLITE_OK == error) {
                                    if (msg->has_trueHeading()) {
                                        error = sqlite3_bind_int(SQLStatement, 9 ,msg->get_trueHeading());
                                    } else {
                                        error = sqlite3_bind_null(SQLStatement, 9);
                                    }
                                    if (SQLITE_OK == error) {
                                        if (msg->has_specialManoeuvreIndicator()) {
                                            error = sqlite3_bind_int(SQLStatement, 9 ,msg->get_specialManoeuvreIndicator());
                                        } else {
                                            error = sqlite3_bind_null(SQLStatement, 10);
                                        }
                                        if (SQLITE_OK == error) {
                                            error = sqlite3_step(SQLStatement);
                                            if (SQLITE_DONE == error) {
                                                error = EXIT_SUCCESS;
                                            } else {
                                                ERROR_MSG("sqlite3_step insert_into_travels_positions error %d (%s)",error,sqlite3_errmsg(database));
                                            }
                                            sqlite3_clear_bindings(SQLStatement); /* Clear bindings */
                                            sqlite3_reset(SQLStatement);          /* Reset VDBE */
                                        } else {
                                            ERROR_MSG("sqlite3_bind_int specialManoeuvreIndicator error %d: %s",error,sqlite3_errmsg(database));
                                        }
                                    } else {
                                        ERROR_MSG("sqlite3_bind_int trueHeading error %d: %s",error,sqlite3_errmsg(database));
                                    }
                                } else {
                                    ERROR_MSG("sqlite3_bind_double courseOverGround error %d: %s",error,sqlite3_errmsg(database));
                                }
                            } else {
                                ERROR_MSG("sqlite3_bind_int positionAccuracy error %d: %s",error,sqlite3_errmsg(database));
                            }
                        } else {
                            ERROR_MSG("sqlite3_bind_double speedOverGround error %d: %s",error,sqlite3_errmsg(database));
                        }
                    } else {
                        ERROR_MSG("sqlite3_bind_double rateOfTurn error %d: %s",error,sqlite3_errmsg(database));
                    }
                } else {
                    ERROR_MSG("sqlite3_bind_int navigationStatus error %d: %s",error,sqlite3_errmsg(database));
                }
            } else {
                ERROR_MSG("sqlite3_bind_double longitude error %d: %s",error,sqlite3_errmsg(database));
            }
        } else {
            ERROR_MSG("sqlite3_bind_double latitude error %d: %s",error,sqlite3_errmsg(database));
        }
    } else {
        ERROR_MSG("sqlite3_bind_int MMSI error %d: %s",error,sqlite3_errmsg(database));
    }
}

void InternalDatabaseWriter::onMessage3(IEC61162::AISMsg3 msg,void *param) {
    int error(EXIT_SUCCESS);
    TransactionsManager transactionsMgr(this);
    // Position reports
    // insert into travels_positions (MMSI,latitude,longitude,navigationStatus,rateOfTurn,speedOverGround,positionAccuracy,courseOverGround,trueHeading,specialManoeuvreIndicator,date)
    // values (@Id,@Lat,@Long,@NavStatus,@ROT,@SOG,@PosAcc,@COG,@TrueHeading,@SMI,datetime())
    sqlite3_stmt *SQLStatement = SQLStatements[insert_into_travels_positions];
    error = sqlite3_bind_int(SQLStatement,1,(unsigned int)msg->get_userID());
    if (SQLITE_OK == error) {
        if (msg->has_latitude()) {
            error = sqlite3_bind_double(SQLStatement, 2, msg->get_latitude());
        } else {
            error = sqlite3_bind_null(SQLStatement, 2);
        }
        if (SQLITE_OK == error) {
            if (msg->has_longitude()) {
                error = sqlite3_bind_double(SQLStatement, 3, msg->get_longitude());
            } else {
                error = sqlite3_bind_null(SQLStatement, 3);
            }
            if (SQLITE_OK == error) {
                if (msg->has_navigationStatus()) {
                    error = sqlite3_bind_int(SQLStatement, 4, msg->get_navigationStatus());
                } else {
                    error = sqlite3_bind_null(SQLStatement, 4);
                }
                if (SQLITE_OK == error) {
                    if (msg->has_rateOfTurn()) {
                        error = sqlite3_bind_double(SQLStatement, 5,msg->get_rateOfTurn());
                    } else {
                        error = sqlite3_bind_null(SQLStatement, 5);
                    }
                    if (SQLITE_OK == error) {
                        if (msg->has_speedOverGround()) {
                            error = sqlite3_bind_double(SQLStatement, 6,msg->get_speedOverGround());
                        } else {
                            error = sqlite3_bind_null(SQLStatement, 6);
                        }
                        if (SQLITE_OK == error) {
                            if (msg->has_positionAccuracy()) {
                                error = sqlite3_bind_int(SQLStatement, 7, msg->get_positionAccuracy());
                            } else {
                                error = sqlite3_bind_null(SQLStatement, 7);
                            }
                            if (SQLITE_OK == error) {
                                if (msg->has_courseOverGround()) {
                                    error = sqlite3_bind_double(SQLStatement, 8,msg->get_courseOverGround());
                                } else {
                                    error = sqlite3_bind_null(SQLStatement, 8);
                                }
                                if (SQLITE_OK == error) {
                                    if (msg->has_trueHeading()) {
                                        error = sqlite3_bind_int(SQLStatement, 9 ,msg->get_trueHeading());
                                    } else {
                                        error = sqlite3_bind_null(SQLStatement, 9);
                                    }
                                    if (SQLITE_OK == error) {
                                        if (msg->has_specialManoeuvreIndicator()) {
                                            error = sqlite3_bind_int(SQLStatement, 9 ,msg->get_specialManoeuvreIndicator());
                                        } else {
                                            error = sqlite3_bind_null(SQLStatement, 10);
                                        }
                                        if (SQLITE_OK == error) {
                                            error = sqlite3_step(SQLStatement);
                                            if (SQLITE_DONE == error) {
                                                error = EXIT_SUCCESS;
                                            } else {
                                                ERROR_MSG("sqlite3_step insert_into_travels_positions error %d (%s)",error,sqlite3_errmsg(database));
                                            }
                                            sqlite3_clear_bindings(SQLStatement); /* Clear bindings */
                                            sqlite3_reset(SQLStatement);          /* Reset VDBE */
                                        } else {
                                            ERROR_MSG("sqlite3_bind_int specialManoeuvreIndicator error %d: %s",error,sqlite3_errmsg(database));
                                        }
                                    } else {
                                        ERROR_MSG("sqlite3_bind_int trueHeading error %d: %s",error,sqlite3_errmsg(database));
                                    }
                                } else {
                                    ERROR_MSG("sqlite3_bind_double courseOverGround error %d: %s",error,sqlite3_errmsg(database));
                                }
                            } else {
                                ERROR_MSG("sqlite3_bind_int positionAccuracy error %d: %s",error,sqlite3_errmsg(database));
                            }
                        } else {
                            ERROR_MSG("sqlite3_bind_double speedOverGround error %d: %s",error,sqlite3_errmsg(database));
                        }
                    } else {
                        ERROR_MSG("sqlite3_bind_double rateOfTurn error %d: %s",error,sqlite3_errmsg(database));
                    }
                } else {
                    ERROR_MSG("sqlite3_bind_int navigationStatus error %d: %s",error,sqlite3_errmsg(database));
                }
            } else {
                ERROR_MSG("sqlite3_bind_double longitude error %d: %s",error,sqlite3_errmsg(database));
            }
        } else {
            ERROR_MSG("sqlite3_bind_double latitude error %d: %s",error,sqlite3_errmsg(database));
        }
    } else {
        ERROR_MSG("sqlite3_bind_int MMSI error %d: %s",error,sqlite3_errmsg(database));
    }
}

void InternalDatabaseWriter::onMessage4(IEC61162::AISMsg4 msg,void *param) {
    int error(EXIT_SUCCESS);
    TransactionsManager transactionsMgr(this);
}


//The initial digits of an MMSI categorize the identity. The meaning of the first digit is:
//0 Ship group, coast station, or group of coast stations
//1 Recently re-assigned for use by SAR aircraft (ITU-R recommendation M.585-4)
//2-7 MMSI's used by individual ships, beginning with an MID:
//2 Europe (e.g., Italy has MID 247; Denmark has MIDs 219 and 220)
//3 North and Central America and Caribbean (e.g., Canada, 316; Panama, 351, 352, 353, 354, 355, 356, 357, 370, 371, 372, 373)
//4 Asia (e.g., PRC, 412, 413, 414; Maldives, 455)
//5 Oceana (Australia, 503)
//6 Africa (Eritrea, 625)
//7 South America (Peru, 760)
//8 Assigned for regional Use
//9 Recently re-assigned to Nav aids and also craft associated with a parent ship (ITU-R recommendation M.585-4)
static inline unsigned int getCountryCode(const IEC61162::MMSI mmsi) {
    unsigned int countryCode(0);
    // first 3 digits iff the first is between 2 to 7
    const float nbDigits(ceil(log10(mmsi)));
    if (nbDigits >= 3) {
        countryCode = mmsi / (exp10(nbDigits-3));
        if ((countryCode <200) || (countryCode >= 800)) {
            countryCode = 0;
        }
    }
    return countryCode;
}

inline int InternalDatabaseWriter::setVesselData(IEC61162::AISMsg5 msg) {
    int error(EXIT_SUCCESS);
    // Ship static and voyage related data
    // - INSERT OR REPLACE INTO vessels(MMSI, IMONumber,callSign,name,country,type,dimension_A,dimension_B,dimension_C,dimension_D,draught,typeOfPositionDevice,CreationTime)
    // VALUES (@Id,@IMO,@CallSign,@Name,@country,@Type,@DimA,@DimB,@DimC,@DimD,@Draught,@TypePosDev,COALESCE((SELECT CreationTime FROM vessels WHERE MMSI=@Id),datetime()))
    sqlite3_stmt *SQLStatement = SQLStatements[insert_update_vessels];
    error = sqlite3_bind_int(SQLStatement,1,(unsigned int)msg->get_userID());
    if (SQLITE_OK == error) {
        if (msg->has_IMONumber()) {
            error = sqlite3_bind_int(SQLStatement, 2 ,msg->get_IMONumber());
        } else {
            error = sqlite3_bind_null(SQLStatement, 2);
        }
        if (SQLITE_OK == error) {
            if (msg->has_callSign()) {
                const char *callSign = msg->get_callSign();
                const size_t n = strlen(callSign);
                error = sqlite3_bind_text(SQLStatement, 3, callSign, n, SQLITE_TRANSIENT);
            } else {
                error = sqlite3_bind_null(SQLStatement, 3);
            }
            if (SQLITE_OK == error) {
                if (msg->has_name()) {
                    const char *name = msg->get_name();
                    const size_t n = strlen(name);
                    error = sqlite3_bind_text(SQLStatement, 4, name, n, SQLITE_TRANSIENT);
                } else {
                    error = sqlite3_bind_null(SQLStatement, 4);
                }
                if (SQLITE_OK == error) {
                    const unsigned int countryCode = getCountryCode(msg->get_userID());
                    error = sqlite3_bind_int(SQLStatement, 5 ,countryCode);
                    if (SQLITE_OK == error) {
                        if (msg->has_typeOfShipAndCargoType()) {
                            error = sqlite3_bind_int(SQLStatement, 6 ,msg->get_typeOfShipAndCargoType());
                        } else {
                            error = sqlite3_bind_null(SQLStatement, 6);
                        }
                        if (SQLITE_OK == error) {
                            if (msg->has_overallDimensionReferenceForPosition()) {
                                IEC61162::ReferencePoint referencePoint = msg->get_overallDimensionReferenceForPosition();
                                error = sqlite3_bind_int(SQLStatement, 7,referencePoint.A);
                                if (SQLITE_OK == error) {
                                    error = sqlite3_bind_int(SQLStatement, 8,referencePoint.B);
                                    if (SQLITE_OK == error) {
                                        error = sqlite3_bind_int(SQLStatement, 9,referencePoint.C);
                                        if (SQLITE_OK == error) {
                                            error = sqlite3_bind_int(SQLStatement, 10,referencePoint.D);
                                            if (error != SQLITE_OK) {
                                                ERROR_MSG("sqlite3_bind_int referencePoint.D error %d: %s",error,sqlite3_errmsg(database));
                                            }
                                        } else {
                                            ERROR_MSG("sqlite3_bind_int referencePoint.C error %d: %s",error,sqlite3_errmsg(database));
                                        }
                                    } else {
                                        ERROR_MSG("sqlite3_bind_int referencePoint.B error %d: %s",error,sqlite3_errmsg(database));
                                    }
                                } else {
                                    ERROR_MSG("sqlite3_bind_int referencePoint.A error %d: %s",error,sqlite3_errmsg(database));
                                }
                            } else {
                                error = sqlite3_bind_null(SQLStatement, 7);
                                if (SQLITE_OK == error) {
                                    error = sqlite3_bind_null(SQLStatement, 8);
                                    if (SQLITE_OK == error) {
                                        error = sqlite3_bind_null(SQLStatement, 9);
                                        if (SQLITE_OK == error) {
                                            error = sqlite3_bind_null(SQLStatement, 10);
                                            if (error != SQLITE_OK) {
                                                ERROR_MSG("sqlite3_bind_null referencePoint.D error %d: %s",error,sqlite3_errmsg(database));
                                            }
                                        } else {
                                            ERROR_MSG("sqlite3_bind_null referencePoint.C error %d: %s",error,sqlite3_errmsg(database));
                                        }
                                    } else {
                                        ERROR_MSG("sqlite3_bind_null referencePoint.B error %d: %s",error,sqlite3_errmsg(database));
                                    }
                                } else {
                                    ERROR_MSG("sqlite3_bind_null referencePoint.A error %d: %s",error,sqlite3_errmsg(database));
                                }
                            }
                            if (SQLITE_OK == error) {
                                error = sqlite3_bind_null(SQLStatement, 11); // Draught not available in message 5
                                if (SQLITE_OK == error) {
                                    if (msg->has_typeOfPositionDevice()) {
                                        error = sqlite3_bind_int(SQLStatement,12,msg->get_typeOfPositionDevice());
                                    } else {
                                        error = sqlite3_bind_null(SQLStatement, 12);
                                    }
                                    if (SQLITE_OK == error) {
                                        error = sqlite3_step(SQLStatement);
                                        sqlite3_clear_bindings(SQLStatement); /* Clear bindings */
                                        sqlite3_reset(SQLStatement);          /* Reset VDBE */
                                        if (SQLITE_DONE == error) {
                                            error = EXIT_SUCCESS;
                                        } else {
                                            ERROR_MSG("sqlite3_step insert_into_vessels error %d (%s)",error,sqlite3_errmsg(database));
                                        }
                                    } else {
                                        ERROR_MSG("sqlite3_bind_int typeOfPositionDevice error %d: %s",error,sqlite3_errmsg(database));
                                    }
                                } else {
                                    ERROR_MSG("sqlite3_bind_null Draught error %d: %s",error,sqlite3_errmsg(database));
                                }
                            } // error already printed
                        } else {
                            ERROR_MSG("sqlite3_bind_int typeOfShipAndCargoType error %d: %s",error,sqlite3_errmsg(database));
                        }
                    } else {
                        ERROR_MSG("sqlite3_bind_int countryCode Number error %d: %s",error,sqlite3_errmsg(database));
                    }
                } else {
                    ERROR_MSG("sqlite3_bind_text name error %d: %s",error,sqlite3_errmsg(database));
                }
            } else {
                ERROR_MSG("sqlite3_bind_text callSign error %d: %s",error,sqlite3_errmsg(database));
            }
        } else {
            ERROR_MSG("sqlite3_bind_int IMO Number error %d: %s",error,sqlite3_errmsg(database));
        }
    } else {
        ERROR_MSG("sqlite3_bind_int MMSI error %d: %s",error,sqlite3_errmsg(database));
    }
    return error;
}

inline int InternalDatabaseWriter::setTravelsData(IEC61162::AISMsg5 msg) {
    int error(EXIT_SUCCESS);
    // fill travels data
    // insert into travels (MMSI,destination,ETA,date) values(@Id,@Dest,@ETA,datetime())
    sqlite3_stmt *SQLStatement = SQLStatements[insert_into_travels];
    error = sqlite3_bind_int(SQLStatement,1,(unsigned int)msg->get_userID());
    if (SQLITE_OK == error) {
        if (msg->has_destination()) {
            const char *name = msg->get_destination();
            const size_t n = strlen(name);
            error = sqlite3_bind_text(SQLStatement, 2, name, n, SQLITE_TRANSIENT);
        } else {
            error = sqlite3_bind_null(SQLStatement, 2);
        }
        if (SQLITE_OK == error) {
            if (msg->has_ETA()) {
                IEC61162::EstimatedTimeOfArrival ETA = msg->get_ETA();
                error = sqlite3_bind_int(SQLStatement, 3, (time_t)ETA);
            } else {
                error = sqlite3_bind_null(SQLStatement, 3);
            }
            if (SQLITE_OK == error) {
                error = sqlite3_step(SQLStatement);
                if (SQLITE_DONE == error) {
                    error = EXIT_SUCCESS;
                } else {
                    ERROR_MSG("sqlite3_step insert_into_travels error %d (%s)",error,sqlite3_errmsg(database));
                }
                sqlite3_clear_bindings(SQLStatement); /* Clear bindings */
                sqlite3_reset(SQLStatement);          /* Reset VDBE */
            } else {
                ERROR_MSG("sqlite3_bind_int ETA error %d: %s",error,sqlite3_errmsg(database));
            }
        } else {
            ERROR_MSG("sqlite3_bind_text destination error %d: %s",error,sqlite3_errmsg(database));
        }
    } else {
        ERROR_MSG("sqlite3_bind_int MMSI error %d: %s",error,sqlite3_errmsg(database));
    }
    return error;
}

void InternalDatabaseWriter::onMessage5(IEC61162::AISMsg5 msg,void *param) {
    TransactionsManager transactionsMgr(this);
    setVesselData(msg); //error (if any) already printed
    setTravelsData(msg); //error (if any) already printed
}

void InternalDatabaseWriter::onMessage6(IEC61162::AISMsg6 msg,void *param) {
    int error(EXIT_SUCCESS);
    TransactionsManager transactionsMgr(this);
}

void InternalDatabaseWriter::onMessage7(IEC61162::AISMsg7 msg,void *param) {
    int error(EXIT_SUCCESS);
    TransactionsManager transactionsMgr(this);
}

void InternalDatabaseWriter::onMessage8(IEC61162::AISMsg8 msg,void *param) {
    int error(EXIT_SUCCESS);
    TransactionsManager transactionsMgr(this);
}

void InternalDatabaseWriter::onMessage9(IEC61162::AISMsg9 msg,void *param) {
    int error(EXIT_SUCCESS);
    TransactionsManager transactionsMgr(this);
    // insert into SAR_Aircrafts_Positions
    // (MMSI, altitude, speedOverGround, positionAccuracy, longitude, latitude, courseOverGround, date)
    // values (@Id,@Alt,@SOG,@PosAcc,@Long,@Lat,@COG,datetime())"
    sqlite3_stmt *SQLStatement = SQLStatements[insert_into_SAR_aircrafts_positions];
    error = sqlite3_bind_int(SQLStatement,1,(unsigned int)msg->get_userID());
    if (SQLITE_OK == error) {
        if (msg->has_altitude()) {
            error = sqlite3_bind_int(SQLStatement, 2, msg->get_altitude());
        } else {
            error = sqlite3_bind_null(SQLStatement, 2);
        }
        if (SQLITE_OK == error) {
            if (msg->has_speedOverGround()) {
                error = sqlite3_bind_double(SQLStatement,3,msg->get_speedOverGround());
            } else {
                error = sqlite3_bind_null(SQLStatement, 3);
            }
            if (SQLITE_OK == error) {
                if (msg->has_positionAccuracy()) {
                    error = sqlite3_bind_int(SQLStatement, 4,msg->get_positionAccuracy());
                } else {
                    error = sqlite3_bind_null(SQLStatement, 4);
                }
                if (SQLITE_OK == error) {
                    if (msg->has_longitude()) {
                        error = sqlite3_bind_double(SQLStatement,5,msg->get_longitude());
                    } else {
                        error = sqlite3_bind_null(SQLStatement, 5);
                    }
                    if (SQLITE_OK == error) {
                        if (msg->has_latitude()) {
                            error = sqlite3_bind_double(SQLStatement,6,msg->get_latitude());
                        } else {
                            error = sqlite3_bind_null(SQLStatement, 6);
                        }
                        if (SQLITE_OK == error) {
                            if (msg->has_courseOverGround()) {
                                error = sqlite3_bind_double(SQLStatement,7,msg->get_courseOverGround());
                            } else {
                                error = sqlite3_bind_null(SQLStatement, 7);
                            }
                            if (SQLITE_OK == error) {
                                error = sqlite3_step(SQLStatement);
                                if (SQLITE_DONE == error) {
                                    error = EXIT_SUCCESS;
                                } else {
                                    ERROR_MSG("sqlite3_step insert_into_SAR_aircrafts_positions error %d (%s)",error,sqlite3_errmsg(database));
                                }
                                sqlite3_clear_bindings(SQLStatement); /* Clear bindings */
                                sqlite3_reset(SQLStatement);          /* Reset VDBE */
                            } else {
                                ERROR_MSG("sqlite3_bind_double courseOverGround error %d: %s",error,sqlite3_errmsg(database));
                            }
                        } else {
                            ERROR_MSG("sqlite3_bind_double latitude error %d: %s",error,sqlite3_errmsg(database));
                        }
                    } else {
                        ERROR_MSG("sqlite3_bind_double longitude error %d: %s",error,sqlite3_errmsg(database));
                    }
                } else {
                    ERROR_MSG("sqlite3_bind_int positionAccuracy error %d: %s",error,sqlite3_errmsg(database));
                }
            } else {
                ERROR_MSG("sqlite3_bind_double speedOverGround error %d: %s",error,sqlite3_errmsg(database));
            }
        } else {
            ERROR_MSG("sqlite3_bind_int altitude error %d: %s",error,sqlite3_errmsg(database));
        }
    } else {
        ERROR_MSG("sqlite3_bind_int MMSI error %d: %s",error,sqlite3_errmsg(database));
    }
}

void InternalDatabaseWriter::onMessage10(IEC61162::AISMsg10 msg,void *param) {
    int error(EXIT_SUCCESS);
    TransactionsManager transactionsMgr(this);
}

void InternalDatabaseWriter::onMessage11(IEC61162::AISMsg11 msg,void *param) {
    int error(EXIT_SUCCESS);
    TransactionsManager transactionsMgr(this);
}

void InternalDatabaseWriter::onMessage12(IEC61162::AISMsg12 msg,void *param) {
    int error(EXIT_SUCCESS);
    TransactionsManager transactionsMgr(this);
}

void InternalDatabaseWriter::onMessage13(IEC61162::AISMsg13 msg,void *param) {
    int error(EXIT_SUCCESS);
    TransactionsManager transactionsMgr(this);
}

void InternalDatabaseWriter::onMessage14(IEC61162::AISMsg14 msg,void *param) {
    int error(EXIT_SUCCESS);
    TransactionsManager transactionsMgr(this);
}

void InternalDatabaseWriter::onMessage15(IEC61162::AISMsg15 msg,void *param) {
    int error(EXIT_SUCCESS);
    TransactionsManager transactionsMgr(this);
}

//void InternalDatabase::onMessage16(IEC61162::AISMsg16 msg,void *param) {
//    int error(EXIT_SUCCESS);
//}
//
//void InternalDatabase::onMessage17(IEC61162::AISMsg17 msg,void *param) {
//    int error(EXIT_SUCCESS);
//}

void InternalDatabaseWriter::onMessage18(IEC61162::AISMsg18 msg,void *param) {
    int error(EXIT_SUCCESS);
    TransactionsManager transactionsMgr(this);
    // Standard Class B equipment position report
    // insert into travels_positions (MMSI,latitude,longitude,navigationStatus,rateOfTurn,speedOverGround,positionAccuracy,courseOverGround,trueHeading,specialManoeuvreIndicator,date)
    // values (@Id,@Lat,@Long,@NavStatus,@ROT,@SOG,@PosAcc,@COG,@TrueHeading,@SMI,datetime())
    sqlite3_stmt *SQLStatement = SQLStatements[insert_into_travels_positions];
    error = sqlite3_bind_int(SQLStatement,1,(unsigned int)msg->get_userID());
    if (SQLITE_OK == error) {
        if (msg->has_latitude()) {
            error = sqlite3_bind_double(SQLStatement, 2, msg->get_latitude());
        } else {
            error = sqlite3_bind_null(SQLStatement, 2);
        }
        if (SQLITE_OK == error) {
            if (msg->has_longitude()) {
                error = sqlite3_bind_double(SQLStatement, 3, msg->get_longitude());
            } else {
                error = sqlite3_bind_null(SQLStatement, 3);
            }
            if (SQLITE_OK == error) {
                error = sqlite3_bind_null(SQLStatement, 4); // navigation status not available in the AIS Msg 18
                if (SQLITE_OK == error) {
                    error = sqlite3_bind_null(SQLStatement, 5); // rate of turn not available in the AIS Msg 18
                    if (SQLITE_OK == error) {
                        if (msg->has_speedOverGround()) {
                            error = sqlite3_bind_double(SQLStatement, 6, msg->get_speedOverGround());
                        } else {
                            error = sqlite3_bind_null(SQLStatement, 6);
                        }
                        if (SQLITE_OK == error) {
                            if (msg->has_positionAccuracy()) {
                                error = sqlite3_bind_int(SQLStatement, 7, msg->get_positionAccuracy());
                            } else {
                                error = sqlite3_bind_null(SQLStatement, 7);
                            }
                            if (SQLITE_OK == error) {
                                if (msg->has_courseOverGround()) {
                                    error = sqlite3_bind_double(SQLStatement, 8, msg->get_courseOverGround());
                                } else {
                                    error = sqlite3_bind_null(SQLStatement, 8);
                                }
                                if (SQLITE_OK == error) {
                                    if (msg->has_trueHeading()) {
                                        error = sqlite3_bind_int(SQLStatement, 9,msg->get_trueHeading());
                                    } else {
                                        error = sqlite3_bind_null(SQLStatement, 9);
                                    }
                                    if (SQLITE_OK == error) {
                                        error = sqlite3_bind_null(SQLStatement, 10); // specialManoeuvreIndicator not available in the AIS Msg 18
                                        if (SQLITE_OK == error) {
                                            error = sqlite3_step(SQLStatement);
                                            if (SQLITE_DONE == error) {
                                                error = EXIT_SUCCESS;
                                            } else {
                                                ERROR_MSG("sqlite3_step insert_into_travels_positions error %d (%s)",error,sqlite3_errmsg(database));
                                            }
                                            sqlite3_clear_bindings(SQLStatement); /* Clear bindings */
                                            sqlite3_reset(SQLStatement);          /* Reset VDBE */
                                        } else {
                                            ERROR_MSG("sqlite3_bind_null specialManoeuvreIndicator error %d: %s",error,sqlite3_errmsg(database));
                                        }
                                    }
                                } else {
                                    ERROR_MSG("sqlite3_bind_double courseOverGround error %d: %s",error,sqlite3_errmsg(database));
                                }
                            } else {
                                ERROR_MSG("sqlite3_bind_int positionAccuracy error %d: %s",error,sqlite3_errmsg(database));
                            }
                        } else {
                            ERROR_MSG("sqlite3_bind_double speedOverGround error %d: %s",error,sqlite3_errmsg(database));
                        }
                    } else {
                        ERROR_MSG("sqlite3_bind_null rateOfTurn error %d: %s",error,sqlite3_errmsg(database));
                    }
                } else {
                    ERROR_MSG("sqlite3_bind_null navigationStatus error %d: %s",error,sqlite3_errmsg(database));
                }
            } else {
                ERROR_MSG("sqlite3_bind_double longitude error %d: %s",error,sqlite3_errmsg(database));
            }
        } else {
            ERROR_MSG("sqlite3_bind_double latitude error %d: %s",error,sqlite3_errmsg(database));
        }
    } else {
        ERROR_MSG("sqlite3_bind_int MMSI error %d: %s",error,sqlite3_errmsg(database));
    }
}

inline int InternalDatabaseWriter::setTravelsData(IEC61162::AISMsg19 msg) {
    //
    // insert into travels_positions (MMSI,latitude,longitude,navigationStatus,rateOfTurn,speedOverGround,positionAccuracy,courseOverGround,trueHeading,specialManoeuvreIndicator,date)
    // values (@Id,@Lat,@Long,@NavStatus,@ROT,@SOG,@PosAcc,@COG,@TrueHeading,@SMI,datetime())
    int error(EXIT_SUCCESS);
    sqlite3_stmt *SQLStatement = SQLStatements[insert_into_travels_positions];
    error = sqlite3_bind_int(SQLStatement,1,(unsigned int)msg->get_userID());
    if (SQLITE_OK == error) {
        if (msg->has_latitude()) {
            error = sqlite3_bind_double(SQLStatement, 2, msg->get_latitude());
        } else {
            error = sqlite3_bind_null(SQLStatement, 2);
        }
        if (SQLITE_OK == error) {
            if (msg->has_longitude()) {
                error = sqlite3_bind_double(SQLStatement, 3, msg->get_longitude());
            } else {
                error = sqlite3_bind_null(SQLStatement, 3);
            }
            if (SQLITE_OK == error) {
                error = sqlite3_bind_null(SQLStatement, 4); // navigation status not available in the AIS Msg 19
                if (SQLITE_OK == error) {
                    error = sqlite3_bind_null(SQLStatement, 5); // rate of turn not available in the AIS Msg 19
                    if (SQLITE_OK == error) {
                        if (msg->has_speedOverGround()) {
                            error = sqlite3_bind_double(SQLStatement, 6, msg->get_speedOverGround());
                        } else {
                            error = sqlite3_bind_null(SQLStatement, 6);
                        }
                        if (SQLITE_OK == error) {
                            if (msg->has_positionAccuracy()) {
                                error = sqlite3_bind_int(SQLStatement, 7, msg->get_positionAccuracy());
                            } else {
                                error = sqlite3_bind_null(SQLStatement, 7);
                            }
                            if (SQLITE_OK == error) {
                                if (msg->has_courseOverGround()) {
                                    error = sqlite3_bind_double(SQLStatement, 8, msg->get_courseOverGround());
                                } else {
                                    error = sqlite3_bind_null(SQLStatement, 8);
                                }
                                if (SQLITE_OK == error) {
                                    if (msg->has_trueHeading()) {
                                        error = sqlite3_bind_int(SQLStatement, 9,msg->get_trueHeading());
                                    } else {
                                        error = sqlite3_bind_null(SQLStatement, 9);
                                    }
                                    if (SQLITE_OK == error) {
                                        error = sqlite3_bind_null(SQLStatement, 10); // specialManoeuvreIndicator not available in the AIS Msg 19
                                        if (SQLITE_OK == error) {
                                            error = sqlite3_step(SQLStatement);
                                            if (SQLITE_DONE == error) {
                                                error = EXIT_SUCCESS;
                                            } else {
                                                ERROR_MSG("sqlite3_step insert_into_travels_positions error %d (%s)",error,sqlite3_errmsg(database));
                                            }
                                            sqlite3_clear_bindings(SQLStatement); /* Clear bindings */
                                            sqlite3_reset(SQLStatement);          /* Reset VDBE */
                                        } else {
                                            ERROR_MSG("sqlite3_bind_null specialManoeuvreIndicator error %d: %s",error,sqlite3_errmsg(database));
                                        }
                                    } else {
                                        ERROR_MSG("sqlite3_bind_int trueHeading error %d: %s",error,sqlite3_errmsg(database));
                                    }
                                } else {
                                    ERROR_MSG("sqlite3_bind_double courseOverGround error %d: %s",error,sqlite3_errmsg(database));
                                }
                            } else {
                                ERROR_MSG("sqlite3_bind_int positionAccuracy error %d: %s",error,sqlite3_errmsg(database));
                            }
                        } else {
                            ERROR_MSG("sqlite3_bind_double speedOverGround error %d: %s",error,sqlite3_errmsg(database));
                        }
                    } else {
                        ERROR_MSG("sqlite3_bind_null rateOfTurn error %d: %s",error,sqlite3_errmsg(database));
                    }
                } else {
                    ERROR_MSG("sqlite3_bind_null navigationStatus error %d: %s",error,sqlite3_errmsg(database));
                }
            } else {
                ERROR_MSG("sqlite3_bind_double longitude error %d: %s",error,sqlite3_errmsg(database));
            }
        } else {
            ERROR_MSG("sqlite3_bind_double latitude error %d: %s",error,sqlite3_errmsg(database));
        }
    } else {
        ERROR_MSG("sqlite3_bind_int MMSI error %d: %s",error,sqlite3_errmsg(database));
    }
    return error;
}

inline int InternalDatabaseWriter::setVesselData(IEC61162::AISMsg19 msg) {
    // INSERT OR REPLACE vessels (MMSI,name,type,dimension_A,dimension_B,dimension_C,dimension_D,typeOfPositionDevice,CreationTime)
    // values(@Id,@name,@Type,@DimA,@DimB,@DimC,@DimD,@typePosDevice,COALESCE((SELECT CreationTime FROM vessels WHERE MMSI=@Id),datetime()))
    int error(EXIT_SUCCESS);
    sqlite3_stmt *SQLStatement = SQLStatements[insert_update_into_vessels_19];
    error = sqlite3_bind_int(SQLStatement,1,(unsigned int)msg->get_userID());
    if (SQLITE_OK == error) {
        if (msg->has_name()) {
            const char *name = msg->get_name();
            const size_t n = strlen(name);
            error = sqlite3_bind_text(SQLStatement, 2, name, n, SQLITE_TRANSIENT);
        } else {
            error = sqlite3_bind_null(SQLStatement, 2);
        }
        if (SQLITE_OK == error) {
            if (msg->has_typeOfShipAndCargoType()) {
                error = sqlite3_bind_int(SQLStatement, 3 ,msg->get_typeOfShipAndCargoType());
            } else {
                error = sqlite3_bind_null(SQLStatement, 3);
            }
            if (SQLITE_OK == error) {
                if (msg->has_dimensionOfShipReferenceForPosition()) {
                    IEC61162::ReferencePoint referencePoint = msg->get_dimensionOfShipReferenceForPosition();
                    error = sqlite3_bind_int(SQLStatement, 4,referencePoint.A);
                    if (SQLITE_OK == error) {
                        error = sqlite3_bind_int(SQLStatement, 5,referencePoint.B);
                        if (SQLITE_OK == error) {
                            error = sqlite3_bind_int(SQLStatement, 6,referencePoint.C);
                            if (SQLITE_OK == error) {
                                error = sqlite3_bind_int(SQLStatement, 7,referencePoint.D);
                                if (error != SQLITE_OK) {
                                    ERROR_MSG("sqlite3_bind_int referencePoint.D error %d: %s",error,sqlite3_errmsg(database));
                                }
                            } else {
                                ERROR_MSG("sqlite3_bind_int referencePoint.C error %d: %s",error,sqlite3_errmsg(database));
                            }
                        } else {
                            ERROR_MSG("sqlite3_bind_int referencePoint.B error %d: %s",error,sqlite3_errmsg(database));
                        }
                    } else {
                        ERROR_MSG("sqlite3_bind_int referencePoint.A error %d: %s",error,sqlite3_errmsg(database));
                    }
                } else {
                    error = sqlite3_bind_null(SQLStatement, 4);
                    if (SQLITE_OK == error) {
                        error = sqlite3_bind_null(SQLStatement, 5);
                        if (SQLITE_OK == error) {
                            error = sqlite3_bind_null(SQLStatement, 6);
                            if (SQLITE_OK == error) {
                                error = sqlite3_bind_null(SQLStatement, 7);
                                if (error != SQLITE_OK) {
                                    ERROR_MSG("sqlite3_bind_null referencePoint.D error %d: %s",error,sqlite3_errmsg(database));
                                }
                            } else {
                                ERROR_MSG("sqlite3_bind_null referencePoint.C error %d: %s",error,sqlite3_errmsg(database));
                            }
                        } else {
                            ERROR_MSG("sqlite3_bind_null referencePoint.B error %d: %s",error,sqlite3_errmsg(database));
                        }
                    } else {
                        ERROR_MSG("sqlite3_bind_null referencePoint.A error %d: %s",error,sqlite3_errmsg(database));
                    }
                }
                if (SQLITE_OK == error) {
                    if (msg->has_typeOfPositionDevice()) {
                        error = sqlite3_bind_int(SQLStatement,8,msg->get_typeOfPositionDevice());
                    } else {
                        error = sqlite3_bind_null(SQLStatement, 8);
                    }
                    if (SQLITE_OK == error) {
                        error = sqlite3_step(SQLStatement);
                        sqlite3_clear_bindings(SQLStatement); /* Clear bindings */
                        sqlite3_reset(SQLStatement);          /* Reset VDBE */
                        if (SQLITE_DONE == error) {
                            error = EXIT_SUCCESS;
                        } else {
                            ERROR_MSG("sqlite3_step insert_update_into_vessels_19 error %d (%s)",error,sqlite3_errmsg(database));
                        }
                    } else {
                        ERROR_MSG("sqlite3_bind_int typeOfPositionDevice error %d: %s",error,sqlite3_errmsg(database));
                    }
               } // error already printed
            } else {
                ERROR_MSG("sqlite3_bind_null typeOfShipAndCargoType error %d: %s",error,sqlite3_errmsg(database));
            }
        } else {
            ERROR_MSG("sqlite3_bind_text name error %d: %s",error,sqlite3_errmsg(database));
        }
    } else {
        ERROR_MSG("sqlite3_bind_int MMSI error %d: %s",error,sqlite3_errmsg(database));
    }
    return error;
}

void InternalDatabaseWriter::onMessage19(IEC61162::AISMsg19 msg,void *param) {
    int error(EXIT_SUCCESS);
    TransactionsManager transactionsMgr(this);
    // Message 19: Extended Class B equipment position report
    error = setTravelsData(msg);

    error = setVesselData(msg);
}

//void InternalDatabase::onMessage20(IEC61162::AISMsg20 msg,void *param) {
//    int error(EXIT_SUCCESS);
//}
//
//void InternalDatabase::onMessage21(IEC61162::AISMsg21 msg,void *param) {
//    int error(EXIT_SUCCESS);
//}
//
//void InternalDatabase::onMessage22(IEC61162::AISMsg22 msg,void *param) {
//    int error(EXIT_SUCCESS);
//}
//
//void InternalDatabase::onMessage23(IEC61162::AISMsg23 msg,void *param) {
//    int error(EXIT_SUCCESS);
//}

/*
 * Message 24 Part A and Part B may be used by any AIS station to associate a MMSI with a name.
 * Message 24 Part A and Part B should be used by Class B “CS” shipborne mobile equipment. The
 * message consists of two parts. Message 24B should be transmitted within 1 min following
 * Message 24A.
 * In case of an interrogation for a Class B “CS” on a Message 24, the response should include Part A
 * and Part B.
*/

void InternalDatabaseWriter::onMessage24A(IEC61162::AISMsg24A msg,void *param) {
    // INSERT OR REPLACE INTO vessels (MMSI, name,CreationTime)
    // VALUES (@Id,@Name,COALESCE((SELECT CreationTime FROM vessels WHERE MMSI=@Id),datetime()))
    int error(EXIT_SUCCESS);
    TransactionsManager transactionsMgr(this);
    sqlite3_stmt *SQLStatement = SQLStatements[insert_update_name_into_vessels];
    error = sqlite3_bind_int(SQLStatement,1,(unsigned int)msg->get_userID());
    if (SQLITE_OK == error) {
        if (msg->has_name()) {
            const char *name = msg->get_name();
            const size_t n = strlen(name);
            error = sqlite3_bind_text(SQLStatement, 2, name, n, SQLITE_TRANSIENT);
        } else {
            error = sqlite3_bind_null(SQLStatement, 2);
        }
        if (SQLITE_OK == error) {
            error = sqlite3_step(SQLStatement);
            sqlite3_clear_bindings(SQLStatement); /* Clear bindings */
            sqlite3_reset(SQLStatement);          /* Reset VDBE */
            if (SQLITE_DONE == error) {
                error = EXIT_SUCCESS;
            } else {
                ERROR_MSG("sqlite3_step insert_update_name_into_vessels error %d (%s)",error,sqlite3_errmsg(database));
            }
        } else {
            ERROR_MSG("sqlite3_bind_text name error %d: %s",error,sqlite3_errmsg(database));
        }
    } else {
        ERROR_MSG("sqlite3_bind_int MMSI error %d: %s",error,sqlite3_errmsg(database));
    }
}

void InternalDatabaseWriter::onMessage24B(IEC61162::AISMsg24B msg,void *param) {
    // INSERT OR REPLACE INTO vessels (MMSI,type,callSign,dimension_A,dimension_B,dimension_C,dimension_D,CreationTime)
    // values(@Id,@Type,@CallSign,@DimA,@DimB,@DimC,@DimD,COALESCE((SELECT CreationTime FROM vessels WHERE MMSI=@Id),datetime()))
    int error(EXIT_SUCCESS);
    TransactionsManager transactionsMgr(this);
    sqlite3_stmt *SQLStatement = SQLStatements[update_vessels_24B];
    error = sqlite3_bind_int(SQLStatement,1,(unsigned int)msg->get_userID());
    if (SQLITE_OK == error) {
        if (msg->has_typeOfShipAndCargoType()) {
            error =sqlite3_bind_int(SQLStatement,2,msg->get_typeOfShipAndCargoType());
        } else {
            error = sqlite3_bind_null(SQLStatement,2);
        }
        if (SQLITE_OK == error) {
            if (msg->has_callSign()) {
                const char *callSign = msg->get_callSign();
                const size_t n = strlen(callSign);
                error = sqlite3_bind_text(SQLStatement, 3, callSign, n, SQLITE_TRANSIENT);
            } else {
                error = sqlite3_bind_null(SQLStatement, 3);
            }
            if (SQLITE_OK == error) {
                if (msg->has_dimension()) {
                    IEC61162::ReferencePoint dimensions = msg->get_dimension();
                    error = sqlite3_bind_int(SQLStatement, 4,dimensions.A);
                    if (SQLITE_OK == error) {
                        error = sqlite3_bind_int(SQLStatement, 5,dimensions.B);
                        if (SQLITE_OK == error) {
                            error = sqlite3_bind_int(SQLStatement, 6,dimensions.C);
                            if (SQLITE_OK == error) {
                                error = sqlite3_bind_int(SQLStatement, 7,dimensions.D);
                                if (error != SQLITE_OK) {
                                    ERROR_MSG("sqlite3_bind_int referencePoint.D error %d: %s",error,sqlite3_errmsg(database));
                                }
                            } else {
                                ERROR_MSG("sqlite3_bind_int referencePoint.C error %d: %s",error,sqlite3_errmsg(database));
                            }
                        } else {
                            ERROR_MSG("sqlite3_bind_int referencePoint.B error %d: %s",error,sqlite3_errmsg(database));
                        }
                    } else {
                        ERROR_MSG("sqlite3_bind_int referencePoint.A error %d: %s",error,sqlite3_errmsg(database));
                    }
                } else {
                    error = sqlite3_bind_null(SQLStatement, 4);
                    if (SQLITE_OK == error) {
                        error = sqlite3_bind_null(SQLStatement, 5);
                        if (SQLITE_OK == error) {
                            error = sqlite3_bind_null(SQLStatement, 6);
                            if (SQLITE_OK == error) {
                                error = sqlite3_bind_null(SQLStatement, 7);
                                if (error != SQLITE_OK) {
                                    ERROR_MSG("sqlite3_bind_null referencePoint.D error %d: %s",error,sqlite3_errmsg(database));
                                }
                            } else {
                                ERROR_MSG("sqlite3_bind_null referencePoint.C error %d: %s",error,sqlite3_errmsg(database));
                            }
                        } else {
                            ERROR_MSG("sqlite3_bind_null referencePoint.B error %d: %s",error,sqlite3_errmsg(database));
                        }
                    } else {
                        ERROR_MSG("sqlite3_bind_null referencePoint.A error %d: %s",error,sqlite3_errmsg(database));
                    }
                }
                if (SQLITE_OK == error) {
                    error = sqlite3_step(SQLStatement);
                    if (SQLITE_DONE == error) {
                        error = EXIT_SUCCESS;
                    } else {
                        ERROR_MSG("sqlite3_step update_vessels_24B error %d (%s)",error,sqlite3_errmsg(database));
                    }
                    sqlite3_clear_bindings(SQLStatement); /* Clear bindings */
                    sqlite3_reset(SQLStatement);          /* Reset VDBE */
                } // error already printed
            } else {
                ERROR_MSG("sqlite3_bind_text callSign error %d: %s",error,sqlite3_errmsg(database));
            }
        } else {
            ERROR_MSG("sqlite3_bind_int typeOfShipAndCargoType error %d: %s",error,sqlite3_errmsg(database));
        }
    } else {
        ERROR_MSG("sqlite3_bind_int MMSI error %d: %s",error,sqlite3_errmsg(database));
    }
}

//void InternalDatabase::onMessage25(IEC61162::AISMsg25 msg,void *param) {
//    int error(EXIT_SUCCESS);
//}
//
//void InternalDatabase::onMessage26(IEC61162::AISMsg26 msg,void *param) {
//    int error(EXIT_SUCCESS);
//}

void InternalDatabaseWriter::onMessage27(IEC61162::AISMsg27 msg,void *param) {
    int error(EXIT_SUCCESS);
    TransactionsManager transactionsMgr(this);
    // Long-range AIS broadcast message
    // insert into travels_positions (MMSI,latitude,longitude,navigationStatus,rateOfTurn,speedOverGround,positionAccuracy,courseOverGround,trueHeading,specialManoeuvreIndicator,date)
    // values (@Id,@Lat,@Long,@NavStatus,@ROT,@SOG,@PosAcc,@COG,@TrueHeading,@SMI,datetime())
    sqlite3_stmt *SQLStatement = SQLStatements[insert_into_travels_positions];
    error = sqlite3_bind_int(SQLStatement,1,(unsigned int)msg->get_userID());
    if (SQLITE_OK == error) {
        if (msg->has_latitude()) {
            error = sqlite3_bind_double(SQLStatement, 2, msg->get_latitude());
        } else {
            error = sqlite3_bind_null(SQLStatement, 2);
        }
        if (SQLITE_OK == error) {
            if (msg->has_longitude()) {
                error = sqlite3_bind_double(SQLStatement, 3, msg->get_longitude());
            } else {
                error = sqlite3_bind_null(SQLStatement, 3);
            }
            if (SQLITE_OK == error) {
                if (msg->has_navigationStatus()) {
                    error = sqlite3_bind_int(SQLStatement, 4, msg->get_navigationStatus());
                } else {
                    error = sqlite3_bind_null(SQLStatement, 4);
                }
                if (SQLITE_OK == error) {
                    error = sqlite3_bind_null(SQLStatement, 5); // Rate Of Turn not available in message 27
                    if (SQLITE_OK == error) {
                        if (msg->has_speedOverGround()) {
                            error = sqlite3_bind_double(SQLStatement, 6, msg->get_speedOverGround());
                        } else {
                            error = sqlite3_bind_null(SQLStatement, 6);
                        }
                        if (SQLITE_OK == error) {
                            if (msg->has_positionAccuracy()) {
                                error = sqlite3_bind_int(SQLStatement, 7, msg->get_positionAccuracy());
                            } else {
                                error = sqlite3_bind_null(SQLStatement, 7);
                            }
                            if (SQLITE_OK == error) {
                                if (msg->has_courseOverGround()) {
                                    error = sqlite3_bind_double(SQLStatement, 8, msg->get_courseOverGround());
                                } else {
                                    error = sqlite3_bind_null(SQLStatement, 8);
                                }
                                if (SQLITE_OK == error) {
                                    error = sqlite3_bind_null(SQLStatement, 9); // True Heading not available in message 27
                                    if (SQLITE_OK == error) {
                                        error = sqlite3_bind_null(SQLStatement, 10); // specialManoeuvreIndicator not available in message 27
                                        if (SQLITE_OK == error) {
                                            error = sqlite3_step(SQLStatement);
                                            if (SQLITE_DONE == error) {
                                                error = EXIT_SUCCESS;
                                            } else {
                                                ERROR_MSG("sqlite3_step insert_into_travels_positions error %d (%s)",error,sqlite3_errmsg(database));
                                            }
                                            sqlite3_clear_bindings(SQLStatement); /* Clear bindings */
                                            sqlite3_reset(SQLStatement);          /* Reset VDBE */
                                        } else {
                                            ERROR_MSG("sqlite3_bind_null specialManoeuvreIndicator error %d: %s",error,sqlite3_errmsg(database));
                                        }
                                    } else {
                                        ERROR_MSG("sqlite3_bind_null True Heading error %d: %s",error,sqlite3_errmsg(database));
                                    }
                                } else {
                                    ERROR_MSG("sqlite3_bind_double courseOverGround error %d: %s",error,sqlite3_errmsg(database));
                                }
                            } else {
                                ERROR_MSG("sqlite3_bind_int positionAccuracy error %d: %s",error,sqlite3_errmsg(database));
                            }
                        } else {
                            ERROR_MSG("sqlite3_bind_double speedOverGround error %d: %s",error,sqlite3_errmsg(database));
                        }
                    } else {
                        ERROR_MSG("sqlite3_bind_null Rate Of Turn error %d: %s",error,sqlite3_errmsg(database));
                    }
                } else {
                    ERROR_MSG("sqlite3_bind_int navigationStatus error %d: %s",error,sqlite3_errmsg(database));
                }
            } else {
                ERROR_MSG("sqlite3_bind_double longitude error %d: %s",error,sqlite3_errmsg(database));
            }
        } else {
            ERROR_MSG("sqlite3_bind_double latitude error %d: %s",error,sqlite3_errmsg(database));
        }
    } else {
        ERROR_MSG("sqlite3_bind_int MMSI error %d: %s",error,sqlite3_errmsg(database));
    }
}
