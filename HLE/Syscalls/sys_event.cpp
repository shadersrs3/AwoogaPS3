#include <HLE/HLE.h>
#include <HLE/PPUWrapper.h>

#include <Common/Logger.h>

static void syscall_sys_event_queue_create(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void syscall_sys_event_queue_receive(PowerProcessor *_cpu, u32 equeue_id, _ptr<u64_be> ev, u64 timeout) {
    // UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void syscall_sys_event_port_create(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void syscall_sys_event_port_send(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void syscall_sys_event_port_connect_local(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void syscall_sys_event_flag_create(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static FunctionMap sys_event_functions = {
    {
        HLE_FUNCTION_EXPORT(0x80, syscall_sys_event_queue_create),
        HLE_FUNCTION_EXPORT(0x82, syscall_sys_event_queue_receive),
        HLE_FUNCTION_EXPORT(0x86, syscall_sys_event_port_create),
        HLE_FUNCTION_EXPORT(0x88, syscall_sys_event_port_connect_local),
        HLE_FUNCTION_EXPORT(0x8A, syscall_sys_event_port_send),
        HLE_FUNCTION_EXPORT(0x52, syscall_sys_event_flag_create),
    }
};

void register_syscall_sys_event_functions() {
    for (auto& i : sys_event_functions) __registerSyscall(i.first, i.second.exec);
}