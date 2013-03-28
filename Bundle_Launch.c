//==================================================================================================================================
// Bundle_Launch.c
//==================================================================================================================================
#include <windows.h>
#include <stdio.h>
#include "bundle.h"

//==================================================================================================================================
void ShowError(char *error)
{
  char description[512];
  sprintf(description, "Error launching bundle!\n  \"%s\"", error);
  MessageBoxA(NULL, description, "Bundle_Launch", MB_ICONERROR | MB_OK);
}
//==================================================================================================================================
int APIENTRY WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmdLine, int showCmd)
{
  //get command line arguments (wide)
  int argc; wchar_t **argv;
  argv = CommandLineToArgvW(GetCommandLineW(), &argc);

  if(argc < 2)
  {
    ShowError("no bundle specified");
    return -1;
  }

  bundle *bundle = Bundle_ReadW(argv[1]);
  if(bundle == NULL)
  {
    ShowError(Bundle_Error);
    return -1;
  }

  BOOL result = Bundle_LaunchW(bundle, &(argv[2]), argc-2);
  if(!result)
  {
    ShowError(Bundle_Error);
    return -1;
  }
  
  //Cleanup
  LocalFree(argv);
  Bundle_Free(bundle);

  return 0;
}

//==================================================================================================================================
//----------------------------------------------------------------------------------------------------------------------------------
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
