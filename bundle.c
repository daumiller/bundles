//==================================================================================================================================
// bundle.c
//==================================================================================================================================
#define _BUNDLE_SOURCE_
#define UNICODE
#include <stdio.h>
#include "mxml.h"
#include "bundle.h"

//==================================================================================================================================
typedef mxml_node_t xmlNode;
BOOL IsDirectory(char *path);
BOOL IsFile(char *path);

//==================================================================================================================================
char *Bundle_Error;

//==================================================================================================================================
bundle *Bundle_Read(char *path)
{
  //allocate bundle
  bundle *bndl = calloc(1, sizeof(bundle *));
  if(bndl == NULL) { Bundle_Error = "unable to allocate memory"; return NULL; }

  //build paths
  bndl->pathBundle = strdup(path);                          if(!bndl->pathBundle) { Bundle_Free(bndl); Bundle_Error = "unable to allocate memory"; return NULL; }
  bndl->pathXml = malloc(sizeof(char) * (slen(path) + 12)); if(!bndl->pathXml)    { Bundle_Free(bndl); Bundle_Error = "unable to allocate memory"; return NULL; }
  sprintf(bndl->pathXml, "%s%s\0", bndl->pathBundle, "\\bundle.xml");
  
  //initial filesystem checks
  if(!IsDirectory(bndl->pathBundle)) { Bundle_Free(bndl); Bundle_Error = "specified path is not a directory/bundle"; return NULL; }
  if(!IsFile(bndl->pathXml))         { Bundle_Free(bndl); Bundle_Error = "bundle doesn't contain configuration xml"; return NULL; }

  //load configuration xml
  FILE *fin = fopen(bndl->pathXml, "r"); if(!fp) { Bundle_Free(bndl); Bundle_Error = "unable to open bundle configuration xml"; return NULL; }
  xmlNode *tree = mxmlLoadFile(NULL, fin, MXML_TEXT_CALLBACK);

  //parse file
  xmlNode *curr = tree;
  char *currKey, *currValue;
  while(curr != NULL)
  {
    if(stricmp(mxmlGetElement(curr), "key") == 0)
    {
      currKey = mxmlGetText(curr, NULL);
      curr = mxmlGetNextSibling(curr);
      if(curr != NULL) if(stricmp(mxmlGetElement(curr), "value") == 0)
      {
        currValue = mxmlGetText(curr, NULL);
        Bundle_SetKeyValue(bndl, currKey, currValue);
      }
    }
    curr = mxmlWalkNext(curr, tree, MXML_DESCEND);
  }

  //clean up
  fclose(fin);
  mxmlDelete(tree);

  return(bndl);
}
//----------------------------------------------------------------------------------------------------------------------------------
BOOL *Bundle_Write(bundle *bundle)
{
}
//----------------------------------------------------------------------------------------------------------------------------------
char *Bundle_GetKeyValue(bundle *bundle, char *key)
{
}
//----------------------------------------------------------------------------------------------------------------------------------
char *Bundle_SetKeyValue(bundle *bundle, char *key, char *value)
{
}
//----------------------------------------------------------------------------------------------------------------------------------
void Bundle_ApplyIcon(bundle *bundle)
{
}
//----------------------------------------------------------------------------------------------------------------------------------
void *Bundle_Free(bundle *bundle)
{
  if(!bundle) return;
  if(bundle->pathBundle)       free(bundle->pathBundle);
  if(bundle->pathXml)          free(bundle->pathXml);
  if(bundle->name)             free(bundle->name);
  if(bundle->indentifier)      free(bundle->identifier);
  if(bundle->icon)             free(bundle->icon);
  if(bundle->version)          free(bundle->version);
  if(bundle->launchDirectory)  free(bundle->launchDirectory);
  if(bundle->launchExecutable) free(bundle->launchExecutable);
  for(uint32_t i=0; i<bundle->otherCount; i++)
  {
    free(bundle->otherKey[i]);
    free(bundle->otherValue[i]);
  }
  if(bundle->otherKey)   free(bundle->otherKey);
  if(bundle->otherValue) free(bundle->otherValue);
  free(bundle);
}

//==================================================================================================================================
BOOL IsDirectory(char *path)
{
  struct stat fs;
  if(fstat(path, &fs) != 0)       return FALSE;
  if((fs.st_mode & S_IFDIR) == 0) return FALSE;
  return TRUE;
}
//----------------------------------------------------------------------------------------------------------------------------------
BOOL IsFile(char *path)
{
  struct stat fs;
  if(fstat(path, &fs) != 0)       return FALSE;
  if((fs.st_mode & S_IFREG) == 0) return FALSE;
  return TRUE;
}

//==================================================================================================================================
//----------------------------------------------------------------------------------------------------------------------------------
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
