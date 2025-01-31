#include <HLE/HLE.h>
#include <HLE/PPUWrapper.h>

#include <Common/Logger.h>

typedef struct sys_memory_info {
    u32_be total_user_memory;
    u32_be available_user_memory;
} sys_memory_info_t;

static void syscall_sys_memory_allocate(PowerProcessor *_cpu, uint64_t size, uint64_t flags, _ptr<u32_be> alloc_addr) {
    static int calledOnce;
    uint64_t memoryArenaStart, memoryArenaEnd;

    if (!calledOnce) {
        memoryArenaStart = 0x20000000uLL;
        memoryArenaEnd = ((0x20000000uLL + size + 0xFFFFuLL) & ~0xFFFF) - 1;
        if (!_cpu->getMemory()->mmap(memoryArenaStart, (memoryArenaEnd - memoryArenaStart) + 1)) {
            LOG_ERROR("CANT ALLOCATE THE FUCKING MEMORY");
        }

        LOG_WARN("%s: Mapped region 0x%08llx .. 0x%08llx", __func__, memoryArenaStart, memoryArenaEnd);
        alloc_addr->setValue((uint32_t) memoryArenaStart);
        calledOnce = 1;
    } else {
        printf("FUCK OFF SYS MEMORY ALLOCATE\n");
        std::exit(0);
    }

    _PPU_RETURN(0);
}

static void syscall_sys_memory_get_user_memory_size(PowerProcessor *_cpu, _ptr<sys_memory_info_t> memInfo) {
    if (!memInfo) {
        LOG_WARN("Memory info structure is unimplemented for %s", __FUNCTION__);
    } else {
        UNIMPLEMENTED_FUNCTION;
        memInfo->total_user_memory.setValue(0x10000000);
        memInfo->available_user_memory.setValue(0x10000000);
    }
    _PPU_RETURN(0);
}

static FunctionMap sys_memory_functions = {
    {
        HLE_FUNCTION_EXPORT(0x160, syscall_sys_memory_get_user_memory_size),
        HLE_FUNCTION_EXPORT(0x15C, syscall_sys_memory_allocate),
    }
};

void register_syscall_sys_memory_functions() {
    for (auto& i : sys_memory_functions) __registerSyscall(i.first, i.second.exec);
}