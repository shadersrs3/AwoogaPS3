#include <HLE/HLE.h>
#include <HLE/PPUWrapper.h>

#include <Common/Logger.h>

typedef struct sys_ppu_thread_stack {
    u32_be pst_addr;
    u32_be pst_size;
} sys_ppu_thread_stack_t;

static void syscall_sys_ppu_thread_set_priority(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void syscall_sys_ppu_thread_get_priority(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void syscall_sys_ppu_thread_get_stack_information(PowerProcessor *_cpu, _ptr<sys_ppu_thread_stack_t> info) {
    info->pst_addr.setValue(0x80000000);
    info->pst_size.setValue(0x4000000);
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void syscall_sys_ppu_thread_yield(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static FunctionMap sys_ppu_thread_functions = {
    {
        HLE_FUNCTION_EXPORT(0x2F, syscall_sys_ppu_thread_set_priority),
        HLE_FUNCTION_EXPORT(0x30, syscall_sys_ppu_thread_get_priority),
        HLE_FUNCTION_EXPORT(0x31, syscall_sys_ppu_thread_get_stack_information),
        HLE_FUNCTION_EXPORT(0x2B, syscall_sys_ppu_thread_yield)
    }
};

void register_syscall_sys_ppu_thread_functions() {
    for (auto& i : sys_ppu_thread_functions) __registerSyscall(i.first, i.second.exec);
}
