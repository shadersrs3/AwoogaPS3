#include <HLE/HLE.h>
#include <HLE/PPUWrapper.h>

#include <Common/Logger.h>

static void cellSysutilRegisterCallback(PowerProcessor *_cpu, int slot) {
    LOG_WARN("UNIMPLEMENTED %s SLOT ID %08X", __func__, slot);
    _PPU_RETURN(0);
}

static void cellSysutilGetSystemParamInt(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellSysutilGetSystemParamString(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellVideoOutGetResolutionAvailability(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellVideoOutConfigure(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellVideoOutGetState(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellSysCacheMount(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellSysutilUnregisterCallback(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellSysutilCheckCallback(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static HLEImportModule Module_cellSysutil = {
    "cellSysutil",
    {
        HLE_FUNCTION_EXPORT(0x9D98AFA0, cellSysutilRegisterCallback),
        HLE_FUNCTION_EXPORT(0x40E895D3, cellSysutilGetSystemParamInt),
        HLE_FUNCTION_EXPORT(0x938013A0, cellSysutilGetSystemParamString),
        HLE_FUNCTION_EXPORT(0xA322DB75, cellVideoOutGetResolutionAvailability),
        HLE_FUNCTION_EXPORT(0x0BAE8772, cellVideoOutConfigure),
        HLE_FUNCTION_EXPORT(0x887572D5, cellVideoOutGetState),
        HLE_FUNCTION_EXPORT(0x1E7BFF94, cellSysCacheMount),
        HLE_FUNCTION_EXPORT(0x02FF3C1B, cellSysutilUnregisterCallback),
        HLE_FUNCTION_EXPORT(0x189A74DA, cellSysutilCheckCallback),
    },
    nullptr, nullptr, nullptr,
};

HLEImportModule *get_cellSysutil() {
    return &Module_cellSysutil;
}