/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               Exif manipulation methods
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metaengine_p.h"

// C++ includes

#include <cctype>

// Qt includes

#include <QBuffer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "metaengine_rotation.h"

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace Digikam
{

bool MetaEngine::canWriteExif(const QString& filePath)
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(filePath.toUtf8().constData());
        Exiv2::AccessMode mode      = image->checkMode(Exiv2::mdExif);

        return ((mode == Exiv2::amWrite) || (mode == Exiv2::amReadWrite));
    }
    catch (Exiv2::AnyError& e)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Cannot check Exif access mode with Exiv2:(Error #"
                                           << (int)e.code() << ": " << QString::fromStdString(e.what()) << ")";
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::hasExif() const
{
    return !d->exifMetadata().empty();
}

bool MetaEngine::clearExif() const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        d->exifMetadata().clear();

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot clear Exif data with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QByteArray MetaEngine::getExifEncoded(bool addExifHeader) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        if (!d->exifMetadata().empty())
        {
            QByteArray _data;
            Exiv2::ExifData& exif = d->exifMetadata();
            Exiv2::Blob blob;
            Exiv2::ExifParser::encode(blob, Exiv2::bigEndian, exif);
            QByteArray ba(reinterpret_cast<const char*>(&blob[0]), (int)blob.size());

            if (addExifHeader)
            {
                const uchar ExifHeader[] = {0x45, 0x78, 0x69, 0x66, 0x00, 0x00};
                _data.resize(ba.size() + sizeof(ExifHeader));
                memcpy(_data.data(), ExifHeader, sizeof(ExifHeader));
                memcpy(_data.data() + sizeof(ExifHeader), ba.data(), ba.size());
            }
            else
            {
                _data = ba;
            }

            return _data;
        }
    }
    catch (Exiv2::AnyError& e)
    {
        if (!d->filePath.isEmpty())
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "From file " << d->filePath.toLatin1().constData();
        }

        d->printExiv2ExceptionError(QLatin1String("Cannot get Exif data with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QByteArray();
}

bool MetaEngine::setExif(const QByteArray& data) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        if (!data.isEmpty())
        {
            Exiv2::ExifParser::decode(d->exifMetadata(), reinterpret_cast<const Exiv2::byte*>(data.data()), data.size());

            return (!d->exifMetadata().empty());
        }
    }
    catch (Exiv2::AnyError& e)
    {
        if (!d->filePath.isEmpty())
        {
            qCCritical(DIGIKAM_METAENGINE_LOG) << "From file " << d->filePath.toLatin1().constData();
        }

        d->printExiv2ExceptionError(QLatin1String("Cannot set Exif data with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QString MetaEngine::getExifComment(bool readDescription) const
{
    QString comment = getExifTagComment("Exif.Photo.UserComment");

    if (!comment.isEmpty())
    {
        return comment;
    }

    if (readDescription)
    {
        comment = getExifTagComment("Exif.Image.ImageDescription");

        return comment;
    }

    return QString();
}

QString MetaEngine::getExifTagComment(const char* exifTagName) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        // Since Exiv2-0.27.3 empty comment fields are output
        // as "binary comment". As workaround we filter it out.

        QStringList blackList;
        blackList << QLatin1String("binary comment");

        if (!d->exifMetadata().empty())
        {
            std::string exifkey(exifTagName);
            Exiv2::ExifData exifData(d->exifMetadata());
            Exiv2::ExifKey key(exifkey);
            Exiv2::ExifData::const_iterator it = exifData.findKey(key);

            if (it != exifData.end())
            {
                QString exifComment;

                if (QLatin1String(exifTagName) == QLatin1String("Exif.Image.XPComment"))
                {
                    exifComment = QString::fromStdString(it->print(&exifData));
                }
                else
                {
                    exifComment = d->convertCommentValue(*it);
                }

                QString trimmedComment = exifComment.trimmed();

                // Some cameras fill in nonsense default values

                if (QLatin1String(exifTagName) == QLatin1String("Exif.Image.ImageDescription"))
                {
                    blackList << QLatin1String("SONY DSC"); // + whitespace
                    blackList << QLatin1String("OLYMPUS DIGITAL CAMERA");
                    blackList << QLatin1String("MINOLTA DIGITAL CAMERA");
                }

                // some cameras fill the UserComment with whitespace

                if (!exifComment.isEmpty() && !trimmedComment.isEmpty() && !blackList.contains(trimmedComment))
                {
                    return exifComment;
                }
            }
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot find Exif User Comment with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QString();
}

static bool is7BitAscii(const QString& s)
{
    for (int i = 0 ; i < s.size() ; ++i)
    {
        if (s.at(i).unicode() > 0x7f)
        {
            return false;
        }
    }

    return true;
}

bool MetaEngine::setExifComment(const QString& comment, bool writeDescription) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        if (writeDescription)
        {
            removeExifTag("Exif.Image.ImageDescription");
        }

        removeExifTag("Exif.Photo.UserComment");

        if (!comment.isNull())
        {
            if (writeDescription)
            {
                setExifTagString("Exif.Image.ImageDescription", comment);
            }

            // Write as Unicode only when necessary.
            // Check if it's in the ASCII 7bit range

            if (is7BitAscii(comment))
            {
                // write as ASCII

                QString exifComment(QString::fromLatin1("charset=Ascii %1").arg(comment));
                d->exifMetadata()["Exif.Photo.UserComment"] = exifComment.toStdString();

                return true;
            }

            // write as Unicode (UCS-2)

            QString exifComment(QString::fromUtf8("charset=Unicode %1").arg(comment));
            d->exifMetadata()["Exif.Photo.UserComment"] = exifComment.toStdString();
        }

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Exif Comment with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QString MetaEngine::getExifTagTitle(const char* exifTagName)
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        std::string exifkey(exifTagName);
        Exiv2::ExifKey ek(exifkey);

        return QString::fromStdString(ek.tagLabel());
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot get metadata tag title with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QString();
}

QString MetaEngine::getExifTagDescription(const char* exifTagName)
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        std::string exifkey(exifTagName);
        Exiv2::ExifKey ek(exifkey);

        return QString::fromStdString(ek.tagDesc());
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot get metadata tag description with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QString();
}

bool MetaEngine::removeExifTag(const char* exifTagName) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::ExifKey exifKey(exifTagName);
        Exiv2::ExifData::iterator it = d->exifMetadata().findKey(exifKey);

        if (it != d->exifMetadata().end())
        {
            d->exifMetadata().erase(it);

            return true;
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot remove Exif tag with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::getExifTagRational(const char* exifTagName, long int& num, long int& den, int component) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::ExifKey exifKey(exifTagName);
        Exiv2::ExifData exifData(d->exifMetadata());
        Exiv2::ExifData::const_iterator it = exifData.findKey(exifKey);

        if (it != exifData.end())
        {
            if ((*it).count())
            {
                num = (*it).toRational(component).first;
                den = (*it).toRational(component).second;

                return true;
            }
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Exif Rational value from key '%1' into image with Exiv2:")
                                    .arg(QLatin1String(exifTagName)), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::setExifTagLong(const char* exifTagName, long val) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        d->exifMetadata()[exifTagName] = static_cast<int32_t>(val);     // krazy:exclude=typedefs

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Exif tag long value into image with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::setExifTagUShort(const char* exifTagName, unsigned int val) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        d->exifMetadata()[exifTagName] = static_cast<uint16_t>(val);     // krazy:exclude=typedefs

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Exif tag unsigned short value into image with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::setExifTagRational(const char* exifTagName, long int num, long int den) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        d->exifMetadata()[exifTagName] = Exiv2::Rational(num, den);

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Exif tag rational value into image with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::setExifTagURational(const char* exifTagName, unsigned long int num, unsigned long int den) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        d->exifMetadata()[exifTagName] = Exiv2::URational(num, den);

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Exif tag unsigned rational value into image with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::setExifTagData(const char* exifTagName, const QByteArray& data) const
{
    if (data.isEmpty())
    {
        return false;
    }

    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::DataValue val(reinterpret_cast<Exiv2::byte*>(const_cast<char*>(data.data())), data.size());
        d->exifMetadata()[exifTagName] = val;

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Exif tag data into image with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::setExifTagVariant(const char* exifTagName, const QVariant& val,
                                   bool rationalWantSmallDenominator) const
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    switch (val.typeId())

#else

    switch (val.type())

#endif

    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        case QMetaType::Int:
        case QMetaType::UInt:
        case QMetaType::Bool:
        case QMetaType::LongLong:
        case QMetaType::ULongLong:

#else

        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::Bool:
        case QVariant::LongLong:
        case QVariant::ULongLong:

#endif

        {
            return setExifTagLong(exifTagName, val.toInt());
        }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        case QMetaType::Double:

#else

        case QVariant::Double:

#endif

        {
            long num = 0;
            long den = 1;

            if (rationalWantSmallDenominator)
            {
                convertToRationalSmallDenominator(val.toDouble(), &num, &den);
            }
            else
            {
                convertToRational(val.toDouble(), &num, &den, 4);
            }

            return setExifTagRational(exifTagName, num, den);
        }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        case qMetaTypeId<QList<QVariant> >():

#else

        case QVariant::List:

#endif

        {
            long num = 0;
            long den = 1;
            QList<QVariant> list = val.toList();

            if (list.size() >= 1)
            {
                num = list[0].toInt();
            }

            if (list.size() >= 2)
            {
                den = list[1].toInt();
            }

            return setExifTagRational(exifTagName, num, den);
        }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        case QMetaType::QDate:
        case QMetaType::QDateTime:

#else

        case QVariant::Date:
        case QVariant::DateTime:

#endif

        {
            QDateTime dateTime = asDateTimeUTC(val.toDateTime());

            if (!dateTime.isValid())
            {
                return false;
            }

            try
            {
                d->exifMetadata()[exifTagName] = dateTime.toString(QLatin1String("yyyy:MM:dd hh:mm:ss")).toStdString();
            }
            catch (Exiv2::AnyError& e)
            {
                d->printExiv2ExceptionError(QLatin1String("Cannot set Date & Time in image with Exiv2:"), e);
            }
            catch (...)
            {
                qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
            }

            return false;
        }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        case QMetaType::QString:
        case QMetaType::QChar:

#else

        case QVariant::String:
        case QVariant::Char:

#endif

        {
            return setExifTagString(exifTagName, val.toString());
        }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        case QMetaType::QByteArray:

#else

        case QVariant::ByteArray:

#endif

        {
            return setExifTagData(exifTagName, val.toByteArray());
        }

        default:
        {
            break;
        }
    }

    return false;
}

QString MetaEngine::createExifUserStringFromValue(const char* exifTagName,
                                                  const QVariant& val, bool escapeCR)
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::ExifKey key(exifTagName);
        Exiv2::Exifdatum datum(key);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        switch (val.typeId())

#else

        switch (val.type())

#endif

        {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            case QMetaType::Int:
            case QMetaType::Bool:
            case QMetaType::LongLong:
            case QMetaType::ULongLong:

#else
            case QVariant::Int:
            case QVariant::Bool:
            case QVariant::LongLong:
            case QVariant::ULongLong:

#endif

            {
                datum = (int32_t)val.toInt();
                break;
            }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            case QMetaType::UInt:

#else

            case QVariant::UInt:

#endif

            {
                datum = (uint32_t)val.toUInt();
                break;
            }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            case QMetaType::Double:

#else

            case QVariant::Double:

#endif

            {
                long num, den;
                convertToRationalSmallDenominator(val.toDouble(), &num, &den);
                Exiv2::Rational rational;
                rational.first  = num;
                rational.second = den;
                datum           = rational;
                break;
            }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            case qMetaTypeId<QList<QVariant> >():

#else

            case QVariant::List:

#endif

            {
                long num             = 0;
                long den             = 1;
                QList<QVariant> list = val.toList();

                if (list.size() >= 1)
                {
                    num = list[0].toInt();
                }

                if (list.size() >= 2)
                {
                    den = list[1].toInt();
                }

                Exiv2::Rational rational;
                rational.first  = num;
                rational.second = den;
                datum           = rational;
                break;
            }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            case QMetaType::QDate:
            case QMetaType::QDateTime:

#else

            case QVariant::Date:
            case QVariant::DateTime:

#endif

            {
                QDateTime dateTime = asDateTimeUTC(val.toDateTime());

                if (!dateTime.isValid())
                {
                    break;
                }

                datum = dateTime.toString(QLatin1String("yyyy:MM:dd hh:mm:ss")).toStdString();
                break;
            }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            case QMetaType::QByteArray:
            case QMetaType::QString:
            case QMetaType::QChar:

#else

            case QVariant::ByteArray:
            case QVariant::String:
            case QVariant::Char:

#endif

            {
                datum = val.toString().toStdString();
                break;
            }

            default:
            {
                break;
            }
        }

        std::ostringstream os;
        os << datum;
        QString tagValue = QString::fromStdString(os.str());

        if (escapeCR)
        {
            tagValue.replace(QLatin1Char('\n'), QLatin1String(" "));
        }

        return tagValue;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot get Exif tag user string with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QString();
}

bool MetaEngine::getExifTagLong(const char* exifTagName, long& val) const
{
    return getExifTagLong(exifTagName, val, 0);
}

bool MetaEngine::getExifTagLong(const char* exifTagName, long& val, int component) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::ExifKey exifKey(exifTagName);
        Exiv2::ExifData exifData(d->exifMetadata());
        Exiv2::ExifData::const_iterator it = exifData.findKey(exifKey);

        if ((it != exifData.end()) && (it->count() > 0))
        {

#if EXIV2_TEST_VERSION(0,27,99)

            val = it->toInt64(component);

#else

            val = it->toLong(component);

#endif

            return true;
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Exif key '%1' into image with Exiv2:")
                                    .arg(QLatin1String(exifTagName)), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QByteArray MetaEngine::getExifTagData(const char* exifTagName) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::ExifKey exifKey(exifTagName);
        Exiv2::ExifData exifData(d->exifMetadata());
        Exiv2::ExifData::const_iterator it = exifData.findKey(exifKey);

        if (it != exifData.end())
        {
            QByteArray _data((*it).size(), '\0');
            (*it).copy(reinterpret_cast<Exiv2::byte*>(const_cast<char*>(_data.data())), Exiv2::bigEndian);

            return _data;
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Exif key '%1' into image with Exiv2:")
                                    .arg(QLatin1String(exifTagName)), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QByteArray();
}

QVariant MetaEngine::getExifTagVariant(const char* exifTagName, bool rationalAsListOfInts, bool stringEscapeCR, int component) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::ExifKey exifKey(exifTagName);
        Exiv2::ExifData exifData(d->exifMetadata());
        Exiv2::ExifData::const_iterator it = exifData.findKey(exifKey);

        if (it != exifData.end())
        {
            switch (it->typeId())
            {
                case Exiv2::unsignedByte:
                case Exiv2::unsignedShort:
                case Exiv2::unsignedLong:
                case Exiv2::signedShort:
                case Exiv2::signedLong:
                {
                    if ((int)it->count() > component)
                    {

#if EXIV2_TEST_VERSION(0,27,99)

                        return QVariant((int)it->toInt64(component));

#else

                        return QVariant((int)it->toLong(component));

#endif

                    }
                    else
                    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                        return QVariant(QMetaType(QMetaType::Int));

#else

                        return QVariant(QVariant::Int);

#endif

                    }
                }

                case Exiv2::unsignedRational:
                case Exiv2::signedRational:
                {
                    if (rationalAsListOfInts)
                    {
                        if ((int)it->count() <= component)
                        {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                            return QVariant(QMetaType(QMetaType::QVariantList));

#else

                            return QVariant(QVariant::List);

#endif

                        }

                        QList<QVariant> list;
                        list << (*it).toRational(component).first;
                        list << (*it).toRational(component).second;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                        return QVariant(QMetaType(QMetaType::QVariantList));

#else

                        return QVariant(list);

#endif

                    }
                    else
                    {
                        if ((int)it->count() <= component)
                        {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                            return QVariant(QMetaType(QMetaType::Double));

#else

                            return QVariant(QVariant::Double);

#endif

                        }

                        // prefer double precision

                        double num = (*it).toRational(component).first;
                        double den = (*it).toRational(component).second;

                        if (den == 0.0)
                        {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                            return QVariant(QMetaType(QMetaType::Double));

#else

                            return QVariant(QVariant::Double);

#endif

                        }

                        return QVariant(num / den);
                    }
                }

                case Exiv2::date:
                case Exiv2::time:
                {
                    QDateTime dateTime = QDateTime::fromString(QString::fromStdString(it->toString()), Qt::ISODate);

                    return QVariant(asDateTimeUTC(dateTime));
                }

                case Exiv2::asciiString:
                case Exiv2::comment:
                case Exiv2::string:
                {
                    std::ostringstream os;
                    it->write(os, &exifData);
                    QString tagValue = QString::fromStdString(os.str());

                    if (stringEscapeCR)
                    {
                        tagValue.replace(QLatin1Char('\n'), QLatin1String(" "));
                    }

                    return QVariant(tagValue);
                }

                default:
                {
                    break;
                }
            }
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Exif key '%1' in the image with Exiv2:")
                                    .arg(QLatin1String(exifTagName)), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QVariant();
}

QString MetaEngine::getExifTagString(const char* exifTagName, bool escapeCR) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::ExifKey exifKey(exifTagName);
        Exiv2::ExifData exifData(d->exifMetadata());
        Exiv2::ExifData::const_iterator it = exifData.findKey(exifKey);

        if (it != exifData.end())
        {
            QString tagValue;
            QString key = QLatin1String(it->key().c_str());

            if (
                (key == QLatin1String("Exif.CanonCs.LensType")) &&

#if EXIV2_TEST_VERSION(0,27,99)

                (it->toInt64() == 65535)

#else

                (it->toLong()  == 65535)

#endif

               )
            {
                // FIXME: workaround for a possible crash in Exiv2 pretty-print function for the Exif.CanonCs.LensType.

                tagValue = QString::fromStdString(it->toString());
            }
            else
            {
                // See BUG #184156 comment #13

                tagValue = QString::fromStdString(it->print(&exifData));
            }

            if (escapeCR)
            {
                tagValue.replace(QLatin1Char('\n'), QLatin1String(" "));
            }

            return tagValue;
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Exif key '%1' into image with Exiv2:")
                                    .arg(QLatin1String(exifTagName)), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QString();
}

bool MetaEngine::setExifTagString(const char* exifTagName, const QString& value) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        d->exifMetadata()[exifTagName] = value.toStdString();

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Exif tag string into image with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QImage MetaEngine::getExifThumbnail(bool fixOrientation) const
{
    QImage thumbnail;

    if (d->exifMetadata().empty())
    {
       return thumbnail;
    }

    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::ExifThumbC thumb(d->exifMetadata());
        Exiv2::DataBuf const c1 = thumb.copy();

#if EXIV2_TEST_VERSION(0,27,99)

        if (c1.size() == 0)
        {
            return thumbnail;
        }

        thumbnail.loadFromData(c1.c_data(), c1.size());

#else

        if (c1.size_ == 0)
        {
            return thumbnail;
        }

        thumbnail.loadFromData(c1.pData_, c1.size_);

#endif

        if (!thumbnail.isNull())
        {
            if (fixOrientation)
            {
                Exiv2::ExifKey key1("Exif.Thumbnail.Orientation");
                Exiv2::ExifKey key2("Exif.Image.Orientation");
                Exiv2::ExifData exifData(d->exifMetadata());
                Exiv2::ExifData::const_iterator it = exifData.findKey(key1);

                if (it == exifData.end())
                {
                    it = exifData.findKey(key2);
                }

                if (it != exifData.end() && it->count())
                {

#if EXIV2_TEST_VERSION(0,27,99)

                    long orientation = it->toInt64();

#else

                    long orientation = it->toLong();

#endif

                    //qCDebug(DIGIKAM_METAENGINE_LOG) << "Exif Thumbnail Orientation: " << (int)orientation;

                    rotateExifQImage(thumbnail, (ImageOrientation)orientation);
                }

                return thumbnail;
            }
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot get Exif Thumbnail with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return thumbnail;
}

bool MetaEngine::rotateExifQImage(QImage& image, ImageOrientation orientation) const
{
    QTransform matrix = MetaEngineRotation::toTransform(orientation);

    if ((orientation != ORIENTATION_NORMAL) && (orientation != ORIENTATION_UNSPECIFIED))
    {
        image = image.transformed(matrix);

        return true;
    }

    return false;
}

bool MetaEngine::setExifThumbnail(const QImage& thumbImage) const
{
    if (thumbImage.isNull())
    {
        return removeExifThumbnail();
    }

    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        QByteArray _data;
        QBuffer buffer(&_data);
        buffer.open(QIODevice::WriteOnly);
        thumbImage.save(&buffer, "JPEG");
        buffer.close();
        Exiv2::ExifThumb thumb(d->exifMetadata());
        thumb.setJpegThumbnail(reinterpret_cast<Exiv2::byte*>(const_cast<char*>(_data.data())), _data.size());

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Exif Thumbnail with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::setTiffThumbnail(const QImage& thumbImage) const
{
    removeExifThumbnail();

    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        // Make sure IFD0 is explicitly marked as a main image

        Exiv2::ExifData::const_iterator pos = d->exifMetadata().findKey(Exiv2::ExifKey("Exif.Image.NewSubfileType"));

        if (
            (pos            == d->exifMetadata().end()) ||
            (pos->count()   != 1)                       ||

#if EXIV2_TEST_VERSION(0,27,99)

            (pos->toInt64() != 0)

#else

            (pos->toLong()  != 0)

#endif

           )
        {

#if EXIV2_TEST_VERSION(0,27,0)

            throw Exiv2::Error(Exiv2::kerErrorMessage, "Exif.Image.NewSubfileType missing or not set as main image");

#else

            throw Exiv2::Error(1, "Exif.Image.NewSubfileType missing or not set as main image");

#endif

        }

        // Remove sub-IFD tags

        std::string subImage1("SubImage1");

        for (Exiv2::ExifData::iterator md = d->exifMetadata().begin() ; md != d->exifMetadata().end() ; )
        {
            if (md->groupName() == subImage1)
            {
                md = d->exifMetadata().erase(md);
            }
            else
            {
                ++md;
            }
        }

        if (!thumbImage.isNull())
        {
            // Set thumbnail tags

            QByteArray _data;
            QBuffer buffer(&_data);
            buffer.open(QIODevice::WriteOnly);
            thumbImage.save(&buffer, "JPEG");
            buffer.close();

            Exiv2::DataBuf buf(reinterpret_cast<Exiv2::byte*>(const_cast<char*>(_data.data())), _data.size());
            Exiv2::ULongValue val;
            val.read("0");

#if EXIV2_TEST_VERSION(0,27,99)

            val.setDataArea(buf.data(), buf.size());

#else

            val.setDataArea(buf.pData_, buf.size_);

#endif

            d->exifMetadata()["Exif.SubImage1.JPEGInterchangeFormat"]       = val;

#if EXIV2_TEST_VERSION(0,27,99)

            d->exifMetadata()["Exif.SubImage1.JPEGInterchangeFormatLength"] = uint32_t(buf.size());

#else

            d->exifMetadata()["Exif.SubImage1.JPEGInterchangeFormatLength"] = uint32_t(buf.size_);

#endif

            d->exifMetadata()["Exif.SubImage1.Compression"]                 = uint16_t(6);          // JPEG (old-style)
            d->exifMetadata()["Exif.SubImage1.NewSubfileType"]              = uint32_t(1);          // Thumbnail image

            return true;
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set TIFF Thumbnail with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::removeExifThumbnail() const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        // Remove all IFD0 subimages.

        Exiv2::ExifThumb thumb(d->exifMetadata());
        thumb.erase();

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot remove Exif Thumbnail with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

MetaEngine::MetaDataMap MetaEngine::getExifTagsDataList(const QStringList& exifKeysFilter,
                                                        bool invertSelection,
                                                        bool extractBinary) const
{
    if (d->exifMetadata().empty())
    {
       return MetaDataMap();
    }

    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::ExifData exifData = d->exifMetadata();
        exifData.sortByKey();

        MetaDataMap        metaDataMap;
        std::ostringstream os;
        QString            key;
        QString            tagValue;
        double             num   = 0.0;
        double             den   = 1.0;

        for (Exiv2::ExifData::const_iterator md = exifData.begin() ; md != exifData.end() ; ++md)
        {
            key = QLatin1String(md->key().c_str());

            // Decode the tag value with a user friendly output.

            if      (key == QLatin1String("Exif.Photo.UserComment"))
            {
                tagValue = d->convertCommentValue(*md);
            }
            else if (key == QLatin1String("Exif.GPSInfo.GPSAreaInformation"))
            {
                // NOTE: special cases to render contents of these GPS info tags. See bug #425884.

                tagValue = d->convertCommentValue(*md);
            }
            else if (key == QLatin1String("Exif.GPSInfo.GPSProcessingMethod"))
            {
                // NOTE: special cases to render contents of these GPS info tags. See bug #425884.

                tagValue = d->convertCommentValue(*md);
            }
            else if (
                     (key == QLatin1String("Exif.GPSInfo.GPSTrack"))      ||
                     (key == QLatin1String("Exif.GPSInfo.GPSImgDirection"))
                    )
            {
                // NOTE: special cases to render contents of these GPS info tags. See bug #435317.
                // Check value byte size, otherwise Exiv2 crashes in the toRational() function.

                if ((*md).count())
                {
                    num  = (*md).toRational().first;
                    den  = (*md).toRational().second;
                }

                tagValue = (den == 0.0) ? QString::fromStdString(md->print())
                                        : QString::fromLatin1("%1 deg").arg(num / den);
            }
            else if (key == QLatin1String("Exif.GPSInfo.GPSSpeed"))
            {
                // NOTE: special cases to render contents of these GPS info tags. See bug #435317.
                // Check value byte size, otherwise Exiv2 crashes in the toRational() function.

                if ((*md).count())
                {
                    num  = (*md).toRational().first;
                    den  = (*md).toRational().second;
                }

                tagValue = (den == 0.0) ? QString::fromStdString(md->print())
                                        : tagValue = QString::number(num / den);
            }
            else if (key == QLatin1String("Exif.Image.0x935c"))
            {
                tagValue = QString::number(md->value().size());
            }
            else if (
                     (key           == QLatin1String("Exif.CanonCs.LensType")) &&

#if EXIV2_TEST_VERSION(0,27,99)

                     (md->toInt64() == 65535)

#else

                     (md->toLong()  == 65535)

#endif

                    )
            {
                // FIXME: workaround for a possible crash in Exiv2 pretty-print function for the Exif.CanonCs.LensType.

                tagValue = QString::fromStdString(md->toString());
            }
            else
            {
                if (!extractBinary && (md->typeId() == Exiv2::undefined))
                {
                    tagValue = i18nc("info", "Binary data %1 bytes", md->size());
                }
                else
                {
                    os.str("");
                    os.clear();
                    md->write(os, &exifData);

                    // Exif tag contents can be a translated strings, not only simple ascii.

                    tagValue = QString::fromStdString(os.str());
                }
            }

            tagValue.replace(QLatin1Char('\n'), QLatin1String(" "));

            // We apply a filter to get only the Exif tags that we need.

            if (!exifKeysFilter.isEmpty())
            {
                if (!invertSelection)
                {
                    if (exifKeysFilter.contains(key.section(QLatin1Char('.'), 1, 1)))
                    {
                        metaDataMap.insert(key, tagValue);
                    }
                }
                else
                {
                    if (!exifKeysFilter.contains(key.section(QLatin1Char('.'), 1, 1)))
                    {
                        metaDataMap.insert(key, tagValue);
                    }
                }
            }
            else // else no filter at all.
            {
                metaDataMap.insert(key, tagValue);
            }
        }

        return metaDataMap;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot parse EXIF metadata with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return MetaDataMap();
}

MetaEngine::TagsMap MetaEngine::getStdExifTagsList() const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        QList<const Exiv2::TagInfo*> tags;
        TagsMap                      tagsMap;

        const Exiv2::GroupInfo* gi = Exiv2::ExifTags::groupList();

        while (gi->tagList_ != nullptr)
        {
            // NOTE: See BUG #375809 : MPF tags = exception Exiv2 0.26

            if (QLatin1String(gi->ifdName_) != QLatin1String("Makernote"))
            {
                Exiv2::TagListFct tl     = gi->tagList_;
                const Exiv2::TagInfo* ti = tl();

                while (ti->tag_ != 0xFFFF)
                {
                    tags << ti;
                    ++ti;
                }
            }

            ++gi;
        }

        for (QList<const Exiv2::TagInfo*>::iterator it = tags.begin() ; it != tags.end() ; ++it)
        {
            do
            {
                const Exiv2::TagInfo* const ti = *it;

                if (ti)
                {
                    QString key = QLatin1String(Exiv2::ExifKey(*ti).key().c_str());
                    QStringList values;
                    values << QLatin1String(ti->name_) << QLatin1String(ti->title_) << QLatin1String(ti->desc_);
                    tagsMap.insert(key, values);
                }
                else
                {
                    break;
                }

                ++(*it);
            }
            while ((*it)->tag_ != 0xffff);
        }

        return tagsMap;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot get Exif Tags list with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return TagsMap();
}

MetaEngine::TagsMap MetaEngine::getMakernoteTagsList() const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        QList<const Exiv2::TagInfo*> tags;
        TagsMap                      tagsMap;

        const Exiv2::GroupInfo* gi = Exiv2::ExifTags::groupList();

        while (gi->tagList_ != nullptr)
        {
            if (QLatin1String(gi->ifdName_) == QLatin1String("Makernote"))
            {
                Exiv2::TagListFct tl     = gi->tagList_;
                const Exiv2::TagInfo* ti = tl();

                while (ti->tag_ != 0xFFFF)
                {
                    tags << ti;
                    ++ti;
                }
            }

            ++gi;
        }

        for (QList<const Exiv2::TagInfo*>::iterator it = tags.begin() ; it != tags.end() ; ++it)
        {
            do
            {
                const Exiv2::TagInfo* const ti = *it;

                if (ti)
                {
                    QString key = QLatin1String(Exiv2::ExifKey(*ti).key().c_str());
                    QStringList values;
                    values << QLatin1String(ti->name_) << QLatin1String(ti->title_) << QLatin1String(ti->desc_);
                    tagsMap.insert(key, values);
                }
                else
                {
                    break;
                }

                ++(*it);
            }
            while ((*it)->tag_ != 0xffff);
        }

        return tagsMap;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot get Makernote Tags list with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return TagsMap();
}

} // namespace Digikam

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif
