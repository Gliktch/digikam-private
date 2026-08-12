/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               Internal private data container.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metaengine_data_p.h"

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

void MetaEngineData::Private::clear()
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        imageComments.clear();
        exifMetadata.clear();
        iptcMetadata.clear();

#ifdef _XMP_SUPPORT_

        xmpMetadata.clear();

#endif

        exifByteOrder = Exiv2::invalidByteOrder;

        iccProfileBuf.reset();

    }
    catch (Exiv2::AnyError& e)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Cannot clear data container with Exiv2 "
                                           << "(Error #" << (int)e.code() << ": "
                                           << QString::fromStdString(e.what())
                                           << ")";
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

}

int MetaEngineData::Private::size() const
{
    QMutexLocker lock(&s_metaEngineMutex);

    int metaSize = 0;

    try
    {
        Exiv2::ExifData::const_iterator it1 = exifMetadata.begin();

        for ( ; it1 != exifMetadata.end() ; ++it1)
        {

#if EXIV2_TEST_VERSION(0,27,99)

            metaSize += it1->size();

#else

            metaSize += it1->size_;

#endif

        }

        Exiv2::IptcData::const_iterator it2 = iptcMetadata.begin();

        for ( ; it2 != iptcMetadata.end() ; ++it2)
        {

#if EXIV2_TEST_VERSION(0,27,99)

            metaSize += it2->size();

#else

            metaSize += it2->size_;

#endif

        }

#ifdef _XMP_SUPPORT_

        Exiv2::XmpData::const_iterator it3 = xmpMetadata.begin();

        for ( ; it3 != xmpMetadata.end() ; ++it3)
        {

#if EXIV2_TEST_VERSION(0,27,99)

            metaSize += it3->size();

#else

            metaSize += it3->size_;

#endif

        }

#endif // _XMP_SUPPORT_

        metaSize += imageComments.capacity();
        metaSize += iccProfileBuf.size();
    }
    catch (Exiv2::AnyError& e)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Cannot get size of data container with Exiv2 "
                                           << "(Error #" << (int)e.code() << ": "
                                           << QString::fromStdString(e.what())
                                           << ")";
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return metaSize;
}

} // namespace Digikam
