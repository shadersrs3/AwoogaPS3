#include <HLE/HLE.h>
#include <HLE/PPUWrapper.h>

#include <Common/Logger.h>

static void sceNp2Init(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void sceNpManagerRegisterCallback(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void sceNpManagerGetStatus(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void sceNpBasicRegisterContextSensitiveHandler(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void sceNpLookupInit(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void sceNpMatching2Init2(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static HLEImportModule Module_sceNp2 = {
    "sceNp2",
    {
        HLE_FUNCTION_EXPORT(0x41251F74, sceNp2Init),
        HLE_FUNCTION_EXPORT(0xE7DCD3B4, sceNpManagerRegisterCallback),
        HLE_FUNCTION_EXPORT(0xA7BFF757, sceNpManagerGetStatus),
        HLE_FUNCTION_EXPORT(0x4026EAC5, sceNpBasicRegisterContextSensitiveHandler),
        HLE_FUNCTION_EXPORT(0x5F2D9257, sceNpLookupInit),
        HLE_FUNCTION_EXPORT(0xF4BABD3F, sceNpMatching2Init2),
    },
    nullptr, nullptr, nullptr
};

HLEImportModule *get_sceNp2() {
    return &Module_sceNp2;
}