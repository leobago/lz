/*
 * =====================================================================================
 *
 *       Filename:  example.c
 *
 *    Description:  Example code to test the lz floating point compression library
 *
 *        Version:  1.0
 *        Created:  04/16/2014 09:23:33 AM CDT
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Leonardo A. Bautista Gomez (leobago@anl.gov),
 *        Company:  Argonne National Laboratory
 *
 * =====================================================================================
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "lz.h"


int printResults(short lossy, ulong inSize, ulong outSize, float tt, int comp)
{
    int mb = 1024*1024, out = outSize;
    if (comp) out = inSize;
    printf("| %d bytes | %4ld MB | %4ld MB | %3ld %% | %04.1f s | %05.1f MB/s |\n",
            lossy, inSize/mb, outSize/mb, (outSize*100)/inSize, tt, out/(mb*tt));
    return EXIT_SUCCESS;
}


int createFileDouble(char *pSrcFn, int sizeInMB, int prec)
{
    int i;
    time_t seed = time(NULL);
    srand(seed);
    double point = 300.0;
    if ((prec != 4) && (prec != 8)) return EXIT_FAILURE;
    long nbPoints = (sizeInMB*1024*1024)/prec;
    FILE *pf = fopen(pSrcFn, "wb");
    for (i = 0; i < nbPoints; i++)
    {
        point = point+(((rand()%1000)/1000.0)*((rand()%3)-1));
        if (prec == 4)
        {
            float fpoint = (float) point;
            fwrite(&fpoint, 1, prec, pf);
        } else {
            fwrite(&point, 1, prec, pf);
        }
    }
    fclose(pf);
    return EXIT_SUCCESS;
}

int compressFile(char *pSrcFn, char *pDstFn, int level)
{
    struct timeval start, end;
    ulong outSize, inSize;

    FILE *pFile = fopen(pSrcFn, "rb");
    if (pFile == NULL)
    {
        printf("Failed to open input file.");
        return EXIT_FAILURE;
    }
    fseek(pFile, 0, SEEK_END);
    inSize = ftell(pFile);
    fclose(pFile);

    uchar *srcBuf = malloc(inSize);
    uchar *dstBuf = malloc(inSize);
    pFile = fopen(pSrcFn, "rb");
    if (pFile == NULL)
    {
        printf("Failed to open input file.");
        return EXIT_FAILURE;
    }
    fread(srcBuf, 1, inSize, pFile);
    fclose(pFile);
    gettimeofday(&start, NULL);
    lzCompress(dstBuf, &outSize, srcBuf, inSize, level);
    gettimeofday(&end, NULL);
    pFile = fopen(pDstFn, "wb");
    if (pFile == NULL)
    {
        printf("Failed to open input file.");
        return EXIT_FAILURE;
    }
    fwrite(&inSize, sizeof(ulong), 1, pFile);
    fwrite(dstBuf, 1, outSize, pFile);
    fclose(pFile);
    float tt = end.tv_sec-start.tv_sec+((end.tv_usec-start.tv_usec)/1000000.0);
    printf("*     Compressing   ");
    printResults(0, inSize, outSize, tt, 1);
    free(srcBuf);
    free(dstBuf);
    return EXIT_SUCCESS;
}


int uncompressFile(char *pDstFn, char *pUcmFn)
{
    struct timeval start, end;
    ulong outSize, inSize;

    FILE *pFile = fopen(pDstFn, "rb");
    if (pFile == NULL)
    {
        printf("Failed to open input file.");
        return EXIT_FAILURE;
    }
    fread(&outSize, sizeof(ulong), 1, pFile);
    fseek(pFile, 0, SEEK_END);
    inSize = ftell(pFile);
    fclose(pFile);

    uchar *srcBuf = malloc(inSize);
    uchar *dstBuf = malloc(outSize);
    pFile = fopen(pDstFn, "rb");
    if (pFile == NULL)
    {
        printf("Failed to open input file.");
        return EXIT_FAILURE;
    }
    fread(&outSize, sizeof(ulong), 1, pFile);
    fread(srcBuf, 1, inSize, pFile);
    fclose(pFile);
    gettimeofday(&start, NULL);
    lzUncompress(dstBuf, &outSize, srcBuf, inSize);
    gettimeofday(&end, NULL);
    pFile = fopen(pUcmFn, "wb");
    if (pFile == NULL)
    {
        printf("Failed to open input file.");
        return EXIT_FAILURE;
    }
    fwrite(dstBuf, 1, outSize, pFile);
    fclose(pFile);
    float tt = end.tv_sec-start.tv_sec+((end.tv_usec-start.tv_usec)/1000000.0);
    printf("*   Uncompressing   ");
    printResults(0, inSize, outSize, tt, 0);
    free(srcBuf);
    free(dstBuf);

    return EXIT_SUCCESS;
}


int lzCompressFile(char *pSrcFn, char *pDstFn, int prec, short level, short lossy)
{
    struct timeval start, end;
    ulong outSize, inSize, nbEle;

    if ((prec != 4) && (prec != 8)) return EXIT_FAILURE;
    FILE *pFile = fopen(pSrcFn, "rb");
    if (pFile == NULL)
    {
        printf("Failed to open input file.");
        return EXIT_FAILURE;
    }
    fseek(pFile, 0, SEEK_END);
    inSize = ftell(pFile);
    nbEle = inSize/prec;
    fclose(pFile);

    uchar *dstBuf = malloc(inSize+88);
    pFile = fopen(pSrcFn, "rb");
    if (pFile == NULL)
    {
        printf("Failed to open input file.");
        return EXIT_FAILURE;
    }
    if (prec == 4)
    {
        float *daBuf = malloc(inSize);
        fread(daBuf, prec, nbEle, pFile);
        gettimeofday(&start, NULL);
        lzCompressFloat(dstBuf, &outSize, daBuf, nbEle, level, lossy);
        gettimeofday(&end, NULL);
        free(daBuf);
    } else {
        double *daBuf = malloc(inSize);
        fread(daBuf, prec, nbEle, pFile);
        gettimeofday(&start, NULL);
        lzCompressDouble(dstBuf, &outSize, daBuf, nbEle, level, lossy);
        gettimeofday(&end, NULL);
        free(daBuf);
    }
    fclose(pFile);
    pFile = fopen(pDstFn, "wb");
    if (pFile == NULL)
    {
        printf("Failed to open input file.");
        return EXIT_FAILURE;
    }
    fwrite(dstBuf, 1, outSize, pFile);
    fclose(pFile);
    float tt = end.tv_sec-start.tv_sec+((end.tv_usec-start.tv_usec)/1000000.0);
    printf("*   lzCompressing   ");
    printResults(lossy, inSize, outSize, tt, 1);
    free(dstBuf);
    return EXIT_SUCCESS;
}


int lzUncompressFile(char *pDstFn, char *pUcmFn, int prec)
{
    struct timeval start, end;
    ulong outSize, inSize, darSize;

    if ((prec != 4) && (prec != 8)) return EXIT_FAILURE;
    FILE *pFile = fopen(pDstFn, "rb");
    if (pFile == NULL)
    {
        printf("Failed to open input file.");
        return EXIT_FAILURE;
    }
    fread(&outSize, 1, sizeof(ulong), pFile);
    fseek(pFile, 0, SEEK_END);
    inSize = ftell(pFile);
    fclose(pFile);

    uchar *srcBuf = malloc(inSize);
    pFile = fopen(pDstFn, "rb");
    if (pFile == NULL)
    {
        printf("Failed to open input file.");
        return EXIT_FAILURE;
    }
    fread(srcBuf, 1, inSize, pFile);
    fclose(pFile);
    if (prec == 4)
    {
        float *daBuf = malloc(outSize);
        gettimeofday(&start, NULL);
        lzUncompressFloat(daBuf, &darSize, srcBuf, inSize);
        gettimeofday(&end, NULL);
        pFile = fopen(pUcmFn, "wb");
        if (pFile == NULL)
        {
            printf("Failed to open input file.");
            return EXIT_FAILURE;
        }
        fwrite(daBuf, prec, darSize, pFile);
        fclose(pFile);
        free(daBuf);
    } else {
        double *daBuf = malloc(outSize);
        gettimeofday(&start, NULL);
        lzUncompressDouble(daBuf, &darSize, srcBuf, inSize);
        gettimeofday(&end, NULL);
        pFile = fopen(pUcmFn, "wb");
        if (pFile == NULL)
        {
            printf("Failed to open input file.");
            return EXIT_FAILURE;
        }
        fwrite(daBuf, prec, darSize, pFile);
        fclose(pFile);
        free(daBuf);
    }
    outSize = darSize*prec;
    float tt = end.tv_sec-start.tv_sec+((end.tv_usec-start.tv_usec)/1000000.0);
    printf("* lzUncompressing   ");
    printResults(0, inSize, outSize, tt, 0);
    free(srcBuf);

    return EXIT_SUCCESS;
}


int main(int argc, char *argv[])
{
    int res, level = 1, size = 1024, prec = sizeof(double);
    char pSrcFn[64], pMzpFn[64], pLzpFn[64], pUmzFn[64], pUlzFn[64];

    if (argc == 2) {
        sprintf(pSrcFn, "doubleDataset");
        prec = atoi(argv[1]);
        if ((prec != 32) && (prec != 64))
        {
            printf("Precision needs to be equal to 32 or 64\n");
            return EXIT_FAILURE;
        }
        prec = prec/8;
        res = createFileDouble(pSrcFn, size, prec);
        if (res == EXIT_FAILURE)
        {
            printf("Error creating floating point data file\n");
            return EXIT_FAILURE;
        }
    } else {
        if (argc == 3) {
            sprintf(pSrcFn, "%s", argv[1]);
            prec = atoi(argv[2]);
            if ((prec != 32) && (prec != 64))
            {
                printf("Precision needs to be equal to 32 or 64\n");
                return EXIT_FAILURE;
            }
            prec = prec/8;
        } else {
            printf("Usage: \n");
            printf("   ./example precision(32 or 64)\n");
            printf("   ./example filename precision(32 or 64)\n");
            return EXIT_FAILURE;
        }
    }

    sprintf(pMzpFn, "%s.mz",  pSrcFn);
    sprintf(pLzpFn, "%s.lz",  pSrcFn);
    sprintf(pUmzFn, "%s.umz", pSrcFn);
    sprintf(pUlzFn, "%s.ulz", pSrcFn);
    printf("Original file %s, %d MB of size, precision %d bits and compression level %d\n", pSrcFn, size, prec*8, level);

    printf("=================================================================================\n");
    printf("                    |  Lossy  |  Input  |  Output | Ratio |  Time  | Troughput  |\n");
    printf("=================================================================================\n");
    res = lzCompressFile(pSrcFn, pLzpFn, prec, level, 2);
    if (res == EXIT_FAILURE) return EXIT_FAILURE;

    res = lzUncompressFile(pLzpFn, pUlzFn, prec);
    if (res == EXIT_FAILURE) return EXIT_FAILURE;

    res = lzCompressFile(pSrcFn, pLzpFn, prec, level, 1);
    if (res == EXIT_FAILURE) return EXIT_FAILURE;

    res = lzUncompressFile(pLzpFn, pUlzFn, prec);
    if (res == EXIT_FAILURE) return EXIT_FAILURE;

    res = lzCompressFile(pSrcFn, pLzpFn, prec, level, 0);
    if (res == EXIT_FAILURE) return EXIT_FAILURE;

    res = lzUncompressFile(pLzpFn, pUlzFn, prec);
    if (res == EXIT_FAILURE) return EXIT_FAILURE;

    res = compressFile(pSrcFn, pMzpFn, level);
    if (res == EXIT_FAILURE) return EXIT_FAILURE;

    res = uncompressFile(pMzpFn, pUmzFn);
    if (res == EXIT_FAILURE) return EXIT_FAILURE;


    return EXIT_SUCCESS;
}



