#ifndef _AWOOGAPS3_MEMORY_H
#define _AWOOGAPS3_MEMORY_H

#include <cstring>

#include <unordered_map>

#include <Common/Types.h>

struct PowerProcessor;

struct Memory {
private:
    struct MemoryMap : private std::unordered_map<uint64_t, uint8_t *> {
    private:
        std::unordered_map<uint64_t, uint8_t *> freeMemoryMap;
    public:
        ~MemoryMap() {
            for (auto& i : freeMemoryMap)
                delete i.second;

            freeMemoryMap.clear();
            clear();
        }

        bool unmapPage(uint64_t address, uint64_t size) {
            if (auto it = find(address); it != end()) {
                erase(it);
                return true;
            }
            return false;
        }

        bool mapPage(uint64_t address, uint64_t size) {
            if (auto it = find(address); it != end())
                return false;

            uint8_t *buf = new uint8_t[size];
            std::memset(buf, 0, size);
            freeMemoryMap[address] = buf;
            for (auto i = 0; i < size; i += 0x10000)
                operator[](address + i) = buf + i;
            return true;
        }

        void *getPtr(uint64_t address) {
            auto it = find(address & ~0xFFFF);
            if (it != end())
                return it->second + (address & 0xFFFF);
            return nullptr;
        }
    };

    MemoryMap map;
public:
    PowerProcessor *ppu;
    Memory();
    ~Memory();
    void *getPtr(uint64_t address);
    bool mmap(uint64_t start, uint64_t size);
    bool munmap(uint64_t start, uint64_t size);
    void copyMemory(uint64_t address, const void *src, uint64_t size);
    void setMemory(uint64_t address, uint8_t _char, uint64_t size);
  
    bool readBytes(uint64_t address, uint8_t *buf, uint32_t len) {
        for (auto i = 0; i < len; i++)
            if (!read8(address + i, &buf[i]))
                return false;
        return true;
    }

    bool writeBytes(uint64_t address, const uint8_t *str, uint32_t len) {
        for (auto i = 0; i < len; i++)
            if (!write8(address + i, str[i]))
                return false;
        return true;
    }

    template<typename T>
    bool readStruct(uint64_t address, T *value) {
        uint8_t val;
        for (int i = 0; i < sizeof(T); i++) {
            if (!read8(address + i, &val))
                return false;
            *((uint8_t *) value + i) = val;
        }
        return true;
    }

    bool read8(uint64_t address, uint8_t *value);
    bool read16(uint64_t address, uint16_t *value);
    bool read32(uint64_t address, uint32_t *value);
    bool read64(uint64_t address, uint64_t *value);
    bool write8(uint64_t address, uint8_t value);
    bool write16(uint64_t address, uint16_t value);
    bool write32(uint64_t address, uint32_t value);
    bool write64(uint64_t address, uint64_t value);
};

#endif