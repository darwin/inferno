#include "stdafx.h"
#include "hack.h"
#include "defines.h"
#define STRUCTURES_ONLY 1
#include "raytracer.br"
#include "raytracer.h"
#include "scene.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// output printers

void print_traverser(int mode)
{
  int x, y;

  printf("traverser: voxels\n");
  for (y=0; y<RAYSY; y++) 
  {
    for(x=0; x<RAYSX; x++)
    {
      printf( "%02.0fx%02.0fx%02.0f|", output_traverserstate[RAYADDR(x,y)].voxel.y, output_traverserstate[RAYADDR(x,y)].voxel.y, output_traverserstate[RAYADDR(x,y)].voxel.z);
    }
    printf("\n");
  }
/*
  printf("traverser: tmax\n");
  for (y=0; y<RAYSY; y++) 
  {
    for(x=0; x<RAYSX; x++)
    {
      printf( "%02.2fx%02.2fx%02.2f|", output_traverserstate[RAYADDR(x,y)].tmax.x, output_traverserstate[RAYADDR(x,y)].tmax.y, output_traverserstate[RAYADDR(x,y)].tmax.z);
    }
    printf("\n");
  }
*/
}

void print_ray(int mode)
{
  int x, y;

  // print result
  printf("rays: state\n");
  for (y=0; y<RAYSY; y++) 
  {
    for(x=0; x<RAYSX; x++)
    {
      printf( "%02.0f|", output_raystate[RAYADDR(x,y)].state.x);
    }
    printf("\n");
  }
}

void print_intersector(int mode)
{
  int x, y;

  printf("intersector: trilist pointer\n");
  for (y=0; y<RAYSY; y++) 
  {
    for(x=0; x<RAYSX; x++)
    {
      printf( "%02.0f|", output_intersectorstate[RAYADDR(x,y)].tripos.x);
    }
    printf("\n");
  }
}

void print_trilist(int mode)
{
  int i;

  // print result
  printf("trilist:\n");
  for (i=0; i<NUMTRILIST; i++) 
  {
    printf( "%03.0f|", scene_trilist[i].triid);
  }
  printf("\n");
}

void display_framebuffer()
{
  imdebug("rgb b=32f w=%d h=%d %p", RAYSX, RAYSY, output_framebufferstate);
}

void display_raystate()
{
  imdebug("rgba b=32f w=%d h=%d %p", RAYSX, RAYSY, output_raystate);
}

void display_rayinfo()
{
  int size = (int)(sizeof(t_rayinfo)/sizeof(float));
  char buf[65536];
  _snprintf(buf, 65536, "#%d rgb=#789 b=32f w=%%d h=%%d %%p", size);
  imdebug(buf, RAYSX, RAYSY, output_rayinfo);
  _snprintf(buf, 65536, "#%d rgb=#456 b=32f w=%%d h=%%d %%p", size);
  imdebug(buf, RAYSX, RAYSY, output_rayinfo);
}

void display_dispatcherstate()
{
  int size = (int)(sizeof(t_dispatcherstate)/sizeof(float));
  char buf[65536];
  _snprintf(buf, 65536, "#%d rgb_=#013 b=32f w=%%d h=%%d %%p", size);
  imdebug(buf, RAYSX, RAYSY, output_dispatcherstate);
}
