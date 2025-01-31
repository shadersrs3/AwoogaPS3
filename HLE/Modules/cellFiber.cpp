#include <HLE/HLE.h>
#include <HLE/PPUWrapper.h>

#include <Common/Logger.h>

static void _cellFiberPpuInitialize(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static HLEImportModule Module_cellFiber = {
    "cellFiber",
    {
        HLE_FUNCTION_EXPORT(0x55870804, _cellFiberPpuInitialize),
    }
};

HLEImportModule *get_cellFiber() {
    return &Module_cellFiber;
}
