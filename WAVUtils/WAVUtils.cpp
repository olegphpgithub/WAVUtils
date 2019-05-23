
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <windows.h>
#include <tchar.h>

#include "CrypUtils.h"
#include "WAVUtils.h"

WAVUtils::WAVUtils(char *lpcWAVFilePath, char *lpcCryptographicKey)
{
	ZeroMemory(this->wavFilePath, MAX_PATH);
	strcpy_s(this->wavFilePath, MAX_PATH, lpcWAVFilePath);

	ZeroMemory(this->cryptographic_key, MAX_PATH);
	strcpy_s(this->cryptographic_key, MAX_PATH, lpcCryptographicKey);

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
	fclose(wavFile);
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
	fseek(wavFile, 0, SEEK_SET);
	fread((void *)&WAVHeader, sizeof(WAVHeader), 1, wavFile);
}

void WAVUtils::ReadTrackToFile(DWORD track, char *outputFilePath)
{
	if(WAVHeader.NumberOfTracks < 1)
		return;
	FILE* outputFile;
	errno_t err = fopen_s(&outputFile, outputFilePath, "wb");
	if (err != 0)
		return;
	fseek(wavFile, WAVHeader.BodyHeader[track].offset, SEEK_SET);
	unsigned char* payloadBuffer = new unsigned char[WAVHeader.BodyHeader[track].size];
	fread(payloadBuffer, WAVHeader.BodyHeader[track].size, 1, wavFile);
	
	/** +++++ Decryption +++++ */
	
	int  res = DecryptCodeSection(cryptographic_key, payloadBuffer, WAVHeader.BodyHeader[track].size);
	
	/** ----- Decryption ----- */

	fwrite(payloadBuffer + 8, WAVHeader.BodyHeader[track].size - 8, 1, outputFile);
	fclose(outputFile);

	delete payloadBuffer;

}

void WAVUtils::ReadTrackToMemory(DWORD track, unsigned char **pointer, DWORD *size)
{
	fseek(wavFile, WAVHeader.BodyHeader[track].offset, SEEK_SET);
	size_t m_size = WAVHeader.BodyHeader[track].size;
	unsigned char *m_data = new unsigned char[m_size];
	fread(m_data, m_size, 1, wavFile);
	
	/** +++++ Decryption +++++ */
	
	int  res = DecryptCodeSection(cryptographic_key, m_data, m_size);
	
	/** ----- Decryption ----- */
	
	*pointer = new unsigned char[m_size - 8];
	memcpy_s(*pointer, m_size - 8, m_data + 8, m_size - 8);
	*size = m_size - 8;
	delete[] m_data;
}

int isFileExists(const char *filename) {
	FILE *file;
	errno_t err = fopen_s(&file, filename, "r");
	if (err == 0){
		fclose(file);
		return 1;
	}
	return 0;
}


int _tmain(int argc, _TCHAR* argv[])
{
	if(argc != 5) {
		printf("Usage: prog.exe import/export <WAVFile> <PayloadFile> <Key>");
		return 1;
	}

	char payloadFilePath[MAX_PATH];
	ZeroMemory(payloadFilePath, MAX_PATH);
	strcpy_s(payloadFilePath, MAX_PATH, argv[3]);

	WAVUtils wav(argv[2], argv[4]);

	if(_strcmpi(argv[1], "import") == 0) {

		if(!isFileExists(payloadFilePath)) {
			printf("PayloadFile does not exist.\n");
			return 1;
		}
		
		wav.AddTrack(payloadFilePath);

	} else {

		/** Read to file */

		wav.ReadTrackToFile(0, payloadFilePath);

		/** Read to memory */
		
		//unsigned char *p;
		//DWORD dw;
		//wav.ReadTrackToMemory(0, &p, &dw);
		
	}
	
	wav.Close();

	return 0;
}

