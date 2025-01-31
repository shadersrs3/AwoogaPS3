#include <HLE/HLE.h>
#include <HLE/PPUWrapper.h>

#include <Common/Logger.h>

static void cellSysmoduleLoadModule(PowerProcessor *_cpu, int moduleId) {
    LOG_WARN("UNIMPLEMENTED %s SYS MODULE ID %08X", __func__, moduleId);
    _PPU_RETURN(0);
}

static void cellSysmoduleIsLoaded(PowerProcessor *_cpu, int moduleId) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellSysmoduleUnloadModule(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static HLEImportModule Module_cellSysmodule = {
    "cellSysmodule",
    {
        HLE_FUNCTION_EXPORT(0x32267A31, cellSysmoduleLoadModule),
        HLE_FUNCTION_EXPORT(0x5A59E258, cellSysmoduleIsLoaded),
        HLE_FUNCTION_EXPORT(0x112A5EE9, cellSysmoduleUnloadModule),
    },
    nullptr, nullptr, nullptr,
};

HLEImportModule *get_cellSysmodule() {
    return &Module_cellSysmodule;
}
