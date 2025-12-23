#ifndef _WINOS_H_
#define _WINOS_H_

extern DWORD GetProcessIdByName(LPCWSTR processName);

extern BOOL KillProcess(DWORD dwProcessId, UINT uExitCode);

//utf_content = to_utf(buf, &realsize);
//gb2312 to utf8, should free memory by caller
extern char* to_utf(char* in, int* out_len);

extern char* u8_to_gb2312(char* in, int* out_len);;

//gbk_content = utf16_to_gbk(buf, &realsize);
//utf16 to gb2312, should free memory by caller
extern char* utf16_to_gbk(char* in, int* out_len);

/*
C:\Users\xxx\AppData\Roaming\Tencent\WeChat\radium\web\profiles
return 0 if OK
return 1 if wrong
*/
extern int delete_cache(char* log);

//curl.exe -o enc.mp4 url
extern int download_video(struct video_info* video);

//.\dec.mp4 to .\videos\title.mp4
extern void moveto_download_dir(char* title);

#endif
