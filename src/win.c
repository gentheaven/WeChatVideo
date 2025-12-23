#include <string.h>
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>

#include "wechat.h"
#include "win.h"


DWORD GetProcessIdByName(LPCWSTR processName)
{
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (Process32First(snapshot, &entry)) {
		while (Process32Next(snapshot, &entry)) {
			if (lstrcmp(entry.szExeFile, processName) == 0) {
				CloseHandle(snapshot);
				return entry.th32ProcessID;
			}
		}
	}

	CloseHandle(snapshot);
	//wprintf(L"%s not found \n", processName);
	return 0; //not find
}

BOOL KillProcess(DWORD dwProcessId, UINT uExitCode)
{
	BOOL bResult = FALSE;
	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);

	if (hProcess != NULL)
	{
		bResult = TerminateProcess(hProcess, uExitCode);
		CloseHandle(hProcess);
	}

	if (bResult)
		printf("Successfully terminated process %d\n", dwProcessId);
	else
		printf("Failed to terminate process% d(Error: % d)\n", dwProcessId, GetLastError());
	return bResult;
}


//utf_content = to_utf(buf, &realsize);
//gb2312 to utf8, should free memory by caller
char* to_utf(char* in, int* out_len)
{
	char* mid;
	char* out;
	int len;

	//in to mid
	//Maps a character string to a UTF-16 (wide character) string
	len = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)in, -1, NULL, 0);
	len = len * sizeof(wchar_t);
	mid = malloc(len);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)in, -1, (LPWSTR)mid, len);

	//mid to out
	//Maps a UTF-16 (wide character) string to a new character string. 
	len = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)mid, -1, NULL, 0, NULL, NULL);
	out = malloc(len);
	WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)mid, -1, (LPSTR)out, len, NULL, NULL);

	free(mid);
	*out_len = len;
	return out;
}

//gbk_content = utf16_to_gbk(buf, &realsize);
//utf16 to gb2312, should free memory by caller
char* utf16_to_gbk(char* in, int* out_len)
{
	char* out;
	int len;

	//Maps a UTF-16 (wide character) string to a new character string. 
	len = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)in, -1, NULL, 0, NULL, NULL);
	out = malloc(len);
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)in, -1, (LPSTR)out, len, NULL, NULL);
	*out_len = len;
	return out;
}

char* u8_to_gb2312(char* in, int* out_len)
{
	char* mid;
	int len;

	//convert an MBCS string to widechar, in to mid
	len = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)in, -1, NULL, 0);
	len = len * sizeof(wchar_t);
	mid = malloc(len);
	MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)in, -1, (LPWSTR)mid, len);

	//mid to out
	len = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)mid, len, NULL, 0, NULL, NULL);
	char* out = malloc(len + 1);

	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)mid, -1, (LPSTR)out, len, NULL, NULL);
	free(mid);

	*out_len = len;
	out[len] = 0;
	return out;
}

/*
C:\Users\xxx\AppData\Roaming\Tencent\WeChat\radium\web\profiles
APPDATA = C:\Users\xxx\AppData\Roaming

return 0 if OK
return 1 if wrong
*/
int delete_cache(char* log)
{
	int ret = 0;
	char* appData = getenv("APPDATA");
	if (appData) {
		printf("APPDATA path: %s\n", appData);
		sprintf(log, "rm -r %s\\Tencent\\WeChat\\radium\\web\\profiles", appData);
		ret = system(log);
		if (ret) {
			sprintf(log, "删除缓存失败，请关闭微信程序");
		} else {
			sprintf(log, "删除缓存成功");
		}
	}
	else {
		printf("failed to get APPDATA path\n");
		return 1;
	}
	return ret;
}


char curl_command[4096];
int download_video(struct video_info* video)
{
	sprintf(curl_command, ".\\curl.exe -o enc.mp4 \"%s\"", video->url);
	system(curl_command);
	return 0;
}


//.\dec.mp4 to .\videos\title.mp4
void moveto_download_dir(char* title)
{
	sprintf(curl_command, "move dec.mp4 \".\\videos\\%s\".mp4", title);
	printf("%s\n", curl_command);
	system(curl_command);
}

//kill mitmdump process
void kill_mitm(void)
{
	DWORD id = GetProcessIdByName(L"mitmdump.exe");
	if (!id) //not find
		return;
	KillProcess(id, 0); //kill mitmdump.exe
	Sleep(2000); //wait sometime
}