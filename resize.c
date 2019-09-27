// Copies a BMP file

#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: copy infile outfile\n");
        return 1;
    }

    // converting string argument into integer
    int n = atoi(argv[1]);

    // remember filenames
    char *infile = argv[2];
    char *outfile = argv[3];



    // checking if the value is positive or not?
    if (n <= 0 || n > 100)
    {
        printf("The value should positive between 0-100!\n");
        return 2;
    }

    // open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 3;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 4;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);


    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 5;
    }

    //remember the old padding
    int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // remember dimensions of resized image
    int old_height = bi.biHeight;
    int old_width = bi.biWidth;

    // calculate dimensions of resized image
    bi.biHeight *= n;
    bi.biWidth *= n;
    int resized_height = bi.biHeight;
    int resized_width = bi.biWidth;

    // determine scanline padding
    int new_padding = (4 - (resized_width * sizeof(RGBTRIPLE)) % 4) % 4;

    // update header info
    bi.biSizeImage = ((resized_width * sizeof(RGBTRIPLE) + new_padding) * abs(resized_height));
    bf.bfSize = bi.biSizeImage + (sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER));

    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // allocating memory for an array
    RGBTRIPLE *colors = malloc(resized_width * sizeof(RGBTRIPLE));

    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(old_height); i < biHeight; i++)
    {
        int count = 0;


        // iterate over pixels in scanline
        for (int j = 0; j < old_width; j++)
        {
            RGBTRIPLE triple;

            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

            for (int z = 0; z < n; z++)
            {
                // write RGB triple to outfile
                colors[count] = triple;
                count++;
            }
        }

        int written = 0;
        for (int index = 0; index < n; index++)
        {
            written = fwrite(colors, sizeof(RGBTRIPLE), abs(resized_width), outptr);
            if (written == 0)
            {
                printf("Erorr writting into the file!");
            }

            // adding padding to output file
            for (int k = 0; k < new_padding; k++)
            {
                fputc(0x00, outptr);
            }
        }

        // skip over padding, if any
        fseek(inptr, padding, SEEK_CUR);
        // freeing the allocated memory
    }

    free(colors);
    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}
