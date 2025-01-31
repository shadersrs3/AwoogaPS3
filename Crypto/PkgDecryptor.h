#ifndef _CRYPTO_PKG_DECRYPTOR_H
#define _CRYPTO_PKG_DECRYPTOR_H

#include <string>

bool decryptPkgFile(const std::string& pkgPath, const std::string& outputDirectoryTo);

#endif