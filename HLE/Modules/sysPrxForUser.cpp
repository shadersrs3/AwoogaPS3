#include <HLE/HLE.h>
#include <HLE/PPUWrapper.h>

#include <Common/Logger.h>

typedef struct sys_lwmutex_lock_info {
    u32_be owner;
    u32_be waiter;
} sys_lwmutex_lock_info_t;

typedef union sys_lwmutex_variable {
    sys_lwmutex_lock_info_t info;
    u64_be all_info;
} sys_lwmutex_variable_t;

typedef struct sys_lwmutex {
    sys_lwmutex_variable_t lock_var;
    u32_be attribute;
    u32_be recursive_count;
    u32_be sleep_queue;
    u32 pad;
} sys_lwmutex_t;

typedef struct lwmutex_attr {
    u32_be attr_protocol;
    u32_be attr_recursive;
    char name[8];
} sys_lwmutex_attribute_t;

static void sys_initialize_tls(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void sys_lwmutex_create(PowerProcessor *_cpu, _ptr<sys_lwmutex_t> mutex, _ptr<sys_lwmutex_attribute_t> attr) {
    std::memset(mutex(), 0x00, sizeof *mutex.data);
    // UNIMPLEMENTED_FUNCTION;
    LOG_INFO("Lwmutex name %s", attr->name);
    _PPU_RETURN(0);
}

static void sys_time_get_system_time(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void sys_lwmutex_lock(PowerProcessor *_cpu, _ptr<sys_lwmutex_t> mutex) {
    // UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void sys_lwmutex_unlock(PowerProcessor *_cpu, _ptr<sys_lwmutex_t> mutex) {
    // UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void sys_ppu_thread_get_id(PowerProcessor *_cpu, _ptr<u32_be> threadId) {
    threadId->setValue(0x1);
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void _sys_process_atexitspawn(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void _sys_process_at_Exitspawn(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

extern std::vector<Thread *> threads;

static void sys_ppu_thread_create(PowerProcessor *_cpu, _ptr<u64_be> thread_id, _ptr<u64_be> entry, uint64_t arg, int prio, uint64_t stacksize, uint64_t flags, _ptr<char> threadname) {
    Thread *t = new Thread;
    std::memset(t, 0, sizeof *t);
    std::strcpy(t->name, threadname());
    t->PC = entry->getValue() >> 32;
    t->GPR[3] = arg;
    static uint64_t _stacksize;
    t->GPR[1] = 0x8000F000 + _stacksize + stacksize;
    _stacksize += stacksize;

    threads.push_back(t);
    LOG_INFO("Thread name %s", threadname());
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
    //std::exit(0);
}

static HLEImportModule Module_sysPrxForUser = {
    "sysPrxForUser",
    {
        HLE_FUNCTION_EXPORT(0x744680A2, sys_initialize_tls),
        HLE_FUNCTION_EXPORT(0x2F85C0EF, sys_lwmutex_create),
        HLE_FUNCTION_EXPORT(0x8461E528, sys_time_get_system_time),
        HLE_FUNCTION_EXPORT(0x1573DC3F, sys_lwmutex_lock),
        HLE_FUNCTION_EXPORT(0x1BC200F4, sys_lwmutex_unlock),
        HLE_FUNCTION_EXPORT(0x350D454E, sys_ppu_thread_get_id),
        HLE_FUNCTION_EXPORT(0x2C847572, _sys_process_atexitspawn),
        HLE_FUNCTION_EXPORT(0x96328741, _sys_process_at_Exitspawn),
        HLE_FUNCTION_EXPORT(0x24A1EA07, sys_ppu_thread_create),
},
nullptr, nullptr, nullptr,
};

HLEImportModule *get_sysPrxForUser() {
    return &Module_sysPrxForUser;
}