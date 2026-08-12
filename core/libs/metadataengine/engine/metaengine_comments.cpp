/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               Comments manipulation methods.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metaengine_p.h"

// Local includes

#include "digikam_debug.h"

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace Digikam
{

bool MetaEngine::canWriteComment(const QString& filePath)
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(filePath.toUtf8().constData());
        Exiv2::AccessMode mode      = image->checkMode(Exiv2::mdComment);

        return ((mode == Exiv2::amWrite) || (mode == Exiv2::amReadWrite));
    }
    catch (Exiv2::AnyError& e)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Cannot check Comment access mode with Exiv2 (Error #"
                                           << (int)e.code() << ": " << QString::fromStdString(e.what()) << ")";
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::hasComments() const
{
    return !d->itemComments().empty();
}

bool MetaEngine::clearComments() const
{
    return setComments(QByteArray());
}

QByteArray MetaEngine::getComments() const
{
    return QByteArray(d->itemComments().data(), (int)d->itemComments().size());
}

QString MetaEngine::getCommentsDecoded() const
{
    return d->detectEncodingAndDecode(d->itemComments());
}

bool MetaEngine::setComments(const QByteArray& data) const
{
    d->itemComments() = std::string(data.data(), data.size());

    return true;
}

QString MetaEngine::detectLanguageAlt(const QString& value, QString& lang)
{
    // Ex. from an Xmp tag Xmp.tiff.copyright: "lang="x-default"

    if ((value.size() > 6) && value.startsWith(QLatin1String("lang=\"")))
    {
        int pos = value.indexOf(QLatin1String("\""), 6);

        if (pos != -1)
        {
            lang = value.mid(6, pos-6);
            return (value.mid(pos+2));
        }
    }

    lang.clear();

    return value;
}

} // namespace Digikam

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif
