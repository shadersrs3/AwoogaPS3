#include <HLE/HLE.h>
#include <HLE/PPUWrapper.h>

#include <Common/Logger.h>

static void sceNpTusInit(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static HLEImportModule Module_sceNpTus = {
    "sceNpTus",
    {
        HLE_FUNCTION_EXPORT(0x8F87A06B, sceNpTusInit),
    },
    nullptr, nullptr, nullptr
};

HLEImportModule *get_sceNpTus() {
    return &Module_sceNpTus;
}