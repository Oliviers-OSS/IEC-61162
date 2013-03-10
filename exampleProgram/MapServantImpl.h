/*
 * MapServantImpl.h
 *
 *  Created on: 21 août 2013
 *      Author: oc
 */

#ifndef _MAP_SERVANTIMPL_H_
#define _MAP_SERVANTIMPL_H_

#include "aisS.h"
#include "InternalDatabaseReader.h"

class MapServantImpl: public POA_AIS::Map {
    MapServantImpl(const MapServantImpl& orig) ACE_THROW_SPEC ((CORBA::SystemException,AIS::Map::FilterError)) {
        // copy not allowed
    }
    InternalDatabaseReader reader;
public:
    MapServantImpl();
    virtual ~MapServantImpl();

    // CORBA interface methods

    // void setFilter(in string query);
    virtual void setFilter (const char * query) ACE_THROW_SPEC ((CORBA::SystemException,AIS::Map::FilterError));

    // void getFilter(out string query);
    virtual void getFilter(::CORBA::String_out query) ACE_THROW_SPEC ((CORBA::SystemException,AIS::Map::FilterError));

    // attribute unsigned long maxAnswerSize;
    virtual ::CORBA::ULong maxAnswerSize (void) ACE_THROW_SPEC ((CORBA::SystemException));
    virtual void maxAnswerSize(::CORBA::ULong maxAnswerSize) ACE_THROW_SPEC ((CORBA::SystemException));

    // void getShips(out Ships vessels);
    virtual void getShips (::AIS::VesselsData_out vessels) ACE_THROW_SPEC ((CORBA::SystemException,AIS::Map::FilterError));
};

#endif /* _MAP_SERVANTIMPL_H_ */
