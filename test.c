#include <stdio.h>
#include "bundle.h"

int main(int argc, char **argv)
{
  //------------------------------------------------------------------------------------------
  bundle *app = Bundle_Read("C:\\MinGW\\msys\\1.0\\home\\dillona\\bundles\\チャットモンチー.app");
  printf("pathBundle       : %s\n", app->pathBundle);
  printf("pathXml          : %s\n", app->pathXml);
  printf("Name             : %s\n", app->name);
  printf("Icon             : %s\n", app->icon);
  printf("Version          : %s\n", app->version);
  printf("Identifier       : %s\n", app->identifier);
  printf("MainExecutable   : %s\n", app->mainExecutable);
  printf("LibraryDirectory : %s\n", app->libraryDirectory);
  printf("WorkingDirectory : %s\n", app->workingDirectory);

  int count;
  char **keys = Bundle_ListOtherKeys(app, &count);
  for(int i=0; i<count; i++)
    printf("\"%s\" : \"%s\"\n", keys[i], Bundle_GetKeyValue(app, keys[i]));

  Bundle_Write(app);
  Bundle_Free(app);

  //------------------------------------------------------------------------------------------
  printf("\n------------------------------------------------------------------------\n\n");
  BOOL result;
  app = Bundle_Read("C:\\MinGW\\msys\\1.0\\home\\dillona\\bundles\\PwrIDE.app");
  result = Bundle_Launch(app, NULL, 0);
  printf("Bundle_Launch    : %s\n", result ? "TRUE" : Bundle_Error);
  result = Bundle_ApplyIcon(app);
  printf("Bundle_ApplyIcon : %s\n", result ? "TRUE" : Bundle_Error);
  Bundle_Free(app);

  //------------------------------------------------------------------------------------------
  return 0;
}
