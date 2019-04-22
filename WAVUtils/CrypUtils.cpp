
#include "MD5.h"
#include <fstream>
#include "time.h"
#include "chacha20.h"

int DecryptCodeSection(char* cryptKey,unsigned char *raw_data,unsigned int raw_size)
{
    std::string skey = MD5String(cryptKey);
    srand((unsigned int)time(NULL));

    chacha20_ctx c20_ctx;
    chacha20_setkey(&c20_ctx, (unsigned char*)skey.c_str(), raw_data);
    chacha20_encrypt(&c20_ctx, raw_data+8, raw_size-8);

    return 1;
}


int EncryptCodeSection(char* cryptKey,unsigned char *raw_data,unsigned int raw_size)
{
    std::string skey = MD5String(cryptKey);

    srand((unsigned int)time(NULL));

    for (int i = 0; i < 8; i++)
        raw_data[i] = (char)rand();

    chacha20_ctx c20_ctx;
    chacha20_setkey(&c20_ctx, (unsigned char*)skey.c_str(), raw_data);
    chacha20_encrypt(&c20_ctx, raw_data+8, raw_size-8);

    return 1;

}