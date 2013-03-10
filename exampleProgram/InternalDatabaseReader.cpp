/*
 * InternalDatabaseReader.cpp
 *
 *  Created on: 8 juil. 2013
 *      Author: oc
 */

#include "globals.h"
#include "InternalDatabaseReader.h"

#define MODULE_FLAG FLAG_IDB_READER

#define NB_MAX_RETRIES 3

InternalDatabaseReader::exception::exception(int code,const char *format,...):error(code) {
    va_list optional_arguments;
    va_start(optional_arguments, format);
    vsprintf(msg,format,optional_arguments);
    va_end(optional_arguments);
}

/*
 create temporary view AISData as
select vessels.MMSI,vessels.IMONumber,vessels.callSign,vessels.name,ref_countries.Country
,vessels.type,vessels.dimension_A,vessels.dimension_B,vessels.dimension_C,vessels.dimension_D
,vessels.draught,vessels.typeOfPositionDevice,vessels.CreationTime,vessels.lastUpdateTime
,travels_positions.latitude,travels_positions.longitude,travels_positions.navigationStatus
,travels_positions.rateOfTurn,travels_positions.speedOverGround,travels_positions.positionAccuracy
,travels_positions.courseOverGround,travels_positions.trueHeading,travels_positions.specialManoeuvreIndicator,travels_positions.date
,travels.destination,travels.ETA
from vessels, ref_countries,travels_positions,travels
where (vessels.country = ref_countries.id) and (travels_positions.MMSI = vessels.MMSI) and (travels.MMSI = travels_positions.MMSI)
order by vessels.lastUpdateTime ASC
limit 200

create index vessels_lastUpdateTime on vessels (lastUpdateTime ASC)

create temporary view toto as
select MMSI from AISData  where callSign = "ZDFC2" order by MMSI DESC
 */

inline int InternalDatabaseReader::openDatabase(const char *fileName) {
    int error(EXIT_SUCCESS);
    error =  sqlite3_open(fileName,&database);
    if (0 == error) {
        char *errorMsg = NULL;
        // the configuration of the database has already be done
        // by the writer but it seems that the reader must set the same parameters
        // to avoid to lock the database (or at least some of them) (?).
        error = sqlite3_exec(database, "PRAGMA synchronous = OFF", NULL, NULL, &errorMsg);
        if (unlikely(error != SQLITE_OK)) {
            //std::string exceptionMsg;
            WARNING_MSG("error setting asynchronous mode %d (%s)",error,errorMsg);
            //sprintf(exceptionMsg,"error during database creation: error setting asynchronous mode (%s)",errorMsg);
            sqlite3_free(errorMsg);
            //throw exception(error,exceptionMsg);
        }
        error = sqlite3_exec(database, "PRAGMA journal_mode = "SQLITE_JOURNAL_MODE, NULL, NULL, &errorMsg);
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
        // just add the public interface: the helpers view to use data received by the AIS
        // and the default query view
        //const char *tail = NULL;
        const char *SQLQueries[] = {
                "create temporary view Last_Positions as SELECT MMSI ,latitude ,longitude ,navigationStatus ,rateOfTurn ,speedOverGround ,positionAccuracy ,courseOverGround ,trueHeading ,specialManoeuvreIndicator , DATE FROM TRAVELS_POSITIONS INNER JOIN(SELECT MMSI AS M ,MAX(DATE) AS D FROM TRAVELS_POSITIONS GROUP BY mmsi) L ON (MMSI = L.M AND DATE = L.D)"
                ,"create temporary view Current_AIS_Map as"
                " select VESSELS.name AS name, VESSELS.callSign AS callsign, VESSELS.type AS type, VESSELS.MMSI AS mmsi, VESSELS.IMONumber AS IMONumber, REF_COUNTRIES.Country AS Country,"
                "REF_TYPE_OF_ELECTRONIC_POSITION_FIXING_DEVICE.DeviceType AS PosDeviceType, VESSELS.dimension_A AS dimension_A,"
                "VESSELS.dimension_B AS dimension_B, VESSELS.dimension_C AS dimension_C, VESSELS.dimension_D AS dimension_D, VESSELS.draught AS draught,"
                "VESSELS.CreationTime AS CreationTime, TRAVELS.destination AS destination, TRAVELS.ETA AS ETA, Last_Positions.latitude AS latitude,"
                "Last_Positions.longitude AS longitude, Last_Positions.navigationStatus AS navigationStatus, Last_Positions.rateOfTurn AS rateOfTurn,"
                "Last_Positions.specialManoeuvreIndicator, Last_Positions.trueHeading, Last_Positions.courseOverGround, Last_Positions.positionAccuracy,"
                "Last_Positions.speedOverGround, Last_Positions.[date], VESSELS.lastUpdateTime"
                " FROM VESSELS INNER JOIN REF_TYPE_OF_ELECTRONIC_POSITION_FIXING_DEVICE ON VESSELS.typeOfPositionDevice = REF_TYPE_OF_ELECTRONIC_POSITION_FIXING_DEVICE.id INNER JOIN REF_COUNTRIES ON VESSELS.country = REF_COUNTRIES.id INNER JOIN TRAVELS ON VESSELS.MMSI = TRAVELS.MMSI INNER JOIN Last_Positions ON VESSELS.MMSI = Last_Positions.MMSI"
                " ORDER BY Last_Positions.[date]"

        };
        register unsigned int nbQueries(sizeof(SQLQueries)/sizeof(SQLQueries[0]));
        register const char **SQLQuery = SQLQueries;
        error = SQLITE_OK;
        unsigned int nbRetries(0);
        while ((nbQueries >0) && (SQLITE_OK == error) && (nbRetries < NB_MAX_RETRIES)) {
            error = sqlite3_exec(database,*SQLQuery,NULL,NULL,&errorMsg);
            if (likely(SQLITE_OK == error)) {
                SQLQuery++;
                nbQueries--;
                nbRetries = 0;
            } else {
                if (SQLITE_BUSY == error) {
                    nbRetries++;
                    WARNING_MSG("error during view creation, database is lock, retrying...(%u)",nbRetries);
                } else {
                    std::string exceptionMsg;
                    ERROR_MSG("error during view creation,%s error %d (%s)",*SQLQuery,error,errorMsg);
                    sprintf(exceptionMsg,"error during view creation by request %s (%s)",*SQLQuery,errorMsg);
                    sqlite3_free(errorMsg);
                    throw exception(error,exceptionMsg);
                }
            }
        } // while ((nbQueries >0) && (SQLITE_OK == error))
    } else {
        std::string exceptionMsg;
        ERROR_MSG("error opening database %s, sqlite3_open error %d",fileName,error);
        sprintf(exceptionMsg,"error opening database %s, sqlite3_open error %d",fileName,error);
        throw exception(error,exceptionMsg);
    }
    return error;
}

int InternalDatabaseReader::buildPublisherView(const char *baseQuery,const unsigned int newLimit /*= 0 */) {
    int error(EXIT_SUCCESS);
    MutexMgr mutexMgr(mutex);
    String SQLQuery;
    const unsigned int queryResultsLimit((0 == newLimit)?theConfiguration.getQueryResultsLimit():newLimit);
    char *errorMsg = NULL;

    if (queryResultsLimit != NO_QUERY_LIMIT) {
        SQLQuery.Format("create temporary view publisher as %s limit %u",baseQuery,queryResultsLimit);
    } else {
        SQLQuery.Format("create temporary view publisher as %s",baseQuery);
    }

    // drop previous view (if exists)
    error = sqlite3_exec(database,"drop view publisher",NULL,NULL,&errorMsg);
    if (unlikely(error != SQLITE_OK)) {
        DEBUG_MSG("drop view publisher error %d (%s)",error,errorMsg);
        sqlite3_free(errorMsg);
    }

    // create the view use to extract data
    error = sqlite3_exec(database,(const char*)SQLQuery,NULL,NULL,&errorMsg);
    if (unlikely(error != SQLITE_OK)) {
        std::string exceptionMsg;
        ERROR_MSG("error during view creation,%s error %d (%s)",(const char*)SQLQuery,error,errorMsg);
        sprintf(exceptionMsg,"error during view creation by request %s (%s)",(const char*)SQLQuery,errorMsg);
        sqlite3_free(errorMsg);
        throw exception(error,exceptionMsg);
    }
    currentBaseQuery = baseQuery;
    return error;
}

int InternalDatabaseReader::buildSQLQueryStatement() {
    int error(EXIT_SUCCESS);
    const char *tail = NULL;
    const char *SQLQuery = "select * from publisher";
    error = sqlite3_prepare_v2(database,SQLQuery,strlen(SQLQuery),&SQLQueryStatement,&tail);
    if (SQLITE_OK == error) {
        error = EXIT_SUCCESS;
    } else {
        std::string exceptionMsg;
        ERROR_MSG("SQL prepare request %s error %d at %s",SQLQuery,error,tail);
        sprintf(exceptionMsg,"SQL prepare request %s error %d at %s",SQLQuery,error,tail);
        throw exception(error,exceptionMsg);
    }
    return error;
}

InternalDatabaseReader::InternalDatabaseReader()
:database(NULL)
,SQLQueryStatement(0) {
    int error(EXIT_SUCCESS);
    try {
        const char *filename = theConfiguration.getDatabase();
        openDatabase(filename);
        buildPublisherView("select * from Current_AIS_Map");
        buildSQLQueryStatement();
    } //try
    catch(...) {
        // free all allocated resources before forwarding the exception
        if (SQLQueryStatement != 0) {
            sqlite3_finalize(SQLQueryStatement);
            SQLQueryStatement = 0;
        }
        if (database != NULL) {
            sqlite3_close(database);
            database = NULL;
        }
        throw;
    }
}

int InternalDatabaseReader::getQueryResults(CorbaClientAdaptor &adaptor) {
    int error(EXIT_SUCCESS);
    MutexMgr mutexMgr(mutex);
    const unsigned int numberOfColumns(sqlite3_column_count(SQLQueryStatement));
    adaptor.setNumberOfSubItems(numberOfColumns);

    // set Labels'name corresponding to the current query'fields
    for(unsigned int i=0;i<numberOfColumns;i++) {
        const char *label = sqlite3_column_name(SQLQueryStatement,i);
        DEBUG_MSG("label %u = %s",i,label);
        adaptor.setLabel(i,label);
    }

    // retrieve the result of the current query
    unsigned int row(0);
    std::string errorMsg;
    unsigned int nbRetries(0);
    do {
        error = sqlite3_step(SQLQueryStatement);
        DEBUG_VAR(error,"%d");
        switch(error) {
            case SQLITE_DONE:
                DEBUG_MSG("SQLITE_DONE");
                break;
            case SQLITE_ROW: {
                for(unsigned int column=0;column<numberOfColumns;column++) {
                    const int cellDataType = sqlite3_column_type(SQLQueryStatement,column);
                    DEBUG_VAR(cellDataType,"%d");
                    switch(cellDataType) {
                        case SQLITE_INTEGER: {
                            const int value = sqlite3_column_int(SQLQueryStatement,column);
                            DEBUG_VAR(value,"%d");
                            adaptor.setValue(row,column,value);
                        }
                        break;
                        case SQLITE_FLOAT: {
                            const double value = sqlite3_column_double(SQLQueryStatement,column);
                            DEBUG_VAR(value,"%f");
                            adaptor.setValue(row,column,value);
                        }
                        break;
                        case SQLITE_TEXT: {
                            const char *value = (char *)sqlite3_column_text(SQLQueryStatement,column);
                            DEBUG_VAR(value,"%s");
                            adaptor.setValue(row,column,value);
                        }
                        break;
                        case SQLITE_BLOB:
                            NOTICE_MSG("BLOB (not yet supported)");
                            break;
                        case SQLITE_NULL:
                            DEBUG_MSG("NULL");
                            // do Nothing
                            break;
                    } //switch(cellDataType)
                } //for(unsigned int column=0;column<numberOfColumns;column++)
            } //case SQLITE_ROW
            break;
            case SQLITE_BUSY:
                DEBUG_MSG("SQLITE_BUSY");
                nbRetries++;
                WARNING_MSG("error during query execution, database is lock, retrying...(%u)",nbRetries);
                break;
            case SQLITE_ERROR: {
                const char *msg = sqlite3_errmsg(database);
                ERROR_MSG("SQLITE_ERROR %s",msg);
                errorMsg = msg;
            }
            break;
            case SQLITE_MISUSE: {
                const char *msg = sqlite3_errmsg(database);
                ERROR_MSG("SQLITE_MISUSE %s",msg);
                errorMsg = msg;
            }
            break;
            default: {
                const char *msg = sqlite3_errmsg(database);
                ERROR_MSG("SQLite return code %d,  %s",msg);
                errorMsg = msg;
            }
            break;
        } // switch(error)
        ++row;
        DEBUG_MSG("new line (%u)",row);
    } while((SQLITE_ROW == error) || ((SQLITE_BUSY == error) && (nbRetries < NB_MAX_RETRIES)));
    sqlite3_reset(SQLQueryStatement);
    if (error != SQLITE_DONE) {
        throw exception(error,errorMsg);
    }
    return error;
}

InternalDatabaseReader::~InternalDatabaseReader() {
    if (database != NULL) {
        if (SQLQueryStatement != 0) {
            sqlite3_finalize(SQLQueryStatement);
            SQLQueryStatement = 0;
        }
        sqlite3_close(database);
        database = NULL;
    }
}

