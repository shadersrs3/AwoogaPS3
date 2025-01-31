#include <HLE/HLE.h>
#include <HLE/PPUWrapper.h>

#include <Common/Logger.h>

static void sceNpScoreInit(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static HLEImportModule Module_sceNp = {
    "sceNp",
    {
        HLE_FUNCTION_EXPORT(0x32CF311F, sceNpScoreInit),
    },
    nullptr, nullptr, nullptr
};

HLEImportModule *get_sceNp() {
    return &Module_sceNp;
}