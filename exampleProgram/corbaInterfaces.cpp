/*
 * corbaInterfaces.cpp
 *
 *  Created on: 22 août 2013
 *      Author: oc
 */

#include "globals.h"
#include "corbaInterfaces.h"
#include "aisS.h"
#include "MapServantImpl.h"
#include <orbsvcs/CosNamingC.h>

#define MODULE_FLAG FLAG_CORBA

#ifdef DISABLE_CORBA_NS_KIND
#define CORBA_NS_KIND ""
#else /* DISABLE_CORBA_NS_KIND */
#define CORBA_NS_KIND "AIS"
#endif /* DISABLE_CORBA_NS_KIND */

static inline void registerServants(CosNaming::NamingContext_var initialNamingContext, ::AIS::Map_var mapServant) {
    CosNaming::Name name;

    name.length(1);
    name[0].id = CORBA::string_dup("map");
    name[0].kind = CORBA::string_dup(CORBA_NS_KIND);
    initialNamingContext->rebind(name, mapServant);
}

static inline void unregisterServants(CosNaming::NamingContext_var initialNamingContext) {
    CosNaming::Name name;

    name.length(1);
    name[0].id = CORBA::string_dup("map");
    name[0].kind = CORBA::string_dup(CORBA_NS_KIND);
    initialNamingContext->unbind(name);
}

void *ORBMainThread (void *parameter) {
    int error(EXIT_SUCCESS);
    ProcessStartParameters *params = (ProcessStartParameters *)parameter;
    try {
        // init ORB.
        CORBA::ORB_var orb = CORBA::ORB_init(params->argc, params->argv);

        // activate root POA.
        CORBA::Object_var poa_object = orb->resolve_initial_references("RootPOA");
        PortableServer::POA_var root_poa = PortableServer::POA::_narrow(poa_object);
        PortableServer::POAManager_var poa_manager = root_poa->the_POAManager();
        poa_manager->activate();

        // create CORBA servants objects.
        MapServantImpl mapImpl;
        ::AIS::Map_var mapServant = mapImpl._this();

        try { //else the process will crash in case of NameService failure because of the ORB hasn't been cleanup
            // get reference to naming service.
            CORBA::Object_var initialNamingContextObject = orb->resolve_initial_references("NameService");
            if (!CORBA::is_nil(initialNamingContextObject)) {
                CosNaming::NamingContext_var initialNamingContext = CosNaming::NamingContext::_narrow(initialNamingContextObject);
                if (!CORBA::is_nil(initialNamingContext)) {
                    registerServants(initialNamingContext,mapServant);

                    // run the ORB & wait for clients
                    INFO_MSG("Servants started");
                    orb->run();
                    INFO_MSG("Servants ended");

                    unregisterServants(initialNamingContext);
                } else {
                    ERROR_MSG("naming_context NameService is nil !");
                    error = EPROTO;
                }
            } else {
                ERROR_MSG("resolve_initial_references NameService is nil !");
                error = EPROTO;
            }

        } //try
        catch (CORBA::Exception &ex) {
            ERROR_STREAM << "CORBA::Exception " << ex << std::endl;
            error = EIO; //TODO: set better (real ?) error code number
        }
        catch(...) {
            ERROR_MSG("catch all: exception caught !");
            error = EFAULT;
        }
        // clean up.
        root_poa->destroy(1, 1);
        orb->destroy();
    } //try
    catch (CORBA::Exception & ex) {
        ERROR_STREAM << "CORBA::Exception " << ex << std::endl;
        error = EIO; //TODO: set better (real ?) error code number
    }
    catch(InternalDatabaseReader::exception &ex) {
        ERROR_STREAM << "InternalDatabaseReader::Exception " << ex << std::endl;
        error = ex.code();
    }


    DEBUG_VAR(error, "%d");
    return NULL;
}

