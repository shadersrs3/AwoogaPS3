#include <HLE/HLE.h>
#include <HLE/PPUWrapper.h>

#include <Common/Logger.h>

static void cellFsStat(PowerProcessor *_cpu, _ptr<char> path) {
    LOG_INFO("Filesystem Path: %s", path());
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
    // std::exit(0);
}

static void cellFsOpen(PowerProcessor *_cpu, _ptr<char> path, int flags, _ptr<int> fd, uint64_t arg, uint64_t size) {
    LOG_INFO("FS Path: %s", path());
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellFsFstat(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellFsLseek(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellFsRead(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static HLEImportModule Module_sys_fs = {
    "sys_fs",
    {
        HLE_FUNCTION_EXPORT(0x7DE6DCED, cellFsStat),
        HLE_FUNCTION_EXPORT(0x718BF5F8, cellFsOpen),
        HLE_FUNCTION_EXPORT(0xEF3EFA34, cellFsFstat),
        HLE_FUNCTION_EXPORT(0xA397D042, cellFsLseek),
        HLE_FUNCTION_EXPORT(0x4D5FF8E2, cellFsRead),
    },
    nullptr, nullptr, nullptr
};

HLEImportModule *get_sys_fs() {
    return &Module_sys_fs;
}
