/*============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Description : Helper functions for the Hugin API
 *
 * SPDX-FileCopyrightText: 2007 by Daniel M German <dmgerman at uvic doooot ca>
 * SPDX-FileCopyrightText: 2012 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// C includes

#include <ctype.h>
#include <stdlib.h>

// Local includes

#include "tparser.h"

/*
void TokenBegin(char *t);
*/

int  panoScriptDataReset(void);
int  panoScriptParserInit(const char* const filename);
void panoScriptParserClose(void);

int  panoScriptScannerGetNextChar(char* b, int maxBuffer);
void panoScriptScannerTokenBegin(char const* t);

#ifndef _MSC_VER   // krazy:exclude=cpp

void panoScriptParserError(char const* errorstring, ...) __attribute__ ((format (printf, 1, 2)));

#else

void panoScriptParserError(char const* errorstring, ...);

#endif

void  yyerror(char const* st);
void* panoScriptReAlloc(void** ptr, size_t size, int* count);
