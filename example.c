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
#include <math.h>
#include "lz.h"


int printResults(short lossy, ulong inSize, ulong outSize, float t1, float t2, double error)
{
    float mb = 1024.0*1024.0;
    printf("| %d bytes | %4.1f MB | %4.1f MB | %5.1f %% |  %05.1f s | %04.1f s | %7.4f |\n",
            lossy, inSize/mb, outSize/mb, (outSize*100.0)/inSize, t1, t2, error);
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

ulong getFileSize(char *pSrcFn)
{
    ulong size;
    FILE *pFile = fopen(pSrcFn, "rb");
    if (pFile == NULL)
    {
        printf("Failed to open input file.");
        return EXIT_FAILURE;
    }
    fseek(pFile, 0, SEEK_END);
    size = ftell(pFile);
    fclose(pFile);
    return size;
}

double compareFiles(char *pSrc1Fn, char *pSrc2Fn, int prec)
{
    ulong size1, size2;
    float maxF = 0;
    double maxD = 0;

    FILE *pFile1 = fopen(pSrc1Fn, "rb");
    if (pFile1 == NULL)
    {
        printf("Failed to open input file.");
        return EXIT_FAILURE;
    }
    fseek(pFile1, 0, SEEK_END);
    size1 = ftell(pFile1)/prec;
    fseek(pFile1, 0, SEEK_SET);

    FILE *pFile2 = fopen(pSrc2Fn, "rb");
    if (pFile2 == NULL)
    {
        printf("Failed to open input file.");
        return EXIT_FAILURE;
    }
    fseek(pFile2, 0, SEEK_END);
    size2 = ftell(pFile2)/prec;
    fseek(pFile2, 0, SEEK_SET);

    if (size1 != size2)
    {
        printf("The files have different sizes: %ld != %ld \n", size1, size2);
        return EXIT_FAILURE;
    }

    while (size1 > 0)
    {
        if (prec == 4)
        {
            float buf1, buf2;
            fread(&buf1, sizeof(float), 1, pFile1);
            fread(&buf2, sizeof(float), 1, pFile2);
            float diff = (float) fabs((double) (buf1 - buf2));
            size1 = size1 - 1;
            if (diff > maxF) maxF = diff;
        } else {
            double buf1, buf2;
            fread(&buf1, sizeof(double), 1, pFile1);
            fread(&buf2, sizeof(double), 1, pFile2);
            double diff = fabs(buf1 - buf2);
            if (diff > maxD) maxD = diff;
            size1 = size1 - 1;
        }
    }
    
    fclose(pFile1);
    fclose(pFile2);

    if (prec == 4)
    {
        return (double)maxF;
    } else {
        return maxD;
    }

    return EXIT_FAILURE;
}


float compressFile(char *pSrcFn, char *pDstFn, int level)
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
    free(srcBuf);
    free(dstBuf);
    return tt;
}


float uncompressFile(char *pDstFn, char *pUcmFn)
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
    free(srcBuf);
    free(dstBuf);

    return tt;
}


float lzCompressFile(char *pSrcFn, char *pDstFn, int prec, short level, short lossy)
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
    free(dstBuf);
    return tt;
}


float lzUncompressFile(char *pDstFn, char *pUcmFn, int prec)
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
    free(srcBuf);

    return tt;
}


int main(int argc, char *argv[])
{
    int i, res, level = 9, size = 1024, prec = sizeof(double);
    char pSrcFn[64], pCmzFn[64], pUmzFn[64], pClzFn[64], pUlzFn[64];
    float cmpTime, dcpTime;
    double error;
    ulong inSize, outSize;

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

    inSize = getFileSize(pSrcFn);
    sprintf(pCmzFn, "%s.cmz", pSrcFn);
    sprintf(pUmzFn, "%s.umz", pSrcFn);
    printf("Original file %s, precision %d bits and compression level %d\n", pSrcFn, prec*8, level);
    printf("===========================================================================================\n");
    printf("                    |  Lossy  |  Input  |  Output | C.Ratio | Compress | Decomp |  Error  |\n");
    printf("===========================================================================================\n");
 
    cmpTime = compressFile(pSrcFn, pCmzFn, level);
    if (res == EXIT_FAILURE) return EXIT_FAILURE;
    dcpTime = uncompressFile(pCmzFn, pUmzFn);
    if (res == EXIT_FAILURE) return EXIT_FAILURE;
    error = compareFiles(pSrcFn, pUmzFn, prec);
    outSize = getFileSize(pCmzFn);
    printf("* zip Compression   ");
    printResults(0, inSize, outSize, cmpTime, dcpTime, error);
   
    for(i = 0; i < 4; i++)
    {
        sprintf(pClzFn, "%s.clz%i", pSrcFn, i);
        sprintf(pUlzFn, "%s.ulz%i", pSrcFn, i);
        cmpTime = lzCompressFile(pSrcFn, pClzFn, prec, level, i);
        if (res == EXIT_FAILURE) return EXIT_FAILURE;
        dcpTime = lzUncompressFile(pClzFn, pUlzFn, prec);
        if (res == EXIT_FAILURE) return EXIT_FAILURE;
        error = compareFiles(pSrcFn, pUlzFn, prec);
        outSize = getFileSize(pClzFn);
        printf("*  lz Compression   ");
        printResults(i, inSize, outSize, cmpTime, dcpTime, error);
    }

    return EXIT_SUCCESS;
}



