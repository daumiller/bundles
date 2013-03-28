//==================================================================================================================================
// bundle.h
//==================================================================================================================================
#ifndef _BUNDLE_HEADER_
#define _BUNDLE_HEADER_

//==================================================================================================================================
#ifndef BOOL
#include <windows.h>
#endif

//==================================================================================================================================
typedef struct s_bundle
{
  //----------------------
  //paths
  char *pathBundle;
  char *pathXml;
  //----------------------
  //standard bundle keys
  char *name;
  char *icon;
  char *version;
  char *identifier;
  char *mainExecutable;
  char *libraryDirectory;
  char *workingDirectory;
  //----------------------
  //additional keys
  void *other;
  //----------------------
} bundle;

//==================================================================================================================================
bundle *Bundle_Read(char *path);
BOOL    Bundle_Write(bundle *bundle);
char   *Bundle_GetKeyValue(bundle *bundle, char *key);
void    Bundle_SetKeyValue(bundle *bundle, char *key, char *value);
char  **Bundle_ListOtherKeys(bundle *bundle, int *count);
BOOL    Bundle_ApplyIcon(bundle *bundle);
BOOL    Bundle_Launch(bundle *bundle, char **arguments, int argCount);
BOOL    Bundle_LaunchW(bundle *bundle, wchar_t **arguments, int argCount);
void    Bundle_Free(bundle *bundle);

//==================================================================================================================================
#ifndef _BUNDLE_SOURCE_
extern char *Bundle_Error;
#endif

//==================================================================================================================================
#endif //_BUNDLE_HEADER_

//==================================================================================================================================
//----------------------------------------------------------------------------------------------------------------------------------
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
