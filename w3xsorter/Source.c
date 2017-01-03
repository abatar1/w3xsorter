#pragma comment(linker, "/MERGE:.data=.text")
#pragma comment(linker, "/MERGE:.rdata=.text")
#pragma comment(linker, "/SECTION:.text,EWR")

#include <windows.h>

#define	READING_START_OFFSET 8
#define	READ_BUFFER_SIZE 64

void entry(void)
{
	WIN32_FIND_DATA 	winFileData;
	HANDLE			hFile;
	HANDLE			w3xFile;

	TCHAR			szPath[MAX_PATH];
	CHAR			readBuffer[READ_BUFFER_SIZE];
	LPWSTR			fileName;

	if (GetCurrentDirectory(sizeof(szPath), szPath))
	{
		LPWSTR postfix = L"\\*.*";
		lstrcat(szPath, postfix);
		hFile = FindFirstFile(szPath, &winFileData);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (winFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) continue;

				fileName = winFileData.cFileName;
				LPWSTR ext = fileName + wcslen(fileName) - 4;

				if (!lstrcmp(ext, L".w3x") ||
					!lstrcmp(ext, L".w3n") ||
					!lstrcmp(ext, L".w3m"))
				{
					w3xFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL,
						OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
					if (!w3xFile) continue;

					if (ReadFile(w3xFile, readBuffer, READ_BUFFER_SIZE, NULL, NULL))
					{
						WCHAR newFileName[MAX_PATH];
						WCHAR wc_maxPlayersNum[2];

						if ((readBuffer[0] == 'H') &&
							(readBuffer[1] == 'M') &&
							(readBuffer[2] == '3') &&
							(readBuffer[3] == 'W'))
						{
							size_t curPos = READING_START_OFFSET;
							while (readBuffer[curPos]) curPos++;
							curPos += 5;

							CHAR littleEndianMaxPlayersNum[4];
							for (size_t i = 0; i < 4; i++)
								littleEndianMaxPlayersNum[3 - i] = readBuffer[curPos + i];

							size_t maxPlayersNum = (littleEndianMaxPlayersNum[0] << 24 |
								littleEndianMaxPlayersNum[1] << 16 |
								littleEndianMaxPlayersNum[2] << 8 |
								littleEndianMaxPlayersNum[3]);

							wsprintf(wc_maxPlayersNum, L"%d", maxPlayersNum);
							lstrcpy(newFileName, wc_maxPlayersNum);
						}
						else
						{
							lstrcpy(newFileName, L"unidentified");
						}
						CreateDirectory(newFileName, NULL);

						lstrcat(newFileName, L"\\");
						lstrcat(newFileName, fileName);

						CloseHandle(w3xFile);

						MoveFile(fileName, newFileName);
						DeleteFile(fileName);
					}
				}
			} while (FindNextFile(hFile, &winFileData) != 0);

			FindClose(hFile);
		}
	}
	ExitProcess(0);
}
