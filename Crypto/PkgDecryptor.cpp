#include <cstring>

#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

#include <Common/Types.h>
#include <Common/Logger.h>

#include <Crypto/AES.h>

#pragma pack(push, 1)

// https://www.psdevwiki.com/ps3/PKG_files#File_Header
struct PkgHdr {
    u32_be magic;
    u16_be pkgRevision;
    u16_be pkgType;
    u32_be pkgMetadataOffset;
    u32_be pkgMetadataCount;
    u32_be pkgMetadataSize;
    u32_be itemCount;
    u64_be totalSize;
    u64_be dataOffset;
    u64_be dataSize;
    char contentId[0x24];
    uint8_t unused[0xC];
    uint8_t digest[0x10];
    uint8_t pkgDataRiv[0x10];
    uint8_t pkgHeaderDigest[0x40];

    bool verifyMagic() { return magic.getValue() == 0x7F504B47; }
};

struct PkgDigest {
    uint8_t cmacHash[0x10];
    uint8_t npdrmSignature[0x28];
    uint8_t sha1Hash[0x8];
};

#pragma pack(push, 4)

static u8 ps3_Key[0x10] = { 0x2E, 0x7B, 0x71, 0xD7, 0xC9, 0xC9, 0xA1, 0x4E, 0xA3, 0x22, 0x1F, 0x18, 0x88, 0x28, 0xB8, 0xF8 };
static u8 pubKey[0x10], staticPubKey[0x10];
static u8 nullKey[0x10] = { 0 };

static void xor128(u8 *dst, u8 *xor1, u8 *xor2)
{
    int i;
    for (i = 0; i < 16; i++)
        dst[i] = xor1[i] ^ xor2[i];
}

static void iter128(u8 *buf)
{
    int i;
    for (i = 15; i >= 0; i--) {
        buf[i]++;

        if (buf[i])
            break;
    }
}

static void setiter128(u8 *dst, int size)
{
    memcpy(dst, staticPubKey, 16);

    int i;
    for (i = 0; i < size; i++) {
        iter128(dst);
    }
}

bool decryptPkgFile(const std::string& pkgPath, const std::string& outputDirectoryTo)
{
    std::fstream f;
    f.open(pkgPath, std::ios_base::in | std::ios_base::binary);

    if (!f.is_open()) {
        LOG_ERROR("Couldn't open pkg file %s", pkgPath.c_str());
        return false;
    }

    PkgHdr pkgHdr;

    f.seekg(0);
    f.read(reinterpret_cast<char *>(&pkgHdr), sizeof(PkgHdr));
    if (!pkgHdr.verifyMagic()) {
        LOG_ERROR("Invalid PKG format");
        return false;
    }

    std::memset((char *)&pkgHdr + offsetof(PkgHdr, unused), 0x0, 0xC); // This is a padding contentId should never overlap in this unused field

    if (pkgHdr.pkgType.getValue() != 0x1) {
        LOG_ERROR("PKG type is not a PS3 package");
        return false;
    }

    char titleId[10];
    std::memcpy(titleId, (u8 *)&pkgHdr + 0x37, 9);
    titleId[9] = 0;

    std::string titleDirectory = (outputDirectoryTo + "/") + titleId;

    std::filesystem::create_directory(titleDirectory);

    // https://github.com/qwikrazor87/pkgrip/blob/master/src/pkgrip.c credits to qwikrazor87
    int metadataOffset = pkgHdr.pkgMetadataOffset.getValue();
    int metadataCount = pkgHdr.pkgMetadataCount.getValue();
    int metadataCurrentOffset = metadataOffset;
    for (int i = 0; i < metadataCount; i++) { // Ty pkg2zip
        u32_be type, size;

        f.seekg(metadataCurrentOffset);
        f.read((char *) &type, sizeof(int));
        f.seekg(metadataCurrentOffset + 4);
        f.read((char *) &size, sizeof(int));
        metadataCurrentOffset += 2 * sizeof(int) + size.getValue();
    }

    AES_ctx ctx;
    std::memset(&ctx, 0, sizeof ctx);
    AES_set_key(&ctx, ps3_Key, AES_KEY_LEN_128);
    std::memcpy(pubKey, pkgHdr.pkgDataRiv, 0x10);
    std::memcpy(staticPubKey, pkgHdr.pkgDataRiv, 0x10);

    u8 xor_key[0x10];
    static u8 buf[0x10];
    auto encryptedDataStart = pkgHdr.dataOffset.getValue();
    f.seekg(encryptedDataStart);

    struct PkgFile {
        uint32_t nameOffset;
        uint32_t nameLength;
        uint32_t fileOffset;
        uint32_t fileSize;
        uint32_t entryType;
    };

    std::vector<PkgFile> files;
    PkgFile file;

    for (uint64_t i = 0, entries = (uint64_t)pkgHdr.itemCount.getValue() << 1; i < entries; i++) {
        f.read((char *)buf, 0x10);
        AES_encrypt(&ctx, pubKey, xor_key);
        xor128(buf, buf, xor_key);
        iter128(pubKey);

        if (i % 2 == 0) {
            file.nameOffset = ((u32_be *) buf)->getValue();
            file.nameLength = ((u32_be *) (buf + 4))->getValue();
            file.fileOffset = ((u32_be *) (buf + 12))->getValue();
        } else {
            file.fileSize = ((u32_be *) (buf + 4))->getValue();
            file.entryType = ((u32_be *) (buf + 8))->getValue();
            files.push_back(file);
        }
    }

    for (int i = 0; i < (int)files.size(); i++) {
        uint32_t offset = files[i].nameOffset;
        uint32_t length = (files[i].nameLength + 15) & -16;
        int isFile = !((files[i].entryType & 0xFF) == 0x04 && !files[i].fileSize);
        auto& _file = files[i];
        std::vector<char> buf(length + 1);

        f.seekg(encryptedDataStart + _file.nameOffset);
        f.read((char *) &buf[0], length);

        setiter128(pubKey, offset >> 4);
        AES_set_key(&ctx, ps3_Key, AES_KEY_LEN_128);

        // Get the filename
        for (int j = 0; j < (int)(length >> 4); j++) {
            AES_encrypt(&ctx, pubKey, xor_key);
            xor128((uint8_t *)&buf[j * 16], (uint8_t *)&buf[j * 16], xor_key);
            iter128(pubKey);
        }

        if (isFile) {
            std::fstream stream;
            stream.open((titleDirectory + "/") + &buf[0], std::ios_base::out | std::ios_base::binary);
            u32 szcheck = 0, mincheck = 0;
            constexpr int MB = 1024 * 1024;
            std::vector<uint8_t> buf(MB);

            f.seekg(encryptedDataStart + _file.fileOffset);
            setiter128(pubKey, _file.fileOffset >> 4);
            f.read((char *)&buf[0], (_file.fileSize >= MB) ? MB : _file.fileSize);
            for (int j = 0; j < (int)(_file.fileSize >> 4); j++) {
                if (szcheck == MB) {
                    szcheck = 0;
                    mincheck += MB;
                    stream.write((char *)&buf[0], MB);
                    f.read((char *)&buf[0], ((_file.fileSize - (j << 4)) >= MB) ? MB : _file.fileSize - (j << 4));
                }

                AES_encrypt(&ctx, pubKey, xor_key);
                xor128(&buf[0] + ((j << 4) - mincheck), &buf[0] + ((j << 4) - mincheck), xor_key);
                iter128(pubKey);

                szcheck += 16;
            }

            if (mincheck < _file.fileSize) {
                stream.write((char *)&buf[0], _file.fileSize - mincheck);
            }

            stream.close();
        } else {
            std::filesystem::create_directory((titleDirectory + "/") + &buf[0]);
        }
    }

    return true;
}
