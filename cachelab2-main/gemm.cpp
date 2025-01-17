#include "cachelab.h"

#define m case0_m
#define n case0_n
#define p case0_p

// 我们用这个 2*2*2 的矩阵乘法来演示寄存器是怎么被分配的
void gemm_case0(ptr_reg A, ptr_reg B, ptr_reg C, ptr_reg buffer) {  // allocate 0 1 2 3
    for (reg row_A = 0; row_A < m; ++row_A) {                       // allocate 4
        for (reg col_B = 0; col_B < p; ++col_B) {                   // allocate 5
            reg sum_C = 0;                                          // allocate 6
            for (reg col_A = 0; col_A < n; ++col_A) {               // allocate 7
                reg val_A = A[row_A * n + col_A];                   // allocate 8
                reg val_B = B[col_A * p + col_B];                   // allocate 9
                sum_C += val_A * val_B;
            }  // free 9 8
            // free 7
            C[row_A * p + col_B] = sum_C;
        }  // free 6
        // free 5
    }
    // free 4
}  // free 3 2 1 0

#undef m
#undef n
#undef p

#define TILE_SIZE_M 8
#define TILE_SIZE_N 8
#define TILE_SIZE_P 8
#define PREFETCH_DISTANCE 8
#define m case1_m
#define n case1_n
#define p case1_p

void gemm_case1(ptr_reg A, ptr_reg B, ptr_reg C, ptr_reg buffer) {
    for (reg bi = 0; bi < m; bi += TILE_SIZE_M) {
        for (reg bj = 0; bj < p; bj += TILE_SIZE_N) {
            for (reg bk = 0; bk < n; bk += TILE_SIZE_P) {
                for (reg ii = 0; ii < TILE_SIZE_M && bi + ii < m; ++ii) {
                    reg temp_row[TILE_SIZE_N] = {0};
                    for (reg kk = 0; kk < TILE_SIZE_P && bk + kk < n; kk += 2) {
                        if (kk + PREFETCH_DISTANCE < TILE_SIZE_P && bk + kk + PREFETCH_DISTANCE < n) {
                            reg _ = A[(bi + ii) * n + (bk + kk + PREFETCH_DISTANCE)];
                            _ = B[(bk + kk + PREFETCH_DISTANCE) * p + bj];
                        }
                        reg tmpa1 = A[(bi + ii) * n + (bk + kk)];
                        reg tmpa2 = 0;
                        if (kk + 1 < TILE_SIZE_P && bk + kk + 1 < n) {
                            tmpa2 = A[(bi + ii) * n + (bk + kk + 1)];
                        }
                        for (reg jj = 0; jj < TILE_SIZE_N && bj + jj < p; ++jj) {
                            reg tmpb1 = B[(bk + kk) * p + (bj + jj)];
                            reg tmpb2 = 0;
                            if (kk + 1 < TILE_SIZE_P && bk + kk + 1 < n) {
                                tmpb2 = B[(bk + kk + 1) * p + (bj + jj)];
                            }
                            temp_row[jj] += tmpa1 * tmpb1 + tmpa2 * tmpb2;
                        }
                    }
                    for (reg jj = 0; jj < TILE_SIZE_N && bj + jj < p; ++jj) {
                        reg tmpc = C[(bi + ii) * p + (bj + jj)];
                        tmpc += temp_row[jj];
                        C[(bi + ii) * p + (bj + jj)] = tmpc;
                    }
                }
            }
        }
    }
}

#undef m
#undef n
#undef p
#define TILE_SIZE_M 16
#define TILE_SIZE_N 16
#define TILE_SIZE_P 16
#define PREFETCH_DISTANCE 16
#define m case2_m
#define n case2_n
#define p case2_p

void gemm_case2(ptr_reg A, ptr_reg B, ptr_reg C, ptr_reg buffer) {
    for (reg bi = 0; bi < m; bi += TILE_SIZE_M) {
        for (reg bj = 0; bj < p; bj += TILE_SIZE_N) {
            for (reg bk = 0; bk < n; bk += TILE_SIZE_P) {
                for (reg ii = 0; ii < TILE_SIZE_M && bi + ii < m; ++ii) {
                    reg temp_row[TILE_SIZE_N] = {0};
                    for (reg kk = 0; kk < TILE_SIZE_P && bk + kk < n; ++kk) {
                        reg tmpa = A[(bi + ii) * n + (bk + kk)];
                        for (reg jj = 0; jj < TILE_SIZE_N && bj + jj < p; ++jj) {
                            reg tmpb = B[(bk + kk) * p + (bj + jj)];
                            temp_row[jj] += tmpa * tmpb;
                        }
                    }
                    for (reg jj = 0; jj < TILE_SIZE_N && bj + jj < p; ++jj) {
                        reg tmpc1 = C[(bi + ii) * p + (bj + jj)];
                        tmpc1 += temp_row[jj];
                        C[(bi + ii) * p + (bj + jj)] = tmpc1;
                    }
                }
            }
        }
    }
}

#undef m
#undef n
#undef p

#define m case3_m
#define n case3_n
#define p case3_p

void gemm_case3(ptr_reg A, ptr_reg B, ptr_reg C, ptr_reg buffer) {
    {
        for (reg i = 0; i < m; ++i) {
            reg tmp[22];
            for (reg j = 0; j < n; ++j) {
                for (reg k = 0; k < 22; ++k) {
                    reg tmpa = A[i * n + j];
                    reg tmpb = B[j * p + k];
                    tmp[k] += tmpa * tmpb;
                }
            }
            for (reg w = 0; w < 22; ++w) {
                C[i * p + w] = tmp[w];
            }
        }
    }
    {
        for (reg i = 0; i < m; ++i) {
            reg tmp[9];
            for (reg j = 0; j < n; ++j) {
                for (reg k = 22; k < p; ++k) {
                    reg tmpa = A[i * n + j];
                    reg tmpb = B[j * p + k];
                    tmp[k - 22] += tmpa * tmpb;
                }
            }
            for (reg w = 22; w < p; ++w) {
                C[i * p + w] = tmp[w - 22];
            }
        }
    }
}

#undef m
#undef n
#undef p
