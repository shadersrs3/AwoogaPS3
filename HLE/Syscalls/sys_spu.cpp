#include <HLE/HLE.h>
#include <HLE/PPUWrapper.h>

#include <Common/Logger.h>

static void syscall_sys_spu_initialize(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void syscall_sys_spu_thread_connect_event(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static FunctionMap sys_spu_functions = {
    {
        HLE_FUNCTION_EXPORT(0xA9, syscall_sys_spu_initialize),
        HLE_FUNCTION_EXPORT(0xBF, syscall_sys_spu_thread_connect_event),
    }
};

void register_syscall_sys_spu_functions() {
    for (auto& i : sys_spu_functions) __registerSyscall(i.first, i.second.exec);
}
