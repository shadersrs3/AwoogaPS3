#include <vector>
#include <list>
#include <cstring>

#include <HLE/HLE.h>

static std::unordered_map<uint64_t, HLEFunction *> registeredFunctions;
static std::vector<HLEImportModule *> knownModules;
static std::list<HLEFunction> unregisteredFunctionsList;
static std::unordered_map<uint64_t, PPUModuleExecutor> registeredSyscallHandlers;

void __registerSyscall(uint64_t syscallId, PPUModuleExecutor exec) {
    registeredSyscallHandlers[syscallId] = exec;
}

bool __hleExecuteSyscall(PowerProcessor *proc, uint64_t syscallId) {
    auto& i = registeredSyscallHandlers[syscallId];
    if (i == nullptr)
        return false;

    i(proc);
    return true;
}

HLEFunction *__hleGetModuleFunction(uint32_t NID) {
    for (auto& i : knownModules)
        if (auto it = i->function.find(NID); it != i->function.end())
            return &it->second;
    return nullptr;
}

HLEImportModule *__hleGetModule(const std::string& name) {
    for (auto& i : knownModules) {
        if (i->name == name)
            return i;
    }
    return nullptr;
}

void __hleAddModule(HLEImportModule *mod) {
    knownModules.push_back(mod);
}

void __registerPPUFunction(uint32_t NID, uint64_t address, const char *unused) {
    auto func = __hleGetModuleFunction(NID);

    if (func == nullptr) {
        HLEFunction unregisteredFunction;
        unregisteredFunction.nid = NID;
        if (unused)
            std::strncpy(unregisteredFunction.name, unused, 128);

        unregisteredFunction.exec = nullptr;
        unregisteredFunctionsList.push_back(unregisteredFunction);
        registeredFunctions[address] = &unregisteredFunctionsList.back();
    } else {
        registeredFunctions[address] = func;
    }
}

HLEFunction *__getRegisteredPPUFunction(uint64_t address) {
    auto it = registeredFunctions.find(address);
    if (it == registeredFunctions.end())
        return nullptr;
    return it->second;
}