#include <HLE/HLE.h>
#include <HLE/PPUWrapper.h>

#include <Common/Logger.h>

static void cellSpursGetNumSpuThread(PowerProcessor *_cpu, _ptr<void> unused, _ptr<u32_be> nThreads) {
    nThreads->setValue(1);
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void _cellSpursTasksetAttribute2Initialize(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellSpursCreateTaskset2(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellSpursGetTasksetId(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellSpursTasksetGetSpursAddress(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellSpursSetPriorities(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}
static void _cellSpursJobChainAttributeInitialize(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellSpursJobChainAttributeSetName(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellSpursCreateJobChainWithAttribute(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellSpursGetJobChainId(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellSpursJobChainGetSpursAddress(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellSpursGetSpuThreadId(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void _cellSpursEventFlagInitialize(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static void cellSpursEventFlagAttachLv2EventQueue(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static HLEImportModule Module_cellSpurs = {
    "cellSpurs",
    {
        HLE_FUNCTION_EXPORT(0xC56DEFB5, cellSpursGetNumSpuThread),
        HLE_FUNCTION_EXPORT(0xC2ACDF43, _cellSpursTasksetAttribute2Initialize),
        HLE_FUNCTION_EXPORT(0x4A6465E3, cellSpursCreateTaskset2),
        HLE_FUNCTION_EXPORT(0xE7DD87E1, cellSpursGetTasksetId),
        HLE_FUNCTION_EXPORT(0x58D58FCF, cellSpursTasksetGetSpursAddress),
        HLE_FUNCTION_EXPORT(0x80A29E27, cellSpursSetPriorities),
        HLE_FUNCTION_EXPORT(0x3548F483, _cellSpursJobChainAttributeInitialize),
        HLE_FUNCTION_EXPORT(0x9FEF70C2, cellSpursJobChainAttributeSetName),
        HLE_FUNCTION_EXPORT(0x303C19CD, cellSpursCreateJobChainWithAttribute),
        HLE_FUNCTION_EXPORT(0x86C864A2, cellSpursGetJobChainId),
        HLE_FUNCTION_EXPORT(0x494613C7, cellSpursJobChainGetSpursAddress),
        HLE_FUNCTION_EXPORT(0x6C960F6D, cellSpursGetSpuThreadId),
        HLE_FUNCTION_EXPORT(0x5EF96465, _cellSpursEventFlagInitialize),
        HLE_FUNCTION_EXPORT(0x87630976, cellSpursEventFlagAttachLv2EventQueue),
    },
    nullptr, nullptr, nullptr,
};


HLEImportModule *get_cellSpurs() {
    return &Module_cellSpurs;
}