/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               Embedded preview loading.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metaengine_previews.h"

// Local includes

#include "digikam_debug.h"
#include "metaengine_p.h"

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace Digikam
{

class Q_DECL_HIDDEN MetaEnginePreviews::Private
{
public:

    Private() = default;

    ~Private()
    {
        delete manager;
    }

    void load(Exiv2::Image::AutoPtr image_)     // clazy:exclude=function-args-by-ref
    {
        QMutexLocker lock(&s_metaEngineMutex);

        try
        {

#if EXIV2_TEST_VERSION(0,27,99)

            image                              = std::move(image_);

#else

            image                              = image_;

#endif
            image->readMetadata();

            manager                            = new Exiv2::PreviewManager(*image);
            Exiv2::PreviewPropertiesList props = manager->getPreviewProperties();

            // reverse order of list, which is smallest-first

            Exiv2::PreviewPropertiesList::reverse_iterator it;

            for (it = props.rbegin() ; it != props.rend() ; ++it)
            {
                properties << *it;
            }
        }
        catch (Exiv2::AnyError& e)
        {
            MetaEngine::Private::printExiv2ExceptionError(QLatin1String("Cannot load preview data with Exiv2:"), e);
        }
        catch (...)
        {
            qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
        }
    }

public:

    Exiv2::Image::AutoPtr           image;
    Exiv2::PreviewManager*          manager = nullptr;
    QList<Exiv2::PreviewProperties> properties;
};

MetaEnginePreviews::MetaEnginePreviews(const QString& filePath)
    : d(new Private)
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::Image::AutoPtr _image = Exiv2::ImageFactory::open(filePath.toUtf8().constData());

#if EXIV2_TEST_VERSION(0,27,99)

        d->load(std::move(_image));

#else

        d->load(_image);

#endif

    }
    catch (Exiv2::AnyError& e)
    {
        MetaEngine::Private::printExiv2ExceptionError(QLatin1String("Cannot load metadata with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }
}

MetaEnginePreviews::MetaEnginePreviews(const QByteArray& imgData)
    : d(new Private)
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::Image::AutoPtr _image = Exiv2::ImageFactory::open(
            reinterpret_cast<Exiv2::byte*>(const_cast<char*>(imgData.data())), imgData.size());

#if EXIV2_TEST_VERSION(0,27,99)

        d->load(std::move(_image));

#else

        d->load(_image);

#endif

    }
    catch (Exiv2::AnyError& e)
    {
        MetaEngine::Private::printExiv2ExceptionError(QLatin1String("Cannot load metadata with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }
}

MetaEnginePreviews::~MetaEnginePreviews()
{
    delete d;
}

bool MetaEnginePreviews::isEmpty()
{
    return d->properties.isEmpty();
}

QSize MetaEnginePreviews::originalSize() const
{
    if (d->image.get())
    {
        return QSize(d->image->pixelWidth(), d->image->pixelHeight());
    }

    return QSize();
}

QString MetaEnginePreviews::originalMimeType() const
{
    if (d->image.get())
    {
       return QString::fromStdString(d->image->mimeType());
    }

    return QString();
}

int MetaEnginePreviews::count() const
{
    return d->properties.size();
}

int MetaEnginePreviews::size() const
{
    return count();
}

int MetaEnginePreviews::dataSize(int index)
{
    if ((index < 0) || (index >= size()))
    {
        return 0;
    }

    return d->properties[index].size_;
}

int MetaEnginePreviews::width(int index)
{
    if ((index < 0) || (index >= size()))
    {
        return 0;
    }

    return d->properties[index].width_;
}

int MetaEnginePreviews::height(int index)
{
    if ((index < 0) || (index >= size()))
    {
        return 0;
    }

    return d->properties[index].height_;
}

QString MetaEnginePreviews::mimeType(int index)
{
    if ((index < 0) || (index >= size()))
    {
        return QString();
    }

    return QString::fromStdString(d->properties[index].mimeType_);
}

QString MetaEnginePreviews::fileExtension(int index)
{
    if ((index < 0) || (index >= size()))
    {
        return QString();
    }

    return QString::fromStdString(d->properties[index].extension_);
}

QByteArray MetaEnginePreviews::data(int index)
{
    if ((index < 0) || (index >= size()))
    {
        return QByteArray();
    }

    qCDebug(DIGIKAM_METAENGINE_LOG) << "index     : " << index;
    qCDebug(DIGIKAM_METAENGINE_LOG) << "properties: " << count();

    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::PreviewImage _image = d->manager->getPreviewImage(d->properties[index]);

        return QByteArray(reinterpret_cast<const char*>(_image.pData()), _image.size());
    }
    catch (Exiv2::AnyError& e)
    {
        MetaEngine::Private::printExiv2ExceptionError(QLatin1String("Cannot load metadata with Exiv2:"), e);

        return QByteArray();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";

        return QByteArray();
    }
}

QImage MetaEnginePreviews::image(int index)
{
    QByteArray previewData = data(index);
    QImage     _image;

    if (previewData.isEmpty() || !_image.loadFromData(previewData))
    {
        return QImage();
    }

    return _image;
}

} // namespace Digikam

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif
