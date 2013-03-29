//==================================================================================================================================
// wide.c
//==================================================================================================================================
#include "wide.h"

//==================================================================================================================================
wchar_t *WideFromUTF8(char *utf8)
{
  //convert UTF8 to Wide16
  wchar_t *buff = NULL; int buffSz = 0;
  buffSz = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
  buff   = malloc(buffSz * sizeof(wchar_t));
  MultiByteToWideChar(CP_UTF8, 0, utf8, -1, buff, buffSz);
  //don't forget to free() me
  return buff;
}
//----------------------------------------------------------------------------------------------------------------------------------
char *WideToUTF8(wchar_t *wide)
{
  //convert Wide16 to UTF8
  char *buff = NULL; int buffSz = 0;
  buffSz = WideCharToMultiByte(CP_UTF8, 0, wide, -1, NULL, 0, NULL, NULL);
  buff   = malloc((buffSz+1) * sizeof(char));
  WideCharToMultiByte(CP_UTF8, 0, wide, -1, buff, buffSz, NULL, NULL);
  return buff;
}
//----------------------------------------------------------------------------------------------------------------------------------
FILE *Wide_fopen(char *path, char *mode)
{
  wchar_t *widePath = WideFromUTF8(path);
  wchar_t *wideMode = WideFromUTF8(mode);
  FILE *fp = _wfopen(widePath, wideMode);
  free(widePath);
  free(wideMode);
  return fp;
}
//----------------------------------------------------------------------------------------------------------------------------------
int Wide_stat(char *path, struct _stat *buf)
{
  wchar_t *widePath = WideFromUTF8(path);
  int ret = _wstat(widePath, buf);
  free(widePath);
  return ret;
}

//==================================================================================================================================
//----------------------------------------------------------------------------------------------------------------------------------
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
