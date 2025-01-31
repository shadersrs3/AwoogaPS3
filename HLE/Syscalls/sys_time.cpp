#include <HLE/HLE.h>
#include <HLE/PPUWrapper.h>

#include <Common/Logger.h>

static void syscall_sys_timer_usleep(PowerProcessor *_cpu, uint64_t sleeptime) {
    _cpu->getThread()->delayDowncount = 333 * sleeptime;
    // UNIMPLEMENTED_FUNCTION;
}

static FunctionMap sys_timer_functions = {
    {
        HLE_FUNCTION_EXPORT(0x8D, syscall_sys_timer_usleep),
    }
};

void register_syscall_sys_timer_functions() {
    for (auto& i : sys_timer_functions) __registerSyscall(i.first, i.second.exec);
}