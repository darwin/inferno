#define NUMSAMPLES_NOHACK 3
#define NUMSAMPLES BROOKHACK_F1(NUMSAMPLES_NOHACK)  // primary, secondary reflection, secondary refraction, ...

#define NUMLIGHTS_NOHACK 2
#define NUMLIGHTS BROOKHACK_F1(NUMLIGHTS_NOHACK)

#define MAXLIGHTS BROOKHACK_F1(8)

#define COLOR_RAYOUT BROOKHACK_F3(float3(0.0, 0.0, 0.0));
#define COLOR_RAYMISSED BROOKHACK_F3(float3(0.2, 0.2, 0.2));

#define SPECULAR_EXPONENT BROOKHACK_F1(32)

#define VOXELSX BROOKHACK_F1(10)
#define VOXELSY BROOKHACK_F1(10)
#define VOXELSZ BROOKHACK_F1(10)

//#define NUMVOXELS ((float)VOXELSX*(float)VOXELSY*(float)VOXELSZ)
#define NUMVOXELS 1000

// voxel size must be [1.0x1.0x1.0] - optimization reasons, traverser code is not general enough
#define VOXELSIZEX BROOKHACK_F1(1.0)
#define VOXELSIZEY BROOKHACK_F1(1.0)
#define VOXELSIZEZ BROOKHACK_F1(1.0)

#define NUMTRILIST 10*2048
#define NUMTRIINFOS 10*2048
//#define NUMTRILIST 2048
//#define NUMTRIINFOS 2048

//#define RAYS 1024
//#define RAYS 512
//#define RAYS 256
#define RAYS 128
//#define RAYS 64
//#define RAYS 32
//#define RAYS 16

#define RAYSX (RAYS)
#define RAYSY (RAYS)

#define VOXELADDR(x,y,z) (((VOXELSX*VOXELSY)*z+(VOXELSX)*y+x))
#define RAYADDR(x,y) (RAYSX*y+x)

#define CG_FLT_MAX			BROOKHACK_F1(99999999.0)
#define CG_FLT_EPSILON		BROOKHACK_F1(0.01)

#define PI				BROOKHACK_F1(3.141592654)
#define HALF_PI			BROOKHACK_F1(1.570796327)

#define KD			BROOKHACK_F1(2.0)
#define KS			BROOKHACK_F1(1.0)

#define FPHAZARD_REFLECTEDRAYSHIFT BROOKHACK_F1(0.01)
#define FPHAZARD_REFRACTEDRAYSHIFT BROOKHACK_F1(0.01)
#define FPHAZARD_ENTERINGRAYSHIFT  BROOKHACK_F1(0.01)
#define FPHAZARD_SHADOWRAYSHIFT    BROOKHACK_F1(0.01)

#define ALMOST_NOLIGHT BROOKHACK_F1(0.1)

#define RAYSTATE_MISSED            BROOKHACK_F1(0)
#define RAYSTATE_OUT               BROOKHACK_F1(1)
#define RAYSTATE_DONE              BROOKHACK_F1(2)
#define RAYSTATE_TRAVERSING        BROOKHACK_F1(3)
#define RAYSTATE_INTERSECTING      BROOKHACK_F1(4)
#define RAYSTATE_DISPATCHING       BROOKHACK_F1(5)
#define RAYSTATE_SHADING           BROOKHACK_F1(6)
#define RAYSTATE_SETUP             BROOKHACK_F1(7)

#define RAYSTATUS_MISSED           BROOKHACK_F1(0)
#define RAYSTATUS_OUT              BROOKHACK_F1(-1)

#define VLENGTH(v) (sqrt(v.x*v.x+v.y*v.y+v.z*v.z))
#define VNORMALIZE(v) (float3(v.x/VLENGTH(v), v.y/VLENGTH(v), v.z/VLENGTH(v)))

#define VCROSS(v1,v2) \
  float3(v1.y*v2.z-v1.z*v2.y, \
  v1.z*v2.x-v1.x*v2.z, \
  v1.x*v2.y-v1.y*v2.x) 
