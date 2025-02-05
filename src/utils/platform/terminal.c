//
//  terminal.c
//  C-ray
//
//  Created by Valtteri on 29.3.2020.
//  Copyright © 2020 Valtteri Koskivuori. All rights reserved.
//

#include "terminal.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#include "signal.h"
#include "../logging.h"

bool isTeleType(void) {
#ifdef _WIN32
	return _isatty(_fileno(stdout));
#else
	return isatty(fileno(stdout));
#endif
}

static void showCursor(bool show) {
	(void)show;
#ifndef XCODE_NO_COLOR
	if (isTeleType()) show ? fputs("\e[?25h", stdout) : fputs("\e[?25l", stdout);
#endif
}

static void handler(int sig) {
	if (sig == 2) { //SIGINT
		printf("\n");
		logr(info, "Aborting initialization.\n");
		restoreTerminal();
		exit(0);
	}
}

void initTerminal() {
	if (registerHandler(sigint, handler)) {
		logr(warning, "Unable to catch SIGINT\n");
	}
	//If we're on a reasonable (non-windows) terminal, hide the cursor.
#ifndef _WIN32
	//Disable output buffering
	setbuf(stdout, NULL);
	showCursor(false);
#endif
	
	//Configure Windows terminals to handle color escape codes
#ifdef _WIN32
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut != INVALID_HANDLE_VALUE) {
		DWORD dwMode = 0;
		if (GetConsoleMode(hOut, &dwMode)) {
			dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
			SetConsoleMode(hOut, dwMode);
		}
	}
#endif
}

void restoreTerminal() {
#ifndef _WIN32
	showCursor(true);
#endif
}
