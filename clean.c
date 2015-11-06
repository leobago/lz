/*
 * =====================================================================================
 *
 *       Filename:  compare.c
 *
 *    Description:  Example code t
 *
 *        Version:  1.0
 *        Created:  09/16/2015 09:23:33 AM CDT
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
#include <math.h>
#include <time.h>
#include "lz.h"


float reverseFloat( const float inFloat )
{
    float retVal;
    char *floatToConvert = ( char* ) & inFloat;
    char *returnFloat = ( char* ) & retVal;
    returnFloat[0] = floatToConvert[3];
    returnFloat[1] = floatToConvert[2];
    returnFloat[2] = floatToConvert[1];
    returnFloat[3] = floatToConvert[0];
    return retVal;
}

double reverseDouble( const float inDouble )
{
    double retVal;
    char *doubleToConvert = ( char* ) & inDouble;
    char *returnDouble = ( char* ) & retVal;
    returnDouble[0] = doubleToConvert[7];
    returnDouble[1] = doubleToConvert[6];
    returnDouble[2] = doubleToConvert[5];
    returnDouble[3] = doubleToConvert[4];
    returnDouble[4] = doubleToConvert[3];
    returnDouble[5] = doubleToConvert[2];
    returnDouble[6] = doubleToConvert[1];
    returnDouble[7] = doubleToConvert[0];
    return retVal;
}

int cleanFile(char *pSrc1Fn, double min, double max, int prec, int swap)
{
    double mi = max, ma = min;
    char pSrc2Fn[64];
    ulong size1;

    FILE *pFile1 = fopen(pSrc1Fn, "rb");
    if (pFile1 == NULL)
    {
        printf("Failed to open input file.");
        return EXIT_FAILURE;
    }
    fseek(pFile1, 0, SEEK_END);
    size1 = ftell(pFile1)/prec;
    fseek(pFile1, 0, SEEK_SET);

    sprintf(pSrc2Fn, "%s.cln", pSrc1Fn);
    FILE *pFile2 = fopen(pSrc2Fn, "w");
    if (pFile2 == NULL)
    {
        printf("Failed to open input file.");
        return EXIT_FAILURE;
    }

    while (size1 > 0)
    {
        if (prec == 4)
        {
            float buf;
            fread(&buf, sizeof(float), 1, pFile1);
            if (swap) buf = reverseFloat(buf);
            if (buf >= (float)min && buf <= (float)max)
            {
                if (buf > ma) ma = buf;
                if (buf < mi) mi = buf;
                fwrite(&buf, sizeof(float), 1, pFile2);
                //printf("%f \n", buf);
            }
        } else {
            double buf;
            fread(&buf, sizeof(double), 1, pFile1);
            if (swap) buf = reverseDouble(buf);
            if (buf >= min && buf <= max)
            {
                if (buf > ma) ma = buf;
                if (buf < mi) mi = buf;
                fwrite(&buf, sizeof(double), 1, pFile2);
                //printf("%f \n", buf);
            }
        }
        size1 = size1 - 1;
    }
    printf("All values between %f and %f\n", mi, ma);
    fclose(pFile1);
    fclose(pFile2);
    return EXIT_SUCCESS;
}


int main(int argc, char *argv[])
{
    int swap, res, prec = sizeof(double);
    double min, max;
    char pSrcFn[64];

    if (argc == 6) {
        sprintf(pSrcFn, "%s", argv[1]);
        min = atoi(argv[2]);
        max = atoi(argv[3]);
        prec = atoi(argv[4]);
        swap = atoi(argv[5]);
        if ((prec != 32) && (prec != 64))
        {
            printf("Precision needs to be equal to 32 or 64\n");
            return EXIT_FAILURE;
        }
        if ((swap != 0) && (swap != 1))
        {
            printf("Swap needs to be equal to 0 or 1\n");
            return EXIT_FAILURE;
        }
        prec = prec/8;
    } else {
        printf("Usage: \n");
        printf("   ./clean filename1 Min Max precision(32 or 64) swap_endianness(1 or 0))\n");
        return EXIT_FAILURE;
    }

    printf("Files %s in %d bits precision.\n", pSrcFn, prec*8);
    res = cleanFile(pSrcFn, min, max, prec, swap);
    if (res == EXIT_FAILURE) return EXIT_FAILURE;
    
    return EXIT_SUCCESS;
}



