#include <HLE/HLE.h>
#include <HLE/PPUWrapper.h>

#include <Common/Logger.h>

static void cellGameBootCheck(PowerProcessor *_cpu, _ptr<u32_be> type, _ptr<u32_be> attributes, _ptr<void> size, _ptr<char> dirName) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellGameGetParamInt(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellGameGetParamString(PowerProcessor *_cpu, int id, _ptr<char> buf, uint32_t bufsize) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellGameContentPermit(PowerProcessor *_cpu, _ptr<char> contentInfoPath, _ptr<char> usrDirPath) {
    strcpy(contentInfoPath(), "/dev_hdd0/NPUB30768");
    strcpy(usrDirPath(), "/dev_hdd0/NPUB30768/USRDIR");
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static HLEImportModule Module_cellGame = {
    "cellGame",
    {
        HLE_FUNCTION_EXPORT(0xF52639EA, cellGameBootCheck),
        HLE_FUNCTION_EXPORT(0xB7A45CAF, cellGameGetParamInt),
        HLE_FUNCTION_EXPORT(0x3A5D726A, cellGameGetParamString),
        HLE_FUNCTION_EXPORT(0x70ACEC67, cellGameContentPermit),
    },
    nullptr, nullptr, nullptr
};

HLEImportModule *get_cellGame() {
    return &Module_cellGame;
}
