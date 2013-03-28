//==================================================================================================================================
// bundle.c
//==================================================================================================================================
#define _BUNDLE_SOURCE_ //<-- for bundle.h (Bundle_Error)
#undef  __STRICT_ANSI__ //<-- for string.h (MinGW: stricmp, strdup)
#define UNICODE         //<-- for windows.h
#include <windows.h>
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
  bundle *bndl = calloc(1, sizeof(bundle));
  if(bndl == NULL) { Bundle_Error = "unable to allocate memory"; return NULL; }
  bndl->other = malloc(sizeof(hash_table));
  if(!bndl->other) { free(bndl); Bundle_Error = "unable to allocate memory"; return NULL; }
  ht_init(bndl->other, HT_NONE, 0.05);

  //build paths
  bndl->pathBundle = strdup(path);                          if(!bndl->pathBundle) { Bundle_Free(bndl); Bundle_Error = "unable to allocate memory"; return NULL; }
  bndl->pathXml = malloc(sizeof(char) * (strlen(path)+12)); if(!bndl->pathXml)    { Bundle_Free(bndl); Bundle_Error = "unable to allocate memory"; return NULL; }
  sprintf(bndl->pathXml, "%s%s\0", bndl->pathBundle, "/bundle.xml");
  
  //initial filesystem checks
  if(!PathIsDirectory(bndl->pathBundle)) { Bundle_Free(bndl); Bundle_Error = "specified path is not a directory/bundle"; return NULL; }
  if(!PathIsFile(bndl->pathXml))         { Bundle_Free(bndl); Bundle_Error = "bundle doesn't contain configuration xml"; return NULL; }

  //load configuration xml
  FILE *fin = Wide_fopen(bndl->pathXml, "r"); if(!fin) { Bundle_Free(bndl); Bundle_Error = "unable to open bundle configuration xml"; return NULL; }
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
          Bundle_SetKeyValue(bndl, currKey, currValue);
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
  return(bndl);
}
//----------------------------------------------------------------------------------------------------------------------------------
BOOL Bundle_Write(bundle *bundle)
{
  xmlNode *xml, *root, *node;
  xml  = mxmlNewXML("1.0");
  root = mxmlNewElement(xml, "bundle");

  //create standard KVPs
  node=mxmlNewElement(root,"key"); mxmlNewText(node,0,"Name"            ); node=mxmlNewElement(root, "value"); if(bundle->name            ) mxmlNewText(node,0,bundle->name            );
  node=mxmlNewElement(root,"key"); mxmlNewText(node,0,"Icon"            ); node=mxmlNewElement(root, "value"); if(bundle->icon            ) mxmlNewText(node,0,bundle->icon            );
  node=mxmlNewElement(root,"key"); mxmlNewText(node,0,"Version"         ); node=mxmlNewElement(root, "value"); if(bundle->version         ) mxmlNewText(node,0,bundle->version         );
  node=mxmlNewElement(root,"key"); mxmlNewText(node,0,"Identifier"      ); node=mxmlNewElement(root, "value"); if(bundle->identifier      ) mxmlNewText(node,0,bundle->identifier      );
  node=mxmlNewElement(root,"key"); mxmlNewText(node,0,"LaunchDirectory" ); node=mxmlNewElement(root, "value"); if(bundle->launchDirectory ) mxmlNewText(node,0,bundle->launchDirectory );
  node=mxmlNewElement(root,"key"); mxmlNewText(node,0,"LaunchExecutable"); node=mxmlNewElement(root, "value"); if(bundle->launchExecutable) mxmlNewText(node,0,bundle->launchExecutable);

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
  else if(keylen ==  7) { if(stricmp(key, "Version"         ) == 0) return bundle->version;          }
  else if(keylen == 10) { if(stricmp(key, "Identifier"      ) == 0) return bundle->identifier;       }
  else if(keylen == 15) { if(stricmp(key, "LaunchDirectory" ) == 0) return bundle->launchDirectory;  }
  else if(keylen == 16) { if(stricmp(key, "LaunchExecutable") == 0) return bundle->launchExecutable; }

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
  else if(keylen ==  7) { if(stricmp(key, "Version"         ) == 0) dest = &(bundle->version);          }
  else if(keylen == 10) { if(stricmp(key, "Identifier"      ) == 0) dest = &(bundle->identifier);       }
  else if(keylen == 15) { if(stricmp(key, "LaunchDirectory" ) == 0) dest = &(bundle->launchDirectory);  }
  else if(keylen == 16) { if(stricmp(key, "LaunchExecutable") == 0) dest = &(bundle->launchExecutable); }
  if(dest)
  {
    if(*dest) free(*dest);
    *dest = value ? strdup(value) : "";
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
void Bundle_ApplyIcon(bundle *bundle)
{
}
//----------------------------------------------------------------------------------------------------------------------------------
void Bundle_Launch(bundle *bundle, char **arguments, int argCount)
{
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
  if(bundle->launchDirectory)  free(bundle->launchDirectory);
  if(bundle->launchExecutable) free(bundle->launchExecutable);
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
  if(Wide_stat(path, &fs) != 0)       return FALSE;
  if((fs.st_mode & S_IFDIR) == 0) return FALSE;
  return TRUE;
}
//----------------------------------------------------------------------------------------------------------------------------------
BOOL PathIsFile(char *path)
{
  struct _stat fs;
  if(Wide_stat(path, &fs) != 0)       return FALSE;
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
