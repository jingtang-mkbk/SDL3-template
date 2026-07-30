#include "platform.h"

void getWithHeightFromPlatform(int *width, int *height)
{
#ifdef __ANDROID__
  *width = 392;
  *height = 872;
#else
  *width = 800;
  *height = 600;
#endif
}