/*============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Description : Hugin parser debug header
 *
 * SPDX-FileCopyrightText: 2007 by Daniel M German <dmgerman at uvic doooot ca>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

/* #define YYDEBUG 1 */

#ifdef YYDEBUG
#   define DEBUG_1(a) fprintf(stderr, #a "\n");
#   define DEBUG_2(a,b) fprintf(stderr, #a "\n", b);
#   define DEBUG_3(a,b,c) fprintf(stderr, #a "\n", b, c);
#   define DEBUG_4(a,b,c,d) fprintf(stderr, #a "\n", b, c, d);
#else
#   define DEBUG_1(a)
#   define DEBUG_2(a,b)
#   define DEBUG_3(a,b,c)
#   define DEBUG_4(a,b,c,d)
#endif
