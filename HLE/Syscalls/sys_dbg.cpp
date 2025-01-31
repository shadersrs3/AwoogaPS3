#include <HLE/HLE.h>
#include <HLE/PPUWrapper.h>

#include <Common/Logger.h>

static void syscall_sys_dbg_unknown_ppu_exception_handler(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static FunctionMap sys_dbg_functions = {
    {
        HLE_FUNCTION_EXPORT(0x3DC, syscall_sys_dbg_unknown_ppu_exception_handler)
    }
};

void register_syscall_sys_dbg_functions() {
    for (auto& i : sys_dbg_functions) __registerSyscall(i.first, i.second.exec);
}
