//==================================================================================================================================
// Bundle_Association.c
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Proxy application for file-type associations.
// Customize defines, recompile (with an icon, probably), and place inside final application bundle.
// Use this executable as your file-type association handler.
// It will take any passed arguments (files to open), and pass them on to a newly launched instance of its containing bundle.
//==================================================================================================================================
#undef   __STRICT_ANSI__
#include <windows.h>
#include <string.h>
#include <wchar.h>
#include "bundle.h"
#include "wide.h"

//==================================================================================================================================
#define APP_NAME          L"Bundle_Association"
#define ERROR_INTRO       L"Error launching bundle!\n  "
#define SHOW_ERRORS       TRUE
#define SHOW_ERROR_DETAIL TRUE

//==================================================================================================================================
wchar_t *GetContainingBundle(wchar_t *path);
int ShowErrorA(char *error);
int ShowErrorW(wchar_t *error);

//==================================================================================================================================
int APIENTRY WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmdLine, int showCmd)
{
  //get command line arguments (wide)
  int argc; wchar_t **argv;
  argv = CommandLineToArgvW(GetCommandLineW(), &argc);

  //find containing bundle
  wchar_t *bundlePath = GetContainingBundle(argv[0]);
  if(!bundlePath) return ShowErrorW(L"launcher not ran from inside a bundle");

  //read bundle
  bundle *bundle = Bundle_ReadW(bundlePath);
  if(bundle == NULL) return ShowErrorA(Bundle_Error);

  //launch bundle
  BOOL result = Bundle_LaunchW(bundle, &(argv[1]), argc-1);
  if(!result) return ShowErrorA(Bundle_Error);
  
  //Cleanup
  LocalFree(argv);
  free(bundlePath);
  Bundle_Free(bundle);

  return 0;
}

//==================================================================================================================================
wchar_t *GetContainingBundle(wchar_t *path)
{
  wchar_t *w = _wcsdup(path); if(!w) return NULL;

  int i = wcslen(w) - 1;
  while(i > 3)
  {
    if(_wcsicmp(&(w[i-3]), L".app") == 0)
      return w; //don't re-copy; even if shorter, we're very likely under 4k
    while((i > 0) && (w[i] != L'\\'))
      i--;
    w[i] = L'\0';
    i--;
  }
  free(w);
  return NULL;
}

//==================================================================================================================================
int ShowErrorA(char *error)
{
  wchar_t *w = WideFromUTF8(error);
  if(w)
    ShowErrorW(w);
  else
    MessageBoxW(NULL, ERROR_INTRO, APP_NAME, MB_ICONERROR | MB_OK);
  free(w);
  return -1;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int ShowErrorW(wchar_t *error)
{
  if(SHOW_ERRORS)
  {
    BOOL detailed = SHOW_ERROR_DETAIL;
    if(detailed)
    {
      wchar_t *description = malloc(sizeof(wchar_t) * (wcslen(ERROR_INTRO)+1+wcslen(error)+2));
      if(!description)
        detailed = FALSE;
      else
      {
        swprintf(description, L"%s\"%s\"", ERROR_INTRO, error);
        MessageBoxW(NULL, description, APP_NAME, MB_ICONERROR | MB_OK);
        free(description);
      }
    }
    if(!detailed)
      MessageBoxW(NULL, ERROR_INTRO, APP_NAME, MB_ICONERROR | MB_OK);
  }
  return -1;
}

//==================================================================================================================================
//----------------------------------------------------------------------------------------------------------------------------------
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
