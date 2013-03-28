//==================================================================================================================================
// bundle.c
//==================================================================================================================================
#define _BUNDLE_SOURCE_ //<-- for bundle.h (Bundle_Error)
#undef  __STRICT_ANSI__ //<-- for string.h (MinGW: stricmp, strdup)
#define UNICODE         //<-- for windows.h
#include <windows.h>
#include <shlobj.h> //SHChangeNotify
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include "mxml.h"
#include "hashtable.h"
#include "bundle.h"
#include "wide.h"

//==================================================================================================================================
typedef mxml_node_t xmlNode;
const char *XmlBeautifier(xmlNode *node, int where);
//----------------------------------------------------------------------------------------------------------------------------------
BOOL PathIsDirectory(char *path);
BOOL PathIsFile(char *path);

//==================================================================================================================================
char *Bundle_Error;

//==================================================================================================================================
bundle *Bundle_Read(char *path)
{
  //allocate bundle
  bundle *bundle = calloc(1, sizeof(struct s_bundle));
  if(bundle == NULL) { Bundle_Error = "unable to allocate memory"; return NULL; }
  bundle->other = malloc(sizeof(hash_table));
  if(!bundle->other) { free(bundle); Bundle_Error = "unable to allocate memory"; return NULL; }
  ht_init(bundle->other, HT_NONE, 0.05);

  //build paths
  bundle->pathBundle = strdup(path);                          if(!bundle->pathBundle) { Bundle_Free(bundle); Bundle_Error = "unable to allocate memory"; return NULL; }
  bundle->pathXml = malloc(sizeof(char) * (strlen(path)+12)); if(!bundle->pathXml)    { Bundle_Free(bundle); Bundle_Error = "unable to allocate memory"; return NULL; }
  sprintf(bundle->pathXml, "%s%s\0", bundle->pathBundle, "/bundle.xml");
  
  //initial filesystem checks
  if(!PathIsDirectory(bundle->pathBundle)) { Bundle_Free(bundle); Bundle_Error = "specified path is not a directory/bundle"; return NULL; }
  if(!PathIsFile(bundle->pathXml))         { Bundle_Free(bundle); Bundle_Error = "bundle doesn't contain configuration xml"; return NULL; }

  //load configuration xml
  FILE *fin = Wide_fopen(bundle->pathXml, "r"); if(!fin) { Bundle_Free(bundle); Bundle_Error = "unable to open bundle configuration xml"; return NULL; }
  xmlNode *tree = mxmlLoadFile(NULL, fin, MXML_OPAQUE_CALLBACK);

  //parse file
  xmlNode *curr = tree;
  char *currElement, *currKey, *currValue;
  while(curr != NULL)
  {
    currElement = (char *)mxmlGetElement(curr);

    if(currElement == NULL)
      curr = mxmlWalkNext(curr, tree, MXML_DESCEND);
    else if(stricmp(currElement, "key") == 0)
    {
      currKey = (char *)mxmlGetOpaque(curr);
      while(1)
      {
        curr = mxmlGetNextSibling(curr);            if(!curr)        break;
        currElement = (char *)mxmlGetElement(curr); if(!currElement) continue;
        if(stricmp(currElement, "key"  ) == 0) break;
        if(stricmp(currElement, "value") == 0)
        {
          currValue = (char *)mxmlGetOpaque(curr);
          Bundle_SetKeyValue(bundle, currKey, currValue);
          break;
        }
      }
      curr = mxmlGetNextSibling(curr);
    }
    else
      curr = mxmlWalkNext(curr, tree, MXML_DESCEND);
  }

  //clean up
  fclose(fin);
  mxmlDelete(tree);

  Bundle_Error = NULL;
  return(bundle);
}
//----------------------------------------------------------------------------------------------------------------------------------
BOOL Bundle_Write(bundle *bundle)
{
  xmlNode *xml, *root, *node;
  xml  = mxmlNewXML("1.0");
  root = mxmlNewElement(xml, "bundle");

  //create standard KVPs
  node=mxmlNewElement(root,"key"); mxmlNewText(node,0,"Name"            ); node=mxmlNewElement(root, "value"); if(bundle->name            ) mxmlNewText(node,0,bundle->name            ); else mxmlNewText(node,0,"");
  node=mxmlNewElement(root,"key"); mxmlNewText(node,0,"Icon"            ); node=mxmlNewElement(root, "value"); if(bundle->icon            ) mxmlNewText(node,0,bundle->icon            ); else mxmlNewText(node,0,"");
  node=mxmlNewElement(root,"key"); mxmlNewText(node,0,"Version"         ); node=mxmlNewElement(root, "value"); if(bundle->version         ) mxmlNewText(node,0,bundle->version         ); else mxmlNewText(node,0,"");
  node=mxmlNewElement(root,"key"); mxmlNewText(node,0,"Identifier"      ); node=mxmlNewElement(root, "value"); if(bundle->identifier      ) mxmlNewText(node,0,bundle->identifier      ); else mxmlNewText(node,0,"");
  node=mxmlNewElement(root,"key"); mxmlNewText(node,0,"MainExecutable"  ); node=mxmlNewElement(root, "value"); if(bundle->mainExecutable  ) mxmlNewText(node,0,bundle->mainExecutable  ); else mxmlNewText(node,0,"");
  node=mxmlNewElement(root,"key"); mxmlNewText(node,0,"LibraryDirectory"); node=mxmlNewElement(root, "value"); if(bundle->libraryDirectory) mxmlNewText(node,0,bundle->libraryDirectory); else mxmlNewText(node,0,"");
  node=mxmlNewElement(root,"key"); mxmlNewText(node,0,"WorkingDirectory"); node=mxmlNewElement(root, "value"); if(bundle->workingDirectory) mxmlNewText(node,0,bundle->workingDirectory); else mxmlNewText(node,0,"");

  //create additional KVPs
  unsigned int keyCount;
  char **keys = (char **)ht_keys(bundle->other, &keyCount);
  char *value;
  for(unsigned int i=0; i<keyCount; i++)
  {
    value = ht_get(bundle->other, keys[i], strlen(keys[i])+1, NULL);
    node = mxmlNewElement(root, "key"  );           mxmlNewText(node,0,keys[i]);
    node = mxmlNewElement(root, "value"); if(value) mxmlNewText(node,0,value  );
  }

  //attempt to write file
  FILE *fout = Wide_fopen(bundle->pathXml, "w"); if(!fout) { mxmlDelete(xml); return FALSE; }
  int result = mxmlSaveFile(xml, fout, XmlBeautifier);
  fclose(fout);
  mxmlDelete(xml);

  Bundle_Error = (result == 0) ? NULL : "error writing bundle xml file";
  return (result == 0);
}
//----------------------------------------------------------------------------------------------------------------------------------
char *Bundle_GetKeyValue(bundle *bundle, char *key)
{
  if(key    == NULL) return NULL;
  if(key[0] == 0x00) return NULL;
  int keylen = strlen(key);

  if(keylen == 4)
  {
    if(stricmp(key, "Name") == 0) return bundle->name;
    if(stricmp(key, "Icon") == 0) return bundle->icon;
  }
  else if(keylen == 16)
  {
    if(stricmp(key, "LibraryDirectory" ) == 0) return bundle->libraryDirectory;
    if(stricmp(key, "WorkingDirectory" ) == 0) return bundle->workingDirectory;
  }
  else if(keylen ==  7) { if(stricmp(key, "Version"         ) == 0) return bundle->version;          }
  else if(keylen == 10) { if(stricmp(key, "Identifier"      ) == 0) return bundle->identifier;       }
  else if(keylen == 14) { if(stricmp(key, "MainExecutable"  ) == 0) return bundle->mainExecutable;   }

  return ht_get(bundle->other, key, keylen+1, NULL);
}
//----------------------------------------------------------------------------------------------------------------------------------
void Bundle_SetKeyValue(bundle *bundle, char *key, char *value)
{
  if(key    == NULL) return;
  if(key[0] == 0x00) return;
  int keylen = strlen(key);

  char **dest = NULL;
  if(keylen == 4)
  {
    if(stricmp(key, "Name") == 0) dest = &(bundle->name);
    if(stricmp(key, "Icon") == 0) dest = &(bundle->icon);
  }
  else if(keylen == 16)
  {
    if(stricmp(key, "LibraryDirectory" ) == 0) dest = &(bundle->libraryDirectory);
    if(stricmp(key, "WorkingDirectory" ) == 0) dest = &(bundle->workingDirectory);
  }
  else if(keylen ==  7) { if(stricmp(key, "Version"       ) == 0) dest = &(bundle->version);        }
  else if(keylen == 10) { if(stricmp(key, "Identifier"    ) == 0) dest = &(bundle->identifier);     }
  else if(keylen == 14) { if(stricmp(key, "MainExecutable") == 0) dest = &(bundle->mainExecutable); }
  if(dest)
  {
    if(*dest) free(*dest);
    *dest = value ? strdup(value) : NULL;
    return;
  }

  if(value == NULL)
    ht_remove(bundle->other, key, keylen+1);
  else
    ht_insert(bundle->other, key, keylen+1, value, strlen(value)+1);
}
//----------------------------------------------------------------------------------------------------------------------------------
char **Bundle_ListOtherKeys(bundle *bundle, int *count)
{
  unsigned int i;
  char **ret = (char **)ht_keys(bundle->other, &i);
  *count = (int)i;
  return ret;
}
//----------------------------------------------------------------------------------------------------------------------------------
BOOL Bundle_ApplyIcon(bundle *bundle)
{
  int len; char *s;

  //sanity check
  if(bundle->icon    == NULL) { Bundle_Error = "bundle doesn't specify an icon"; return FALSE; }
  if(bundle->icon[0] == 0x00) { Bundle_Error = "bundle doesn't specify an icon"; return FALSE; }

  //bundle path
  wchar_t *wBundlePath = WideFromUTF8(bundle->pathBundle);

  //build icon path
  len = strlen(bundle->pathBundle) + 1 + strlen(bundle->icon) + 1;
  s = malloc(len); if(!s) { free(wBundlePath); Bundle_Error = "unable to allocate memory"; return FALSE; }
  sprintf(s, "%s\\%s", bundle->pathBundle, bundle->icon);

  //is icon path actual file?
  if(!PathIsFile(s)) { free(s); free(wBundlePath); Bundle_Error = "specified icon does not exist, or is not a file"; return FALSE; }
  free(s);

  //use relative path when actually assigning the icon
  wchar_t *wPathIcon = WideFromUTF8(bundle->icon);

  //build desktop.ini path
  len = strlen(bundle->pathBundle) + 1 + 11 + 1;
  s = malloc(len); if(!s) { free(wPathIcon); free(wBundlePath); Bundle_Error = "unable to allocate memory"; return FALSE; }
  sprintf(s, "%s\\%s", bundle->pathBundle, "desktop.ini");
  wchar_t *wPathIni = WideFromUTF8(s); free(s);

  //mark bundle directory as 'read-only'
  if(SetFileAttributesW(wBundlePath, FILE_ATTRIBUTE_READONLY) == 0)
    { free(wPathIni); free(wPathIcon); free(wBundlePath); Bundle_Error = "unable to edit bundle permissions"; return FALSE; }

  //create desktop.ini file
  HANDLE dIni = CreateFileW(wPathIni, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, NULL);
  free(wPathIni);
  if(dIni == INVALID_HANDLE_VALUE)
    { free(wPathIcon); free(wBundlePath); Bundle_Error = "unable to create bundle desktop.ini file"; return FALSE; }
  
  //create output string
  wchar_t *fixed = L"[.ShellClassInfo]\r\nIconResource=";
  wchar_t *content = malloc(sizeof(wchar_t) * (wcslen(fixed) + wcslen(wPathIcon) + 4));
  wsprintf(content+1, L"%s%s\r\n", fixed, wPathIcon);
  content[0] = 0xFEFF; //windows' "unicode-document" marker
  free(wPathIcon);
  
  //Write Output
  DWORD wrote = 0;
  DWORD toWrite = wcslen(content) * sizeof(wchar_t);
  if(WriteFile(dIni, (void *)content, toWrite, &wrote, NULL) == FALSE)
    { CloseHandle(dIni); free(content); free(wBundlePath); Bundle_Error = "error writing to bundle's desktop.ini file"; return FALSE; }
  if(wrote != toWrite)
    { CloseHandle(dIni); free(content); free(wBundlePath); Bundle_Error = "unable to write full contents to bundle's desktop.ini file"; return FALSE; }
  free(content);
  CloseHandle(dIni);
  
  //Notify Explorer of Directory Customization Change
  SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH , wBundlePath, NULL);
  SHChangeNotify(SHCNE_ATTRIBUTES, SHCNF_PATH , wBundlePath, NULL);
  SHChangeNotify(SHCNE_UPDATEDIR , SHCNF_PATH , wBundlePath, NULL);

  free(wBundlePath);
  Bundle_Error = NULL;
  return TRUE;
}
//----------------------------------------------------------------------------------------------------------------------------------
BOOL Bundle_Launch(bundle *bundle, char **arguments, int argCount)
{
  int i, len; char *s; wchar_t *w;

  //sanity check
  if(bundle->mainExecutable    == NULL) { Bundle_Error = "bundle doesn't specify an executable"; return FALSE; }
  if(bundle->mainExecutable[0] == 0x00) { Bundle_Error = "bundle doesn't specify an executable"; return FALSE; }

  //executable file (relative to bundle)
  len = strlen(bundle->pathBundle) + 1 + strlen(bundle->mainExecutable) + 1;
  s = malloc(len); if(!s) { Bundle_Error = "unable to allocate memory"; return FALSE; }
  sprintf(s, "%s\\%s", bundle->pathBundle, bundle->mainExecutable);
  wchar_t *wExePath = WideFromUTF8(s); free(s);

  //working directory (relative to bundle)
  char *bWorking = (bundle->workingDirectory ? bundle->workingDirectory : "");
  len = strlen(bundle->pathBundle) + 1 + strlen(bWorking) + 1;
  s = malloc(len); if(!s) { free(wExePath); Bundle_Error = "unable to allocate memory"; return FALSE; }
  sprintf(s, "%s\\%s", bundle->pathBundle, bWorking);
  wchar_t *wWorkingPath = WideFromUTF8(s); free(s);

  //full command line length, combined
  len = wcslen(wExePath) + 2;
  if((wExePath[0] != L'"') && (wcschr(wExePath, L' ') != NULL)) len += 2;
  for(i=0; i<argCount; i++)
  {
    len += strlen(arguments[i]) + 1;
    if((arguments[i][0] != '"') && (strchr(arguments[i], ' ') != NULL)) len += 2;
  }
  if(len > 32768) len = 32768; //win32 limits command line to 32k
  wchar_t *wCmdLine = malloc(sizeof(wchar_t) * len);
  if(!wCmdLine) { free(wWorkingPath); free(wExePath); Bundle_Error = "unable to allocate memory"; return FALSE; }
  
  //full command line contents, combined
  int wrote = 0;
  if((wExePath[0] != L'"') && (wcschr(wExePath, L' ') != NULL))
    wrote += swprintf(wCmdLine, L"\"%s\" ", wExePath);
  else
    wrote += swprintf(wCmdLine, L"%s ", wExePath);
  for(i=0; i<argCount; i++)
  {
    w = WideFromUTF8(arguments[i]);
    if((wrote + wcslen(w)) > 32767) break;
    if((w[0] != L'"') && (wcschr(w, L' ') != NULL))
      wrote += swprintf(wCmdLine+wrote, L"\"%s\" ", w);
    else
      wrote += swprintf(wCmdLine+wrote, L"%s ", w);
    free(w);
  }

  //add library directory to PATH environment variable
  if(bundle->libraryDirectory != NULL) if(bundle->libraryDirectory[0] != 0x00)
  {
    len = strlen(bundle->pathBundle) + 1 + strlen(bundle->libraryDirectory) + 1;
    s = malloc(len); if(!s) { free(wCmdLine); free(wWorkingPath); free(wExePath); Bundle_Error = "unable to allocate memory"; return FALSE; }
    sprintf(s, "%s\\%s", bundle->pathBundle, bundle->libraryDirectory);
    w = WideFromUTF8(s); free(s);

    len = GetEnvironmentVariableW(L"PATH", NULL, 0);
    wchar_t *wPath = malloc(sizeof(wchar_t) * (len + wcslen(w) + 2));
    if(!wPath) { free(w); free(wCmdLine); free(wWorkingPath); free(wExePath); Bundle_Error = "unable to allocate memory"; return FALSE; }

    i = swprintf(wPath, L"%s;", w); free(w);
    GetEnvironmentVariableW(L"PATH", wPath+i, len+1);
    SetEnvironmentVariableW(L"PATH", wPath);
    free(wPath);
  }

  //launch process  
  int launchFlags = NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT;
  STARTUPINFO         si; memset(&si, 0, sizeof(STARTUPINFO        )); si.cb = sizeof(STARTUPINFO);
  PROCESS_INFORMATION pi; memset(&pi, 0, sizeof(PROCESS_INFORMATION));
  i = CreateProcessW(wExePath,  // relative or absolute executable file name
                     wCmdLine,  // full command line
                         NULL,  // process security attributes
                         NULL,  // thread security attributes
                        FALSE,  // inherit (file/...) handles
                  launchFlags,  // creation flags
                         NULL,  // evnironment variables (NULL -> inherit)
                 wWorkingPath,  // working directory
                          &si,  // STARTUPINFO crap
                          &pi); // PROCESS_INFORMATION crap

  free(wCmdLine); free(wWorkingPath); free(wExePath);
  Bundle_Error = (i == 0) ? "error creating application process" : NULL;
  return (i != 0);
}
//----------------------------------------------------------------------------------------------------------------------------------
BOOL Bundle_LaunchW(bundle *bundle, wchar_t **arguments, int argCount)
{
  int i, len; char *s; wchar_t *w;

  //sanity check
  if(bundle->mainExecutable    == NULL) { Bundle_Error = "bundle doesn't specify an executable"; return FALSE; }
  if(bundle->mainExecutable[0] == 0x00) { Bundle_Error = "bundle doesn't specify an executable"; return FALSE; }

  //executable file (relative to bundle)
  len = strlen(bundle->pathBundle) + 1 + strlen(bundle->mainExecutable) + 1;
  s = malloc(len); if(!s) { Bundle_Error = "unable to allocate memory"; return FALSE; }
  sprintf(s, "%s\\%s", bundle->pathBundle, bundle->mainExecutable);
  wchar_t *wExePath = WideFromUTF8(s); free(s);

  //working directory (relative to bundle)
  char *bWorking = (bundle->workingDirectory ? bundle->workingDirectory : "");
  len = strlen(bundle->pathBundle) + 1 + strlen(bWorking) + 1;
  s = malloc(len); if(!s) { free(wExePath); Bundle_Error = "unable to allocate memory"; return FALSE; }
  sprintf(s, "%s\\%s", bundle->pathBundle, bWorking);
  wchar_t *wWorkingPath = WideFromUTF8(s); free(s);

  //full command line length, combined
  len = wcslen(wExePath) + 2;
  if((wExePath[0] != L'"') && (wcschr(wExePath, L' ') != NULL)) len += 2;
  for(i=0; i<argCount; i++)
  {
    len += wcslen(arguments[i]) + 1;
    if((arguments[i][0] != L'"') && (wcschr(arguments[i], L' ') != NULL)) len += 2;
  }
  if(len > 32768) len = 32768; //win32 limits command line to 32k
  wchar_t *wCmdLine = malloc(sizeof(wchar_t) * len);
  if(!wCmdLine) { free(wWorkingPath); free(wExePath); Bundle_Error = "unable to allocate memory"; return FALSE; }
  
  //full command line contents, combined
  int wrote = 0;
  if((wExePath[0] != L'"') && (wcschr(wExePath, L' ') != NULL))
    wrote += swprintf(wCmdLine, L"\"%s\" ", wExePath);
  else
    wrote += swprintf(wCmdLine, L"%s ", wExePath);
  for(i=0; i<argCount; i++)
  {
    w = arguments[i];
    if((wrote + wcslen(w)) > 32767) break;
    if((w[0] != L'"') && (wcschr(w, L' ') != NULL))
      wrote += swprintf(wCmdLine+wrote, L"\"%s\" ", w);
    else
      wrote += swprintf(wCmdLine+wrote, L"%s ", w);
  }

  //add library directory to PATH environment variable
  if(bundle->libraryDirectory != NULL) if(bundle->libraryDirectory[0] != 0x00)
  {
    len = strlen(bundle->pathBundle) + 1 + strlen(bundle->libraryDirectory) + 1;
    s = malloc(len); if(!s) { free(wCmdLine); free(wWorkingPath); free(wExePath); Bundle_Error = "unable to allocate memory"; return FALSE; }
    sprintf(s, "%s\\%s", bundle->pathBundle, bundle->libraryDirectory);
    w = WideFromUTF8(s); free(s);

    len = GetEnvironmentVariableW(L"PATH", NULL, 0);
    wchar_t *wPath = malloc(sizeof(wchar_t) * (len + wcslen(w) + 2));
    if(!wPath) { free(w); free(wCmdLine); free(wWorkingPath); free(wExePath); Bundle_Error = "unable to allocate memory"; return FALSE; }

    i = swprintf(wPath, L"%s;", w); free(w);
    GetEnvironmentVariableW(L"PATH", wPath+i, len+1);
    SetEnvironmentVariableW(L"PATH", wPath);
    free(wPath);
  }

  //launch process  
  int launchFlags = NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT;
  STARTUPINFO         si; memset(&si, 0, sizeof(STARTUPINFO        )); si.cb = sizeof(STARTUPINFO);
  PROCESS_INFORMATION pi; memset(&pi, 0, sizeof(PROCESS_INFORMATION));
  i = CreateProcessW(wExePath,  // relative or absolute executable file name
                     wCmdLine,  // full command line
                         NULL,  // process security attributes
                         NULL,  // thread security attributes
                        FALSE,  // inherit (file/...) handles
                  launchFlags,  // creation flags
                         NULL,  // evnironment variables (NULL -> inherit)
                 wWorkingPath,  // working directory
                          &si,  // STARTUPINFO crap
                          &pi); // PROCESS_INFORMATION crap

  free(wCmdLine); free(wWorkingPath); free(wExePath);
  Bundle_Error = (i == 0) ? "error creating application process" : NULL;
  return (i != 0);
}
//----------------------------------------------------------------------------------------------------------------------------------
void Bundle_Free(bundle *bundle)
{
  if(!bundle) return;
  if(bundle->pathBundle)       free(bundle->pathBundle);
  if(bundle->pathXml)          free(bundle->pathXml);
  if(bundle->name)             free(bundle->name);
  if(bundle->icon)             free(bundle->icon);
  if(bundle->version)          free(bundle->version);
  if(bundle->identifier)       free(bundle->identifier);
  if(bundle->mainExecutable)   free(bundle->mainExecutable);
  if(bundle->libraryDirectory) free(bundle->libraryDirectory);
  if(bundle->workingDirectory) free(bundle->workingDirectory);
  if(bundle->other)
  {
    ht_destroy(bundle->other);
    free(bundle->other);
  }
  free(bundle);
}

//==================================================================================================================================
BOOL PathIsDirectory(char *path)
{
  struct _stat fs;
  if(Wide_stat(path, &fs)   != 0) return FALSE;
  if((fs.st_mode & S_IFDIR) == 0) return FALSE;
  return TRUE;
}
//----------------------------------------------------------------------------------------------------------------------------------
BOOL PathIsFile(char *path)
{
  struct _stat fs;
  if(Wide_stat(path, &fs)   != 0) return FALSE;
  if((fs.st_mode & S_IFREG) == 0) return FALSE;
  return TRUE;
}

//==================================================================================================================================
const char *XmlBeautifier(xmlNode *node, int where)
{
  const char *el = mxmlGetElement(node);
  if(el != NULL)
  {
    if(where == MXML_WS_BEFORE_OPEN)
    {
      if(stricmp(el, "bundle") == 0) return "\n";
      if(stricmp(el, "key"   ) == 0) return "\n  ";
      if(stricmp(el, "value" ) == 0) return "\n  ";
    }
    else if(where == MXML_WS_AFTER_CLOSE)
    {
      if(stricmp(el, "bundle") == 0) return "\n";
      if(stricmp(el, "value" ) == 0) return "\n";
    }
  }
  return NULL;
}

//==================================================================================================================================
//----------------------------------------------------------------------------------------------------------------------------------
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
