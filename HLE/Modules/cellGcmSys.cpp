#include <HLE/HLE.h>
#include <HLE/PPUWrapper.h>

#include <Common/Logger.h>

static void cellGcmGetTiledPitchSize(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void _cellGcmInitBody(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellGcmGetConfiguration(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellGcmMapMainMemory(PowerProcessor *_cpu, uint64_t address, uint32_t size, _ptr<u32_be> offset) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellGcmAddressToOffset(PowerProcessor *_cpu, uint64_t address, _ptr<u32_be> offset) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellGcmMapEaIoAddress(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellGcmSetDisplayBuffer(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellGcmGetLabelAddress(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0x2F000000);
}

static void cellGcmSetFlipMode(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellGcmResetFlipStatus(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellGcmSetUserHandler(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static HLEImportModule Module_cellGcm = {
    "cellGcmSys",
    {
        HLE_FUNCTION_EXPORT(0x055BD74D, cellGcmGetTiledPitchSize),
        HLE_FUNCTION_EXPORT(0x15BAE46B, _cellGcmInitBody),
        HLE_FUNCTION_EXPORT(0xE315A0B2, cellGcmGetConfiguration),
        HLE_FUNCTION_EXPORT(0xA114EC67, cellGcmMapMainMemory),
        HLE_FUNCTION_EXPORT(0x21AC3697, cellGcmAddressToOffset),
        HLE_FUNCTION_EXPORT(0x63441CB4, cellGcmMapEaIoAddress),
        HLE_FUNCTION_EXPORT(0xA53D12AE, cellGcmSetDisplayBuffer),
        HLE_FUNCTION_EXPORT(0xF80196C1, cellGcmGetLabelAddress),
        HLE_FUNCTION_EXPORT(0x4AE8D215, cellGcmSetFlipMode),
        HLE_FUNCTION_EXPORT(0xB2E761D4, cellGcmResetFlipStatus),
        HLE_FUNCTION_EXPORT(0x06EDEA9E, cellGcmSetUserHandler),
    },
    nullptr, nullptr, nullptr
};

HLEImportModule *get_cellGcmSys() {
    return &Module_cellGcm;
}