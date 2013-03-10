/*
 * CorbaClientAdaptor.h
 *
 *  Created on: 25 août 2013
 *      Author: oc
 * Adaptor between SQLIte data type and CORBA data type
 * to try to minimize copies from a result of a SQLite request
 * to the CORBA interfaces to send them
 */

#ifndef _CORBA_CLIENT_ADAPTOR_H_
#define _CORBA_CLIENT_ADAPTOR_H_

#include "globals.h"
#include "aisS.h"
#include "Configuration.h"

#define MODULE_FLAG FLAG_CORBA

class CorbaClientAdaptor {
    ::AIS::VesselsData_var data; // from IDL
    unsigned int numberOfsubItems; // number of 'columns'
    unsigned int numberOfItems; // number of 'rows'
    //unsigned int currentItemNumber;

    unsigned int getQueryResultsLimit() {
        unsigned int resultsLimit = theConfiguration.getQueryResultsLimit();
        if (resultsLimit > 200) resultsLimit = 200;
        return resultsLimit;
    }
public:
    CorbaClientAdaptor(::AIS::VesselsData_var v)
        :data(v)
        ,numberOfsubItems(0)
        ,numberOfItems(0) {
        // initialize inner sequences
        const unsigned int resultsLimit = getQueryResultsLimit();
        data->data.length(resultsLimit);
    }

    virtual ~CorbaClientAdaptor() {

    }

    void setNumberOfSubItems(const unsigned int value) {
        numberOfsubItems = value;
        data->names.length(numberOfsubItems);
        unsigned int i;
        const unsigned int resultsLimit = getQueryResultsLimit();
        data->data.length(resultsLimit);
        for(i=0;i<resultsLimit;i++) {
            data->data[i].length(numberOfsubItems);
        }
    }

    void setLabel(const unsigned int subItemNumber,const char *label) {
        data->names[subItemNumber] = CORBA::string_dup(label);
    }

    const char *getLabel(const unsigned int subItemNumber) {
         return (const char*)data->names[subItemNumber];
    }

    CORBA::Any getValue(const unsigned int itemNumber,const unsigned int subItemNumber) {
        return data->data[itemNumber][subItemNumber];
    }

    void setValue(const unsigned int itemNumber,const unsigned int subItemNumber,const char *value) {
        data->data[itemNumber][subItemNumber] <<= value;
    }

    void setValue(const unsigned int itemNumber,const unsigned int subItemNumber,const double &value) {
        data->data[itemNumber][subItemNumber] <<= value;
    }

    void setValue(const unsigned int itemNumber,const unsigned int subItemNumber,const int value) {
        data->data[itemNumber][subItemNumber] <<= value;
    }

    void setValue(const unsigned int itemNumber,const unsigned int subItemNumber,const long long value) {
        data->data[itemNumber][subItemNumber] <<= value;
    }

    void reSizeData(const size_t newSize) {
        data->data.length(newSize);
    }

    size_t getSizeData() {
        return data->data.length();
    }
};

#undef MODULE_FLAG

#endif /* _CORBA_CLIENT_ADAPTOR_H_ */
