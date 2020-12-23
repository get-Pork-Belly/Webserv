#ifndef BASE64_HPP
# define BASE64_HPP

#include <iostream>
#include <string>

const char kBase64Alphabet[] = {
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz"
      "0123456789+/"
      };

class Base64
{
private:
    static inline void a3Toa4(unsigned char* a4, unsigned char* a3);
    static inline void a4Toa3(unsigned char* a3, unsigned char* a4);
    static inline unsigned char base64LookUp(unsigned char c);
public:
    static bool encode(const std::string& in, std::string& out);
    static bool decode(const std::string& in, std::string& out);
    static size_t decodedLength(const std::string& in);
    inline static size_t encodedLength(const std::string& in);
};

#endif