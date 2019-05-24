
#include <stdio.h>
#include <windows.h>
#include <tchar.h>

#include "WAVUtils.h"


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

		//wav.ReadTrackToFile(0, payloadFilePath);

		/** Read to memory */
		
		//unsigned char *p;
		//DWORD dw;
		//wav.ReadTrackToMemory(0, &p, &dw);

		/** Read from memory to memory */

		FILE *f;
		errno_t err = fopen_s(&f, argv[2], "r");
		fseek(f, 0, SEEK_END);
		long wavFileSize = ftell(f);
		unsigned char *buffer = new unsigned char[wavFileSize];
		fread(buffer, 1, wavFileSize, f);
		fclose(f);
		
		WAVUtils wav2(buffer, (size_t)wavFileSize, argv[4]);
		unsigned char *pointer;
		DWORD size;
		wav2.ReadTrackToMemory(0, &pointer, &size);

	}
	
	wav.Close();

	return 0;
}

