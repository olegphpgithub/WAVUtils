
#include <stdio.h>
#include <windows.h>
#include <tchar.h>

#include "WAVUtils.h"
#include "CmdLineParser.h"


int isFileExists(const char *filename) {
	FILE *file;
	errno_t err = fopen_s(&file, filename, "r");
	if (err == 0){
		fclose(file);
		return 1;
	}
	return 0;
}


void testReadTrackFromMemoryToMemory(LPCTSTR path, LPCTSTR key, DWORD track)
{
	FILE *f;
	errno_t err = fopen_s(&f, path, "rb");
	DWORD dv = GetLastError();
	fseek(f, 0, SEEK_END);
	long wavFileSize = ftell(f);
	unsigned char *buffer = new unsigned char[wavFileSize];
	fseek(f, 0, SEEK_SET);
	fread(buffer, 1, wavFileSize, f);
	fclose(f);
	
	WAVUtils wav2(buffer, (size_t)wavFileSize, key);
	unsigned char *pointer;
	DWORD size;
	wav2.ReadTrackToMemory(track, &pointer, &size);
}


void testReadTrackFromMemoryToFile(LPCTSTR path, LPCTSTR key, DWORD track, LPCTSTR payload)
{
	FILE *f;
	errno_t err = fopen_s(&f, path, "rb");
	DWORD dv = GetLastError();
	fseek(f, 0, SEEK_END);
	long wavFileSize = ftell(f);
	unsigned char *buffer = new unsigned char[wavFileSize];
	fseek(f, 0, SEEK_SET);
	fread(buffer, 1, wavFileSize, f);
	fclose(f);
	
	WAVUtils wav2(buffer, (size_t)wavFileSize, key);
	wav2.ReadTrackToFile(track, (TCHAR*)payload);
}


int _tmain(int argc, _TCHAR* argv[])
{

	CCmdLineParser realParser(::GetCommandLine());

	bool ok = false;
	do {
		if ( (!realParser.HasKey(TEXT("ACTION")))
			|| (!realParser.HasKey(TEXT("WAV")))
			|| (!realParser.HasKey(TEXT("PAYLOAD")))
			|| (!realParser.HasKey(TEXT("KEY"))) ) {
				break;
		}
		ok = true;
	} while(false);

	if(!ok) {
		printf("Usage: prog.exe /ACTION=import/export /WAV=<WAVFile> /PAYLOAD=<PayloadFile> /KEY=<Key>");
		return 1;
	}

	char payloadFilePath[MAX_PATH];
	ZeroMemory(payloadFilePath, MAX_PATH);
	strcpy_s(payloadFilePath, MAX_PATH, realParser.GetVal(_T("PAYLOAD")));

	if(_strcmpi(realParser.GetVal(_T("ACTION")), "import") == 0) {

		WAVUtils wav(realParser.GetVal(_T("WAV")), realParser.GetVal(_T("KEY")));

		if(!isFileExists(payloadFilePath)) {
			printf("PayloadFile does not exist.\n");
			return 1;
		}
		
		wav.AddTrack(payloadFilePath);
		wav.Close();

	} else {

		/** Read to file */
		
		WAVUtils wav(realParser.GetVal(_T("WAV")), realParser.GetVal(_T("KEY")));
		wav.ReadTrackToFile(0, payloadFilePath);

		/** Read from memory to memory */

		/*
		testReadTrackFromMemoryToMemory(
			realParser.GetVal(_T("WAV")),
			realParser.GetVal(_T("KEY")),
			0
		);
		*/

		/** Read from memory to memory */
		
		/*
		testReadTrackFromMemoryToFile(
			realParser.GetVal(_T("WAV")),
			realParser.GetVal(_T("KEY")),
			0,
			payloadFilePath
		);
		*/

	}
	
	

	return 0;
}

