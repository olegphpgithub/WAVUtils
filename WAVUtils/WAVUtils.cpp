
#include <stdio.h>
#include <fstream>
#include <math.h>
#include <time.h>
#include <windows.h>
#include <tchar.h>

#include "MD5.h"
#include "WAVUtils.h"

#define R(v, n)(((v) >> (n)) | ((v) << (32 - (n))))
#define F(n) for (i = 0; i < n; i++)
#define X(a, b)(t) = (a), (a) = (b), (b) = (t)

WAVUtils::WAVUtils(const char *lpcWAVFilePath, const char *lpcCryptographicKey)
	: m_pszWAVBuffer(NULL)
	, m_nWAVBuffer(0)
{

	ZeroMemory(this->wavFilePath, MAX_PATH);
	strcpy_s(this->wavFilePath, MAX_PATH, lpcWAVFilePath);

	ZeroMemory(this->cryptographic_key, 1024);
	strcpy_s(this->cryptographic_key, 1024, lpcCryptographicKey);

	ZeroMemory(WAVHeader.BodyHeader, sizeof(WAVHeader.BodyHeader));

	errno_t err = fopen_s(&wavFile, wavFilePath, "rb");
	if (err == 0){
		fclose(wavFile);
		fopen_s(&wavFile, wavFilePath, "r+b");
		ReadHeader();
		return;
	} else {
		fopen_s(&wavFile, wavFilePath, "wb");
		InitHeader();
	}

}

WAVUtils::WAVUtils(const unsigned char *pszWAVBuffer, size_t nWAVBuffer, const char *pszCryptographicKey)
	: m_pszWAVBuffer(pszWAVBuffer)
	, m_nWAVBuffer(nWAVBuffer)
{

	ZeroMemory(this->cryptographic_key, 1024);
	strcpy_s(this->cryptographic_key, 1024, pszCryptographicKey);

	ZeroMemory(WAVHeader.BodyHeader, sizeof(WAVHeader.BodyHeader));
	ReadHeader();

}

void WAVUtils::InitHeader()
{
	memcpy_s((char *)&WAVHeader.ChunkId, 4, "RIFF", 4);
	memcpy_s((char *)&WAVHeader.Format, 4, "WAVE", 4);
	memcpy_s((char *)&WAVHeader.Subchunk1Id, 4, "fmt ", 4);
	WAVHeader.Subchunk1Size = 16;
	WAVHeader.AudioFormat = 1;
	WAVHeader.NumChannels = 2;
	WAVHeader.SampleRate = 44100;
	WAVHeader.ByteRate = 176400;
	WAVHeader.BlockAlign = 4;
	WAVHeader.BitsPerSample = 16;
	memcpy_s((char *)&WAVHeader.Subchunk2Id, 4, "data", 4);
	WAVHeader.NumberOfTracks = 0;
	ZeroMemory(WAVHeader.BodyHeader, sizeof(WAVHeader.BodyHeader));
}

void WAVUtils::TrimHeader()
{
	fseek(wavFile, 0, SEEK_END);
	long wavFileSize = ftell(wavFile);
	WAVHeader.ChunkSize = wavFileSize - 8;
	WAVHeader.Subchunk2Size = wavFileSize - 36 + 8;
}

void WAVUtils::Close()
{
	if(m_pszWAVBuffer == NULL) {
		fclose(wavFile);
	}
	SecureZeroMemory(cryptographic_key, 1024);
}

void WAVUtils::AddTrack(char* payloadFilePath)
{
	
	if(WAVHeader.NumberOfTracks == 0) {
		fseek(wavFile, 0, SEEK_SET);
		fwrite((void *)&WAVHeader, sizeof(WAVHeader), 1, wavFile);
	}
	
	FILE* payloadFile;
	errno_t err = fopen_s(&payloadFile, payloadFilePath, "rb");
	if (err != 0)
		return;
	fseek(payloadFile, 0, SEEK_END);
	long payloadFileSize = ftell(payloadFile);
	
	fseek(payloadFile, 0, SEEK_SET);
	unsigned char* payloadBuffer = new unsigned char[payloadFileSize + 8];
	ZeroMemory(payloadBuffer, payloadFileSize + 8);
	fread(payloadBuffer + 8, 1, payloadFileSize, payloadFile);
	fclose(payloadFile);

	/** +++++ Encryption +++++ */
	
	payloadFileSize = payloadFileSize + 8;
	int  res = EncryptCodeSection(cryptographic_key, payloadBuffer, payloadFileSize);
	
	/** ----- Encryption ----- */
	
	fseek(wavFile, 0, SEEK_END);
	WAVHeader.BodyHeader[WAVHeader.NumberOfTracks].offset = ftell(wavFile);
	WAVHeader.BodyHeader[WAVHeader.NumberOfTracks].size = payloadFileSize;
	WAVHeader.NumberOfTracks++;

	fwrite(payloadBuffer, 1, payloadFileSize, wavFile);
	delete[] payloadBuffer;
	
	TrimHeader();
	fseek(wavFile, 0, SEEK_SET);
	fwrite((void *)&WAVHeader, sizeof(WAVHeader), 1, wavFile);

}

void WAVUtils::ReadHeader()
{
	if(m_pszWAVBuffer != NULL) {
		memcpy_s((void *)&WAVHeader, sizeof(WAVHeader), m_pszWAVBuffer, sizeof(WAVHeader));
	} else {
		fseek(wavFile, 0, SEEK_SET);
		fread((void *)&WAVHeader, sizeof(WAVHeader), 1, wavFile);
	}
}

errno_t WAVUtils::ReadTrackToFile(DWORD track, char *outputFilePath)
{
	const unsigned char *m_start;
	size_t m_size;
	unsigned char *m_data;

	if(WAVHeader.NumberOfTracks < 1)
		return ERROR_ASSERTION_FAILURE;
	FILE* outputFile;
	errno_t err = fopen_s(&outputFile, outputFilePath, "wb");
	if (err != 0)
		return err;

	if(m_pszWAVBuffer != NULL) {
		m_start = m_pszWAVBuffer + WAVHeader.BodyHeader[track].offset;
		m_size = WAVHeader.BodyHeader[track].size;
		m_data = new unsigned char[m_size];
		memcpy_s((void *)m_data, m_size, m_start, m_size);
	} else {
		fseek(wavFile, WAVHeader.BodyHeader[track].offset, SEEK_SET);
		m_size = WAVHeader.BodyHeader[track].size;
		m_data = new unsigned char[m_size];
		fread(m_data, m_size, 1, wavFile);
	}
	
	/** +++++ Decryption +++++ */
	
	errno_t result = DecryptCodeSection(cryptographic_key, m_data, m_size);
	
	/** ----- Decryption ----- */

	fwrite(m_data + 8, m_size - 8, 1, outputFile);
	fclose(outputFile);

	SecureZeroMemory(m_data, m_size);
	delete[] m_data;
	
	return result;
}

errno_t WAVUtils::ReadTrackToMemory(DWORD track, unsigned char **pointer, DWORD *size)
{
	const unsigned char *m_start;
	size_t m_size;
	unsigned char *m_data;

	if(m_pszWAVBuffer != NULL) {
		m_start = m_pszWAVBuffer + WAVHeader.BodyHeader[track].offset;
		m_size = WAVHeader.BodyHeader[track].size;
		m_data = new unsigned char[m_size];
		memcpy_s((void *)m_data, m_size, m_start, m_size);
	} else {
		fseek(wavFile, WAVHeader.BodyHeader[track].offset, SEEK_SET);
		m_size = WAVHeader.BodyHeader[track].size;
		m_data = new unsigned char[m_size];
		fread(m_data, m_size, 1, wavFile);
	}
	
	/** +++++ Decryption +++++ */
	
	errno_t result = DecryptCodeSection(cryptographic_key, m_data, m_size);
	
	/** ----- Decryption ----- */
	
	*pointer = new unsigned char[m_size - 8];
	memcpy_s(*pointer, m_size - 8, m_data + 8, m_size - 8);
	*size = m_size - 8;

	SecureZeroMemory(m_data, m_size);
	delete[] m_data;
	
	return result;
	
}

void WAVUtils::chacha20_core(void* input, void* output)
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

void WAVUtils::chacha20_setkey(chacha20_ctx* c, void* key, void* nonce)
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

void WAVUtils::chacha20_encrypt(chacha20_ctx* ctx, void* in, unsigned int len)
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

errno_t WAVUtils::DecryptCodeSection(char* cryptKey, unsigned char *raw_data, unsigned int raw_size)
{
	std::string skey = MD5String(cryptKey);
	srand((unsigned int)time(NULL));

	chacha20_ctx c20_ctx;
	chacha20_setkey(&c20_ctx, (unsigned char*)skey.c_str(), raw_data);
	chacha20_encrypt(&c20_ctx, raw_data+8, raw_size-8);

	return ERROR_SUCCESS;
}

errno_t WAVUtils::EncryptCodeSection(char* cryptKey, unsigned char *raw_data, unsigned int raw_size)
{
	std::string skey = MD5String(cryptKey);

	srand((unsigned int)time(NULL));

	for (int i = 0; i < 8; i++)
		raw_data[i] = (char)rand();

	chacha20_ctx c20_ctx;
	chacha20_setkey(&c20_ctx, (unsigned char*)skey.c_str(), raw_data);
	chacha20_encrypt(&c20_ctx, raw_data+8, raw_size-8);

	return ERROR_SUCCESS;

}


