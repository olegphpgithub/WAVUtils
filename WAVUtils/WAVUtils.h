#pragma once

typedef union _chacha20_ctx_t
{
	unsigned char b[64];
	unsigned int w[16];
	long long q[8];
} chacha20_ctx;

class WAVUtils {
public:

	struct _BodyHeader
	{
		DWORD offset;
		DWORD size;
	};

	struct _WAVHeader {
		DWORD ChunkId;
		DWORD ChunkSize;
		DWORD Format;
		DWORD Subchunk1Id;
		DWORD Subchunk1Size;
		WORD AudioFormat;
		WORD NumChannels;
		DWORD SampleRate;
		DWORD ByteRate;
		WORD BlockAlign;
		WORD BitsPerSample;
		DWORD Subchunk2Id;
		DWORD Subchunk2Size;
		DWORD NumberOfTracks;
		_BodyHeader BodyHeader[16];
	} WAVHeader;

	char wavFilePath[MAX_PATH];
	char payloadFilePath[MAX_PATH];
	char cryptographic_key[1024];
	FILE* wavFile;
	
	WAVUtils(const char *lpcWAVFilePath, const char *lpcCryptographicKey);

	void InitHeader();
	void TrimHeader();
	void Close();
	void AddTrack(char *payloadFilePath);
	void ReadHeader();
	void ReadTrackToFile(DWORD track, char *outputFilePath);
	void ReadTrackToMemory(DWORD track, unsigned char **pointer, DWORD *size);
	void chacha20_core(void* input, void* output);
	void chacha20_setkey(chacha20_ctx* c, void* key, void* nonce);
	void chacha20_encrypt(chacha20_ctx* ctx, void* in, unsigned int len);
	int DecryptCodeSection(char*, unsigned char*, unsigned int);
	int EncryptCodeSection(char*, unsigned char*, unsigned int);

};
