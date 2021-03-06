// Copyright 2011, Tokyo Institute of Technology.
// All rights reserved.
//
// This file is distributed under the license described in
// LICENSE.txt.
//
// Author: Naoya Maruyama (naoya@matsulab.is.titech.ac.jp)

#ifndef PHYSIS_PHYSIS_MPI_CUDA_H_
#define PHYSIS_PHYSIS_MPI_CUDA_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#if !defined(PHYSIS_USER)
#include <mpi.h>
#include <cuda_runtime.h>
#endif

#include "physis/physis_common.h"

#ifdef __cplusplus
extern "C" {
#endif

  // __PSGridMPI: dummy type for grid objects. We use the different
  // name than the reference implementation so that CPU and GPU
  // versions could coexist in the future implementation.

  typedef void __PSGridMPI;
  
#ifdef PHYSIS_USER
 extern __PSGridDimDev(void *p, int);
  extern void dim3(int, ...);
#else
  typedef __PSGridMPI *PSGrid1DFloat;
  typedef __PSGridMPI *PSGrid2DFloat;
  typedef __PSGridMPI *PSGrid3DFloat;
  typedef __PSGridMPI *PSGrid1DDouble;
  typedef __PSGridMPI *PSGrid2DDouble;
  typedef __PSGridMPI *PSGrid3DDouble;
  extern PSIndex PSGridDim(void *p, int d);
#define __PSGridDimDev(p, d) ((p)->dim[d])
  extern cudaStream_t stream_inner;
  extern cudaStream_t stream_boundary_copy;
  extern int num_stream_boundary_kernel;
  extern cudaStream_t stream_boundary_kernel[];
#endif

  typedef struct {
    void *p0;
#ifdef AUTO_DOUBLE_BUFFERING    
    void *p1;
#endif    
    int dim[3];
    int local_size[3];
    int local_offset[3];        
    int pitch;
    void *halo[3][2];
    int halo_width[3][2];        
    int diag;    
  } __PSGridDev3D;

  typedef struct {
    float *p0;
#ifdef AUTO_DOUBLE_BUFFERING    
    float *p1;
#endif    
    int dim[3];
    int local_size[3];
    int local_offset[3];            
    int pitch;
    float *halo[3][2];    
    int halo_width[3][2];    
    int diag;    
  } __PSGridDev3DFloat;

  typedef struct {
    double *p0;
#ifdef AUTO_DOUBLE_BUFFERING    
    double *p1;
#endif    
    int dim[3];
    int local_size[3];
    int local_offset[3];    
    int pitch;
    double *halo[3][2];    
    int halo_width[3][2];
    int diag;
  } __PSGridDev3DDouble;

#ifdef __CUDACC__
#define PS_FUNCTION_DEVICE __device__
#else
#define PS_FUNCTION_DEVICE static inline
#endif

#if ! defined(PHYSIS_RUNTIME) && ! defined(PHYSIS_USER)
  PS_FUNCTION_DEVICE size_t __PSGridCalcOffset3D(int x, int y, int z,
                                            int pitch, int dimy) {
    return x + y * pitch + z * pitch * dimy;
  }

  PS_FUNCTION_DEVICE float* __PSGridGetAddrNoHaloFloat3D(
      __PSGridDev3DFloat *g,
      int x, int y, int z) {
    x -= g->local_offset[0];
    y -= g->local_offset[1];
    z -= g->local_offset[2];
    return g->p0 + __PSGridCalcOffset3D(
        x, y, z, g->pitch, g->local_size[1]);    
  }

  PS_FUNCTION_DEVICE float* __PSGridGetAddrNoHaloFloat3DLocal(
      __PSGridDev3DFloat *g,
      int x, int y, int z) {
    return g->p0 + __PSGridCalcOffset3D(
        x, y, z, g->pitch, g->local_size[1]);    
  }
  

  PS_FUNCTION_DEVICE float* __PSGridEmitAddrFloat3D(
      __PSGridDev3DFloat *g,
      int x, int y, int z) {
    x -= g->local_offset[0];
    y -= g->local_offset[1];
    z -= g->local_offset[2];
    return g->p0 + __PSGridCalcOffset3D(x, y, z, g->pitch,
                                        g->local_size[1]);    
  }


  // z
  PS_FUNCTION_DEVICE float* __PSGridGetAddrFloat3D_2_fw(
      __PSGridDev3DFloat *g, int x, int y, int z) {
    int indices[] = {x - g->local_offset[0], y - g->local_offset[1],
		     z - g->local_offset[2]};
    if (indices[2] < g->local_size[2]) {
      return __PSGridGetAddrNoHaloFloat3DLocal(
          g, indices[0], indices[1], indices[2]);
    } else {
      indices[2] -= g->local_size[2];
      size_t offset = __PSGridCalcOffset3D(
          indices[0], indices[1], indices[2],
          g->local_size[0], g->local_size[1]);
      return g->halo[2][1] + offset;
    }
  }

  PS_FUNCTION_DEVICE float* __PSGridGetAddrFloat3D_2_bw(
      __PSGridDev3DFloat *g, int x, int y, int z) {
    int indices[] = {x - g->local_offset[0], y - g->local_offset[1],
		     z - g->local_offset[2]};
    if (indices[2] >= 0) {
      return __PSGridGetAddrNoHaloFloat3DLocal(
          g, indices[0], indices[1], indices[2]);
    } else {      
      indices[2] += g->halo_width[2][0];
      size_t offset = __PSGridCalcOffset3D(
          indices[0], indices[1], indices[2],
         g->local_size[0], g->local_size[1]);
      return g->halo[2][0] + offset;
    }
  }

  // y
  PS_FUNCTION_DEVICE float* __PSGridGetAddrFloat3D_1_fw(
      __PSGridDev3DFloat *g, int x, int y, int z) {
    int indices[] = {x - g->local_offset[0], y - g->local_offset[1],
		     z - g->local_offset[2]};
    if (indices[1] < g->local_size[1]) {
      if (indices[2] < g->local_size[2] &&
          indices[2] >= 0) {
        return __PSGridGetAddrNoHaloFloat3DLocal(
            g, indices[0], indices[1], indices[2]);
      } else if (indices[2] >= g->local_size[2]) {
        return __PSGridGetAddrFloat3D_2_fw(g, x, y, z);
      } else {
        return __PSGridGetAddrFloat3D_2_bw(g, x, y, z);        
      }
    } else {
      if (g->diag) indices[2] += g->halo_width[2][0];        
      indices[1] -= g->local_size[1];
      size_t offset = __PSGridCalcOffset3D(
          indices[0], indices[1], indices[2],
          g->local_size[0], g->halo_width[1][1]);
      return g->halo[1][1] + offset;
    }
  }

  PS_FUNCTION_DEVICE float* __PSGridGetAddrFloat3D_1_bw(
      __PSGridDev3DFloat *g, int x, int y, int z) {
    int indices[] = {x - g->local_offset[0], y - g->local_offset[1],
		     z - g->local_offset[2]};
    if (indices[1] >= 0) {
      if (indices[2] < g->local_size[2] &&
          indices[2] >= 0) {
        return __PSGridGetAddrNoHaloFloat3DLocal(
            g, indices[0], indices[1], indices[2]);
      } else if (indices[2] >= g->local_size[2]) {
        return __PSGridGetAddrFloat3D_2_fw(g, x, y, z);
      } else {
        return __PSGridGetAddrFloat3D_2_bw(g, x, y, z);        
      }
    } else {
      if (g->diag) indices[2] += g->halo_width[2][0];        
      indices[1] += g->halo_width[1][0];
      size_t offset = __PSGridCalcOffset3D(
          indices[0], indices[1], indices[2],
          g->local_size[0], g->halo_width[1][0]);          
      return g->halo[1][0] + offset;
    }
  }

  // x
  PS_FUNCTION_DEVICE float* __PSGridGetAddrFloat3D_0_fw(
      __PSGridDev3DFloat *g, int x, int y, int z) {
    int indices[] = {x - g->local_offset[0], y - g->local_offset[1],
		     z - g->local_offset[2]};
    if (indices[0] < g->local_size[0]) {
      // not in the halo region of this dimension
      if (indices[1] < g->local_size[1] &&
          indices[1] >= 0 &&
          indices[2] < g->local_size[2] &&
          indices[2] >= 0) {
        // must be inside region
        return __PSGridGetAddrNoHaloFloat3DLocal(
            g, indices[0], indices[1], indices[2]);
      } else if (indices[1] >= g->local_size[1]) {
        return __PSGridGetAddrFloat3D_1_fw(g, x, y, z);
      } else if (indices[1] < 0) {
        return __PSGridGetAddrFloat3D_1_bw(g, x, y, z);        
      } else if (indices[2] >= g->local_size[2]) {
        return __PSGridGetAddrFloat3D_2_fw(g, x, y, z);
      } else {
        return __PSGridGetAddrFloat3D_2_bw(g, x, y, z);
      }
    } else {
      size_t halo_size1 = g->local_size[1];
      if (g->diag) {
        indices[1] += g->halo_width[1][0];
        indices[2] += g->halo_width[2][0];        
        halo_size1 += g->halo_width[1][0] +
            g->halo_width[1][1];
      }
      indices[0] -= g->local_size[0];
      size_t offset = __PSGridCalcOffset3D(
          indices[0], indices[1], indices[2],
          g->halo_width[0][1], halo_size1);
      return g->halo[0][1] + offset;
    }
   }
#endif
  
#if defined(PHYSIS_TRANSLATOR) || defined(PHYSIS_RUNTIME) || defined(PHYSIS_USER)
  extern float* __PSGridGetAddrFloat3D_0_bw(
      __PSGridDev3DFloat *g, int x, int y, int z);
#else
  __device__ float* __PSGridGetAddrFloat3D_0_bw(
      __PSGridDev3DFloat *g, int x, int y, int z) {
    int indices[] = {x - g->local_offset[0], y - g->local_offset[1],
		     z - g->local_offset[2]};
    if (indices[0] >= 0) { // not in the halo region of this dimension
      if (indices[1] < g->local_size[1] &&
          indices[1] >= 0 &&
          indices[2] < g->local_size[2] &&
          indices[2] >= 0) {
        // must be inside region
        return __PSGridGetAddrNoHaloFloat3DLocal(
            g, indices[0], indices[1], indices[2]);
      } else if (indices[1] >= g->local_size[1]) {
        return __PSGridGetAddrFloat3D_1_fw(g, x, y, z);
      } else if (indices[1] < 0) {
        return __PSGridGetAddrFloat3D_1_bw(g, x, y, z);        
      } else if (indices[2] >= g->local_size[2]) {
        return __PSGridGetAddrFloat3D_2_fw(g, x, y, z);
      } else {
        return __PSGridGetAddrFloat3D_2_bw(g, x, y, z);
      }
    } else {
      size_t halo_size1 = g->local_size[1];      
      if (g->diag) {
        indices[1] += g->halo_width[1][0];
        indices[2] += g->halo_width[2][0];        
        halo_size1 += g->halo_width[1][0] +
            g->halo_width[1][1];
      }
      indices[0] += g->halo_width[0][0];
      size_t offset = __PSGridCalcOffset3D(
          indices[0], indices[1], indices[2],
          g->halo_width[0][0], halo_size1);
      return g->halo[0][0] + offset;
    }
  }
#endif  

#if defined(PHYSIS_TRANSLATOR) || defined(PHYSIS_RUNTIME) || defined(PHYSIS_USER)
  extern float* __PSGridGetAddrFloat3D(__PSGridDev3DFloat *g,
                                       int x, int y, int z);
#else  
  __device__ float* __PSGridGetAddrFloat3D(__PSGridDev3DFloat *g,
                                           int x, int y, int z) {
    int indices[] = {x - g->local_offset[0],
		     y - g->local_offset[1],
		     z - g->local_offset[2]};
    size_t halo_size[3] = {g->local_size[0], g->local_size[1],
                           g->local_size[2]};          
    for (int i = 0; i < 3; ++i) {
      if (indices[i] < 0 || indices[i] >= g->local_size[i]) {
        float *buf;
        if (g->diag) {
          for (int j = i+1; j < 3; ++j) {
            indices[j] += g->halo_width[j][0];
            halo_size[j] += g->halo_width[j][0] +
                            g->halo_width[j][1];
          }
        }
        size_t offset;
        if (indices[i] < 0) {
          indices[i] += g->halo_width[i][0];
          halo_size[i] = g->halo_width[i][0];
          buf = g->halo[i][0];
        } else {
          indices[i] -= g->local_size[i];
          halo_size[i] = g->halo_width[i][1];
          buf = g->halo[i][1];
        }
        offset = __PSGridCalcOffset3D(
				      indices[0], indices[1], indices[2],
				      halo_size[0], halo_size[1]);
        return buf + offset;
      }
    }
    return __PSGridGetAddrNoHaloFloat3D(g, x, y, z);
  }
#endif  

  extern void __PSDomainSetLocalSize(__PSDomain *dom);  
  extern __PSGridMPI* __PSGridNewMPI(PSType type, int elm_size, int dim,
                                     const PSVectorInt size,
                                     int double_buffering,
                                     int attr,
                                     const PSVectorInt global_offset);
  extern void __PSGridSwap(__PSGridMPI *g);
  extern void __PSGridMirror(__PSGridMPI *g);
  extern int __PSGridGetID(__PSGridMPI *g);
  extern __PSGridMPI *__PSGetGridByID(int id);
  extern void __PSGridSet(__PSGridMPI *g, void *buf, ...);
  extern float __PSGridGetFloat(__PSGridMPI *g, ...);
  extern double __PSGridGetDouble(__PSGridMPI *g, ...);
  

  extern void __PSStencilRun(int id, int iter, int num_stencils, ...);

  extern int __PSBcast(void *buf, size_t size);

  extern void __PSLoadNeighbor(__PSGridMPI *g,
                               const PSVectorInt halo_fw_width,
                               const PSVectorInt halo_bw_width,
                               int diagonal, int reuse,
                               int overlap, int periodic);
  extern void __PSLoadNeighborStage1(__PSGridMPI *g,
                               const PSVectorInt halo_fw_width,
                               const PSVectorInt halo_bw_width,
                               int diagonal, int reuse,
                               int overlap, int periodic);
  extern void __PSLoadNeighborStage2(__PSGridMPI *g,
                               const PSVectorInt halo_fw_width,
                               const PSVectorInt halo_bw_width,
                               int diagonal, int reuse,
                               int overlap, int periodic);
  extern void __PSLoadSubgrid(__PSGridMPI *g, const __PSGridRange *gr,
                              int reuse);
  extern void __PSLoadSubgrid2D(__PSGridMPI *g, 
                                int min_dim1, PSIndex min_offset1,
                                int min_dim2, PSIndex min_offset2,
                                int max_dim1, PSIndex max_offset1,
                                int max_dim2, PSIndex max_offset2,
                                int reuse);
  extern void __PSLoadSubgrid3D(__PSGridMPI *g,
                                int min_dim1, PSIndex min_offset1,
                                int min_dim2, PSIndex min_offset2,
                                int min_dim3, PSIndex min_offset3,
                                int max_dim1, PSIndex max_offset1,
                                int max_dim2, PSIndex max_offset2,
                                int max_dim3, PSIndex max_offset3,
                                int reuse);
  extern void __PSActivateRemoteGrid(__PSGridMPI *g,
                                     int active);
  extern int __PSIsRoot();

  extern void *__PSGridGetDev(void *g);
  extern PSIndex __PSGetLocalSize(int dim);
  extern PSIndex __PSGetLocalOffset(int dim);

  extern __PSDomain __PSDomainShrink(__PSDomain *dom, int width);

  //! Reduces a grid with an operator.
  /*!
    \param buf A pointer to the output buffer.
    \param op A binary operator to reduce elements.
    \param g A grid.
   */
  extern void __PSReduceGridFloat(void *buf, enum PSReduceOp op,
                                  __PSGridMPI *g);
  extern void __PSReduceGridDouble(void *buf, enum PSReduceOp op,
                                  __PSGridMPI *g);

  // CUDA Runtime APIs. Have signatures here to verify generated
  // ASTs.
#ifdef PHYSIS_USER
  typedef void* cudaStream_t;
  typedef int cudaError_t;
  extern cudaStream_t stream_inner;
  extern cudaStream_t stream_boundary_kernel;  
  extern cudaError_t cudaThreadSynchronize(void);
  extern cudaError_t cudaStreamSynchronize(cudaStream);
  extern cudaError_t cudaFuncSetCacheConfig(const char* func,
                                            int);
#endif
  

#ifdef __cplusplus
}
#endif

#endif /* PHYSIS_PHYSIS_MPI_CUDA_H_ */
