
////////////////////////////////////////////
// Generated by BRCC v0.1
// BRCC Compiled on: Jun  4 2004 15:23:16
////////////////////////////////////////////

#include <brook.hpp>
#include <stdio.h>

static void  callKernel(::brook::stream a, ::brook::stream b);

static const void* __sum_ps20 = 0;


static const void* __sum_fp30 = 0;


static const void* __sum_arb = 0;

void  __sum_cpu_inner (const __BrtFloat1  &a,
                       const __BrtFloat1  &b,
                       __BrtFloat1  &c){
  c = a + b;
}
void  __sum_cpu (const std::vector<void *>&args,
                 const std::vector<const unsigned int *>&extents,
                 const std::vector<unsigned int>&dims,
                 unsigned int mapbegin, 
                 unsigned int mapextent) {
  __BrtFloat1 *arg0 = (__BrtFloat1 *)args[0];
  __BrtFloat1 *arg1 = (__BrtFloat1 *)args[1];
  __BrtFloat1 *arg2 = (__BrtFloat1 *)args[2];
  unsigned int dim=dims[2];
  unsigned int newline=extents[2][dim-1];
  unsigned int ratio0 = extents[2][dim-1]/extents[0][dim-1];
  unsigned int scale0=extents[0][dim-1]/extents[2][dim-1];
  if (scale0<1) scale0 = 1;
  unsigned int ratioiter0 = 0;
  if (ratio0) ratioiter0 = mapbegin%ratio0;
  unsigned int iter0 = getIndexOf(mapbegin,extents[0], dim, extents[2]);
  unsigned int ratio1 = extents[2][dim-1]/extents[1][dim-1];
  unsigned int scale1=extents[1][dim-1]/extents[2][dim-1];
  if (scale1<1) scale1 = 1;
  unsigned int ratioiter1 = 0;
  if (ratio1) ratioiter1 = mapbegin%ratio1;
  unsigned int iter1 = getIndexOf(mapbegin,extents[1], dim, extents[2]);
  arg2+=mapbegin;
  unsigned int i=0; 
  while (i<mapextent) {
    __sum_cpu_inner (
      *(arg0 + iter0),
      *(arg1 + iter1),
      *arg2);
    i++;
    if (++ratioiter0>=ratio0){
      ratioiter0=0;
      iter0+=scale0;
    }
    if (++ratioiter1>=ratio1){
      ratioiter1=0;
      iter1+=scale1;
    }
    ++arg2;
    if ((mapbegin+i)%newline==0) {
      iter0=getIndexOf(i+mapbegin,extents[0],dim, extents[2]);
      iter1=getIndexOf(i+mapbegin,extents[1],dim, extents[2]);
    }
  }
}

void  sum (::brook::stream a,
		::brook::stream b,
		::brook::stream c) {
  static const void *__sum_fp[] = {
     "fp30", __sum_fp30,
     "arb", __sum_arb,
     "ps20", __sum_ps20,
     "cpu", (void *) __sum_cpu,
     NULL, NULL };
  static __BRTKernel k(__sum_fp);

  k->PushStream(a);
  k->PushStream(b);
  k->PushOutput(c);
  k->Map();

}


static void  callKernel(::brook::stream a, ::brook::stream b)
{
  sum(a,b,b);
}

int  main(int  argc, char  *argv[])
{
  float  input_a1[100];
  float  input_b1[100];
  float  output_b1[100];
  float  output_b2[10][10];
  ::brook::stream a1(::brook::getStreamType(( float  *)0), 100,-1);
  ::brook::stream b1(::brook::getStreamType(( float  *)0), 100,-1);
  ::brook::stream a2(::brook::getStreamType(( float  *)0), 10 , 10,-1);
  ::brook::stream b2(::brook::getStreamType(( float  *)0), 10 , 10,-1);
  int  i;
  int  j;

  for (i = 0; i < 10; i++)
  {
    for (j = 0; j < 10; j++)
    {
      input_a1[10 * i + j] = 0.125000f * (float ) (10 * i + j);
      input_b1[10 * i + j] = (float ) (0);
    }

  }

  streamRead(a1,input_a1);
  streamRead(a2,input_a1);
  streamRead(b1,input_b1);
  streamRead(b2,input_b1);
  for (i = 0; i < 12; i++)
  {
    callKernel(a1,b1);
    callKernel(a2,b2);
  }

  streamWrite(b1,output_b1);
  streamWrite(b2,output_b2);
  for (i = 0; i < 10; i++)
  {
    for (j = 0; j < 10; j++)
      printf("%6.2f ",output_b1[10 * i + j]);
    printf("\n");
  }

  for (i = 0; i < 10; i++)
  {
    for (j = 0; j < 10; j++)
      printf("%6.2f ",output_b2[i][j]);
    printf("\n");
  }

  return 0;
}


