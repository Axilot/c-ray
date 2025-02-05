//
//  perf_base64.h
//  C-Ray
//
//  Created by Valtteri Koskivuori on 18/03/2021.
//  Copyright © 2021 Valtteri Koskivuori. All rights reserved.
//

#include "../../src/utils/fileio.h"

time_t base64_bigfile_encode(void) {
	size_t bytes = 0;
	char *bigfile = loadFile("input/venusscaled.obj", &bytes);
	ASSERT(bigfile);
	
	struct timeval test;
	startTimer(&test);
	
	char *encoded = b64encode(bigfile, bytes);
	(void)encoded;
	
	time_t us = getUs(test);
	free(bigfile);
	free(encoded);
	return us;
}

time_t base64_bigfile_decode(void) {
	size_t bytes = 0;
	char *bigfile = loadFile("input/venusscaled.obj", &bytes);
	ASSERT(bigfile);
	
	char *encoded = b64encode(bigfile, bytes);
	size_t encodedLength = strlen(encoded);
	
	struct timeval test;
	startTimer(&test);
	
	char *decoded = b64decode(encoded, encodedLength, NULL);
	(void)decoded;
	
	time_t us = getUs(test);
	free(bigfile);
	free(encoded);
	free(decoded);
	return us;
}
