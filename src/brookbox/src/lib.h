float dot(float3 v1, float3 v2);
float3 cross(float3 v1, float3 v2);

float4 lit(float diffuse, float specular, float n);

bool all(float3 v);
float3 voxel_addr(float3 p);
bool point_in_box(float3 boxa, float3 boxb, float3 p);
float3 intersect_box(float3 boxa, float3 boxb, float3 ro, float3 rd);
float4 intersect_triangle(float3 rorig, float3 rdir, float3 v0, float3 v1, float3 v2);
float3 reflect(float3 I, float3 N);
float3 refract(float3 I, float3 N, float eta);

/*
inline __BrtFloat1 __all_cpu_inner(__BrtFloat3 v)
{
  float3 _v(v.unsafeGetAt(0), v.unsafeGetAt(1), v.unsafeGetAt(2));
  bool res = all(_v);
  return res?__BrtFloat1(1.0):__BrtFloat1(0.0);
}
*/

inline __BrtFloat3 __voxel_addr_cpu_inner(__BrtFloat3 p)
{
  float3 _p(p.unsafeGetAt(0), p.unsafeGetAt(1), p.unsafeGetAt(2));
  float3 res = voxel_addr(_p);
  return __BrtFloat3(res.x, res.y, res.z);
}

#define __VOXELADDR_cpu_inner VOXELADDR

inline __BrtFloat3 __reflect_cpu_inner(__BrtFloat3 I, __BrtFloat3 N)
{
  float3 _I(I.unsafeGetAt(0), I.unsafeGetAt(1), I.unsafeGetAt(2));
  float3 _N(N.unsafeGetAt(0), N.unsafeGetAt(1), N.unsafeGetAt(2)); 
  float3 res = reflect(_I, _N);
  return __BrtFloat3(res.x, res.y, res.z);
}

inline __BrtFloat3 __refract_cpu_inner(__BrtFloat3 I, __BrtFloat3 N, __BrtFloat1 eta)
{
  float3 _I(I.unsafeGetAt(0), I.unsafeGetAt(1), I.unsafeGetAt(2));
  float3 _N(N.unsafeGetAt(0), N.unsafeGetAt(1), N.unsafeGetAt(2)); 
  float _eta = eta.unsafeGetAt(0); 
  float3 res = refract(_I, _N, _eta);
  return __BrtFloat3(res.x, res.y, res.z);
}

inline bool __point_in_box_cpu_inner(__BrtFloat3 boxa, __BrtFloat3 boxb, __BrtFloat3 p)
{
  float3 _boxa(boxa.unsafeGetAt(0), boxa.unsafeGetAt(1), boxa.unsafeGetAt(2));
  float3 _boxb(boxb.unsafeGetAt(0), boxb.unsafeGetAt(1), boxb.unsafeGetAt(2)); 
  float3 _p(p.unsafeGetAt(0), p.unsafeGetAt(1), p.unsafeGetAt(2));
  return point_in_box(_boxa, _boxb, _p);
}

inline __BrtFloat3 __intersect_box_cpu_inner(__BrtFloat3 boxa, __BrtFloat3 boxb, __BrtFloat3 ro, __BrtFloat3 rd)
{
  float3 _boxa(boxa.unsafeGetAt(0), boxa.unsafeGetAt(1), boxa.unsafeGetAt(2));
  float3 _boxb(boxb.unsafeGetAt(0), boxb.unsafeGetAt(1), boxb.unsafeGetAt(2)); 
  float3 _ro(ro.unsafeGetAt(0), ro.unsafeGetAt(1), ro.unsafeGetAt(2)); 
  float3 _rd(rd.unsafeGetAt(0), rd.unsafeGetAt(1), rd.unsafeGetAt(2)); 
  float3 res = intersect_box(_boxa, _boxb, _ro, _rd);
  return __BrtFloat3(res.x, res.y, res.z);
}

inline __BrtFloat4 __intersect_triangle_cpu_inner(__BrtFloat3 rorig, __BrtFloat3 rdir, __BrtFloat3 v0, __BrtFloat3 v1, __BrtFloat3 v2)
{
  float3 _rorig(rorig.unsafeGetAt(0), rorig.unsafeGetAt(1), rorig.unsafeGetAt(2)); 
  float3 _rdir(rdir.unsafeGetAt(0), rdir.unsafeGetAt(1), rdir.unsafeGetAt(2)); 
  float3 _v0(v0.unsafeGetAt(0), v0.unsafeGetAt(1), v0.unsafeGetAt(2)); 
  float3 _v1(v1.unsafeGetAt(0), v1.unsafeGetAt(1), v1.unsafeGetAt(2)); 
  float3 _v2(v2.unsafeGetAt(0), v2.unsafeGetAt(1), v2.unsafeGetAt(2)); 
  float4 res = intersect_triangle(_rorig, _rdir, _v0, _v1, _v2);
  return __BrtFloat4(res.x, res.y, res.z, res.w);
}

inline __BrtFloat4 __lit_cpu_inner(__BrtFloat1 diffuse, __BrtFloat1 specular, __BrtFloat1 n)
{
  float _diffuse(diffuse.unsafeGetAt(0));
  float _specular(specular.unsafeGetAt(0));
  float _n(n.unsafeGetAt(0));
  float4 res = lit(_diffuse, _specular, _n);
  return __BrtFloat4(res.x, res.y, res.z, res.w);
}

