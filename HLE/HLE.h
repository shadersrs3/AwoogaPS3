#ifndef _AWOOGAPS3_HLE_H
#define _AWOOGAPS3_HLE_H

#include <cstdint>
#include <string>
#include <unordered_map>

#define IMPORT_FUNCTION_SIGNATURE(name, ...) static void name(PowerProcessor *_cpu, __VA_ARGS__)
#define UNIMPLEMENTED_FUNCTION LOG_WARN("UNIMPLEMENTED %s PC 0x%llX", __FUNCTION__, _cpu->getPrevPC())
#define HLE_FUNCTION_EXPORT(nid, name) std::pair<uint32_t, HLEFunction>(nid, { #name, nid, WRAP_PPU_FUNCTION(name) })
#define _PPU_RETURN32(x) _cpu->setGPR(3, (int32_t)(x))
#define _PPU_RETURN(x) _cpu->setGPR(3, x)

struct PowerProcessor;

typedef void (*PPUModuleExecutor)(PowerProcessor *);

struct HLEFunction {
    char name[128];
    uint32_t nid;
    PPUModuleExecutor exec;
};

typedef std::unordered_map<uint32_t, HLEFunction> FunctionMap;

struct HLEImportModule {
    char name[128];
    FunctionMap function;
    void (*initModule)();
    void (*resetModule)();
    void (*destroyModule)();
};

void __hleAddAllModulesAndRegisterSyscalls();
HLEFunction *__hleGetModuleFunction(const char *name);
HLEFunction *__hleGetModuleFunction(uint32_t NID);
HLEImportModule *__hleGetModule(const std::string& name);
void __hleAddModule(HLEImportModule *mod);
void __registerPPUFunction(uint32_t NID, uint64_t address, const char *unused);
void __registerSyscall(uint64_t syscallId, PPUModuleExecutor exec);
HLEFunction *__getRegisteredPPUFunction(uint64_t address);
bool __hleExecuteSyscall(PowerProcessor *proc, uint64_t syscallId);

#endif