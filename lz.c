/*
 * =====================================================================================
 *
 *       Filename:  lz.c
 *
 *    Description:  Floating point compression library
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "lz.h"


int lzCompress(uchar *dstBuf, ulong *outSize, uchar *srcBuf, ulong inSize, short level)
{
    *outSize = 2*inSize;
    return compress2(dstBuf, outSize, srcBuf, inSize, level);
}


int lzUncompress(uchar *dstBuf, ulong *outSize, uchar *srcBuf, ulong inSize)
{
    return uncompress(dstBuf, outSize, srcBuf, inSize);
}


int entropyAnalysis(uchar *tmpBuf, ulong size)
{
    ulong i = 0, code = 0, step = 1, max = 0, limit = size;
    float byteStats[256];
    if (size > MAX_STATS)
    { // Analyse maximum MAX_STATS bytes
        step = size/MAX_STATS;
        limit = MAX_STATS;
    }
    for (i = 0; i < 256; i++) byteStats[i] = 0.0;
    while (i < size)
    { // Count how many ocurrences for each of the 256 options
        byteStats[tmpBuf[i]] = byteStats[tmpBuf[i]] + 1;
        i = i + step;
    }
    for (i = 0; i < 256; i++)
    { // Convert counters into percentage and get percentage of the most frequent
        byteStats[i] = (byteStats[i]*100.0)/limit;
        if (byteStats[i] > max) max = byteStats[i];
        //printf(" %3.1f ", byteStats[i]);
    }
    if (max >= BYTE_FREQ)
    { // Convert maximum frequency to compression level (Not used now)
        if (max >= MAX_LEVEL)
        {
            code = 9;
        } else {
            code = MAX_LEVEL - max;
        }
    }
    return code;
}

int getCode(int code[8], ushort prec, short lossy)
{
    int i;
    if (VERBOSE) printf("Lossy : %d  ", lossy);
    if (VERBOSE) printf(" -Byte compression layout: ");
    for (i = 0; i < prec; i++)
    {
        if ((lossy/8) >= (i+1))
        {
            if (LIT_ENDIAN) code[i] = -1;
            else code[prec-(i+1)] = -1;
        }
        if ((FORCE_COMP) && code[i] != -1) code[i] = 1;
        if (((lossy/8) == i) && ((lossy%8) != 0)) code[i] = 2;
        if (VERBOSE) printf("%d ", code[i]);
    }
    if (VERBOSE) printf("\n");
    return EXIT_SUCCESS;
}

int maskArray(uchar *tmpBuf, ulong offset, short lossy)
{
    unsigned int i;
    char preset[8] =   {255, 254, 252, 248, 240, 224, 192, 128};
    unsigned char mask = preset[lossy%8];
    if (VERBOSE) printf("Mask : %02X \n",mask);
    for(i = 0; i < offset; i++) tmpBuf[i] = tmpBuf[i] & mask;
    return EXIT_SUCCESS;
}

int lzCompressFlopnt(
        uchar *dstBuf, 
        ulong *outSize, 
        uchar **tmpBuf, 
        ulong offset, 
        ushort prec, 
        short level, 
        short lossy )
{
    struct timeval start, end;
    ulong finalSize;
    float t0 = 0, t1 = 0, t2 = 0;
    int i, code[8];
    if ((prec != 4) && (prec != 8)) return EXIT_FAILURE;
    if ((level < 1) || (level > MAX_LEVEL)) return EXIT_FAILURE;
    if ((lossy < 0) || (lossy > (prec*8))) return EXIT_FAILURE;
    memcpy(dstBuf, outSize, sizeof(ulong));
    finalSize = sizeof(ulong);
    memcpy(dstBuf+finalSize, &lossy, sizeof(short));
    finalSize = finalSize + sizeof(short);
    getCode(code, prec, lossy);
    for (i = 0; i < prec; i++)
    {
        gettimeofday(&start, NULL);
        if (!FORCE_COMP) code[i] = entropyAnalysis(tmpBuf[i], offset);
        gettimeofday(&end, NULL);
        t0 = t0 + (end.tv_sec-start.tv_sec)+((end.tv_usec-start.tv_usec)/1000000.0);
        memcpy(dstBuf+finalSize, code+i, sizeof(int));
        finalSize = finalSize + sizeof(int);
        if (code[i] > 0)
        { // Bytes that need to be compressed
            if (code[i] == 2) maskArray(tmpBuf[i], offset, lossy);
            gettimeofday(&start, NULL);
            uchar *xtrBuf = malloc(2*offset);
            *outSize = 2*offset;
            memset(xtrBuf, 0, offset);
            int r = lzCompress(xtrBuf, outSize, tmpBuf[i], offset, level);
            //if (VERBOSE) printf(": %d, ", r);
            if (r < 0) return EXIT_FAILURE;
            memcpy(dstBuf+finalSize, outSize, sizeof(ulong));
            finalSize = finalSize + sizeof(ulong);
            memcpy(dstBuf+finalSize, xtrBuf, *outSize);
            finalSize = finalSize + *outSize;
            free(xtrBuf);
            gettimeofday(&end, NULL);
            t1 = t1 + (end.tv_sec-start.tv_sec)+((end.tv_usec-start.tv_usec)/1000000.0);
        } else {
            gettimeofday(&start, NULL);
            if (code[i] == 0)
            { // Bytes that are writen plain
                memcpy(dstBuf+finalSize, &offset, sizeof(ulong));
                finalSize = finalSize + sizeof(ulong);
                memcpy(dstBuf+finalSize, tmpBuf[i], offset);
                finalSize = finalSize + offset;
            } else { // Bytes that are lost
                memcpy(dstBuf+finalSize, &offset, sizeof(ulong));
                finalSize = finalSize + sizeof(ulong);
            }
            gettimeofday(&end, NULL);
            t2 = t2 + (end.tv_sec-start.tv_sec)+((end.tv_usec-start.tv_usec)/1000000.0);
        }
    }
    if (VERBOSE) printf("Entropy time: %f compressing time : %f Copy and discard time %f \n", t0, t1, t2);
    *outSize = finalSize;
    return EXIT_SUCCESS;
}


int lzUncompressFlopnt(uchar **tmpBuf, ulong offset, uchar *srcBuf, ulong inSize, ushort size)
{
    short lossy, i;
    int code[8];
    ulong parSize, outSize, finalSize = sizeof(ulong);
    memcpy(&lossy, srcBuf+finalSize, sizeof(short));
    finalSize = finalSize + sizeof(short);
    if (VERBOSE) printf(" -Byte decompression layout: ");
    for (i = 0; i < size; i++)
    {
        outSize = offset;
        memcpy(code+i, srcBuf+finalSize, sizeof(int));
        finalSize = finalSize + sizeof(int);
        if (VERBOSE) printf("%d ", code[i]);
        memcpy(&parSize, srcBuf+finalSize, sizeof(ulong));
        finalSize = finalSize + sizeof(ulong);
        if (code[i] > 0)
        {
            lzUncompress(tmpBuf[i], &outSize, srcBuf+finalSize, parSize);
            finalSize = finalSize + parSize;
        } else {
            if (code[i] == 0) {
                memcpy(tmpBuf[i], srcBuf+finalSize, parSize);
                finalSize = finalSize + parSize;
            } else {
                memset(tmpBuf[i], 0, parSize);
            }
        }
    }
    if (VERBOSE) printf("\n");
    if (finalSize != inSize) printf("Error while decoding array!\n");
    return EXIT_SUCCESS;
}


int lzCompressFloat(uchar *dstBuf, ulong *outSize, float *darBuf, ulong daSize, short level, short protect)
{
    lfloat lfBuf;
    uchar *tmpBuf[4];
    ushort j, size = sizeof(float);
    ulong i, offset = daSize;
    *outSize = offset*size;
    short lossy = (sizeof(float)*8)-protect;
    for (i = 0; i < size; i++) tmpBuf[i] = malloc(offset);
    for (i = 0; i < daSize; i++)
    {
        lfBuf.value = darBuf[i];
        for (j = 0; j < size; j++) memcpy(tmpBuf[j]+(i*sizeof(char)), &(lfBuf.byte[j]), sizeof(char));
    }
    lzCompressFlopnt(dstBuf, outSize, tmpBuf, offset, size, level, lossy);
    for (i = 0; i < size; i++) free(tmpBuf[i]);
    return EXIT_SUCCESS;
}


int lzUncompressFloat(float *darBuf, ulong *darSize, uchar *srcBuf, ulong inSize)
{
    lfloat lfBuf;
    uchar *tmpBuf[4];
    ushort j, size = sizeof(float);
    ulong i, offset;

    memcpy(&offset, srcBuf, sizeof(ulong));
    offset = offset/size;
    *darSize = offset;
    for (i = 0; i < size; i++) tmpBuf[i] = malloc(offset);
    lzUncompressFlopnt(tmpBuf, offset, srcBuf, inSize, size);
    for (i = 0; i < *darSize; i++)
    {
        for (j = 0; j < size; j++) memcpy(&(lfBuf.byte[j]), tmpBuf[j]+(i*sizeof(char)), sizeof(char));
        darBuf[i] = lfBuf.value;
    }
    for (i = 0; i < size; i++) free(tmpBuf[i]);
    return EXIT_SUCCESS;
}


int lzCompressDouble(uchar *dstBuf, ulong *outSize, double *daBuf, ulong daSize, short level, short protect)
{
    float t0, t1;
    ldouble ldBuf;
    uchar *tmpBuf[8];
    struct timeval start, end;
    short lossy = (sizeof(double)*8) - protect;
    ushort j, sChar = sizeof(char), size = sizeof(double);
    ulong i, offset = daSize;
    *outSize = offset*size;

    if (sChar != 1) return EXIT_FAILURE;
    gettimeofday(&start, NULL);
    for (i = 0; i < size; i++) tmpBuf[i] = malloc(offset);
    for (i = 0; i < daSize; i++)
    {
        ldBuf.value = daBuf[i];
        for (j = 0; j < size; j++) memcpy(tmpBuf[j]+i, &(ldBuf.byte[j]), sizeof(char));
        //for (j = 0; j < size; j++) tmpBuf[j][i] = ldBuf.byte[j];
    }
    gettimeofday(&end, NULL);
    t0 = end.tv_sec-start.tv_sec+((end.tv_usec-start.tv_usec)/1000000.0);
    gettimeofday(&start, NULL);
    lzCompressFlopnt(dstBuf, outSize, tmpBuf, offset, size, level, lossy);
    gettimeofday(&end, NULL);
    t1 = end.tv_sec-start.tv_sec+((end.tv_usec-start.tv_usec)/1000000.0);
    if (VERBOSE) printf("Reformatting time: %f, compression time : %f \n", t0, t1);
    for (i = 0; i < size; i++) free(tmpBuf[i]);
    return EXIT_SUCCESS;
}


int lzUncompressDouble(double *daBuf, ulong *darSize, uchar *srcBuf, ulong inSize)
{
    ldouble ldBuf;
    uchar *tmpBuf[8];
    ushort j, size = sizeof(double);
    ulong i, offset;

    memcpy(&offset, srcBuf, sizeof(ulong));
    offset = offset/size;
    *darSize = offset;
    for (i = 0; i < size; i++) tmpBuf[i] = malloc(offset);
    lzUncompressFlopnt(tmpBuf, offset, srcBuf, inSize, size);
    for (i = 0; i < *darSize; i++)
    {
        for (j = 0; j < size; j++) memcpy(&(ldBuf.byte[j]), tmpBuf[j]+(i*sizeof(char)), sizeof(char));
        daBuf[i] = ldBuf.value;
    }
    for (i = 0; i < size; i++) free(tmpBuf[i]);
    return EXIT_SUCCESS;
}


