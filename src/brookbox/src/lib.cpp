#include "stdafx.h"

#include "hack.h"
#include "defines.h"
#include "unhack.h"

bool all(float3 v)
{
  return (v.x && v.y && v.z);
}

float dot(float3 v1, float3 v2)
{
  return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

float3 cross(float3 v1, float3 v2)
{
  float3 res;
  res.x = v1.y*v2.z-v1.z*v2.y;
  res.y = v1.z*v2.x-v1.x*v2.z;
  res.z = v1.x*v2.y-v1.y*v2.x; 
  return res;
}


bool __all_cpu_inner(float3 v)
{
  return (v.x && v.y && v.z);
}

float4 lit(float diffuse, float specular, float n)
{
  float4 result;
  result.x = 1.0;  
  result.y = max(diffuse, 0);
  result.z = (result.y > 0.0)?(float)pow(specular, n):0.0f;
  result.w = 1.0;
  return result;
}


float3 voxel_addr(float3 p)
{
  float3 res;
  res.x = (float)floor((float)p.x / VOXELSIZEX);
  res.y = (float)floor((float)p.y / VOXELSIZEY);
  res.z = (float)floor((float)p.z / VOXELSIZEZ);
  return res;
}
     
bool point_in_box(float3 boxa, float3 boxb, float3 p) 
{
  float3 res;
  res.x = (float)(boxa.x < p.x && p.x < boxb.x);
  res.y = (float)(boxa.y < p.y && p.y < boxb.y);
  res.z = (float)(boxa.z < p.z && p.z < boxb.z);
  return __all_cpu_inner(res);
}

// returns (intersect, tmin, tmax)
float3 intersect_box(float3 boxa, float3 boxb, float3 ro, float3 rd) 
{
  float tmin, tmax, tymin, tymax, tzmin, tzmax;

  bool res = true;

  float3 ird(1/(float)rd.x, 1/(float)rd.y, 1/(float)rd.z);

  if ((float)ird.x >= 0) 
  {
    tmin = (boxa.x - ro.x) * ird.x;
    tmax = (boxb.x - ro.x) * ird.x;
  }
  else 
  {
    tmin = (boxb.x - ro.x) * ird.x;
    tmax = (boxa.x - ro.x) * ird.x;
  }
  if ((float)ird.y >= 0) 
  {
    tymin = (boxa.y - ro.y) * ird.y;
    tymax = (boxb.y - ro.y) * ird.y;
  }
  else 
  {
    tymin = (boxb.y - ro.y) * ird.y;
    tymax = (boxa.y - ro.y) * ird.y;
  }
  if ( (tmin > tymax) || (tymin > tmax) ) res = false;
  if (tymin > tmin) tmin = tymin;
  if (tymax < tmax) tmax = tymax;
  if ((float)ird.z >= 0) 
  {
    tzmin = (boxa.z - ro.z) * ird.z;
    tzmax = (boxb.z - ro.z) * ird.z;
  }
  else 
  {
    tzmin = (boxb.z - ro.z) * ird.z;
    tzmax = (boxa.z - ro.z) * ird.z;
  }
  if ( (tmin > tzmax) || (tzmin > tmax) ) res = false;
  if (tzmin > tmin) tmin = tzmin;
  if (tzmax < tmax) tmax = tzmax;

  if (tmax<0) res = false;
  if (tmin<0) tmin = 0;

  if (res)
  {
    return float3((float)RAYSTATE_TRAVERSING, tmin, tmax);
  }
  else
    return float3((float)RAYSTATE_SHADING, tmin, tmax);
}

// intersect trinagle 2-sided
float4 intersect_triangle_2(float3 rorig, float3 rdir, float3 v0, float3 v1, float3 v2)
{
  float3 edge1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
  float3 edge2(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);
  float3 pvec = cross(rdir, edge2);
  float det = dot(edge1, pvec);
  float inv_det = 1.0f/det;
  float3 tvec(rorig.x - v0.x, rorig.y- v0.y, rorig.z - v0.z);
  float u = dot(tvec, pvec) * inv_det;
  float3 qvec = cross(tvec, edge1);
  float v = dot(rdir, qvec) * inv_det;
  float t = dot(edge2, qvec) * inv_det;
  
  bool hit = (u>=0)?true:false;
  hit = (v>=0)?hit:false;
  hit = (u+v<=1)?hit:false;
  hit = (t >= 0)?hit:false;

  float b = (hit)?1.0f:0.0f;
  return float4(t, u, v, b);
}

// intersect triangle 1-sided
//! precision problems
float4 intersect_triangle(float3 rorig, float3 rdir, float3 v0, float3 v1, float3 v2)
{
  float3 edge1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
  float3 edge2(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);
  float3 pvec = cross(rdir, edge2);
  float det = dot(edge1, pvec);
  float3 tvec(rorig.x - v0.x, rorig.y - v0.y, rorig.z - v0.z);  float u = dot(tvec, pvec);

  float3 qvec = cross(tvec, edge1);
  float v = dot(rdir, qvec);

  float t = dot(edge2, qvec);

  float inv_det = 1.0f/det;

  bool hit = (u>=0)?true:false;
  hit = (v>=0)?hit:false;
  hit = (u+v<=det)?hit:false;

  u *= inv_det;
  v *= inv_det;
  t *= inv_det;

  hit = (t >= 0.0)?hit:false;
  float b = (hit)?1.0f:0.0f;
  return float4(t, u, v, b);
}

float3 reflect(float3 I, float3 N)
{
  // return I - 2.0 * N * dot(N, I);
  float3 res;
  res.x = I.x - 2.0f * N.x * dot(N, I);
  res.y = I.y - 2.0f * N.y * dot(N, I);
  res.z = I.z - 2.0f * N.z * dot(N, I);
  return res;
}

/*
vector refract( vector I, vector N, float eta )
{
  float cos_i = N * -I;
  float cos_t = 1 + (pow(eta,2)) * (pow(cos_i,2)-1);
  cos_t = sqrt( cos_t );
  vector Rr = eta * I + (eta * cos_i - cos_t) * N;
  return Rr;
}
*/

/*
float3 refract(float3 I, float3 N, float eta)
{
  // http://www.developer.com/lang/other/article.php/10942_2169281_3
  float3 negI = float3(-I.x, -I.y, -I.z);
  float cosI = dot(negI, N);
  float cosT2 = 1.0f + (eta * eta) * ((cosI * cosI) - 1.0f);
  float3 res;
  cosT2 = sqrt(abs(cosT2));
 
  res.x = eta * I.x + ((eta * cosI - cosT2) * N.x);
  res.y = eta * I.y + ((eta * cosI - cosT2) * N.y);
  res.z = eta * I.z + ((eta * cosI - cosT2) * N.z);
  if (cosT2 > 0)
    return res;
  else 
    return float3(0,0,0);
}
*/

/*
float3 refract(float3 I, float3 N, float index)
{
  float p, t;
  
  p = dot(N, I);
  if(p < 0) {
    t = 1 - ( 1 - p*p ) / ( index*index );
    if( t <= 0 )
      return float3(0,0,0);
    t = -p/index - sqrt(t);
  } else {
    index = 1 / index;
    t = 1 - ( 1 - p*p ) / ( index*index );
    if( t <= 0 )
      return float3(0,0,0);
    t = -p/index + sqrt(t);
  }

  // v2 = vadd( svproduct(1/index, v1), svproduct(t, n) );

  float3 res;
  res.x = 1/index * I.x + t * N.x;
  res.y = 1/index * I.y + t * N.y;
  res.z = 1/index * I.z + t * N.z;

  return res;
}
*/

/*
// Foley & vanDam: Computer Graphics: Principles and Practice, 
//     2nd Ed. pp 756ff.
Point3 SContext::RefractVector(float ior)
{
  SCDebugPrint("SContext::RefractVector\n");
  Point3 N = Normal();
  float VN,nur,k;
  VN = DotProd(-viewDir,N);
  if (backFace) nur = ior;
  else nur = (ior!=0.0f) ? 1.0f/ior: 1.0f;
  k = 1.0f-nur*nur*(1.0f-VN*VN);
  if (k<=0.0f) {
    // Total internal reflection: 
    return ReflectVector();
  }
  else {
    return (nur*VN-(float)sqrt(k))*N + nur*viewDir;
  }
}
*/

float3 refract(float3 I, float3 N, float ior)
{
  float3 negI = float3(-I.x, -I.y, -I.z);
  float p = dot(negI, N);

  float k = 1.0f-ior*ior*(1.0f-p*p);
  if (k<=0.0f) 
  {
    return reflect(I, N);
  }
  else 
  {
    float3 res;
    res.x = (ior*p-(float)sqrt(k))*N.x + ior*I.x;
    res.y = (ior*p-(float)sqrt(k))*N.y + ior*I.y;
    res.z = (ior*p-(float)sqrt(k))*N.z + ior*I.z;

    return res;
  }
}