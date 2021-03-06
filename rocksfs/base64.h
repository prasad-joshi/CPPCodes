#ifndef __BASE64_H__
#define __BASE64_H__

#include <vector>
#include <string>

typedef unsigned char BYTE;

std::string base64_encode(BYTE const* buf, unsigned int bufLen);
std::vector<BYTE> base64_decode(std::string const&);

#endif
