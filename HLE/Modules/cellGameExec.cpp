#include <HLE/HLE.h>
#include <HLE/PPUWrapper.h>

#include <Common/Logger.h>

static void cellGameGetBootGameInfo(PowerProcessor *_cpu, uint32_t type, _ptr<char> dirName, _ptr<u32_be> execData) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellGameGetHomeDataExportPath(PowerProcessor *_cpu, _ptr<char> dirName) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static HLEImportModule Module_cellGameExec = {
    "cellGameExec",
    {
        HLE_FUNCTION_EXPORT(0xF6ACD0BC, cellGameGetBootGameInfo),
        HLE_FUNCTION_EXPORT(0x59B1EDE1, cellGameGetHomeDataExportPath),
    },
    nullptr, nullptr, nullptr
};

HLEImportModule *get_cellGameExec() {
    return &Module_cellGameExec;
}
