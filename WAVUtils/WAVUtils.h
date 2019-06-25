#pragma once

class WAVUtils {
public:

	typedef union _chacha20_ctx_t
	{
		unsigned char b[64];
		unsigned int w[16];
		long long q[8];
	} chacha20_ctx;

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
	const unsigned char *m_pszWAVBuffer;
	const size_t m_nWAVBuffer;
	FILE* wavFile;
	
	WAVUtils(const char *lpcWAVFilePath, const char *lpcCryptographicKey);
	WAVUtils(const unsigned char *pszWAVBuffer, size_t nWAVBuffer, const char *pszCryptographicKey);

	void InitHeader();
	void TrimHeader();
	void Close();
	void AddTrack(char *payloadFilePath);
	void ReadHeader();
	errno_t ReadTrackToFile(DWORD track, char *outputFilePath);
	errno_t ReadTrackToMemory(DWORD track, unsigned char **pointer, DWORD *size);
	void chacha20_core(void* input, void* output);
	void chacha20_setkey(chacha20_ctx* c, void* key, void* nonce);
	void chacha20_encrypt(chacha20_ctx* ctx, void* in, unsigned int len);
	errno_t DecryptCodeSection(char*, unsigned char*, unsigned int);
	errno_t EncryptCodeSection(char*, unsigned char*, unsigned int);

};
