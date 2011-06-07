#include <stdio.h>
#include "physis/physis.h"

#define N 4

void kernel(const int x, const int y, const int z,
            PSGrid3DFloat g1, PSGrid3DFloat g2) {
  float v = PSGridGet(g2, 0, y, z);
  PSGridEmit(g1, v);
  return;
}

void dump(float *buf, size_t len, FILE *out) {
  int i;
  for (i = 0; i < len; ++i) {
    fprintf(out, "%f\n", buf[i]);
  }
}

#define IDX3(x, y, z) ((x) + (y) * N + (z) * N * N)
#define IDX2(x, y) ((x) + (y) * N)

int main(int argc, char *argv[]) {
  PSInit(&argc, &argv, 3, N, N, N);
  PSGrid3DFloat g1 = PSGrid3DFloatNew(N, N, N);
  PSGrid3DFloat g2 = PSGrid3DFloatNew(1, N, N);
  
  size_t nelms = N*N*N;
  int i, j, k;
  
  float *indata = (float *)malloc(sizeof(float) * nelms);

  for (i = 0; i < N*N; i++) {
    indata[i] = i;
  }
  PSGridCopyin(g2, indata);

  PSDomain3D d = PSDomain3DNew(0, N, 0, N, 0, N);
  PSStencilRun(PSStencilMap(kernel, d, g1, g2), 1);
  
  float *outdata = (float *)malloc(sizeof(float) * nelms);
  PSGridCopyout(g1, outdata);

#if 0
  for (i = 0; i < 1; ++i) {
    for (j = 0; j < N; ++j) {
      for (k = 0; k < N; ++k) {
        if (indata[IDX3(i, j, k)] != outdata[IDX2(j, k)]) {
          printf("Error: mismatch at %d,%d,%d, in: %f, out: %f\n",
                 i, j, k, indata[IDX3(i, j, k)], outdata[IDX2(j, k)]);
        }
      }
    }
  }
#endif /* 0 */
  
  PSGridFree(g1);
  PSGridFree(g2);  
  PSFinalize();

  dump(outdata, N*N*N, stdout);
  free(indata);
  free(outdata);
  return 0;
}

