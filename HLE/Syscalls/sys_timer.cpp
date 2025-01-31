#include <HLE/HLE.h>
#include <HLE/PPUWrapper.h>

#include <Common/Logger.h>

#define TBR_FIXED_VALUE 0x2000000

static void syscall_sys_time_get_timebase_frequency(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(TBR_FIXED_VALUE);
}

static FunctionMap sys_time_functions = {
    {
        HLE_FUNCTION_EXPORT(0x93, syscall_sys_time_get_timebase_frequency),
    }
};

void register_syscall_sys_time_functions() {
    for (auto& i : sys_time_functions) __registerSyscall(i.first, i.second.exec);
}