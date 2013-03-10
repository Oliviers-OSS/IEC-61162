/*
 * MapServantImpl.cpp
 *
 *  Created on: 21 août 2013
 *      Author: oc
 */

#include "globals.h"

#include "MapServantImpl.h"
#define MODULE_FLAG FLAG_CORBA

MapServantImpl::MapServantImpl() {
    TRACE_FCT_CALL;
}

MapServantImpl::~MapServantImpl() {
    TRACE_FCT_CALL;
}

// CORBA interface methods

// void setFilter(in string query);
void MapServantImpl::setFilter (const char * query) ACE_THROW_SPEC ((CORBA::SystemException,AIS::Map::FilterError)) {
    TRACE_FCT_CALL;
    try {
        reader.setFilter(query);
    }
    catch(InternalDatabaseReader::exception &e) {
        throw AIS::Map::FilterError(e.code(),CORBA::string_dup(e.what()));
    }
}

// void getFilter(out string query);
void MapServantImpl::getFilter(::CORBA::String_out query) ACE_THROW_SPEC ((CORBA::SystemException,AIS::Map::FilterError)) {
    TRACE_FCT_CALL;
    query = CORBA::string_dup(reader.getFilter());
}

// attribute unsigned long maxAnswerSize;
::CORBA::ULong MapServantImpl::maxAnswerSize (void) ACE_THROW_SPEC ((CORBA::SystemException)) {
    TRACE_FCT_CALL;
    return reader.getQueryResultsLimit();
}

void MapServantImpl::maxAnswerSize(::CORBA::ULong maxAnswerSize) ACE_THROW_SPEC ((CORBA::SystemException)) {
    TRACE_FCT_CALL;
    try {
        reader.setQueryResultsLimit(maxAnswerSize);
    }
    catch(InternalDatabaseReader::exception &e) {
        throw AIS::Map::FilterError(e.code(),CORBA::string_dup(e.what()));
    }
}

// void getShips(out Ships vessels);
void MapServantImpl::getShips (::AIS::VesselsData_out vessels) ACE_THROW_SPEC ((CORBA::SystemException,AIS::Map::FilterError)) {
    TRACE_FCT_CALL;
    try {
        ::AIS::VesselsData_var result = new ::AIS::VesselsData;
        CorbaClientAdaptor adaptor(result);
        reader.getQueryResults(adaptor);
        vessels = result.out();
    }
    catch(InternalDatabaseReader::exception &e) {
        ERROR_MSG("InternalDatabaseReader::exception: %s",e.what());
        throw AIS::Map::FilterError(e.code(),CORBA::string_dup(e.what()));
    }
    catch(std::bad_alloc &e) {
        std::string errorMsg;
        sprintf(errorMsg,"Failed to allocate %u byte for a new ::AIS::VesselsData",sizeof(::AIS::VesselsData));
        ERROR_MSG("std::bad_alloc: %s",errorMsg.c_str());
        throw AIS::Map::FilterError(ENOMEM,errorMsg.c_str());
    }
}
