/*
 * String.h
 *
 *  Created on: 22 août 2013
 *      Author: oc
 * Very basic string class using cstdio to quickly solve a few issues
 * Not intended to be a substitute of std::string
 */

#ifndef _STRING_H_
#define _STRING_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif //_GNU_SOURCE

#include <cstdio>
#include <cstring>
#include <string>
#include <ostream>
#include <cstdarg>
#include <stdexcept>
#include <malloc.h>

#if (GCC_VERSION > 40000) /* GCC 4.0.0 */
#define CHECK_NON_NULL_PTR(n)   __attribute__ ((nonnull(n)))
#else
#define CHECK_NON_NULL_PTR(n)
#endif /* (GCC_VERSION > 40000) */

class String {
    char *buffer;
public:
    String():buffer(0) {
    }

    String(const String &string):buffer(0) {
        const char *sourceBuffer = (const char*)string;
        if (sourceBuffer != buffer) {
            if (buffer) {
                free(buffer);
            }
            if (sourceBuffer) {
                buffer = strdup(sourceBuffer);
                if (0 == buffer) {
                    throw std::bad_alloc();
                }
            } else {
                buffer = 0;
            }
        }
    }

    String(const char *string):buffer(0) {
        if (string) {
            buffer = strdup(string);
            if (0 == buffer) {
                throw std::bad_alloc();
            }
        }
    }

    String(const std::string s):buffer(0) {
        buffer = strdup(s.c_str());
        if (0 == buffer) {
            throw std::bad_alloc();
        }
    }

    virtual ~String() {
        if (buffer) {
            free(buffer);
        }
    }

    operator const char*() const {
        return buffer;
    }

    operator std::string() const {
        if (buffer) {
            return std::string(buffer);
        } else {
            return std::string();
        }
    }

    bool isEmpty() const {
        return (0 == buffer);
    }

    size_t length() const {
        if (buffer) {
            return strlen(buffer);
        } else {
            return 0;
        }
    }

    int Format(const char *format,...) CHECK_NON_NULL_PTR(1);
    int vFormat(const char *format,va_list optional_arguments) CHECK_NON_NULL_PTR(1);

    char& operator[](const unsigned int i) const {
        if (buffer) {
            const size_t limit = strlen(buffer);
            if (i <= limit) {
                return buffer[i];
            }
            String errorMsg;
            errorMsg.Format("error index value %u is not valid (max is %u)",i,limit);
            throw std::range_error(errorMsg);
        } else {
            throw std::range_error("error empty string");
        }
    }

    String& operator=(const String& src) {
        const char *sourceBuffer = (const char*)src;
        if (sourceBuffer != buffer) {
            if (buffer) {
                free(buffer);
            }
            if (sourceBuffer) {
                buffer = strdup(sourceBuffer);
                if (0 == buffer) {
                    throw std::bad_alloc();
                }
            } else {
                buffer = 0;
            }
        }
        return (*this);
    }

    String& operator=(const char *src) {
        if (buffer != src) {
            if (buffer) {
                free(buffer);
            }
            if (src) {
                buffer = strdup(src);
                if (0 == buffer) {
                    throw std::bad_alloc();
                }
            } else {
                buffer = 0;
            }
        }
        return (*this);
    }

    String& operator=(const std::string &src) {
        if (buffer) {
            free(buffer);
        }
        buffer = strdup(src.c_str());
        if (0 == buffer) {
            throw std::bad_alloc();
        }
        return (*this);
    }
};

inline std::ostream& operator<< (std::ostream &o,const String &string) {
  if (!string.isEmpty()) {
      return o << (const char *)string;
  } else {
      return o;
  }
}

inline std::ostream& operator<< (std::ostream &o,const String *string) {
  if (!string->isEmpty()) {
      return o << (const char *) (*string);
  } else {
      return o;
  }
}

inline bool operator==(const String &string1,const String &string2) {
    if (string1.isEmpty()) {
        if (string2.isEmpty()) {
            return true;
        } else {
            return false;
        }
    } else {
        if (string2.isEmpty()) {
            return false;
        } else {
            return (strcmp(string1,string2) == 0);
        }
    }
}
#endif /* _STRING_H_ */
