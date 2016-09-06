/*
 * Copyright 2014-2016 Intel Corporation.
 *
 * This file is subject to the Intel Sample Source Code License. A copy
 * of the Intel Sample Source Code License is included.
 */
#include <stdio.h>
#include <mkl.h>
#include <math.h>
#ifndef WIN32
#include <unistd.h>
#include <sys/time.h>
#endif
#include <omp.h>
#include <stdlib.h>
#include <cmath>
#include <string.h>

#include <hStreams_source.h>
#include <hStreams_app_api.h>

#include "helper_functions.h"
#include "dtime.h"
#include "hStreams_custom_init.h"
#include "hStreams_custom.h"
#include "host_cpu_mask.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define SWITCH_CHAR  '-'

#define HSTR_MAX_THREADS 500

//Modify this if need to reserve some other cpu masks
//Always reserve a full core, i.e. if HT is enabled, reserve all threads on the core.
//Reserving 2 to 3 threads give best performance
#ifdef HOST_HT_ON
//replace 28 with the cpu_mask corresponding to the hyper-thread on core 0
const int num_resv_cpus = 2;
const int resv_cpus[] = {0, 28};
#else
const int num_resv_cpus = 2;
const int resv_cpus[] = {0, 1};
#endif
//
//
//#define HOST_HT_ON
//
extern double *dpo_generate(size_t);
int resv_cpu_master;
int mach_wide_league;

HSTR_CPU_MASK avail_cpu_mask, avoid_cpu_mask;

static int loc_verbose = 0;

int cholesky_tiled(double *mat, int tile_size, int num_tiles, int mat_size,
                   int niter, int max_log_str, bool layRow, int verify, int num_doms, int use_host, int num_mics,
                   int host_ht_offset)
{
    //verification result
    bool result;
    //total number of tiles
    int tot_tiles = num_tiles * num_tiles;

    //memory allocation for matrix for tiled-Cholesky
    double *A_my = (double *)malloc(mat_size * mat_size * sizeof(double));

    //memory allocation for matrix for MKL cholesky (for comparison)
    double *A_MKL = (double *)malloc(mat_size * mat_size * sizeof(double));

    //memory allocation for tiled matrix
    double **Asplit = new double* [tot_tiles];
    int mem_size_tile = tile_size * tile_size * sizeof(double);

#define HSTR_BUFFER_PROPS_VALUES {        \
        HSTR_MEM_TYPE_NORMAL,             \
        HSTR_MEM_ALLOC_PREFERRED,         \
        HSTR_BUF_PROP_ALIASED}

    HSTR_BUFFER_PROPS buffer_props = HSTR_BUFFER_PROPS_VALUES;
    for (int i = 0; i < tot_tiles; ++i) {
        //Buffer per tile, host allocation
        Asplit[i] = (double *)_mm_malloc(mem_size_tile, 64);

        //Buffer creation and allocation on the card
        //hStreams_app_create_buf((void *)Asplit[i], mem_size_tile);
        CHECK_HSTR_RESULT(hStreams_Alloc1DEx(
                              (void *)Asplit[i],
                              mem_size_tile,
                              &buffer_props,
                              -1,
                              NULL));
    }

    double tbegin, tend;

    int iter;
    int info;

    //Events are needed for various synchronizations to enforce
    //data dependence between and among data-transfers/computes
    HSTR_EVENT *eventcpyto = new HSTR_EVENT[tot_tiles];
    HSTR_EVENT *eventcpyto_trsm = new HSTR_EVENT[tot_tiles * num_doms];
    HSTR_EVENT *eventcpyfr = new HSTR_EVENT[tot_tiles];
    HSTR_EVENT *eventpotrf = new HSTR_EVENT[tot_tiles];
    HSTR_EVENT *eventtrsm = new HSTR_EVENT[tot_tiles];
    HSTR_EVENT *eventsyrk = new HSTR_EVENT[tot_tiles];
    HSTR_EVENT *eventgemm = new HSTR_EVENT[tot_tiles];

    //for timing tiled cholesky
    double *totTimeMsec = new double [niter];

    //for timing MKL cholesky
    double *totTimeMsecMKL = new double [niter];


    mkl_mic_disable();

    //these queues are used for queining up compute on the card and
    //data transfers to/from the card.
    //q_trsm for dtrsm, q_potrf for dportf, q_syrk_gemm for both dsyrk and dgemm.
    //The queues are incremented by one for every compute queued and wrap
    //around the max_log_str available. This ensures good load-balancing.
    int q_trsm, q_potrf;
    int q_syrk_gemm[10];

    CBLAS_ORDER blasLay;
    int lapackLay;

    if (layRow) {
        blasLay = CblasRowMajor;
        lapackLay = LAPACK_ROW_MAJOR;
    } else {
        blasLay = CblasColMajor;
        lapackLay = LAPACK_COL_MAJOR;
    }

    for (iter = 0; iter < niter; ++iter) {

        //copying matrices into separate variables for tiled cholesky (A_my)
        //and MKL cholesky (A_MKL)
        //The output overwrites the matrices and hence the need to copy
        //for each iteration
        copy_mat(mat, A_my, mat_size);
        copy_mat(mat, A_MKL, mat_size);

        unsigned int m, n, k;

        printf("\nIteration = %d\n", iter);

        //splitting time included in the timing
        //This splits the input matrix into tiles (or blocks)
        split_into_blocks(A_my, Asplit, num_tiles, tile_size, mat_size, layRow);

        //beginning of timing
        tbegin = dtimeGet();

        int ic;
        int is_mic;
        for (ic = 0; ic < num_doms; ++ic) {
            q_syrk_gemm[ic] = 0;
        }
        q_potrf = 0;
        q_trsm = 0;
        for (k = 0; k < num_tiles; ++k) {
            //POTRF
            //dpotrf is executed on the host on the diagonal tile
            if (mach_wide_league) {
                q_potrf = 0;
            } else {
                q_potrf = q_syrk_gemm[0];
            }

            int qindex = (int)q_potrf % max_log_str;
            if (use_host) {
                if (k == 0) {
                    if (loc_verbose > 0)
                        printf("Sending tile[%d][%d] to host in queue %d, triggering event eventcpyto[%d][%d]\n",
                               k, k, (int)(qindex), k, k);

                    hStreams_app_xfer_memory((int)(qindex),
                                             Asplit[k * num_tiles + k],
                                             Asplit[k * num_tiles + k], mem_size_tile,
                                             HSTR_SRC_TO_SINK,
                                             &eventcpyto[k * num_tiles + k]);
                }
            }

            if (k > 0) {
                if (use_host) {
                    hStreams_app_event_wait_in_stream(qindex, 1, &eventcpyfr[k * num_tiles + k], 0, NULL, NULL);
                } else {
                    hStreams_app_event_wait(1, &eventcpyfr[k * num_tiles + k]);
                }

                if (loc_verbose > 0) {
                    printf("Waiting on eventcpyfr[%d]\n", k * num_tiles + k);
                }
            }

            if (loc_verbose > 0)
                printf("Executing potrf on host for tile[%d][%d], in queue (if use_host) %d, triggerring eventpotrf[%d][%d]\n",
                       k, k, qindex, k, k);

            if (use_host) {
                CHECK_HSTR_RESULT(hStreams_custom_dpotrf(lapackLay, 'L', tile_size,
                                  Asplit[k * num_tiles + k], tile_size, qindex, &eventpotrf[k * num_tiles + k]));
            } else {
                info = LAPACKE_dpotrf(lapackLay, 'L', tile_size,
                                      Asplit[k * num_tiles + k], tile_size);
            }


            if (mach_wide_league) {
                q_trsm = q_syrk_gemm[0];
            } else {
                q_potrf++;
                q_trsm = q_potrf;
            }

            for (m = k + 1; m < num_tiles; ++m) {

                if (mach_wide_league) {
                    qindex = (int)(q_trsm % max_log_str + 1);
                } else {
                    qindex = (int)(q_trsm % max_log_str);
                }

                if (use_host) {
                    if (k == 0) {
                        if (loc_verbose > 0)
                            printf("Sending tile[%d][%d] to host in queue %d, triggering event eventcpyto[%d][%d]\n",
                                   m, k, (int)(qindex), m, k);

                        hStreams_app_xfer_memory((int)(qindex),
                                                 Asplit[m * num_tiles + k],
                                                 Asplit[m * num_tiles + k], mem_size_tile,
                                                 HSTR_SRC_TO_SINK,
                                                 &eventcpyto[m * num_tiles + k]);
                    }
                }

                if (k > 0) {
                    if (use_host) {
                        hStreams_app_event_wait_in_stream(qindex, 1, &eventcpyfr[m * num_tiles + k], 0, NULL, NULL);
                    } else {
                        hStreams_app_event_wait(1, &eventcpyfr[m * num_tiles + k]);
                    }

                    if (loc_verbose > 0) {
                        printf("Waiting on eventcpyfr[%d]\n", m * num_tiles + k);
                    }
                }

                if (use_host)
                    //hStreams_app_event_wait(1, &eventpotrf[k*num_tiles + k]);
                {
                    hStreams_app_event_wait_in_stream(qindex, 1, &eventpotrf[k * num_tiles + k], 0, NULL, NULL);
                }

                //dtrsm is executed on the host
                if (loc_verbose > 0)
                    printf("Executing trsm for tile[%d][%d] on host, in queue (if use_host) %d, triggering eventtrsm[%d][%d]\n",
                           m, k, qindex, m, k);

                if (use_host) {
                    CHECK_HSTR_RESULT(hStreams_custom_dtrsm(blasLay, CblasRight, CblasLower,
                                                            CblasTrans, CblasNonUnit, tile_size, tile_size, 1.0,
                                                            Asplit[k * num_tiles + k], tile_size, Asplit[m * num_tiles + k],
                                                            tile_size, qindex,
                                                            &eventtrsm[m * num_tiles + k]));
                } else {
                    cblas_dtrsm(blasLay, CblasRight, CblasLower,
                                CblasTrans, CblasNonUnit, tile_size, tile_size, 1.0,
                                Asplit[k * num_tiles + k], tile_size, Asplit[m * num_tiles + k],
                                tile_size);
                }

                //transfer to all cards
                for (ic = 0; ic < num_doms; ++ic) {
                    if ((use_host == 1) && (num_mics >= 1)) {
                        if (ic == 0) {
                            is_mic = 0;    //this is host
                        } else {
                            is_mic = 1;
                        }
                    } else {
                        is_mic = 0;
                    }

                    if (mach_wide_league) {
                        qindex = (int)q_trsm % max_log_str + ic * max_log_str + 1 + is_mic * host_ht_offset;
                    } else {
                        qindex = (int)q_trsm % max_log_str + ic * max_log_str + is_mic * host_ht_offset;
                    }

                    if (use_host)
                        //hStreams_app_event_wait(1, &eventtrsm[m*num_tiles + k]);
                    {
                        hStreams_app_event_wait_in_stream(qindex, 1, &eventtrsm[m * num_tiles + k], 0, NULL, NULL);
                    }

                    if (loc_verbose > 0)
                        printf("Sending tile[%d][%d] to card %d in queue %d, triggering event eventcpyto_trsm[%d]\n",
                               m, k, ic, (int)(qindex), m * num_tiles + k + ic * tot_tiles);

                    hStreams_app_xfer_memory((int)(qindex),
                                             Asplit[m * num_tiles + k],
                                             Asplit[m * num_tiles + k], mem_size_tile,
                                             HSTR_SRC_TO_SINK,
                                             &eventcpyto_trsm[m * num_tiles + k + ic * tot_tiles]);
                }

                q_trsm++;
            }

            if (use_host) {
                q_syrk_gemm[0] = q_trsm;
                for (ic = 1; ic < num_doms; ++ic) {
                    q_syrk_gemm[ic] = 0;
                }
            } else {
                for (ic = 0; ic < num_doms; ++ic) {
                    q_syrk_gemm[ic] = 0;
                }
            }

            for (n = k + 1; n < num_tiles; ++n) {
                ic = n % num_doms; //round-robin rows across num_doms

                if ((use_host == 1) && (num_mics >= 1)) {
                    if (ic == 0) {
                        is_mic = 0;    //this is host
                    } else {
                        is_mic = 1;
                    }
                } else {
                    is_mic = 0;
                }

                if (mach_wide_league) {
                    qindex  = q_syrk_gemm[ic] % max_log_str + ic * max_log_str + 1 + is_mic * host_ht_offset;
                } else {
                    qindex  = q_syrk_gemm[ic] % max_log_str + ic * max_log_str + is_mic * host_ht_offset;
                }

                if (k == 0) {
                    if (loc_verbose > 0)
                        printf("Sending tile[%d][%d] to card in queue %d\n",
                               n, n, (int)(qindex));

                    hStreams_app_xfer_memory((int)(qindex),
                                             Asplit[n * num_tiles + n],
                                             Asplit[n * num_tiles + n], mem_size_tile,
                                             HSTR_SRC_TO_SINK,
                                             &eventcpyto[n * num_tiles + n]);
                }

                //DSYRK
                //hStreams_app_event_wait(1, &eventcpyto_trsm[n*num_tiles + k + ic*tot_tiles]);
                hStreams_app_event_wait_in_stream(qindex, 1, &eventcpyto_trsm[n * num_tiles + k + ic * tot_tiles], 0, NULL, NULL);
                if (loc_verbose > 0) {
                    printf("Waiting on eventcpyto_trsm[%d]\n", n * num_tiles + k + ic * tot_tiles);
                }

                if (k > 0) {
                    //hStreams_app_event_wait(1, &eventsyrk[n*num_tiles + n]);
                    hStreams_app_event_wait_in_stream(qindex, 1, &eventsyrk[n * num_tiles + n], 0, NULL, NULL);
                    if (loc_verbose > 0) {
                        printf("Waiting on eventsyrk[%d]\n", n * num_tiles + n);
                    }
                }

                //dsyrk is executed on the card
                if (loc_verbose > 0)
                    printf("Executing syrk for tile[%d][%d] on card in queue %d, triggering event eventsyrk[%d]\n",
                           n, n, (int)(qindex), n * num_tiles + n);


                CHECK_HSTR_RESULT(hStreams_custom_dsyrk(blasLay, CblasLower, CblasNoTrans,
                                                        tile_size, tile_size, -1.0, Asplit[n * num_tiles + k],
                                                        tile_size, 1.0, Asplit[n * num_tiles + n], tile_size,
                                                        (int)(qindex), &eventsyrk[n * num_tiles + n]));

                //send tile to host (only if n = k+1)
                if (n == k + 1) {
                    if (loc_verbose > 0)
                        printf("Sending tile[%d][%d] from card  to host in queue %d, triggering event eventcpyfr[%d]\n",
                               n, n, (int)(qindex), n * num_tiles + n);

                    hStreams_app_xfer_memory((int)(qindex),
                                             Asplit[n * num_tiles + n],
                                             Asplit[n * num_tiles + n], mem_size_tile,
                                             HSTR_SINK_TO_SRC,
                                             &eventcpyfr[n * num_tiles + n]);

                }

                q_syrk_gemm[ic]++;


                for (m = n + 1; m < num_tiles; ++m) {
                    ic = m % num_doms; //round-robin rows across num_doms

                    if ((use_host == 1) && (num_mics >= 1)) {
                        if (ic == 0) {
                            is_mic = 0;    //this is host
                        } else {
                            is_mic = 1;
                        }
                    } else {
                        is_mic = 0;
                    }

                    if (mach_wide_league) {
                        qindex = q_syrk_gemm[ic] % max_log_str + ic * max_log_str + 1 + is_mic * host_ht_offset;
                    } else {
                        qindex = q_syrk_gemm[ic] % max_log_str + ic * max_log_str + is_mic * host_ht_offset;
                    }

                    if (k == 0) {
                        if (loc_verbose > 0)
                            printf("Sending tile[%d][%d] to card in queue %d\n",
                                   m, n, (int)(qindex));

                        hStreams_app_xfer_memory((int)(qindex),
                                                 Asplit[m * num_tiles + n],
                                                 Asplit[m * num_tiles + n], mem_size_tile,
                                                 HSTR_SRC_TO_SINK,
                                                 &eventcpyto[m * num_tiles + n]);
                    }

                    //DGEMM
                    if (loc_verbose > 0) {
                        printf("Waiting on eventcpyto_trsm[%d]\n", m * num_tiles + k + ic * tot_tiles);
                    }
                    //hStreams_app_event_wait(1, &eventcpyto_trsm[m*num_tiles + k + ic*tot_tiles]);
                    hStreams_app_event_wait_in_stream(qindex, 1, &eventcpyto_trsm[m * num_tiles + k + ic * tot_tiles], 0, NULL, NULL);

                    if (loc_verbose > 0) {
                        printf("Waiting on eventcpyto_trsm[%d]\n", n * num_tiles + k + ic * tot_tiles);
                    }
                    //hStreams_app_event_wait(1, &eventcpyto_trsm[n*num_tiles + k + ic*tot_tiles]);
                    hStreams_app_event_wait_in_stream(qindex, 1, &eventcpyto_trsm[n * num_tiles + k + ic * tot_tiles], 0, NULL, NULL);

                    if (k > 0) {
                        //hStreams_app_event_wait(1, &eventgemm[m*num_tiles + n]);
                        hStreams_app_event_wait_in_stream(qindex, 1, &eventgemm[m * num_tiles + n], 0, NULL, NULL);
                        if (loc_verbose > 0) {
                            printf("Waiting on eventgemm[%d]\n", m * num_tiles + n);
                        }
                    }

                    //dgemm is executed on the card
                    if (loc_verbose > 0)
                        printf("Executing gemm for tile[%d][%d] on card in queue %d, triggering event eventgemm[%d]\n",
                               m, n, (int)(qindex), m * num_tiles + n);

                    CHECK_HSTR_RESULT(hStreams_app_dgemm((int)(qindex),
                                                         blasLay, CblasNoTrans, CblasTrans,
                                                         tile_size, tile_size, tile_size, -1.0, Asplit[m * num_tiles + k],
                                                         tile_size, Asplit[n * num_tiles + k], tile_size, 1.0,
                                                         Asplit[m * num_tiles + n], tile_size,
                                                         &eventgemm[m * num_tiles + n]));

                    //send tile to host (only if n = k+1)
                    if (n == k + 1) {
                        if (loc_verbose > 0)
                            printf("Sending tile[%d][%d] from card to host in queue %d, triggering event eventcpyfr[%d]\n",
                                   m, n, (int)(qindex), m * num_tiles + n);

                        hStreams_app_xfer_memory(
                            (int)(qindex),
                            Asplit[m * num_tiles + n],
                            Asplit[m * num_tiles + n], mem_size_tile,
                            HSTR_SINK_TO_SRC,
                            &eventcpyfr[m * num_tiles + n]);
                    }


                    q_syrk_gemm[ic]++;
                }
            }
        }


        //syncrhonizing all the streams
        hStreams_app_thread_sync();

        //end of timing
        tend = dtimeGet();

        totTimeMsec[iter] = 1e3 * (tend - tbegin);
        printf("time for Tiled hstreams Cholesky for iteration %d = %.2f msec\n",
               iter, totTimeMsec[iter]);

        //assembling of tiles back into full matrix
        assemble(Asplit, A_my, num_tiles, tile_size, mat_size, layRow);

        //calling mkl cholesky for verification and timing comparison.
        //Using auto-offload feature of MKL
        tbegin = dtimeGet();

        //calling MKL dpotrf on the full matrix
        info = LAPACKE_dpotrf(lapackLay, 'L', mat_size, A_MKL, mat_size);

        tend = dtimeGet();
        totTimeMsecMKL[iter] = 1e3 * (tend - tbegin);
        printf("time for MKL Cholesky (AO) for iteration %d = %.2f msec\n",
               iter, totTimeMsecMKL[iter]);

        if (info != 0) {
            printf("error with dpotrf\n");
        }
        mkl_mic_disable();

        if (verify == 1) {
            result = verify_results(A_my, A_MKL, mat_size * mat_size);
            if (result == true) {
                printf("Tiled Cholesky successful\n");
            } else {
                printf("Tiled Chloesky failed\n");
            }
        }
    }

    double meanTimeMsec, stdDevMsec;
    double meanTimeMsecMKL, stdDevMsecMKL;
    mean_and_stdev(totTimeMsec, meanTimeMsec, stdDevMsec, niter);
    mean_and_stdev(totTimeMsecMKL, meanTimeMsecMKL, stdDevMsecMKL, niter);

    double gflops = pow(mat_size, 3.0) / 3.0 * 1e-9;

    printf("\nMatrix size = %d\n", mat_size);

    printf("Tiled hStreams Cholesky: for %d iterations (ignoring first),\n"
           "mean Time = %.2f msec, stdDev Time = %.2f msec,\n"
           "Mean Gflops (using mean Time) = %.2f\n",
           niter - 1, meanTimeMsec, stdDevMsec, gflops / (meanTimeMsec * 1e-3));

    printf("\nMKL AO Cholesky: for %d iterations (ignoring first),\n"
           "mean Time = %.2f msec, stdDev Time = %.2f msec,\n"
           "Mean Gflops (using meanTime) = %.2f\n\n",
           niter - 1, meanTimeMsecMKL, stdDevMsecMKL, gflops / (meanTimeMsecMKL * 1e-3));

    //Free
    free(A_my);
    free(A_MKL);
    for (int i = 0; i < tot_tiles; ++i) {
        _mm_free(Asplit[i]);
    }
    delete [] Asplit;
    delete [] eventcpyto;
    delete [] eventcpyto_trsm;
    delete [] eventcpyfr;
    delete [] eventpotrf;
    delete [] eventtrsm;
    delete [] eventsyrk;
    delete [] eventgemm;
    delete [] totTimeMsec;
    delete [] totTimeMsecMKL;

    // true result indicates all OK
    if (result) {
        return 0;
    }
    return 1;

}

int main(int argc, char **argv)
{
    HSTR_OPTIONS hstreams_options;
    CHECK_HSTR_RESULT(hStreams_GetCurrentOptions(&hstreams_options, sizeof(HSTR_OPTIONS)));

    char *libNames[200] = {NULL, NULL};

    //Library to be loaded for sink-side code
    libNames[0] = "cholesky_sink_1.so";
    hstreams_options.libNameCnt = 1;
    hstreams_options.libNames = libNames;
    hstreams_options.libFlags = NULL;

    hstreams_options.libNameCntHost = 0;
    hstreams_options.libNamesHost = NULL;

    int mat_size_m, num_tiles, niter, tile_size;
    niter = 5;
    num_tiles = 1;
    mat_size_m = 0; //must be an input
    bool layRow = true;

    //max_log_str defines the no. of physical partitions on the card
    int use_host = 1, num_mics = 1;
    int nstreams_host = 4, nstreams_mic = 4;

    int verify = 1;

    CHECK_HSTR_RESULT(hStreams_SetOptions(&hstreams_options));
    for (int i = 1; i < argc; i++) {
        if (*argv[i] == SWITCH_CHAR) {
            switch (*(argv[i] + 1)) {
            case 'm':
                mat_size_m = (int)atol(argv[i] + 3);
                break;

            case 't':
                num_tiles = (int)atol(argv[i] + 3);
                break;

            case 's':
                nstreams_mic = (int)atol(argv[i] + 3);
                break;

            case 'l':
                if ((strcmp("row", argv[i] + 3) == 0) ||
                        (strcmp("ROW", argv[i] + 3) == 0)) {
                    layRow = true;
                    printf("matrix is in Row major format\n");
                } else {
                    layRow = false;
                    printf("matrix is in Col major format\n");
                }
                break;

            case 'i':
                niter = (int)atol(argv[i] + 3);
                //if( niter < 3 ) niter=3;
                break;

            case 'h':
                use_host = (int)atol(argv[i] + 3);
                break;

            case 'c':
                num_mics = (int)atol(argv[i] + 3);
                break;

            case 'v':
                verify = (int)atol(argv[i] + 3);
                break;

            default:
                break;
            }
        }
    }
    dtimeInit();

    //Check that mat_size is divisible by num_tiles
    if (mat_size_m % num_tiles != 0) {
        printf("matrix size MUST be divisible by num_tiles.. aborting\n");
        exit(0);
    }

    if (mat_size_m == 0) {
        printf("mat_size_m is not defined\n");
        exit(0);
    }

    tile_size = mat_size_m / num_tiles;

    //This allocates memory for the full input matrix
    double *A = (double *)malloc(mat_size_m * mat_size_m * sizeof(double));

    //Generate a symmetric positve-definite matrix
    A = dpo_generate(mat_size_m);

    int num_doms = use_host + num_mics;
    int max_log_str;

    if (use_host == 0 && num_mics == 0) {
        printf("Cannot run if not using either host or MIC cards\n");
        exit(-1);
    }

    if (use_host == 1) {
        printf("Using the host CPU for compute.. and\n");
    }
    printf("Using %d MIC cards for compute..\n", num_mics);

    if (use_host == 1 && num_mics >= 1) {
#ifdef HOST_HT_ON
        nstreams_host = 2 * nstreams_mic;
#else
        nstreams_host = nstreams_mic;
#endif
        max_log_str = nstreams_mic;
    } else if (num_mics == 0) {
        nstreams_host = nstreams_mic;
        max_log_str = nstreams_host;
#ifdef HOST_HT_ON
        nstreams_host = 2 * nstreams_host;
#endif
    } else if (use_host == 0) {
        max_log_str = nstreams_mic;
    }

    int host_ht_offset = 0;

#ifdef HOST_HT_ON
    host_ht_offset = nstreams_host - max_log_str;
#endif

    if (use_host) {
        printf("number of streams used on host = %d\n", nstreams_host);
        if (loc_verbose) {
            printf("if HT is enabled on host, only top half streams will be used\n");
            printf("if number of streams on host do not evenly divide with number of cores, performance can suffer\n");
        }
    }
    if (num_mics >= 1) {
        printf("number of streams used on mic = %d\n", nstreams_mic);
    }

    if (use_host == 1) {
        resv_cpu_master = 1;
        mach_wide_league = 1;
    } else {
        resv_cpu_master = 0;
        mach_wide_league = 0;
    }

    HSTR_PHYS_DOM *physDomID = new HSTR_PHYS_DOM[num_doms];
    HSTR_LOG_DOM *logDomID = new HSTR_LOG_DOM[num_doms + 1]; //+1 for creating a machine wide stream

    HSTR_CPU_MASK out_CPUmask, src_hstr_cpu_mask;
    HSTR_PHYS_DOM *out_pPhysDomainID = new HSTR_PHYS_DOM;
    HSTR_OVERLAP_TYPE *out_pOverlap = new HSTR_OVERLAP_TYPE;

    uint32_t *places = new uint32_t[num_doms];

    for (int i = 0; i < num_doms; ++i) {
        if (i == 0) {
            if (use_host == 1) {
                places[i] = nstreams_host;
                physDomID[i] = -1;
            } else {
                places[i] = nstreams_mic;
                physDomID[i] = i;
            }
        } else {
            places[i] = nstreams_mic;
            if (use_host == 1) {
                physDomID[i] = i - 1;
            } else {
                physDomID[i] = i;
            }
        }
    }

    if (resv_cpu_master) {
        HostCPUMask host_cpu_mask;
        host_cpu_mask.cpu_zero();

        for (int i = 0; i < num_resv_cpus; ++i) {
            host_cpu_mask.cpu_set(resv_cpus[i]);
        }

        int ret;
        HSTR_CPU_MASK_ZERO(src_hstr_cpu_mask);
        ret = setCurrentProcessAffinityMask(host_cpu_mask);
        if (ret != 0) {
            printf("setCurrentProcessAffinityMask failed, counted Gflops values may be inaccurate.");
        }

        int first, last, num_set;
        last = 0;
        first = HSTR_MAX_THREADS;
        num_set = 0;
        for (int i = 0; i < HSTR_MAX_THREADS; i++) {
            if (host_cpu_mask.cpu_isset(i)) {
                if (i < first) {
                    first = i;
                }
                last = i;
                num_set++;
                HSTR_CPU_MASK_SET(i, src_hstr_cpu_mask);
            }
        }
        if (loc_verbose) {
            printf("Reserving the following cpu_set for master on CPU\n");
            ShowLimitCPUmask(src_hstr_cpu_mask);
        }
    }

    uint32_t str_offset = 0;
    uint32_t places_mach_wide = 1;
    int iret;

    //create a machine wide stream on host for potrf
    if (mach_wide_league) {
        if (resv_cpu_master) {
            iret = hStreams_custom_init_selected_domains(
                       1,
                       physDomID,
                       1,
                       &places_mach_wide,
                       1,
                       str_offset,
                       src_hstr_cpu_mask);
        } else {
            iret = hStreams_app_init_selected_domains(
                       1,
                       physDomID,
                       1,
                       &places_mach_wide,
                       1,
                       str_offset);
        }
        str_offset = 1;
    }

    //create rest of the streams
    if (resv_cpu_master) {
        iret = hStreams_custom_init_selected_domains(
                   num_doms,
                   physDomID,
                   num_doms,
                   places,
                   1,
                   str_offset,
                   src_hstr_cpu_mask);
    } else {
        iret = hStreams_app_init_selected_domains(
                   num_doms,
                   physDomID,
                   num_doms,
                   places,
                   1,
                   str_offset);
    }
    if (iret != 0) {
        printf("hstreams_app_init failed!\r\n");
        exit(-1);
    }

    mkl_mic_disable();
    //10 max streams for printout
    HSTR_LOG_STR *out_pLogStreamIDs = new HSTR_LOG_STR[10];

    if (loc_verbose) {
        if (mach_wide_league) {
            //host
            CHECK_HSTR_RESULT(hStreams_GetLogDomainIDList(physDomID[0], 2, &logDomID[0]));
            for (int idom = 0; idom < 2; ++idom) {
                CHECK_HSTR_RESULT(hStreams_GetLogDomainDetails(logDomID[idom], out_pPhysDomainID, out_CPUmask));
                //ShowLimitCPUmask(out_CPUmask);
                if (idom == 0) {
                    CHECK_HSTR_RESULT(hStreams_GetLogStreamIDList(logDomID[idom], 1, out_pLogStreamIDs));
                } else {
                    CHECK_HSTR_RESULT(hStreams_GetLogStreamIDList(logDomID[idom], places[0], out_pLogStreamIDs));
                }
                if (idom > 0) {
                    for (int i = 0; i < places[0]; ++i) {
                        CHECK_HSTR_RESULT(hStreams_GetLogStreamDetails(out_pLogStreamIDs[i], logDomID[idom], out_CPUmask));
                        printf("streamId = %d\n", (int)out_pLogStreamIDs[i]);
                        ShowLimitCPUmask(out_CPUmask);
                    }
                } else {
                    CHECK_HSTR_RESULT(hStreams_GetLogStreamDetails(out_pLogStreamIDs[0], logDomID[idom], out_CPUmask));
                    printf("streamId = %d\n", (int)out_pLogStreamIDs[0]);
                    ShowLimitCPUmask(out_CPUmask);
                }
            }

            for (int idom = 1; idom < num_doms; ++idom) {
                CHECK_HSTR_RESULT(hStreams_GetLogDomainIDList(physDomID[idom], 1, &logDomID[idom]));
                CHECK_HSTR_RESULT(hStreams_GetLogDomainDetails(logDomID[idom], out_pPhysDomainID, out_CPUmask));
                //ShowLimitCPUmask(out_CPUmask);
                CHECK_HSTR_RESULT(hStreams_GetLogStreamIDList(logDomID[idom], places[idom], out_pLogStreamIDs));
                for (int i = 0; i < places[idom]; ++i) {
                    CHECK_HSTR_RESULT(hStreams_GetLogStreamDetails(out_pLogStreamIDs[i], logDomID[idom], out_CPUmask));
                    printf("streamId = %d\n", (int)out_pLogStreamIDs[i]);
                    ShowLimitCPUmask(out_CPUmask);
                }
            }
        } else {
            for (int idom = 0; idom < num_doms; ++idom) {
                CHECK_HSTR_RESULT(hStreams_GetLogDomainIDList(physDomID[idom], 1, &logDomID[idom]));
                CHECK_HSTR_RESULT(hStreams_GetLogDomainDetails(logDomID[idom], out_pPhysDomainID, out_CPUmask));
                //ShowLimitCPUmask(out_CPUmask);
                CHECK_HSTR_RESULT(hStreams_GetLogStreamIDList(logDomID[idom], places[idom], out_pLogStreamIDs));
                for (int i = 0; i < places[idom]; ++i) {
                    CHECK_HSTR_RESULT(hStreams_GetLogStreamDetails(out_pLogStreamIDs[i], logDomID[idom], out_CPUmask));
                    printf("streamId = %d\n", (int)out_pLogStreamIDs[i]);
                    ShowLimitCPUmask(out_CPUmask);
                }
            }
        }
    }

    //Calling the tiled Cholesky function. This does the factorization of the full matrix using a tiled implementation.
    int cholesky_code = cholesky_tiled(A, tile_size, num_tiles, mat_size_m, niter,
                                       max_log_str, layRow, verify, num_doms, use_host, num_mics, host_ht_offset);

    CHECK_HSTR_RESULT(hStreams_app_fini());

    free(A);

    return cholesky_code;
}
