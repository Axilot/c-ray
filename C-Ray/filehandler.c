//
//  filehandler.c
//  C-Ray
//
//  Created by Valtteri Koskivuori on 28/02/15.
//  Copyright (c) 2015 Valtteri Koskivuori. All rights reserved.
//

#include "filehandler.h"

void saveImageFromArray(char *filename, unsigned char *imgdata, int width, int height) {
	//File pointer
	FILE *f;
	//Open the file
	f = fopen(filename, "w");
	//Write the PPM format header info
	fprintf(f, "P6 %d %d %d\n", width, height, 255);
	//Write given image data to the file, 3 bytes/pixel
	fwrite(imgdata, 3, width*height, f);
	//Close the file
	fclose(f);
}

void saveBmpFromArray(char *filename, unsigned char *imgData, int width, int height) {
	int i;
	int error;
	FILE *f;
	int filesize = 54 + 3*width*height;
	
	unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
	unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
	unsigned char bmppadding[3] = {0,0,0};
	
	//Create header with filesize data
	bmpfileheader[2] = (unsigned char)(filesize    );
	bmpfileheader[3] = (unsigned char)(filesize>> 8);
	bmpfileheader[4] = (unsigned char)(filesize>>16);
	bmpfileheader[5] = (unsigned char)(filesize>>24);
	
	//create header width and height info
	bmpinfoheader[ 4] = (unsigned char)(width    );
	bmpinfoheader[ 5] = (unsigned char)(width>>8 );
	bmpinfoheader[ 6] = (unsigned char)(width>>16);
	bmpinfoheader[ 7] = (unsigned char)(width>>24);
	
	bmpinfoheader[ 8] = (unsigned char)(height    );
	bmpinfoheader[ 9] = (unsigned char)(height>>8 );
	bmpinfoheader[10] = (unsigned char)(height>>16);
	bmpinfoheader[11] = (unsigned char)(height>>24);
	
	f = fopen(filename,"wb");
	error = (unsigned int)fwrite(bmpfileheader,1,14,f);
	if (error != 14) {
		printf("Error writing BMP file header data\n");
	}
	error = (unsigned int)fwrite(bmpinfoheader,1,40,f);
	if (error != 40) {
		printf("Error writing BMP info header data\n");
	}
	
	for (i = 0; i < height; i++) {
		error = (unsigned int)fwrite(imgData+(width*(i)*3),3,width,f);
		if (error != width) {
			printf("Error writing image line to BMP\n");
		}
		error = (unsigned int)fwrite(bmppadding,1,(4-(width*3)%4)%4,f);
		if (error != (4-(width*3)%4)%4) {
			printf("Error writing BMP padding data\n");
		}
	}
	fclose(f);
}