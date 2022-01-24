#include "pixor.h"

unsigned int Pixor::red(RGBA c)
{
  return (0x000000ff & c) >> 0;
}

unsigned int Pixor::green(RGBA c)
{
  return (0x0000ff00 & c) >> 8;
}

unsigned int Pixor::blue(RGBA c)
{
  return (0x00ff0000 & c) >> 16;
}

unsigned int Pixor::alpha(RGBA c)
{
  return (0xff000000 & c) >> 24;
}

RGBA Pixor::rgba(unsigned int r, unsigned int g, unsigned int b, unsigned int a)
{
  return (r << 0) | (g << 8) | (b << 16) | (a << 24);
}
