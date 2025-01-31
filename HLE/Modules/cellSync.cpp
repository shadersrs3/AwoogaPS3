#include <HLE/HLE.h>
#include <HLE/PPUWrapper.h>

#include <Common/Logger.h>

static void cellSyncMutexInitialize(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static HLEImportModule Module_cellSync = {
    "cellSync",
    {
        HLE_FUNCTION_EXPORT(0xA9072DEE, cellSyncMutexInitialize),
    },
    nullptr, nullptr, nullptr
};

HLEImportModule *get_cellSync() {
    return &Module_cellSync;
}
