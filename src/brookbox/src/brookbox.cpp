// brookbox.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int raytracer(const char*);

int _tmain(int argc, _TCHAR* argv[])
{
  if (argc==2)
    raytracer(argv[1]);
  else
    printf("Usage: brookbox.exe <scene.dat>");
	return 0;
}

