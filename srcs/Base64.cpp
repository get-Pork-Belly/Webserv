#include "Base64.hpp"

inline void
Base64::a3Toa4(unsigned char *a4, unsigned char *a3)
{
    a4[0] = (a3[0] & 0xfc) >> 2;
    a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
    a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
    a4[3] = (a3[2] & 0x3f);
}

inline void
Base64::a4Toa3(unsigned char *a3, unsigned char *a4)
{
    a3[0] = (a4[0] << 2) + ((a4[1] & 0x30) >> 4);
    a3[1] = ((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2);
    a3[2] = ((a4[2] & 0x3) << 6) + a4[3];
}

inline unsigned char
Base64::base64LookUp(unsigned char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A';
    if (c >= 'a' && c <= 'z')
        return c - 71;
    if (c >= '0' && c <= '9')
        return c + 4;
    if (c == '+')
        return 62;
    if (c == '/')
        return 63;
    return 255;
}

/*============================================================================*/
/*********************************  Util  *************************************/
/*============================================================================*/

bool Base64::encode(const std::string& in, std::string& out)
{
    int i = 0;
    int j = 0;
    size_t enc_len = 0;
    unsigned char a3[3];
    unsigned char a4[4];

    out.resize(encodedLength(in));

    size_t input_len = in.size();
    std::string::const_iterator input = in.begin();
    while (input_len--)
    {
        a3[i++] = *(input++);
        if (i == 3)
        {
            a3Toa4(a4, a3);
            for (i = 0; i < 4; i++)
                out[enc_len++] = kBase64Alphabet[a4[i]];
            i = 0;
        }
    }
    if (i)
    {
        for (j = i; j < 3; j++)
            a3[j] = '\0';
        a3Toa4(a4, a3);
        for (j = 0; j < i + 1; j++)
            out[enc_len++] = kBase64Alphabet[a4[j]];
        while ((i++ < 3))
            out[enc_len++] = '=';
    }
    return (enc_len == out.size());
}

bool Base64::decode(const std::string& in, std::string& out)
{
    int i = 0;
    int j = 0;
    size_t dec_len = 0;
    unsigned char a3[3];
    unsigned char a4[4];

    size_t input_len = in.size();
    std::string::const_iterator input = in.begin();

    out.resize(decodedLength(in));
    while (input_len--)
    {
        if (*input == '=')
            break;
        a4[i++] = *(input++);
        if (i == 4)
        {
            for (i = 0; i < 4; i++)
                a4[i] = base64LookUp(a4[i]);
            a4Toa3(a3, a4);
            for (i = 0; i < 3; i++)
                out[dec_len++] = a3[i];
            i = 0;
        }
    }
    if (i)
    {
        for (j = i; j < 4; j++)
            a4[j] = '\0';
        for (j = 0; j < 4; j++)
            a4[j] = base64LookUp(a4[j]);
        a4Toa3(a3, a4);
        for (j = 0; j < i - 1; j++)
            out[dec_len++] = a3[j];
    }
    return (dec_len == out.size());
}

size_t
Base64::decodedLength(const std::string& in)
{
    int num_eq = 0;
    size_t n = in.size();

    for (std::string::const_reverse_iterator it = in.rbegin(); *it == '='; ++it)
        ++num_eq;
    return ((6 * n) / 8) - num_eq;
}

inline size_t
Base64::encodedLength(const std::string& in)
{
    size_t length = in.length();
    return ((length + 2 - ((length + 2) % 3)) / 3 * 4);
}
