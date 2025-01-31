#include <HLE/HLE.h>
#include <HLE/PPUWrapper.h>

#include <Common/Logger.h>

static void sys_net_initialize_network_ex(PowerProcessor *_cpu) {
    UNIMPLEMENTED_FUNCTION;
    _PPU_RETURN(0);
}

static HLEImportModule Module_sys_net = {
    "sys_net",
    {
        HLE_FUNCTION_EXPORT(0x139A9E9B, sys_net_initialize_network_ex),
    },
    nullptr, nullptr, nullptr
};

HLEImportModule *get_sys_net() {
    return &Module_sys_net;
}
