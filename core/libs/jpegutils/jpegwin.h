/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-22
 * Description : some workaround functions to read jpeg
 *               files without relying on libjpeg
 *
 * SPDX-FileCopyrightText: 2008 by Patrick Spendrin <ps ml at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// C+ includes

#include <cstdio>
#include <cstdlib>

// Local includes

extern "C"
{
#include "iccjpeg.h"
}

namespace Digikam
{

namespace JPEGUtils
{

typedef struct
{
    struct jpeg_source_mgr pub;
    JOCTET eoi[2];
} digikam_source_mgr;

void    init_source(j_decompress_ptr cinfo);
boolean fill_input_buffer(j_decompress_ptr cinfo);
void    skip_input_data(j_decompress_ptr cinfo, long nbytes);
void    term_source(j_decompress_ptr cinfo);
void    jpeg_memory_src(j_decompress_ptr cinfo, const JOCTET* buffer, size_t bufsize);

} // namespace JPEGUtils

} // namespace Digikam
