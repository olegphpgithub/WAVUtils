
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
	
	WAVUtils(char *lpcWAVFilePath, char *lpcCryptographicKey);

	void InitHeader();
	void TrimHeader();
	void Close();
	void AddTrack(char *payloadFilePath);
	void ReadHeader();
	void ReadTrackToFile(DWORD track, char *outputFilePath);
	void ReadTrackToMemory(DWORD track, unsigned char **pointer, DWORD *size);

};
