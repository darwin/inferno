#include "stdafx.h"

#include "hack.h"
#include "defines.h"
#include "unhack.h"

#define STRUCTURES_ONLY 1
#include "raytracer.br"
#include "raytracer.h"

t_voxel* scene_voxels;
t_trilist* scene_trilist;
t_triinfo* scene_triinfos;
t_camera* scene_camera;
t_light* scene_lights;

t_rayinfo* input_rayinfo;
t_raystate* input_raystate;
t_framebufferstate* input_framebufferstate;
t_dispatcherstate* input_dispatcherstate;

t_rayinfo* output_rayinfo;
t_raystate* output_raystate;
t_traverserstate* output_traverserstate;
t_intersectorstate* output_intersectorstate;
t_framebufferstate* output_framebufferstate;
t_dispatcherstate* output_dispatcherstate;

/////////////////////////////////////////////////////////////////////////////////////////////
// helper functions

void fail(char* msg)
{
  printf(msg);
  exit(1);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// load functions

// load globals
int alight = 0;
int atriangle = 1; // 0 is reserved as a null pointer

int load_camera(char* str)
{
  float posx, posy, posz;
  float dirx, diry, dirz;
  float hfov, vfov;
  float rotx, roty, rotz, rotw;
  int res = 
    sscanf(str, "[%f,%f,%f],[%f,%f,%f],%f,%f,(quat %f %f %f %f)\n", 
    &posx, &posy, &posz,
    &dirx, &diry, &dirz,
    &hfov, &vfov,
    &rotx, &roty, &rotz, &rotw 
    );

  if (res!=12) return 1;

  scene_camera->pos = float3(posx, posy, posz);
  scene_camera->dir = float3(-dirx, -diry, -dirz); // MAX camera is facing -dir
  scene_camera->fov = float2(hfov*Wml::Mathf::DEG_TO_RAD, vfov*Wml::Mathf::DEG_TO_RAD);
  scene_camera->quat = float4(rotx, roty, rotz, rotw);

  return 0;
}

int load_light(char* str)
{
  if (alight>=MAXLIGHTS) return 2;

  float posx, posy, posz;
  int colr, colg, colb;
  int res = 
    sscanf(str, "[%f,%f,%f],(color %d %d %d)\n", 
    &posx, &posy, &posz,
    &colr, &colg, &colb
    );

  if (res!=6) return 1;

  scene_lights[alight].pos = float3(posx, posy, posz);
  scene_lights[alight].color = float3(colr/255.f, colg/255.f, colb/255.f);

  alight++;

  return 0;
}

int load_triangle(char* str)
{
  if (atriangle>=NUMTRIINFOS) return 2;

  float v1x, v1y, v1z;
  float v2x, v2y, v2z;
  float v3x, v3y, v3z;
  float n1x, n1y, n1z;
  float n2x, n2y, n2z;
  float n3x, n3y, n3z;
  float mm, mx, my, mz;
  float cr, cb, cg;
  int res = 
    sscanf(str, "[%f,%f,%f],[%f,%f,%f],[%f,%f,%f],[%f,%f,%f],[%f,%f,%f],[%f,%f,%f],(color %f %f %f),(mat %f %f %f %f)\n", 
    &v1x, &v1y, &v1z,
    &v2x, &v2y, &v2z,
    &v3x, &v3y, &v3z,
    &n1x, &n1y, &n1z,
    &n2x, &n2y, &n2z,
    &n3x, &n3y, &n3z,
    &cr, &cg, &cb,
    &mm, &mx, &my, &mz
    );

  if (res!=9+9+3+4) return 1;

  scene_triinfos[atriangle].v0 = float3(v1x, v1y, v1z);
  scene_triinfos[atriangle].v1 = float3(v2x, v2y, v2z);
  scene_triinfos[atriangle].v2 = float3(v3x, v3y, v3z);
  scene_triinfos[atriangle].n0 = float3(n1x, n1y, n1z);
  scene_triinfos[atriangle].n1 = float3(n2x, n2y, n2z);
  scene_triinfos[atriangle].n2 = float3(n3x, n3y, n3z);
  scene_triinfos[atriangle].color = float3(cr/255.0f, cg/255.0f, cb/255.0f);
  scene_triinfos[atriangle].mat = float4(mm, mx, my, (1.0f/mz));
  atriangle++;

  return 0;
}

int parse_line(FILE* f)
{
  char buf[64*1024];

  if (feof(f)) return 1;
  fgets(buf, 64*1024, f);

  if (strlen(buf)<2) return 1;
  
  int res;
  switch (buf[0]) {
    case 'c': res = load_camera(buf+2); break;
    case 'l': res = load_light(buf+2); break;
    case 't': res = load_triangle(buf+2); break;
    default: res = 0;
  }

  return res;
}

int load_scene(const char* filename, int* tris)
{
  FILE* f;
  int res;

  alight = 0;
  atriangle = 1;

  f = fopen(filename, "rt");

  if (f)
  {
    while (!feof(f))
    {
      res = parse_line(f);  
      if (res) return res;
    }
    fclose(f);
    *tris = atriangle;
    return 0;
  }
  return 1;
}

int alloc_scene()
{
  int i;
  int voxel_size = VOXELSX*VOXELSY*VOXELSZ*sizeof(t_voxel);
  int trilist_size = NUMTRILIST * sizeof(t_trilist);
  int triinfos_size = NUMTRIINFOS * sizeof(t_triinfo);

  // alloc voxels
  scene_voxels = (t_voxel*)malloc(voxel_size);
  for (i=0; i<VOXELSX*VOXELSY*VOXELSZ; i++)
  {
    scene_voxels[i].trilist = 0.0;
  }

  // alloc trilist
  scene_trilist = (t_trilist*)malloc(trilist_size);
  for (i=0; i<NUMTRILIST; i++)
  {
    scene_trilist[i].triid = 0.0;
  }

  // alloc triinfos
  scene_triinfos = (t_triinfo*)malloc(triinfos_size);
  for (i=0; i<NUMTRIINFOS; i++)
  {
    scene_triinfos[i].v0 = float3(0.0, 0.0, 0.0);
    scene_triinfos[i].v1 = float3(0.0, 0.0, 0.0);
    scene_triinfos[i].v2 = float3(0.0, 0.0, 0.0);
    scene_triinfos[i].n0 = float3(0.0, 0.0, 1.0);
    scene_triinfos[i].n1 = float3(0.0, 0.0, 1.0);
    scene_triinfos[i].n2 = float3(0.0, 0.0, 1.0);
    scene_triinfos[i].color = float3(0.0, 0.0, 0.0);
    scene_triinfos[i].mat = float4(1.0, 0.0, 0.0, 0.0);
  }

  scene_camera = (t_camera*)malloc(sizeof(t_camera));

  scene_lights = (t_light*)malloc(MAXLIGHTS*sizeof(t_light));
  for (i=0; i<NUMLIGHTS; i++)
  {
    scene_lights[i].pos = float3(0.0, 0.0, 0.0);
    scene_lights[i].color = float3(0.0, 0.0, 0.0);
  }

  return 0;
}

int testhit_trianglevoxel(int triid, int vx, int vy, int vz)
{
  float boxcenter[3];
  float boxhalfsize[3];
  float triverts[3][3];
  
  boxcenter[0] = vx + (float)VOXELSIZEX/2;
  boxcenter[1] = vy + (float)VOXELSIZEY/2;
  boxcenter[2] = vz + (float)VOXELSIZEZ/2;

  boxhalfsize[0] = (float)VOXELSIZEX/2;
  boxhalfsize[1] = (float)VOXELSIZEY/2;
  boxhalfsize[2] = (float)VOXELSIZEZ/2;

  triverts[0][0] = scene_triinfos[triid].v0.x;
  triverts[0][1] = scene_triinfos[triid].v0.y;
  triverts[0][2] = scene_triinfos[triid].v0.z;

  triverts[1][0] = scene_triinfos[triid].v1.x;
  triverts[1][1] = scene_triinfos[triid].v1.y;
  triverts[1][2] = scene_triinfos[triid].v1.z;

  triverts[2][0] = scene_triinfos[triid].v2.x;
  triverts[2][1] = scene_triinfos[triid].v2.y;
  triverts[2][2] = scene_triinfos[triid].v2.z;

  return triBoxOverlap(boxcenter, boxhalfsize, triverts);
}


int build_scene(int tris)
{
  // example
/*  
  // fill in some scene data
  // note: scene_triinfos[0] is reserved as a null pointer
  scene_triinfos[1].va = float3(3.0, 3.0, 3.5);
  scene_triinfos[1].vb = float3(3.5, 3.5, 3.5);
  scene_triinfos[1].vc = float3(3.0, 3.9, 3.5);
  scene_triinfos[1].color = float3(0.0, 1.0, 0.0);

  // note: scene_trilist[0] is reserved as a null pointer
  scene_trilist[1].triid = 1;
  scene_trilist[2].triid = 0; // end marker

  scene_voxels[VOXELADDR(3,3,3)].trilist = 1;
*/

  int vx,vy,vz;
  int triid;
  int listpos = 1;  // first position is reserved as a null pointer
  int somehits = 0;
  int i;

  // go trough all voxels
  for (vz=0; vz<VOXELSZ; vz++)
  {
    for (vy=0; vy<VOXELSY; vy++)
    {
      for (vx=0; vx<VOXELSX; vx++)
      {
        int savelistpos = listpos;
        triid = 1;
        somehits = 0;
        while (triid<tris)
        {
          int hit = testhit_trianglevoxel(triid, vx, vy, vz);
          if (hit)
          {
            somehits = 1;
            scene_trilist[listpos].triid = (float)triid;
            listpos++;
            if (listpos>=NUMTRILIST) fail("trilist short !");
          }
          triid++;
        }

        if (somehits)
        {
          // add end marker to the list
          scene_trilist[listpos].triid = (float)0;
          listpos++;
          if (listpos>=NUMTRILIST) fail("trilist short !");

          // set starting position for voxel's list of triangles
          scene_voxels[VOXELADDR(vx,vy,vz)].trilist = (float)savelistpos;

          // debug:

          printf("voxel [%d,%d,%d]=%d:", vx, vy, vz, (int)savelistpos);
          i = savelistpos;
          while (scene_trilist[i].triid)
          {
            printf("%03.0f,", scene_trilist[i].triid);
            i++;
          }
          printf("\n");

        }
        else
        {
          // set null pointer as there are no tris in voxel
          scene_voxels[VOXELADDR(vx,vy,vz)].trilist = 0;
        }
      }
    }
  }

  return 0;
}

int init_engine()
{
  int x,y;
  int rays_size = RAYSX * RAYSY * sizeof(t_rayinfo);
  int ray_size = RAYSX * RAYSY * sizeof(t_raystate);
  int traverser_size = RAYSX * RAYSY * sizeof(t_traverserstate);
  int intersector_size = RAYSX * RAYSY * sizeof(t_intersectorstate);
  int framebuffer_size = RAYSX * RAYSY * sizeof(t_framebufferstate);
  int dispatcher_size = RAYSX * RAYSY * sizeof(t_dispatcherstate);
  Vector3f cdir, cup, cright;

  // alloc output_rayinfo
  output_rayinfo = (t_rayinfo*)malloc(rays_size);

  // alloc input_rayinfo
  input_rayinfo = (t_rayinfo*)malloc(rays_size);


  cdir = Vector3f(scene_camera->dir.x, scene_camera->dir.y, scene_camera->dir.z);
  cup = Vector3f(0,0,1);
  cright = cdir.Cross(cup);

  Matrix3f hrot;
  Matrix3f vrot;

  float hfov = scene_camera->fov.x;
  float vfov = scene_camera->fov.y;

/*
  Matrix3f hrotpos;
  Matrix3f hrotneg;
  Matrix3f vrotpos;
  Matrix3f vrotneg;
  
  hrotpos.FromAxisAngle(cup, Mathf::DEG_TO_RAD*hfov);
  hrotneg.FromAxisAngle(cup, Mathf::DEG_TO_RAD*-hfov);
  vrotpos.FromAxisAngle(cright, Mathf::DEG_TO_RAD*vfov);
  vrotneg.FromAxisAngle(cright, Mathf::DEG_TO_RAD*-vfov);

  Vector3f ul = hrotneg*vrotneg*cdir;
  Vector3f ur = hrotpos*vrotneg*cdir;
  Vector3f ll = hrotneg*vrotpos*cdir;
  Vector3f lr = hrotpos*vrotpos*cdir;
*/

  float front = 5;
  float w = (float)(tan(hfov*0.5)*front);
  float h = (float)(tan(vfov*0.5)*front);

  Vector3f tv(0, 0, -front); 

  Vector3f ul(w, -h, -front);
  Vector3f ur(-w, -h, -front);
  Vector3f ll(w, h, -front);
  Vector3f lr(-w, h, -front);

  Quaternionf q(-scene_camera->quat.w, scene_camera->quat.x, scene_camera->quat.y, scene_camera->quat.z);
  tv = q * tv;
  ul = q * ul;
  ur = q * ur;
  ll = q * ll;
  lr = q * lr;

  Vector3f tul = Vector3f(scene_camera->pos.x, scene_camera->pos.y, scene_camera->pos.z) + ul;;
  Vector3f tur = Vector3f(scene_camera->pos.x, scene_camera->pos.y, scene_camera->pos.z) + ur;;
  Vector3f tll = Vector3f(scene_camera->pos.x, scene_camera->pos.y, scene_camera->pos.z) + ll;;
  Vector3f tlr = Vector3f(scene_camera->pos.x, scene_camera->pos.y, scene_camera->pos.z) + lr;;

  for (y=0; y<RAYSY; y++)
  {
    for (x=0; x<RAYSX; x++)
    {
//      input_rayinfo[RAYADDR(x,y)].origin = float3((float)(((x+0.5)/(float)RAYSX)*VOXELSIZEX*VOXELSX), (float)(((y+0.5)/(float)RAYSY)*VOXELSIZEY*VOXELSY), -25.0);
//      input_rayinfo[RAYADDR(x,y)].direction = float3(0.0, 0.0, 1); // NOTE: avoid division by zero ?
/*
      // interpolation of angles
      float xc = -((float)(x / (float)RAYSX) - 0.5f) * 2.0f; // range -1 .. 1
      float yc = -((float)(y / (float)RAYSY) - 0.5f) * 2.0f; // range -1 .. 1

      hrot.FromAxisAngle(cup, Mathf::DEG_TO_RAD*hfov*xc);
      vrot.FromAxisAngle(cright, Mathf::DEG_TO_RAD*vfov*yc);

      Vector3f ray = hrot*vrot*cdir;
      ray.Normalize();
*/
//      printf("[%d,%d] %02.2f, %02.2f, %02.2f\n", x,y, ray.X(), ray.Y(), ray.Z());

      // interpolation of points
      float xc = ((float)(x / (float)RAYSX)); // range 0 .. 1
      float yc = ((float)(y / (float)RAYSY)); // range 0 .. 1

      Vector3f up = xc*ul + (1-xc)*ur;
      Vector3f lp = xc*ll + (1-xc)*lr;

      Vector3f ray = yc*up + (1-yc)*lp;
      ray.Normalize();
//      printf("[%d,%d] %02.2f, %02.2f, %02.2f\n", x,y, ray.X(), ray.Y(), ray.Z());

      input_rayinfo[RAYADDR(x,y)].info = float4((float)x,(float)y, (float)CG_FLT_MAX, 1.0f); 
      input_rayinfo[RAYADDR(x,y)].origin = scene_camera->pos;
      input_rayinfo[RAYADDR(x,y)].direction = float3(ray.X(), ray.Y(), ray.Z());
    }
  }

  // alloc output_raystate
  output_raystate = (t_raystate*)malloc(ray_size);
  for (y=0; y<RAYSY; y++)
  {
    for (x=0; x<RAYSX; x++)
    {
      // output_raystate[RAYADDR(x,y)].state = float3(RAYSTATE_TRAVERSING, 0, 0);
    }
  }

  // alloc output_traverserstate
  output_traverserstate = (t_traverserstate*)malloc(traverser_size);
  for (y=0; y<RAYSY; y++)
  {
    for (x=0; x<RAYSX; x++)
    {
      // output_traverserstate[RAYADDR(x,y)].voxel = float3(0, 0, 0);
      // output_traverserstate[RAYADDR(x,y)].tmax = float3(0, 0, 0);
    }
  }
  
  // alloc output_intersectorstate
  output_intersectorstate = (t_intersectorstate*)malloc(intersector_size);
  for (y=0; y<RAYSY; y++)
  {
    for (x=0; x<RAYSX; x++)
    {
      // output_intersectorstate[RAYADDR(x,y)].tripos = float3(0, 0, 0);
    }
  }

  // alloc output_framebufferstate
  output_framebufferstate = (t_framebufferstate*)malloc(framebuffer_size);

  // alloc input_framebufferstate
  input_framebufferstate = (t_framebufferstate*)malloc(framebuffer_size);
  for (y=0; y<RAYSY; y++)
  {
    for (x=0; x<RAYSX; x++)
    {
      input_framebufferstate[RAYADDR(x,y)].color = float3(0, 0, 0); // init framebufer to black
    }
  }

  // alloc output_dispatcherstate
  output_dispatcherstate = (t_dispatcherstate*)malloc(dispatcher_size);

  // alloc input_dispatcherstate
  input_dispatcherstate = (t_dispatcherstate*)malloc(dispatcher_size);
  for (y=0; y<RAYSY; y++)
  {
    for (x=0; x<RAYSX; x++)
    {
      input_dispatcherstate[RAYADDR(x,y)].phase = float3(0, 0, 0); // samplephase, shadowphase, ?
      input_dispatcherstate[RAYADDR(x,y)].hitinfo = float4(-1, 0, 0, 0); // raystatus, u, v, weight
      input_dispatcherstate[RAYADDR(x,y)].hitdirection = input_rayinfo[RAYADDR(x,y)].direction;
    }
  }

  // alloc input_raystate
  input_raystate = (t_raystate*)malloc(ray_size);
  for (y=0; y<RAYSY; y++)
  {
    for (x=0; x<RAYSX; x++)
    {
      input_raystate[RAYADDR(x,y)].state = float4(RAYSTATE_SETUP, 0, 0, 0); // SETUP is important here
    }
  }


  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// clean functions

int clean_scene()
{
  free(scene_voxels);
  free(scene_trilist);
  free(scene_triinfos);
  free(scene_camera);
  free(scene_lights);
  return 0;
}

int clean_engine()
{
  free(input_rayinfo);
  free(input_raystate);
  free(input_framebufferstate);
  free(input_dispatcherstate);

  free(output_rayinfo);
  free(output_raystate);
  free(output_traverserstate);
  free(output_intersectorstate);
  free(output_dispatcherstate);
  free(output_framebufferstate);
  return 0;
}
