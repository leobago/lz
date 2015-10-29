/*
 * =====================================================================================
 *
 *       Filename:  compare.c
 *
 *    Description:  Example code to test the lz floating point compression library
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


int compareFiles(char *pSrc1Fn, char *pSrc2Fn, int prec)
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
        printf("Max. error is %f \n", maxF);
    } else {
        printf("Max. error is %f \n", maxD);
    }

    return EXIT_SUCCESS;
}


int main(int argc, char *argv[])
{
    int res, prec = sizeof(double);
    char pSrc1Fn[64], pSrc2Fn[64];

    if (argc == 4) {
        sprintf(pSrc1Fn, "%s", argv[1]);
        sprintf(pSrc2Fn, "%s", argv[2]);
        prec = atoi(argv[3]);
        if ((prec != 32) && (prec != 64))
        {
            printf("Precision needs to be equal to 32 or 64\n");
            return EXIT_FAILURE;
        }
        prec = prec/8;
    } else {
        printf("Usage: \n");
        printf("   ./compare filename1 filename2 precision(32 or 64)\n");
        return EXIT_FAILURE;
    }

    printf("Files %s and %s in %d bits precision.\n", pSrc1Fn, pSrc2Fn, prec*8);
    res = compareFiles(pSrc1Fn, pSrc2Fn, prec);
    if (res == EXIT_FAILURE) return EXIT_FAILURE;
    
    return EXIT_SUCCESS;
}



