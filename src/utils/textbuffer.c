//
//  textbuffer.c
//  C-ray
//
//  Created by Valtteri on 12.4.2020.
//  Copyright © 2020 Valtteri Koskivuori. All rights reserved.
//

#include <stddef.h>
#include "textbuffer.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "fileio.h"
#include "logging.h"
#include "string.h"
#include <stdio.h>
#include "assert.h"

textBuffer *newTextView(textBuffer *original, const size_t start, const size_t lines) {
	ASSERT(original);
	ASSERT(lines > 0);
	ASSERT(start + lines <= original->amountOf.lines);
	
	char *head = goToLine(original, start);
	size_t start_offset = original->currentByteOffset;
	head = goToLine(original, start + (lines - 1));
	size_t len = strlen(head) + 1;
	size_t end_offset = original->currentByteOffset + len;
	head = goToLine(original, start);
	size_t bytes = end_offset - start_offset;

	char *buf = malloc(bytes * sizeof(*buf));
	memcpy(buf, head, bytes);
	
	textBuffer *new = calloc(1, sizeof(*new));
	new->buf = buf;
	new->buflen = bytes;
	new->amountOf.lines = lines;
	new->current.line = 0;
	logr(debug, "Created new textView handle of size %zu, that has %zu lines\n", new->buflen, new->amountOf.lines);
	return new;
}

textBuffer *newTextBuffer(const char *contents) {
	if (!contents) return NULL;
	textBuffer *new = calloc(1, sizeof(*new));
	new->buf = stringCopy(contents);
	new->buflen = strlen(contents);
	
	//Figure out the line count and convert newlines
	size_t lines = 0;
	for (size_t i = 0; i < new->buflen; ++i) {
		if (new->buf[i] == '\n') {
			new->buf[i] = '\0';
			lines++;
		}
	}
	
	new->amountOf.lines = lines;
	new->current.line = 0;
	logr(debug, "Created new textBuffer handle of size %zu, that has %zu lines\n", new->buflen, new->amountOf.lines);
	return new;
}

lineBuffer *newLineBuffer(void) {
	lineBuffer *new = calloc(1, sizeof(*new));
	new->buf = calloc(LINEBUFFER_MAXSIZE, sizeof(char));
	return new;
}

void dumpBuffer(textBuffer *buffer) {
	logr(debug, "Dumping buffer:\n\n\n");
	char *head = firstLine(buffer);
	while (head) {
		printf("%s\n", head);
		head = nextLine(buffer);
	}
	printf("\n\n");
}

char *goToLine(textBuffer *file, size_t line) {
	if (line < file->amountOf.lines) {
		file->currentByteOffset = 0;
		char *head = file->buf;
		for (size_t i = 0; i < line; ++i) {
			size_t offset = strlen(head) + 1;
			head += offset;
			file->currentByteOffset += offset;
		}
		file->current.line = line;
		return head;
	} else {
		return NULL;
	}
}

char *peekLine(textBuffer *file, size_t line) {
	if (line < file->amountOf.lines) {
		char *head = file->buf;
		for (size_t i = 0; i < line; ++i) {
			size_t offset = strlen(head) + 1;
			head += offset;
		}
		return head;
	} else {
		return NULL;
	}
}

char *nextLine(textBuffer *file) {
	char *head = file->buf + file->currentByteOffset;
	if (file->current.line + 1 < file->amountOf.lines) {
		size_t offset = strlen(head) + 1;
		file->current.line++;
		file->currentByteOffset += offset;
		return head + offset;
	} else {
		return NULL;
	}
}

char *previousLine(textBuffer *file) {
	char *head = goToLine(file, file->current.line - 1);
	return head;
}

char *peekNextLine(textBuffer *file) {
	char *head = file->buf + file->currentByteOffset;
	if (file->current.line + 1 < file->amountOf.lines) {
		size_t offset = strlen(head) + 1;
		return head + offset;
	} else {
		return NULL;
	}
}

char *firstLine(textBuffer *file) {
	char *head = file->buf;
	file->current.line = 0;
	file->currentByteOffset = 0;
	return head;
}

char *currentLine(textBuffer *file) {
	return file->buf + file->currentByteOffset;
}

char *lastLine(textBuffer *file) {
	char *head = goToLine(file, file->amountOf.lines - 1);
	file->current.line = file->amountOf.lines - 1;
	return head;
}

void freeTextBuffer(textBuffer *file) {
	if (file) {
		if (file->buf) free(file->buf);
		free(file);
	}
}

void fillLineBuffer(lineBuffer *line, const char *contents, char *delimiters) {
	const char *buf = contents;
	if (!buf) return;
	size_t contentLen = strlen(buf);
	size_t copyLen = contentLen < LINEBUFFER_MAXSIZE ? contentLen : LINEBUFFER_MAXSIZE;
	memcpy(line->buf, buf, copyLen);
	line->buf[copyLen] = '\0';
	line->buflen = copyLen;
	size_t delimLength = strlen(delimiters);
	
	size_t tokens = 0;
	for (size_t i = 0; i < line->buflen + 1; ++i) {
		for (size_t d = 0; d < delimLength; ++d) {
			if (line->buf[i] == delimiters[d] || line->buf[i] == '\0') {
				line->buf[i] = '\0';
				tokens++;
			}
		}
	}
	
	line->amountOf.tokens = tokens;
}

char *goToToken(lineBuffer *line, size_t token) {
	return goToLine(line, token);
}

char *peekToken(lineBuffer *line, size_t token) {
	return peekLine(line, token);
}

char *nextToken(lineBuffer *line) {
	return nextLine(line);
}

char *previousToken(lineBuffer *line) {
	return previousLine(line);
}

char *peekNextToken(lineBuffer *line) {
	return peekNextLine(line);
}

char *firstToken(lineBuffer *line) {
	return firstLine(line);
}

char *currentToken(lineBuffer *line) {
	return currentLine(line);
}

char *lastToken(lineBuffer *line) {
	return lastLine(line);
}

void destroyLineBuffer(lineBuffer *line) {
	if (line) {
		if (line->buf) free(line->buf);
		free(line);
	}
}
