/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               Xmp manipulation methods.
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

bool MetaEngine::canWriteXmp(const QString& filePath)
{

#ifdef _XMP_SUPPORT_

    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(filePath.toUtf8().constData());
        Exiv2::AccessMode mode      = image->checkMode(Exiv2::mdXmp);

        return ((mode == Exiv2::amWrite) || (mode == Exiv2::amReadWrite));
    }
    catch (Exiv2::AnyError& e)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Cannot check Xmp access mode with Exiv2:(Error #"
                                           << (int)e.code() << ": " << QString::fromStdString(e.what()) << ")";
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(filePath);

#endif // _XMP_SUPPORT_

    return false;
}

bool MetaEngine::hasXmp() const
{

#ifdef _XMP_SUPPORT_

    return !d->xmpMetadata().empty();

#else

    return false;

#endif // _XMP_SUPPORT_

}

bool MetaEngine::clearXmp() const
{

#ifdef _XMP_SUPPORT_

    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        d->xmpMetadata().clear();

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot clear Xmp data with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#endif // _XMP_SUPPORT_

    return false;
}

QByteArray MetaEngine::getXmp() const
{

#ifdef _XMP_SUPPORT_

    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        if (!d->xmpMetadata().empty())
        {

            std::string xmpPacket;
            Exiv2::XmpParser::encode(xmpPacket, d->xmpMetadata());
            QByteArray _data(xmpPacket.data(), (int)xmpPacket.size());

            return _data;
        }
    }
    catch (Exiv2::AnyError& e)
    {
        if (!d->filePath.isEmpty())
        {
            d->printExiv2ExceptionError(QLatin1String("Cannot get Xmp data with Exiv2:"), e);
        }
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#endif // _XMP_SUPPORT_

    return QByteArray();
}

bool MetaEngine::setXmp(const QByteArray& data) const
{

#ifdef _XMP_SUPPORT_

    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        if (!data.isEmpty())
        {
            std::string xmpPacket;
            xmpPacket.assign(data.data(), data.size());

            if (Exiv2::XmpParser::decode(d->xmpMetadata(), xmpPacket) != 0)
            {
                return false;
            }
            else
            {
                return true;
            }
        }
    }
    catch (Exiv2::AnyError& e)
    {
        if (!d->filePath.isEmpty())
        {
            qCCritical(DIGIKAM_METAENGINE_LOG) << "From file " << d->filePath.toLatin1().constData();
        }

        d->printExiv2ExceptionError(QLatin1String("Cannot set Xmp data with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(data);

#endif // _XMP_SUPPORT_

    return false;
}

MetaEngine::MetaDataMap MetaEngine::getXmpTagsDataList(const QStringList& xmpKeysFilter, bool invertSelection) const
{

#ifdef _XMP_SUPPORT_

    if (d->xmpMetadata().empty())
    {
       return MetaDataMap();
    }

    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::XmpData xmpData = d->xmpMetadata();
        xmpData.sortByKey();

        MetaDataMap metaDataMap;

        for (Exiv2::XmpData::const_iterator md = xmpData.begin() ; md != xmpData.end() ; ++md)
        {
            QString key = QLatin1String(md->key().c_str());

            // Decode the tag value with a user friendly output.

            std::ostringstream os;
            os << *md;
            QString value = QString::fromStdString(os.str());

            // If the tag is a language alternative type, parse content to detect language.

            if (md->typeId() == Exiv2::langAlt)
            {
                QString lang;
                value = detectLanguageAlt(value, lang);
            }
            else
            {
                value = QString::fromStdString(os.str());
            }

            // To make a string just on one line.

            value.replace(QLatin1Char('\n'), QLatin1String(" "));

            // Some XMP key are redondancy. check if already one exist...

            MetaDataMap::const_iterator it = metaDataMap.constFind(key);

            // We apply a filter to get only the XMP tags that we need.

            if (!xmpKeysFilter.isEmpty())
            {
                if (!invertSelection)
                {
                    if (xmpKeysFilter.contains(key.section(QLatin1Char('.'), 1, 1)))
                    {
                        if (it == metaDataMap.constEnd())
                        {
                            metaDataMap.insert(key, value);
                        }
                        else
                        {
                            QString v = *it;
                            v.append(QLatin1String(", "));
                            v.append(value);
                            metaDataMap.insert(key, v);
                        }
                    }
                }
                else
                {
                    if (!xmpKeysFilter.contains(key.section(QLatin1Char('.'), 1, 1)))
                    {
                        if (it == metaDataMap.constEnd())
                        {
                            metaDataMap.insert(key, value);
                        }
                        else
                        {
                            QString v = *it;
                            v.append(QLatin1String(", "));
                            v.append(value);
                            metaDataMap.insert(key, v);
                        }
                    }
                }
            }
            else // else no filter at all.
            {
                if (it == metaDataMap.constEnd())
                {
                    metaDataMap.insert(key, value);
                }
                else
                {
                    QString v = *it;
                    v.append(QLatin1String(", "));
                    v.append(value);
                    metaDataMap.insert(key, v);
                }
            }
        }

        return metaDataMap;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot parse Xmp metadata with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpKeysFilter);
    Q_UNUSED(invertSelection);

#endif // _XMP_SUPPORT_

    return MetaDataMap();
}

QString MetaEngine::getXmpTagTitle(const char* xmpTagName)
{

#ifdef _XMP_SUPPORT_

    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        std::string xmpkey(xmpTagName);
        Exiv2::XmpKey xk(xmpkey);

        return QString::fromLocal8Bit(Exiv2::XmpProperties::propertyTitle(xk));
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot get Xmp metadata tag title with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);

#endif // _XMP_SUPPORT_

    return QString();
}

QString MetaEngine::getXmpTagDescription(const char* xmpTagName)
{

#ifdef _XMP_SUPPORT_

    try
    {
        std::string xmpkey(xmpTagName);
        Exiv2::XmpKey xk(xmpkey);

        return QString::fromLocal8Bit(Exiv2::XmpProperties::propertyDesc(xk));
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot get Xmp metadata tag description with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);

#endif // _XMP_SUPPORT_

    return QString();
}

QString MetaEngine::getXmpTagString(const char* xmpTagName, bool escapeCR) const
{

#ifdef _XMP_SUPPORT_

    try
    {
        Exiv2::XmpData xmpData(d->xmpMetadata());
        Exiv2::XmpKey key(xmpTagName);
        Exiv2::XmpData::const_iterator it = xmpData.findKey(key);

        if (it != xmpData.end())
        {
            std::ostringstream os;
            os << *it;
            QString tagValue = QString::fromStdString(os.str());

            if (escapeCR)
            {
                tagValue.replace(QLatin1Char('\n'), QLatin1String(" "));
            }

            return tagValue;
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Xmp key '%1' into image with Exiv2:")
                                    .arg(QLatin1String(xmpTagName)), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(escapeCR);

#endif // _XMP_SUPPORT_

    return QString();
}

bool MetaEngine::setXmpTagString(const char* xmpTagName, const QString& value) const
{

#ifdef _XMP_SUPPORT_

    try
    {
        Exiv2::Value::AutoPtr xmpTxtVal = Exiv2::Value::create(Exiv2::xmpText);
        xmpTxtVal->read(value.toStdString());
        d->xmpMetadata()[xmpTagName].setValue(xmpTxtVal.get());

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Xmp tag string into image with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(value);

#endif // _XMP_SUPPORT_

    return false;
}
bool MetaEngine::setXmpTagString(const char* xmpTagName,
                                 const QString& value,
                                 MetaEngine::XmpTagType type) const
{
#ifdef _XMP_SUPPORT_

    try
    {
        Exiv2::XmpTextValue xmpTxtVal("");

        if (type == MetaEngine::NormalTag)      // normal type
        {
            xmpTxtVal.read(value.toStdString());
            d->xmpMetadata().add(Exiv2::XmpKey(xmpTagName), &xmpTxtVal);

            return true;
        }

        if (type == MetaEngine::ArrayBagTag)    // xmp type = bag
        {
            xmpTxtVal.setXmpArrayType(Exiv2::XmpValue::xaBag);
            xmpTxtVal.read("");
            d->xmpMetadata().add(Exiv2::XmpKey(xmpTagName), &xmpTxtVal);

            return true;
        }

        if (type == MetaEngine::StructureTag)   // xmp type = struct
        {
            xmpTxtVal.setXmpStruct();
            d->xmpMetadata().add(Exiv2::XmpKey(xmpTagName), &xmpTxtVal);

            return true;
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Xmp tag string into image with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(value);

#endif // _XMP_SUPPORT_

    return false;
}

MetaEngine::AltLangMap MetaEngine::getXmpTagStringListLangAlt(const char* xmpTagName, bool escapeCR) const
{

#ifdef _XMP_SUPPORT_

    try
    {
        Exiv2::XmpData xmpData = d->xmpMetadata();

        for (Exiv2::XmpData::const_iterator it = xmpData.begin() ; it != xmpData.end() ; ++it)
        {
            if ((it->key() == xmpTagName) && (it->typeId() == Exiv2::langAlt))
            {
                AltLangMap map;
                const Exiv2::LangAltValue &value = static_cast<const Exiv2::LangAltValue &>(it->value());

                for (Exiv2::LangAltValue::ValueType::const_iterator it2 = value.value_.begin() ;
                     it2 != value.value_.end() ; ++it2)
                {
                    QString lang = QString::fromUtf8(it2->first.c_str());
                    QString text = QString::fromUtf8(it2->second.c_str());

                    if (escapeCR)
                    {
                        text.replace(QLatin1Char('\n'), QLatin1String(" "));
                    }

                    map.insert(lang, text);
                }

                return map;
            }
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Xmp key '%1' into image with Exiv2:")
                                    .arg(QLatin1String(xmpTagName)), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(escapeCR);

#endif // _XMP_SUPPORT_

    return AltLangMap();
}

bool MetaEngine::setXmpTagStringListLangAlt(const char* xmpTagName, const MetaEngine::AltLangMap& values) const
{

#ifdef _XMP_SUPPORT_

    try
    {
        // Remove old XMP alternative Language tag.

        removeXmpTag(xmpTagName);

        if (!values.isEmpty())
        {
            Exiv2::Value::AutoPtr xmpTxtVal = Exiv2::Value::create(Exiv2::langAlt);

            for (AltLangMap::const_iterator it = values.constBegin() ; it != values.constEnd() ; ++it)
            {
                QString lang       = it.key();
                QString text       = it.value();
                QString txtLangAlt = QString::fromLatin1("lang=%1 %2").arg(lang).arg(text);
                xmpTxtVal->read(txtLangAlt.toStdString());
            }

            // ...and add the new one instead.

            d->xmpMetadata().add(Exiv2::XmpKey(xmpTagName), xmpTxtVal.get());
        }

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Xmp tag string lang-alt into image with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(values);

#endif // _XMP_SUPPORT_

    return false;
}

QString MetaEngine::getXmpTagStringLangAlt(const char* xmpTagName, const QString& langAlt, bool escapeCR) const
{

#ifdef _XMP_SUPPORT_

    try
    {
        Exiv2::XmpData xmpData(d->xmpMetadata());
        Exiv2::XmpKey key(xmpTagName);

        for (Exiv2::XmpData::const_iterator it = xmpData.begin() ; it != xmpData.end() ; ++it)
        {
            if ((it->key() == xmpTagName) && (it->typeId() == Exiv2::langAlt))
            {
                for (int i = 0 ; i < (int)it->count() ; ++i)
                {
                    std::ostringstream os;
                    os << it->toString(i);
                    QString lang;
                    QString tagValue = QString::fromStdString(os.str());
                    tagValue         = detectLanguageAlt(tagValue, lang);

                    if (langAlt == lang)
                    {
                        if (escapeCR)
                        {
                            tagValue.replace(QLatin1Char('\n'), QLatin1String(" "));
                        }

                        return tagValue;
                    }
                }
            }
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Xmp key '%1' into image with Exiv2:")
                                    .arg(QLatin1String(xmpTagName)), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(langAlt);
    Q_UNUSED(escapeCR);

#endif // _XMP_SUPPORT_

    return QString();
}

bool MetaEngine::setXmpTagStringLangAlt(const char* xmpTagName,
                                        const QString& value,
                                        const QString& langAlt) const
{

#ifdef _XMP_SUPPORT_

    try
    {
        QString language(QLatin1String("x-default")); // default alternative language.

        if (!langAlt.isEmpty())
        {
            language = langAlt;
        }

        QString txtLangAlt              = QString::fromLatin1("lang=%1 %2").arg(language).arg(value);
        Exiv2::Value::AutoPtr xmpTxtVal = Exiv2::Value::create(Exiv2::langAlt);

        // Search if an Xmp tag already exist.

        AltLangMap map = getXmpTagStringListLangAlt(xmpTagName, false);

        if (!map.isEmpty())
        {
            for (AltLangMap::const_iterator it = map.constBegin() ; it != map.constEnd() ; ++it)
            {
                if (it.key() != langAlt)
                {
                    xmpTxtVal->read((*it).toStdString());
                    qCDebug(DIGIKAM_METAENGINE_LOG) << *it;
                }
            }
        }

        xmpTxtVal->read(txtLangAlt.toStdString());
        removeXmpTag(xmpTagName);
        d->xmpMetadata().add(Exiv2::XmpKey(xmpTagName), xmpTxtVal.get());

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Xmp tag string lang-alt into image with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(value);
    Q_UNUSED(langAlt);

#endif // _XMP_SUPPORT_

    return false;
}

QStringList MetaEngine::getXmpTagStringSeq(const char* xmpTagName, bool escapeCR) const
{

#ifdef _XMP_SUPPORT_

    try
    {
        Exiv2::XmpData xmpData(d->xmpMetadata());
        Exiv2::XmpKey key(xmpTagName);
        Exiv2::XmpData::const_iterator it = xmpData.findKey(key);

        if (it != xmpData.end())
        {
            if (it->typeId() == Exiv2::xmpSeq)
            {
                QStringList seq;

                for (int i = 0 ; i < (int)it->count() ; ++i)
                {
                    std::ostringstream os;
                    os << it->toString(i);
                    QString seqValue = QString::fromStdString(os.str());

                    if (escapeCR)
                    {
                        seqValue.replace(QLatin1Char('\n'), QLatin1String(" "));
                    }

                    seq.append(seqValue);
                }

                qCDebug(DIGIKAM_METAENGINE_LOG) << "XMP String Seq (" << xmpTagName << "): " << seq;

                return seq;
            }
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Xmp key '%1' into image with Exiv2:")
                                    .arg(QLatin1String(xmpTagName)), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(escapeCR);

#endif // _XMP_SUPPORT_

    return QStringList();
}

bool MetaEngine::setXmpTagStringSeq(const char* xmpTagName, const QStringList& seq) const
{
#ifdef _XMP_SUPPORT_

    try
    {
        if (seq.isEmpty())
        {
            removeXmpTag(xmpTagName);
        }
        else
        {
            const QStringList list          = seq;
            Exiv2::Value::AutoPtr xmpTxtSeq = Exiv2::Value::create(Exiv2::xmpSeq);

            for (QStringList::const_iterator it = list.constBegin() ; it != list.constEnd() ; ++it)
            {
                xmpTxtSeq->read((*it).toStdString());
            }

            d->xmpMetadata()[xmpTagName].setValue(xmpTxtSeq.get());
        }

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Xmp tag string Seq into image with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(seq);

#endif // _XMP_SUPPORT_

    return false;
}

QStringList MetaEngine::getXmpTagStringBag(const char* xmpTagName, bool escapeCR) const
{

#ifdef _XMP_SUPPORT_

    try
    {
        Exiv2::XmpData xmpData(d->xmpMetadata());
        Exiv2::XmpKey key(xmpTagName);
        Exiv2::XmpData::const_iterator it = xmpData.findKey(key);

        if (it != xmpData.end())
        {
            if (it->typeId() == Exiv2::xmpBag)
            {
                QStringList bag;

                for (int i = 0 ; i < (int)it->count() ; ++i)
                {
                    std::ostringstream os;
                    os << it->toString(i);
                    QString bagValue = QString::fromStdString(os.str());

                    if (escapeCR)
                    {
                        bagValue.replace(QLatin1Char('\n'), QLatin1String(" "));
                    }

                    bag.append(bagValue);
                }

                return bag;
            }
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Xmp key '%1' into image with Exiv2:")
                                    .arg(QLatin1String(xmpTagName)), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(escapeCR);

#endif // _XMP_SUPPORT_

    return QStringList();
}

bool MetaEngine::setXmpTagStringBag(const char* xmpTagName, const QStringList& bag) const
{

#ifdef _XMP_SUPPORT_

    try
    {
        if (bag.isEmpty())
        {
            removeXmpTag(xmpTagName);
        }
        else
        {
            QStringList list                = bag;
            Exiv2::Value::AutoPtr xmpTxtBag = Exiv2::Value::create(Exiv2::xmpBag);

            for (QStringList::const_iterator it = list.constBegin() ; it != list.constEnd() ; ++it)
            {
                xmpTxtBag->read((*it).toStdString());
            }

            d->xmpMetadata()[xmpTagName].setValue(xmpTxtBag.get());
        }

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Xmp tag string Bag into image with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(bag);

#endif // _XMP_SUPPORT_

    return false;
}

bool MetaEngine::addToXmpTagStringBag(const char* xmpTagName, const QStringList& entriesToAdd) const
{
    QStringList oldEntries = getXmpTagStringBag(xmpTagName, false);
    QStringList newEntries = entriesToAdd;

    // Create a list of keywords including old one which already exists.

    for (QStringList::const_iterator it = oldEntries.constBegin() ; it != oldEntries.constEnd() ; ++it)
    {
        if (!newEntries.contains(*it))
        {
            newEntries.append(*it);
        }
    }

    if (setXmpTagStringBag(xmpTagName, newEntries))
    {
        return true;
    }

    return false;
}

bool MetaEngine::removeFromXmpTagStringBag(const char* xmpTagName, const QStringList& entriesToRemove) const
{
    QStringList currentEntries = getXmpTagStringBag(xmpTagName, false);
    QStringList newEntries;

    // Create a list of current keywords except those that shall be removed

    for (QStringList::const_iterator it = currentEntries.constBegin() ; it != currentEntries.constEnd() ; ++it)
    {
        if (!entriesToRemove.contains(*it))
        {
            newEntries.append(*it);
        }
    }

    if (setXmpTagStringBag(xmpTagName, newEntries))
    {
        return true;
    }

    return false;
}

QVariant MetaEngine::getXmpTagVariant(const char* xmpTagName, bool rationalAsListOfInts, bool stringEscapeCR) const
{

#ifdef _XMP_SUPPORT_

    try
    {
        Exiv2::XmpData xmpData(d->xmpMetadata());
        Exiv2::XmpKey key(xmpTagName);
        Exiv2::XmpData::const_iterator it = xmpData.findKey(key);

        if (it != xmpData.end())
        {
            // We get the Exiv2::xmpText type for all XMP.exif.* metadata.
            // Check XMP tag names and set the correct type.
            // Possibly an Exiv2 bug?

            Exiv2::TypeId xmpTypeId = it->typeId();
            QString tagName         = QLatin1String(xmpTagName);

            if      (
                     (tagName == QLatin1String("Xmp.exif.ShutterSpeedValue")) ||
                     (tagName == QLatin1String("Xmp.exif.ApertureValue"))     ||
                     (tagName == QLatin1String("Xmp.exif.ExposureTime"))      ||
                     (tagName == QLatin1String("Xmp.exif.FocalLength"))       ||
                     (tagName == QLatin1String("Xmp.exif.FNumber"))
                    )
            {
                xmpTypeId            = Exiv2::unsignedRational;
                rationalAsListOfInts = false;
            }
            else if (
                     (tagName == QLatin1String("Xmp.exif.FocalLengthIn35mmFilm")) ||
                     (tagName == QLatin1String("Xmp.exif.SubjectDistanceRange"))  ||
                     (tagName == QLatin1String("Xmp.exif.ExposureProgram"))       ||
                     (tagName == QLatin1String("Xmp.exif.ISOSpeedRatings"))       ||
                     (tagName == QLatin1String("Xmp.exif.Flash/exif:Mode"))       ||
                     (tagName == QLatin1String("Xmp.exif.SubjectDistance"))       ||
                     (tagName == QLatin1String("Xmp.exif.ISOSpeedRatings"))       ||
                     (tagName == QLatin1String("Xmp.exif.ExposureMode"))          ||
                     (tagName == QLatin1String("Xmp.exif.WhiteBalance"))          ||
                     (tagName == QLatin1String("Xmp.exif.MeteringMode"))
                    )
            {
                xmpTypeId = Exiv2::unsignedShort;
            }

            switch (xmpTypeId)
            {
                case Exiv2::unsignedByte:
                case Exiv2::unsignedShort:
                case Exiv2::unsignedLong:
                case Exiv2::signedShort:
                case Exiv2::signedLong:
                {

#if EXIV2_TEST_VERSION(0,27,99)

                    return QVariant((int)it->toInt64());

#else

                    return QVariant((int)it->toLong());

#endif

                }

                case Exiv2::unsignedRational:
                case Exiv2::signedRational:
                {
                    if (rationalAsListOfInts)
                    {
                        if (!(*it).count())
                        {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                            return QVariant(QMetaType(QMetaType::QVariantList));

#else

                            return QVariant(QVariant::List);

#endif

                        }

                        QList<QVariant> list;
                        list << (*it).toRational().first;
                        list << (*it).toRational().second;

                        return QVariant(list);
                    }
                    else
                    {
                        if (!(*it).count())
                        {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                            return QVariant(QMetaType(QMetaType::Double));

#else

                            return QVariant(QVariant::Double);

#endif

                        }

                        // prefer double precision

                        double num = (*it).toRational().first;
                        double den = (*it).toRational().second;

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
                    os << *it;
                    QString tagValue = QString::fromStdString(os.str());

                    if (stringEscapeCR)
                    {
                        tagValue.replace(QLatin1Char('\n'), QLatin1String(" "));
                    }

                    return QVariant(tagValue);
                }

                case Exiv2::xmpText:
                {
                    std::ostringstream os;
                    os << *it;
                    QString tagValue = QString::fromStdString(os.str());

                    if (stringEscapeCR)
                    {
                        tagValue.replace(QLatin1Char('\n'), QLatin1String(" "));
                    }

                    return tagValue;
                }

                case Exiv2::xmpBag:
                case Exiv2::xmpSeq:
                case Exiv2::xmpAlt:
                {
                    QStringList list;

                    for (int i = 0 ; i < (int)it->count() ; ++i)
                    {
                        list << QString::fromStdString(it->toString(i));
                    }

                    return list;
                }

                case Exiv2::langAlt:
                {
                    // access the value directly

                    const Exiv2::LangAltValue &value = static_cast<const Exiv2::LangAltValue &>(it->value());
                    QMap<QString, QVariant> map;

                    // access the ValueType std::map< std::string, std::string>

                    Exiv2::LangAltValue::ValueType::const_iterator i;

                    for (i = value.value_.begin() ; i != value.value_.end() ; ++i)
                    {
                        map[QString::fromUtf8(i->first.c_str())] = QString::fromUtf8(i->second.c_str());
                    }

                    return map;
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
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Xmp key '%1' into image with Exiv2:")
                                    .arg(QLatin1String(xmpTagName)), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(rationalAsListOfInts);
    Q_UNUSED(stringEscapeCR);

#endif // _XMP_SUPPORT_

    return QVariant();
}

bool MetaEngine::registerXmpNameSpace(const QString& uri, const QString& prefix)
{

#ifdef _XMP_SUPPORT_

    try
    {
        QString ns = uri;

        if (!uri.endsWith(QLatin1Char('/')))
        {
            ns.append(QLatin1Char('/'));
        }

        Exiv2::XmpProperties::registerNs(ns.toLatin1().constData(), prefix.toLatin1().constData());

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        Private::printExiv2ExceptionError(QLatin1String("Cannot register a new Xmp namespace with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(uri);
    Q_UNUSED(prefix);

#endif // _XMP_SUPPORT_

    return false;
}

bool MetaEngine::unregisterXmpNameSpace(const QString& uri)
{

#ifdef _XMP_SUPPORT_

    try
    {
        QString ns = uri;

        if (!uri.endsWith(QLatin1Char('/')))
        {
            ns.append(QLatin1Char('/'));
        }

        Exiv2::XmpProperties::unregisterNs(ns.toLatin1().constData());

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        Private::printExiv2ExceptionError(QLatin1String("Cannot unregister a new Xmp namespace with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(uri);

#endif // _XMP_SUPPORT_

    return false;
}

bool MetaEngine::removeXmpTag(const char* xmpTagName, bool family) const
{

#ifdef _XMP_SUPPORT_

    try
    {
        Exiv2::XmpKey xmpKey(xmpTagName);
        Exiv2::XmpData::iterator it = d->xmpMetadata().findKey(xmpKey);

        if (it != d->xmpMetadata().end())
        {
            if (!family)
            {
                d->xmpMetadata().erase(it);
            }
            else
            {
                d->xmpMetadata().eraseFamily(it);
            }

            return true;
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot remove Xmp tag with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);

#endif // _XMP_SUPPORT_

    return false;
}

QStringList MetaEngine::getXmpKeywords() const
{
    return (getXmpTagStringBag("Xmp.dc.subject", false));
}

bool MetaEngine::setXmpKeywords(const QStringList& newKeywords) const
{
    return addToXmpTagStringBag("Xmp.dc.subject", newKeywords);
}

bool MetaEngine::removeXmpKeywords(const QStringList& keywordsToRemove)
{
    return removeFromXmpTagStringBag("Xmp.dc.subject", keywordsToRemove);
}

QStringList MetaEngine::getXmpSubCategories() const
{
    return (getXmpTagStringBag("Xmp.photoshop.SupplementalCategories", false));
}

bool MetaEngine::setXmpSubCategories(const QStringList& newSubCategories) const
{
    return addToXmpTagStringBag("Xmp.photoshop.SupplementalCategories", newSubCategories);
}

bool MetaEngine::removeXmpSubCategories(const QStringList& subCategoriesToRemove)
{
    return removeFromXmpTagStringBag("Xmp.photoshop.SupplementalCategories", subCategoriesToRemove);
}

QStringList MetaEngine::getXmpSubjects() const
{
    return (getXmpTagStringBag("Xmp.iptc.SubjectCode", false));
}

bool MetaEngine::setXmpSubjects(const QStringList& newSubjects) const
{
    return addToXmpTagStringBag("Xmp.iptc.SubjectCode", newSubjects);
}

bool MetaEngine::removeXmpSubjects(const QStringList& subjectsToRemove)
{
    return removeFromXmpTagStringBag("Xmp.iptc.SubjectCode", subjectsToRemove);
}

MetaEngine::TagsMap MetaEngine::getXmpTagsList() const
{
    TagsMap tagsMap;
    d->getXMPTagsListFromPrefix(QLatin1String("acdsee"),         tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("audio"),          tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("aux"),            tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("crs"),            tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("dc"),             tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("digiKam"),        tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("dwc"),            tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("exif"),           tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("iptc"),           tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("iptcExt"),        tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("kipi"),           tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("lr"),             tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("MicrosoftPhoto"), tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("MP"),             tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("mwg-rs"),         tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("pdf"),            tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("photoshop"),      tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("plus"),           tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("tiff"),           tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("video"),          tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("xmp"),            tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("xmpBJ"),          tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("xmpDM"),          tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("xmpMM"),          tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("xmpRights"),      tagsMap);
    d->getXMPTagsListFromPrefix(QLatin1String("xmpTPg"),         tagsMap);

    return tagsMap;
}

} // namespace Digikam

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif
