#ifndef FILEIO_H
#define FILEIO_H

#include <stdio.h>
#include "Types.h"
#include "HMatrix3.h"
#include "Bitvector.h"

void ReadVoxFile( char *fname, Tuple3i &grid_dim, Tuple3f &grid_min,
		  Tuple3f &grid_max, Tuple3f &grid_vsize,
		  BitVector* &grid_bitvector, int* &grid_trilist_offset,
		  int &trilist_size, int* &grid_trilist, int &numtris,
		  Point3* &v0, Point3* &v1, Point3* &v2,
		  Normal3* &n0, Normal3* &n1, Normal3* &n2,
		  Spectra* &c0, Spectra* &c1, Spectra* &c2 );

void WriteVoxFile( char *fname, int *ntris, int modelno,
		   const Tuple3i &grid_dim, const Tuple3f &grid_min,
		   const Tuple3f &grid_max, const Tuple3f &grid_vsize,
		   BitVector* bv, int ****voxels,
		   Point3 **v0, Point3 **v1, Point3 **v2,
		   Normal3 **n0, Normal3 **n1, Normal3 **n2,
		   Spectra **c0, Spectra **c1, Spectra **c2 );

void ReadPlyFile( char *filename, int &numtris, HMatrix3 xform,
		  Point3* &v0, Point3* &v1, Point3* &v2,
		  Normal3* &n0, Normal3* &n1, Normal3* &n2,
		  Spectra* &c0, Spectra* &c1, Spectra* &c2);

#endif /* FILEIO_H */
