
#include "chacha20.h"

#define R(v, n)(((v) >> (n)) | ((v) << (32 - (n))))
#define F(n) for (i = 0; i < n; i++)
#define X(a, b)(t) = (a), (a) = (b), (b) = (t)


// int main(int argc, char** argv)
// {
// 	if (argc < 5)
// 		return EXIT_FAILURE;
// 
// 	char* mode = argv[1];
// 	char* input_path = argv[2];
// 	char* output_path = argv[3];
// 	char* key = argv[4];
// 
// 	if (strcmp(mode, "encrypt") == 0)
// 	{
// 		encrypt_file(input_path, output_path, key);
// 
// 		return EXIT_SUCCESS;
// 	}
// 
// 	if (strcmp(mode, "decrypt") == 0)
// 	{
// 		decrypt_file(input_path, output_path, key);
// 
// 		return EXIT_SUCCESS;
// 	}
// 
// 	return EXIT_FAILURE;
// }

void chacha20_core(void* input, void* output)
{
	unsigned int a, b, c, d, i, t, r, *s = (unsigned*)input, *x = (unsigned*)output;
	unsigned int v[8] = {
		0xC840, 0xD951, 0xEA62, 0xFB73,
		0xFA50, 0xCB61, 0xD872, 0xE943
	};

	F(16)x[i] = s[i];

	F(80)
	{
		d = v[i % 8];
		a = (d & 15);
		b = (d >> 4 & 15);
		c = (d >> 8 & 15);
		d >>= 12;

		for (r = 0x19181410; r; r >>= 8)
			x[a] += x[b], x[d] = R(x[d] ^ x[a], (r & 255)), X(a, c), X(b, d);
	}
	F(16)x[i] += s[i];
}

void chacha20_setkey(chacha20_ctx* c, void* key, void* nonce)
{
	unsigned int *k = (unsigned int*)key, *n = (unsigned int*)nonce;
	int i;

	c->w[0] = 0x61707865;
	c->w[1] = 0x3320646E;
	c->w[2] = 0x79622D32;
	c->w[3] = 0x6B206574;

	F(8)c->w[i + 4] = k[i];
	c->w[12] = 0;
	c->w[13] = 0;
	F(2)c->w[i + 14] = n[i];
}

void chacha20_encrypt(chacha20_ctx* ctx, void* in, unsigned int len)
{
	unsigned char c[64], *p = (unsigned char*)in;
	unsigned int i, r;

	if (len)
	{
		while (len)
		{
			chacha20_core(ctx, c);
			ctx->q[6]++;
			r = (len > 64) ? 64 : len;
			F(r)p[i] ^= c[i];
			len -= r;
			p += r;
		}
	}
}

// char* to_hex(unsigned char* bin, size_t len)
// {
// 	if (bin == NULL || len == 0)
// 		return NULL;
// 
// 	char* out = (char*)malloc(len * 2 + 1);
// 	for (size_t i = 0; i < len; i++)
// 	{
// 		out[i * 2] = "0123456789ABCDEF"[bin[i] >> 4];
// 		out[i * 2 + 1] = "0123456789ABCDEF"[bin[i] & 0x0F];
// 	}
// 	out[len * 2] = '\0';
// 
// 	return out;
// }

// void encrypt_file(const char* file_in, const char* file_out, const char key[32])
// {
// 	ifstream file_is(file_in, ios::in | ios::binary | ios::ate);
// 	if (!file_is.is_open())
// 		return;
// 
// 	const size_t file_is_size = file_is.tellg();
// 	file_is.seekg(0, ios::beg);
// 
// 	char* file_is_data = new char[file_is_size];
// 	file_is.read(file_is_data, file_is_size);
// 	file_is.close();
// 
// 	srand((unsigned int)time(NULL));
// 
// 	unsigned char nonce[8] = {0};
// 	for (int i = 0; i < 8; i++)
// 		nonce[i] = rand();
// 
// 	chacha20_ctx c20_ctx;
// 	chacha20_setkey(&c20_ctx, (unsigned char*)key, nonce);
// 	chacha20_encrypt(&c20_ctx, file_is_data, file_is_size);
// 
// 	const size_t file_os_size = file_is_size + 8;
// 	char* file_os_data = new char[file_os_size];
// 	memcpy_s(file_os_data, file_os_size, nonce, 8);
// 	memcpy_s(file_os_data + 8, file_os_size, file_is_data, file_is_size);
// 	delete[] file_is_data;
// 
// 	ofstream file_os(file_out, ios::out | ios::binary);
// 	file_os.write(file_os_data, file_os_size);
// 	file_os.close();
// 	delete[] file_os_data;
// }

// void decrypt_file(const char* file_in, const char* file_out, const char key[32])
// {
// 	ifstream file_is(file_in, ios::in | ios::binary | ios::ate);
// 	if (!file_is.is_open())
// 		return;
// 
// 	const size_t file_is_size = file_is.tellg();
// 	file_is.seekg(0, ios::beg);
// 
// 	char* file_is_data = new char[file_is_size];
// 	file_is.read(file_is_data, file_is_size);
// 	file_is.close();
// 
// 	unsigned char nonce[8] = {0};
// 	memcpy_s(nonce, 8, file_is_data, 8);
// 
// 	const size_t file_os_size = file_is_size - 8;
// 
// 	chacha20_ctx c20_ctx;
// 	chacha20_setkey(&c20_ctx, (unsigned char*)key, nonce);
// 	chacha20_encrypt(&c20_ctx, file_is_data + 8, file_os_size);
// 
// 	char* file_os_data = new char[file_os_size];
// 	memcpy_s(file_os_data, file_os_size, file_is_data + 8, file_os_size);
// 	delete[] file_is_data;
// 
// 	ofstream file_os(file_out, ios::out | ios::binary);
// 	file_os.write(file_os_data, file_os_size);
// 	file_os.close();
// 	delete[] file_os_data;
// }
