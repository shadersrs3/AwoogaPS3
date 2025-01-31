#include <HLE/HLE.h>
#include <HLE/PPUWrapper.h>

#include <Common/Logger.h>

static void cellNetCtlInit(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellNetCtlAddHandler(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellNetCtlGetState(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellNetCtlGetInfo(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static HLEImportModule Module_cellNetCtl = {
    "cellNetCtl",
    {
        HLE_FUNCTION_EXPORT(0xBD5A59FC, cellNetCtlInit),
        HLE_FUNCTION_EXPORT(0x0CE13C6B, cellNetCtlAddHandler),
        HLE_FUNCTION_EXPORT(0x8B3EBA69, cellNetCtlGetState),
        HLE_FUNCTION_EXPORT(0x1E585B5D, cellNetCtlGetInfo),
},
nullptr, nullptr, nullptr
};

HLEImportModule *get_cellNetCtl() {
    return &Module_cellNetCtl;
}