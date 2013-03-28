#include <windows.h>
#include <stdio.h>
#include "bundle.h"

int main(int argc, char **argv)
{
  bundle *app = Bundle_Read("C:\\MinGW\\msys\\1.0\\home\\dillona\\bundles\\test.app");
  printf("pathBundle       : %s\n", app->pathBundle);
  printf("pathXml          : %s\n", app->pathXml);
  printf("Name             : %s\n", app->name);
  printf("Icon             : %s\n", app->icon);
  printf("Version          : %s\n", app->version);
  printf("Identifier       : %s\n", app->identifier);
  printf("LaunchDirectory  : %s\n", app->launchDirectory);
  printf("LaunchExecutable : %s\n", app->launchExecutable);

  printf("----------------------------------------------------------------------\n");

  int count;
  char **keys = Bundle_ListOtherKeys(app, &count);
  for(int i=0; i<count; i++)
    printf("\"%s\" : \"%s\"\n", keys[i], Bundle_GetKeyValue(app, keys[i]));

  Bundle_Write(app);

  Bundle_Free(app);
  return 0;
}
