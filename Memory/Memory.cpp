#include <Cell/PowerProcessor.h>
#include <Memory/Memory.h>

Memory::Memory() {}

Memory::~Memory() {}

void *Memory::getPtr(uint64_t address) {
    return map.getPtr(address);
}

bool Memory::read8(uint64_t address, uint8_t *value) {
    auto ptr = getPtr(address);
    if (ptr) {
        *value = *(uint8_t *) ptr;
        return true;
    }
    return false;
}

bool Memory::read16(uint64_t address, uint16_t *value) {
    auto ptr = (uint8_t *) getPtr(address);

    if (ptr) {
        *value = ((u16_be *) ptr)->getValue();
        return true;
    }
    return false;
}

bool Memory::read32(uint64_t address, uint32_t *value) {
    auto ptr = (uint8_t *) getPtr(address);
    if (ptr) {
        *value = ((u32_be *) ptr)->getValue();
        return true;
    }
    return false;
}

bool Memory::read64(uint64_t address, uint64_t *value) {
    auto ptr = (uint8_t *) getPtr(address);

    if (ptr) {
        *value = ((u64_be *) ptr)->getValue();
        return true;
    }
    return false;
}

bool Memory::write8(uint64_t address, uint8_t value) {
    auto ptr = getPtr(address);
    if (ptr) {
        *(uint8_t *) ptr = value;
        return true;
    }
    return false;
}

bool Memory::write16(uint64_t address, uint16_t value) {
    auto ptr = getPtr(address);

    if (ptr) {
        ((u16_be *) ptr)->setValue(value);
        return true;
    }
    return false;
}

bool Memory::write32(uint64_t address, uint32_t value) {
    auto ptr = (u8 *) getPtr(address);

    if (ptr) {
        ((u32_be *) ptr)->setValue(value);
        return true;
    }
    return false;
}

bool Memory::write64(uint64_t address, uint64_t value) {
    auto ptr = (u8 *) getPtr(address);

    if (ptr) {
        ((u64_be *) ptr)->setValue(value);
        return true;
    }
    return false;
}

bool Memory::mmap(uint64_t start, uint64_t size) {
    const uint64_t alignedAddress = start & ~0xFFFFuLL;
    const uint64_t addressEnd = (size + 0xFFFF) & ~0xFFFFuLL;
    bool state = true;
    state = map.mapPage(alignedAddress, size);
    return state;
}

bool Memory::munmap(uint64_t start, uint64_t size) {
    return false;
}

void Memory::copyMemory(uint64_t address, const void *src, uint64_t size) {
    for (uint64_t i = address; i < address + size; i++)
        *(uint8_t *) getPtr(i) = *((uint8_t *)src + (i - address));
}

void Memory::setMemory(uint64_t address, uint8_t _char, uint64_t size) {
    for (uint64_t i = address; i < address + size; i++)
        *(uint8_t *) getPtr(i) = _char;
}