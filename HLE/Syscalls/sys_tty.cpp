#include <HLE/HLE.h>
#include <HLE/PPUWrapper.h>

#include <Common/Logger.h>

static void syscall_sys_tty_write(PowerProcessor *_cpu, uint32_t ch, _ptr<void> buf, uint32_t len, _ptr<u32_be> preadLen) {
    //std::exit(0);
    UNIMPLEMENTED_FUNCTION;
}

static FunctionMap sys_tty_functions = {
    {
        HLE_FUNCTION_EXPORT(0x193, syscall_sys_tty_write)
    }
};

void register_syscall_sys_tty_functions() {
    for (auto& i : sys_tty_functions) __registerSyscall(i.first, i.second.exec);
}
