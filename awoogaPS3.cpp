#include <iostream>
#include <fstream>
#include <string>
#include <cstdarg>
#include <vector>
#include <filesystem>
#include <map>
#include <cmath>
#include <algorithm>
#include <cstring>

#include <Common/Types.h>
#include <Memory/Memory.h>
#include <ELF/elf64.h>
#include <ELF/sce.h>
#include <Cell/PowerProcessor.h>
#include <HLE/PPUWrapper.h>
#include <HLE/HLE.h>

#include <HLE/ModuleSyscallList.h>

#include <Common/Logger.h>

std::vector<Thread *> threads;

void __hleAddAllModulesAndRegisterSyscalls() {
    __hleAddModule(get_sceNpTus());
    __hleAddModule(get_sceNp());
    __hleAddModule(get_sceNp2());
    __hleAddModule(get_cellNetCtl());
    __hleAddModule(get_sys_net());
    __hleAddModule(get_sys_fs());
    __hleAddModule(get_cellGameExec());
    __hleAddModule(get_cellGame());
    __hleAddModule(get_cellGcmSys());
    __hleAddModule(get_cellSync());
    __hleAddModule(get_cellSpurs());
    __hleAddModule(get_cellFiber());
    __hleAddModule(get_cellSysutil());
    __hleAddModule(get_sysPrxForUser());
    __hleAddModule(get_cellSysmodule());
    register_syscall_sys_timer_functions();
    register_syscall_sys_event_functions();
    register_syscall_sys_spu_functions();
    register_syscall_sys_time_functions();
    register_syscall_sys_memory_functions();
    register_syscall_sys_dbg_functions();
    register_syscall_sys_tty_functions();
    register_syscall_sys_ppu_thread_functions();
}

struct ProgramSegment {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
};

struct SectionHeader {
    uint32_t sh_name;
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addralign;
    uint64_t sh_entsize;
};

static void sortMemoryAddressesAndToFixedArea(std::vector<std::pair<uint64_t, uint64_t>>& input,
                                std::vector<std::pair<uint64_t, uint64_t>>& output) {
    std::map<uint64_t, uint8_t> address64KB;
    std::vector<uint64_t> keys;

    std::sort(input.begin(), input.end(), [](const std::pair<uint64_t, uint64_t>& a,
            const std::pair<uint64_t, uint64_t>& b) { return a.first < b.first; });

    for (auto it = input.begin(); it != input.end(); it++) {
        uint64_t addressStart = it->first & ~0xFFFF;
        uint64_t addressSize = (it->second + 0x10000) & ~0xFFFF;
        uint64_t addressEnd = ((it->first + addressSize) & ~0xFFFF); 
        for (auto i = addressStart; i < addressEnd; i += 0x10000) {
            address64KB[i] = 1;
        }
    }

    keys.reserve(address64KB.size());
    for (auto& i : address64KB)
        keys.push_back(i.first);

    std::sort(keys.begin(), keys.end());

    uint64_t startAddress = keys.front();
    uint64_t size = 0;
    for (auto it = keys.begin(); it < keys.end() - 1; it += 2) {
        
        if (*(it + 1) == *it + 0x10000) {
            size += 0x20000;
        } else {
            printf("hi sorted key (please myself do this)\n");
            std::exit(0);
        }
    }

    if ((keys.size() % 2) != 0) {
        printf("Odd addresses\n");
        std::exit(0);
    } else {
        if (size != 0)
            output.push_back({ startAddress, size });
    }
}

bool loadELF(const std::string& filename, Memory *memory, uint64_t& gpValue, uint64_t& startAddress) {
    std::fstream f;
    f.open("../../../eboot.bin", std::ios_base::in | std::ios_base::binary);
    if (!f.is_open()) {
        LOG_ERROR("Bad file");
        std::exit(0);
    }

    Elf64_Ehdr ehdr;
    f.read((char *)&ehdr, sizeof ehdr);
    if (ehdr.e_type.getValue() == ET_SCE_PPURELEXEC) {
        printf("Unhandled ET_SCE_PPURELEXEC\n");
        return false;
    }


    auto programHeaderCount = ehdr.e_phnum.getValue();
    auto programHeaderSize = ehdr.e_phentsize.getValue();
    auto programHeaderOffset = ehdr.e_phoff.getValue();
    std::vector<ProgramSegment> segments;

    for (auto i = programHeaderOffset; i < (programHeaderOffset + programHeaderCount * programHeaderSize); i += programHeaderSize) {
        Elf64_Phdr phdr;
        ProgramSegment seg;

        f.seekg(i);
        f.read((char *)&phdr, sizeof phdr);

        seg.p_align = phdr.p_align.getValue();
        seg.p_filesz = phdr.p_filesz.getValue();
        seg.p_flags = phdr.p_flags.getValue();
        seg.p_memsz = phdr.p_memsz.getValue();
        seg.p_offset = phdr.p_offset.getValue();
        seg.p_paddr = phdr.p_paddr.getValue();
        seg.p_type = phdr.p_type.getValue();
        seg.p_vaddr = phdr.p_vaddr.getValue();
        segments.push_back(seg);
    }

    auto sectionHeaderCount = ehdr.e_shnum.getValue();
    auto sectionHeaderSize = ehdr.e_shentsize.getValue();
    auto sectionHeaderOffset = ehdr.e_shoff.getValue();

    std::vector<SectionHeader> sections;

    for (auto i = sectionHeaderOffset; i < (sectionHeaderOffset + sectionHeaderCount * sectionHeaderSize); i += sectionHeaderSize) {
        Elf64_Shdr shdr;
        SectionHeader section;

        f.seekg(i);
        f.read((char *)&shdr, sizeof shdr);

        section.sh_addr = shdr.sh_addr.getValue();
        section.sh_addralign = shdr.sh_addralign.getValue();
        section.sh_entsize = shdr.sh_entsize.getValue();
        section.sh_flags = shdr.sh_flags.getValue();
        section.sh_info = shdr.sh_info.getValue();
        section.sh_link = shdr.sh_link.getValue();
        section.sh_name = shdr.sh_name.getValue();
        section.sh_offset = shdr.sh_offset.getValue();
        section.sh_size = shdr.sh_size.getValue();
        section.sh_type = shdr.sh_type.getValue();
        sections.push_back(section);
    }

    if (sections.size() > 0) {
        std::vector<std::pair<uint64_t, uint64_t>> input;
        std::vector<std::pair<uint64_t, uint64_t>> output;
        for (auto& i : sections) {
            std::pair<uint64_t, uint64_t> pair = { i.sh_addr, i.sh_size };
            if ((i.sh_flags & SHF_ALLOC) && i.sh_size > 0) {
                if (i.sh_type == SHT_NOBITS || i.sh_type == SHT_PROGBITS) {
                    input.push_back({ i.sh_addr, i.sh_size });
                } else {
                    LOG_ERROR("Unhandled section type 0x%08X", i.sh_type);
                    std::exit(0);
                }
            }
        }

        sortMemoryAddressesAndToFixedArea(input, output);
        for (auto& i : output) {
            if (!memory->mmap(i.first, i.second))
                return false;
        }

        for (auto& i : sections) {
            if ((i.sh_flags & SHF_ALLOC) && i.sh_size > 0) {
                if (i.sh_type == SHT_NULL)
                    continue;

                if (i.sh_type == SHT_NOBITS) {
                    memory->setMemory(i.sh_addr, 0x0, i.sh_size);
                } else if (i.sh_type == SHT_PROGBITS) {
                    std::vector<char> buffer(i.sh_size);
                    f.seekg(i.sh_offset);
                    f.read((char *)&buffer[0], i.sh_size);
                    memory->copyMemory(i.sh_addr, &buffer[0], i.sh_size);
                } else {
                    LOG_ERROR("Unhandled section type 0x%08X", i.sh_type);
                    std::exit(0);
                }

                // printf("%X\n", i.sh_type);
                // printf("%llX %llX %llX\n", i.sh_addr, i.sh_offset, i.sh_size);
            }
        }
    }

    sys_process_prx_info_t prxInfo;

    for (auto& seg : segments) {
        if (seg.p_type == PT_LOAD) {
            std::vector<uint8_t> buf(seg.p_filesz);
            f.seekg(seg.p_offset);
            f.read((char *)&buf[0], seg.p_filesz);
            memory->copyMemory(seg.p_vaddr, &buf[0], seg.p_filesz);
        } else if (seg.p_type == PT_PROC_PRX) {
            if (seg.p_filesz == 0) {
                LOG_ERROR("VSH Zeroed out PT_PROC_PRX");
                std::exit(0);
            } else {
                for (uint8_t i = 0; i < sizeof(sys_process_prx_info_t); i += sizeof(uint8_t)) {
                    uint8_t val;
                    if (!memory->read8(seg.p_vaddr + i, &val)) {
                        printf("Invalid address");
                        std::exit(0);
                    }

                    *((uint8_t *)&prxInfo + i) = val;
                }
            }

            prxInfo.size = ((u32_be *)&prxInfo.size)->getValue();
            prxInfo.magic = ((u32_be *)&prxInfo.magic)->getValue();
            prxInfo.version = ((u32_be *)&prxInfo.version)->getValue();
            prxInfo.sdk_version = ((u32_be *)&prxInfo.sdk_version)->getValue();
            prxInfo.libent_start = ((u32_be *)&prxInfo.libent_start)->getValue();
            prxInfo.libent_end = ((u32_be *)&prxInfo.libent_end)->getValue();
            prxInfo.libstub_start = ((u32_be *)&prxInfo.libstub_start)->getValue();
            prxInfo.libstub_end = ((u32_be *)&prxInfo.libstub_end)->getValue();

            _scelibent_common libentCommon;
            _scelibstub_common libstubCommon;
            _scelibstub_ppu32 ppu32;
            _scelibstub_ppu64 ppu64;

            if (!memory->readStruct(prxInfo.libstub_start, &libstubCommon))
                return false;

            memory->readStruct(prxInfo.libstub_start, &ppu32);
            for (auto i = prxInfo.libstub_start; i < prxInfo.libstub_end; i += libstubCommon.structsize) {

                int nFunctions = libstubCommon.nfunc.getValue();
                // int nVariables = libstubCommon.nvar.getValue();

                if (libstubCommon.structsize == sizeof(_scelibstub_ppu32)) {
                    uint32_t libnameAddr = ppu32.libname.getValue();
                    std::string name = (const char *) memory->getPtr(libnameAddr);

                    LOG_INFO("Attempting to register module %s with functions and variables..", name.c_str());

                    uint32_t functionTable, functionNidTable;
                    functionTable = ppu32.func_table.getValue();
                    functionNidTable = ppu32.func_nidtable.getValue();
                    for (int i = 0; i < nFunctions; i++) {
                        uint32_t functionAddress, NID;
                        memory->read32(functionTable + i * 4, &functionAddress);
                        memory->read32(functionNidTable + i * 4, &NID);
                        __registerPPUFunction(NID, functionAddress, name.c_str());
                    }
                    memory->readStruct(i, &ppu32);
                } else if (libstubCommon.structsize == sizeof(_scelibstub_ppu64)) {
                    memory->readStruct(i, &ppu64);
                    LOG_ERROR("UNIMPLEMENTED PPC64 LIBSTUB");
                    std::exit(0);
                }
                if (!memory->readStruct(i, &libstubCommon))
                    return false;
            }

            LOG_WARN("Variable imports not used (TODO)");
        }
    }

    uint32_t gp;
    uint32_t startFunction;
    memory->read32(ehdr.e_entry.getValue() + 4, &gp);
    memory->read32(ehdr.e_entry.getValue(), &startFunction);
    startAddress = startFunction;

    gpValue = gp;
    f.close();
    return true;
}

int main()
{
    Memory m;
    uint64_t gpValue;
    uint64_t startAddress;
    PowerProcessor ppu;
    Thread thr;

    m.ppu = &ppu;

    __hleAddAllModulesAndRegisterSyscalls();
    loadELF("../../../eboot.bin", &m, gpValue, startAddress);

    ppu.reset();
    ppu.setMemory(&m);
    ppu.setThread(&thr);
    std::memset(&thr, 0, sizeof thr);
    thr.PC = startAddress;
    thr.TBR = 0xDEADBEEF;
    std::strcpy(thr.name, "mainThread");

    threads.push_back(&thr);

    auto& vr = ppu.getVR(0);
    uint8_t data[16];
    for (int i = 0; i < 16; i++)
        data[i] = 15 - i;

    vr.data[0] = ((u64_be *)&data[0])->getValue();
    vr.data[1] = ((u64_be *)&data[8])->getValue();

    m.mmap(0x80000000, 0x4000000);
    
    ppu.setGPR(1, 0x8000F000);

    const uint8_t gamePath[] = "/dev_hdd0/NPUB30768/USRDIR/EBOOT.BIN";
    ppu.setGPR(3, 1);
    ppu.setGPR(4, 0x8000F800);
    m.write32(0x8000F804, 0x8000FF00);

    ppu.setGPR(6, 1);
    ppu.setGPR(5, 0x8000F808);
    m.write32(0x8000F80C, 0x8000FF00 + sizeof(gamePath) + 1);

    m.mmap(0x81000000, 0x10000);
    ppu.setGPR(13, 0x81000000+0x10000-4);

    m.writeBytes(0x8000FF00, gamePath, sizeof gamePath);
    uint64_t PC = startAddress;
    bool fail = false;
    while (true) {
        for (auto& i : threads) {
            if (i->delayDowncount <= 0) {
                uint64_t addr;
                ppu.setThread(i);
                addr = ppu.getPC();
                if ((addr = ppu.executeInterpreterSingleStep(ppu.getPC())) != ~0uLL) { ppu.getPC() = addr; } else fail = true;
            } else
                --i->delayDowncount;
            break;
        }
    }
    return 0;
}