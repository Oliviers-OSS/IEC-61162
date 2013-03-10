/*
 * basicsmartpointer.h
 *
 *  Created on: 7 mars 2013
 *      Author: oc
 */

#ifndef _BASIC_SMART_POINTER_H_
#define _BASIC_SMART_POINTER_H_

//#define LOGGER consoleLogger
#include <stdexcept>
#include <dbgflags-1/loggers.h>
#include <dbgflags/debug_macros.h>
#include <cassert>

//#define MODULE_FLAG SMART_POINTER

template <typename Object> class basic_smart_pointer {
	Object *managed;
	unsigned long *counter;

	void copy(const basic_smart_pointer<Object> &src) {
		managed = src.managed;
		counter = src.counter;
		__sync_add_and_fetch(counter,1);
		//DEBUG_VAR(*counter,"%u");
		//DEBUG_VAR(managed,"0x%X");
	}

	void release() {
		const unsigned long newValue = __sync_sub_and_fetch(counter,1);
		if (0 == newValue) {
			delete managed;
			delete counter;
			//DEBUG_MSG("0x%X freed",managed);
		} else {
		    //DEBUG_VAR(*counter,"%u");
		}
		managed = 0;
		counter = 0;
	}

public:

	basic_smart_pointer(Object *obj) __attribute__ ((nonnull(1)))
		:managed(obj),counter(new unsigned long(1)) {
	}

	basic_smart_pointer(const basic_smart_pointer<Object> &src) {
		copy(src);
	}

	~basic_smart_pointer() {
		release();
	}

	Object& operator* () {
	    return *managed;
	}

	Object* operator-> () {
	   return managed;
	}

	basic_smart_pointer<Object>& operator = (const basic_smart_pointer<Object>& src) {
		release();
		copy(src);
		return *this;
	}

	basic_smart_pointer<Object>& operator = (Object *obj) {
		release();
		managed = obj;
		counter = new unsigned long(1);
		return *this;
	}

	bool isNull() {
		return (0 == managed);
	}

	// WARNING: this pointer may be freed at any time !!!
	Object* ptr() {
	    return managed;
	}

	// TODO: set this constructor private
	basic_smart_pointer(Object *obj,unsigned long *c) __attribute__ ((nonnull(1))) __attribute__ ((nonnull(2)))
        :managed(obj),counter(c) {
        if (counter != 0) {
            __sync_add_and_fetch(counter,1);
        } else {
            throw std::runtime_error("invalid counter pointer");
        }
    }

	// TODO: use typeof to check the destination at compile time instead of rtti at runtime
	template <typename DerivedObject>
	basic_smart_pointer<DerivedObject> down_cast() {
	    DerivedObject *down_cast_object = dynamic_cast<DerivedObject*>(managed);
	    if (down_cast_object) {
	        basic_smart_pointer<DerivedObject> down_cast_smart_ptr(down_cast_object,counter);
	        return down_cast_smart_ptr;
	    }else {
            throw std::runtime_error("bad cast");
        }
	}
};


#undef MODULE_FLAG

#endif /* _BASIC_SMART_POINTER_H_ */
