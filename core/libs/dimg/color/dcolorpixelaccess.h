/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-02
 * Description : methods to access on pixels color
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

namespace Digikam
{

/**
 * @brief These methods are used in quite a few image effects,
 * typically in loops iterating the data.
 * Providing them as inline methods allows the compiler to optimize better.
 */
inline void DColor::setColor(uchar* const data, bool _sixteenBit)
{
    m_sixteenBit = _sixteenBit;

    if (!m_sixteenBit)          // 8 bits image
    {
        setBlue (data[0]);
        setGreen(data[1]);
        setRed  (data[2]);
        setAlpha(data[3]);
    }
    else                      // 16 bits image
    {
        const unsigned short* data16 = reinterpret_cast<unsigned short*>(data);
        setBlue (data16[0]);
        setGreen(data16[1]);
        setRed  (data16[2]);
        setAlpha(data16[3]);
    }
}

inline void DColor::setPixel(uchar* const data) const
{
    if (sixteenBit())       // 16 bits image.
    {
        unsigned short* data16 = reinterpret_cast<unsigned short*>(data);
        data16[0]              = (unsigned short)blue();
        data16[1]              = (unsigned short)green();
        data16[2]              = (unsigned short)red();
        data16[3]              = (unsigned short)alpha();
    }
    else                    // 8 bits image.
    {
        data[0] = (uchar)blue();
        data[1] = (uchar)green();
        data[2] = (uchar)red();
        data[3] = (uchar)alpha();
    }
}

} // namespace Digikam
