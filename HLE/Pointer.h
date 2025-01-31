#ifndef _HLE_POINTER_H
#define _HLE_POINTER_H

#include <Common/Types.h>

struct _ptr_base {};

template<typename T>
struct _ptr : public _ptr_base {
    uint64_t address;
    T *data;

    typedef T type;
    operator bool() { return !!data; }
    T *operator->() { return data; }
    T *operator()() { return data; }
    const uint64_t& getAddress() { return address; }
};

#endif