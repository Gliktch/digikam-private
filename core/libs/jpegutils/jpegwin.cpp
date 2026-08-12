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

#include "jpegwin.h"

// Qt includes

#include <QFile>
#include <QByteArray>

namespace Digikam
{

namespace JPEGUtils
{

void init_source(j_decompress_ptr /*cinfo*/)
{
}

boolean fill_input_buffer(j_decompress_ptr cinfo)
{
    digikam_source_mgr* const src = reinterpret_cast<digikam_source_mgr*>(cinfo->src);

    // Create a fake EOI marker

    src->eoi[0]                   = (JOCTET) 0xFF;
    src->eoi[1]                   = (JOCTET) JPEG_EOI;
    src->pub.next_input_byte      = src->eoi;
    src->pub.bytes_in_buffer      = 2;

    return true;
}

void skip_input_data(j_decompress_ptr cinfo, long nbytes)
{
    digikam_source_mgr* const src = reinterpret_cast<digikam_source_mgr*>(cinfo->src);

    if (nbytes > 0)
    {
        while (nbytes > (long) src->pub.bytes_in_buffer)
        {
            nbytes -= (long) src->pub.bytes_in_buffer;
            (void) fill_input_buffer(cinfo);
        }

        src->pub.next_input_byte += (size_t) nbytes;
        src->pub.bytes_in_buffer -= (size_t) nbytes;
    }
}

void term_source(j_decompress_ptr /*cinfo*/)
{
}

void jpeg_memory_src(j_decompress_ptr cinfo, const JOCTET* buffer, size_t bufsize)
{
    digikam_source_mgr* src = NULL;   // krazy:exclude=null

    if (cinfo->src == NULL)           // krazy:exclude=null
    {
        cinfo->src = (struct jpeg_source_mgr*) (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo,
                     JPOOL_PERMANENT,
                     sizeof(digikam_source_mgr));
    }

    src                        = reinterpret_cast<digikam_source_mgr*>(cinfo->src);
    src->pub.init_source       = init_source;
    src->pub.fill_input_buffer = fill_input_buffer;
    src->pub.skip_input_data   = skip_input_data;
    src->pub.resync_to_restart = jpeg_resync_to_restart;    // default
    src->pub.term_source       = term_source;
    src->pub.next_input_byte   = buffer;
    src->pub.bytes_in_buffer   = bufsize;
}

} // namespace JPEGUtils

} // namespace Digikam
