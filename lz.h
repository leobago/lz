/*
 * =====================================================================================
 *
 *       Filename:  lz.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  04/15/2014 04:44:03 PM CDT
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:   (),
 *        Company:
 *
 * =====================================================================================
 */

#ifndef  _LZ_H
#define  _LZ_H

#ifdef __cplusplus
extern "C" {
#endif

#define FORCE_COMP          1
#define VERBOSE             0
#define BYTE_FREQ           1
#define MAX_LEVEL           9
#define LIT_ENDIAN          1
#define MAX_STATS           10000
#define BUF_SIZE            (1024 * 1024)
#define compress            mz_compress
#define compress2           mz_compress2
#define uncompress          mz_uncompress

typedef unsigned long ulong;
typedef unsigned char uchar;
typedef unsigned short ushort;

typedef union ldouble
{
    double value;
    int half[2];
    char byte[8];
} ldouble;

typedef union lfloat
{
    float value;
    int half;
    char byte[4];
} lfloat;

extern int   compress(uchar *pDest, ulong *pDest_len, uchar *pSource, ulong source_len);
extern int  compress2(uchar *pDest, ulong *pDest_len, uchar *pSource, ulong source_len, int level);
extern int uncompress(uchar *pDest, ulong *pDest_len, uchar *pSource, ulong source_len);

extern int         lzCompress(uchar *dstBuf, ulong *outSize, uchar *srcBuf, ulong inSize, short level);
extern int       lzUncompress(uchar *dstBuf, ulong *outSize, uchar *srcBuf, ulong inSize);
extern int    lzCompressFloat(uchar *dstBuf, ulong *outSize, float *darBuf, ulong daSize, short level, short lossy);
extern int  lzUncompressFloat(float *darBuf, ulong *darSize, uchar *srcBuf, ulong inSize);
extern int   lzCompressDouble(uchar *dstBuf, ulong *outSize, double *daBuf, ulong daSize, short level, short lossy);
extern int lzUncompressDouble(double *daBuf, ulong *darSize, uchar *srcBuf, ulong inSize);

#ifdef __cplusplus
}
#endif

#endif   /* ----- #ifndef _LZ_H  ----- */

