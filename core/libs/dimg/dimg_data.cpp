/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : digiKam 8/16 bits image management API.
 *               Data management.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimg_p.h"

namespace Digikam
{

DImg& DImg::operator=(const DImg& image)
{
    m_priv = image.m_priv;

    return *this;
}

bool DImg::operator==(const DImg& image) const
{
    return (m_priv == image.m_priv);
}

void DImg::reset()
{
    m_priv = new Private;
}

void DImg::detach()
{
    // are we being shared?

    if (m_priv->ref == 1)
    {
        return;
    }

    QExplicitlySharedDataPointer<Private> old(m_priv);

    m_priv = new Private;
    copyImageData(old);
    copyMetaData(old);

    if (old->data)
    {
        size_t bsize = allocateData();
        memcpy(m_priv->data, old->data, bsize);
    }
}

void DImg::putImageData(uint width, uint height, bool sixteenBit, bool alpha, uchar* const data, bool copyData)
{
    // set image data, metadata is untouched

    bool null = (width == 0) || (height == 0);

    // allocateData, or code below will set null to false

    setImageData(true, width, height, sixteenBit, alpha);

    // replace data

    delete [] m_priv->data;
    m_priv->data = nullptr;

    if (null)
    {
        // image is null - no data

        return;
    }
    else if (copyData)
    {
        size_t bsize = allocateData();

        // cppcheck-suppress knownConditionTrueFalse
        if (m_priv->data && data)
        {
            memcpy(m_priv->data, data, bsize);
        }
    }
    else
    {
        if (data)
        {
            m_priv->data = data;
            m_priv->null = false;
        }
        else
        {
            allocateData();
        }
    }
}

void DImg::putImageData(uchar* const data, bool copyData)
{
    if      (!data)
    {
        delete [] m_priv->data;
        m_priv->data = nullptr;
        m_priv->null = true;
    }
    else if (copyData)
    {
        memcpy(m_priv->data, data, numBytes());
    }
    else
    {
        m_priv->data = data;
    }
}

void DImg::resetMetaData()
{
    m_priv->attributes.clear();
    m_priv->embeddedText.clear();
    m_priv->metaData = MetaEngineData();
}

uchar* DImg::stripImageData()
{
    uchar* const data  = m_priv->data;
    m_priv->data       = nullptr;
    m_priv->null       = true;

    return data;
}

void DImg::copyMetaData(const QExplicitlySharedDataPointer<Private>& src)
{
    m_priv->metaData     = src->metaData;
    m_priv->attributes   = src->attributes;
    m_priv->embeddedText = src->embeddedText;
    m_priv->iccProfile   = src->iccProfile;
    m_priv->imageHistory = src->imageHistory;

    // FIXME: what about sharing and deleting lanczos_func?
}

void DImg::copyImageData(const QExplicitlySharedDataPointer<Private>& src)
{
    setImageData(src->null, src->width, src->height, src->sixteenBit, src->alpha);
}

size_t DImg::allocateData() const
{
    quint64 bsize = (quint64)m_priv->width  *
                    (quint64)m_priv->height *
                    (quint64)(m_priv->sixteenBit ? 8 : 4);

    if (bsize >= std::numeric_limits<size_t>::max())
    {
        m_priv->null = true;

        return 0;
    }

    m_priv->data = DImgLoader::new_failureTolerant(bsize);

    if (!m_priv->data)
    {
        m_priv->null = true;

        return 0;
    }

    m_priv->null = false;

    return bsize;
}

void DImg::setImageDimension(uint width, uint height)
{
    m_priv->width  = width;
    m_priv->height = height;
}

void DImg::setImageData(bool null, uint width, uint height, bool sixteenBit, bool alpha)
{
    m_priv->null       = null;
    m_priv->width      = width;
    m_priv->height     = height;
    m_priv->alpha      = alpha;
    m_priv->sixteenBit = sixteenBit;
}

} // namespace Digikam
