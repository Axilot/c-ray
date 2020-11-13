//
//  bmp.c
//  C-ray
//
//  Created by Valtteri on 8.4.2020.
//  Copyright © 2020 Valtteri Koskivuori. All rights reserved.
//

#include "../../../includes.h"

#include "bmp.h"

#include <stdlib.h>
#include <stdio.h>
#include "../../logging.h"

//FIXME: Make this use writeFile() instead of directly encoding to the file like this.
//Or maybe just delete it, and rewrite.
void encodeBMPFromArray(const char *filename, unsigned char *imgData, size_t width, size_t height) {
	//Apparently BMP is BGR, whereas C-ray's internal buffer is RGB (Like it should be)
	//So we need to convert the image data before writing to file.
	unsigned char *bgrData = calloc(3 * width * height, sizeof(*bgrData));
	for (unsigned y = 0; y < height; ++y) {
		for (unsigned x = 0; x < width; ++x) {
			bgrData[(x + (height - (y + 1)) * width) * 3 + 0] = imgData[(x + (height - (y + 1)) * width) * 3 + 2];
			bgrData[(x + (height - (y + 1)) * width) * 3 + 1] = imgData[(x + (height - (y + 1)) * width) * 3 + 1];
			bgrData[(x + (height - (y + 1)) * width) * 3 + 2] = imgData[(x + (height - (y + 1)) * width) * 3 + 0];
		}
	}
	unsigned error;
	FILE *f;
	size_t filesize = 54 + 3 * width * height;
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
	error = (unsigned)fwrite(bmpfileheader,1,14,f);
	if (error != 14) {
		logr(warning, "Error writing BMP file header data\n");
	}
	error = (unsigned)fwrite(bmpinfoheader,1,40,f);
	if (error != 40) {
		logr(warning, "Error writing BMP info header data\n");
	}
	for (unsigned i = 1; i <= height; ++i) {
		error = (unsigned)fwrite(bgrData+(width*(height - i)*3),3,width,f);
		if (error != width) {
			logr(warning, "Error writing image line to BMP\n");
		}
		error = (unsigned)fwrite(bmppadding,1,(4-(width*3)%4)%4,f);
		if (error != (4-(width*3)%4)%4) {
			logr(warning, "Error writing BMP padding data\n");
		}
	}
	free(bgrData);
	fclose(f);
}
