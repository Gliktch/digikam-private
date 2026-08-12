/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               Iptc manipulation methods.
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

bool MetaEngine::canWriteIptc(const QString& filePath)
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(filePath.toUtf8().constData());
        Exiv2::AccessMode mode      = image->checkMode(Exiv2::mdIptc);

        return ((mode == Exiv2::amWrite) || (mode == Exiv2::amReadWrite));
    }
    catch (Exiv2::AnyError& e)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Cannot check Iptc access mode with Exiv2:(Error #"
                                           << (int)e.code() << ": " << QString::fromStdString(e.what()) << ")";
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::hasIptc() const
{
    return !d->iptcMetadata().empty();
}

bool MetaEngine::clearIptc() const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        d->iptcMetadata().clear();
        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot clear Iptc data with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QByteArray MetaEngine::getIptc(bool addIrbHeader) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        if (!d->iptcMetadata().empty())
        {
            Exiv2::IptcData& iptc = d->iptcMetadata();
            Exiv2::DataBuf c2;

            if (addIrbHeader)
            {
                c2 = Exiv2::Photoshop::setIptcIrb(nullptr, 0, iptc);
            }
            else
            {
                c2 = Exiv2::IptcParser::encode(d->iptcMetadata());
            }

#if EXIV2_TEST_VERSION(0,27,99)

            QByteArray _data(reinterpret_cast<const char*>(c2.data()), c2.size());

#else

            QByteArray _data(reinterpret_cast<const char*>(c2.pData_), c2.size_);

#endif

            return _data;
        }
    }
    catch (Exiv2::AnyError& e)
    {
        if (!d->filePath.isEmpty())
        {
            qCCritical(DIGIKAM_METAENGINE_LOG) << "From file " << d->filePath.toLatin1().constData();
        }

        d->printExiv2ExceptionError(QLatin1String("Cannot get Iptc data with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QByteArray();
}

bool MetaEngine::setIptc(const QByteArray& data) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        if (!data.isEmpty())
        {
            Exiv2::IptcParser::decode(d->iptcMetadata(),
                                      reinterpret_cast<Exiv2::byte*>(const_cast<char*>(data.data())), data.size());
            return (!d->iptcMetadata().empty());
        }
    }
    catch (Exiv2::AnyError& e)
    {
        if (!d->filePath.isEmpty())
        {
            qCCritical(DIGIKAM_METAENGINE_LOG) << "From file " << d->filePath.toLatin1().constData();
        }

        d->printExiv2ExceptionError(QLatin1String("Cannot set Iptc data with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

MetaEngine::MetaDataMap MetaEngine::getIptcTagsDataList(const QStringList& iptcKeysFilter, bool invertSelection) const
{
    if (d->iptcMetadata().empty())
    {
       return MetaDataMap();
    }

    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::IptcData iptcData = d->iptcMetadata();
        iptcData.sortByKey();

        MetaDataMap metaDataMap;

        for (Exiv2::IptcData::const_iterator md = iptcData.begin() ; md != iptcData.end() ; ++md)
        {
            QString key = QString::fromStdString(md->key());
            QString value;

            if (key == QLatin1String("Iptc.Envelope.CharacterSet"))
            {
                value = QLatin1String(iptcData.detectCharset());
            }
            else
            {
                value = d->extractIptcTagString(iptcData, *md);
            }

            // To make a string just on one line.

            value.replace(QLatin1Char('\n'), QLatin1String(" "));

            // Some Iptc key are redondancy. check if already one exist...

            MetaDataMap::const_iterator it = metaDataMap.constFind(key);

            // We apply a filter to get only the Iptc tags that we need.

            if (!iptcKeysFilter.isEmpty())
            {
                if (!invertSelection)
                {
                    if (iptcKeysFilter.contains(key.section(QLatin1Char('.'), 1, 1)))
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
                    if (!iptcKeysFilter.contains(key.section(QLatin1Char('.'), 1, 1)))
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
        d->printExiv2ExceptionError(QLatin1String("Cannot parse Iptc metadata with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return MetaDataMap();
}

QString MetaEngine::getIptcTagTitle(const char* iptcTagName)
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        std::string iptckey(iptcTagName);
        Exiv2::IptcKey ik(iptckey);

        return QString::fromLocal8Bit(Exiv2::IptcDataSets::dataSetTitle(ik.tag(), ik.record()));
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

QString MetaEngine::getIptcTagDescription(const char* iptcTagName)
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        std::string iptckey(iptcTagName);
        Exiv2::IptcKey ik(iptckey);

        return QString::fromLocal8Bit(Exiv2::IptcDataSets::dataSetDesc(ik.tag(), ik.record()));
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

bool MetaEngine::removeIptcTag(const char* iptcTagName) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::IptcData::iterator it = d->iptcMetadata().begin();
        int i                        = 0;

        while(it != d->iptcMetadata().end())
        {
            QString key = QString::fromStdString(it->key());

            if (key == QLatin1String(iptcTagName))
            {
                it = d->iptcMetadata().erase(it);
                ++i;
            }
            else
            {
                ++it;
            }
        };

        if (i > 0)
        {
            return true;
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot remove Iptc tag with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::setIptcTagData(const char* iptcTagName, const QByteArray& data) const
{
    if (data.isEmpty())
    {
        return false;
    }

    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::DataValue val(reinterpret_cast<Exiv2::byte*>(const_cast<char*>(data.data())), data.size());
        d->iptcMetadata()[iptcTagName] = val;

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Iptc tag data into image with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QByteArray MetaEngine::getIptcTagData(const char* iptcTagName) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::IptcKey  iptcKey(iptcTagName);
        Exiv2::IptcData iptcData(d->iptcMetadata());
        Exiv2::IptcData::const_iterator it = iptcData.findKey(iptcKey);

        if (it != iptcData.end())
        {
            QByteArray _data((*it).size(), '\0');
            (*it).copy(reinterpret_cast<Exiv2::byte*>(const_cast<char*>(_data.data())), Exiv2::bigEndian);

            return _data;
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Iptc key '%1' into image with Exiv2:")
                                    .arg(QLatin1String(iptcTagName)), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QByteArray();
}

QString MetaEngine::getIptcTagString(const char* iptcTagName, bool escapeCR) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::IptcKey  iptcKey(iptcTagName);
        Exiv2::IptcData iptcData(d->iptcMetadata());
        Exiv2::IptcData::const_iterator it = iptcData.findKey(iptcKey);

        if (it != iptcData.end())
        {
            QString tagValue = d->extractIptcTagString(iptcData, *it);

            if (escapeCR)
            {
                tagValue.replace(QLatin1Char('\n'), QLatin1String(" "));
            }

            return tagValue;
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Iptc key '%1' into image with Exiv2:")
                                    .arg(QLatin1String(iptcTagName)), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QString();
}

bool MetaEngine::setIptcTagString(const char* iptcTagName, const QString& value) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        d->iptcMetadata()[iptcTagName] = value.toStdString();

        // Make sure we have set the charset to UTF-8

        d->iptcMetadata()["Iptc.Envelope.CharacterSet"] = "\33%G";

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Iptc tag string into image with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QStringList MetaEngine::getIptcTagsStringList(const char* iptcTagName, bool escapeCR) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        if (!d->iptcMetadata().empty())
        {
            QStringList values;
            Exiv2::IptcData iptcData(d->iptcMetadata());

            for (Exiv2::IptcData::const_iterator it = iptcData.begin() ; it != iptcData.end() ; ++it)
            {
                QString key = QString::fromStdString(it->key());

                if (key == QLatin1String(iptcTagName))
                {
                    QString tagValue = d->extractIptcTagString(iptcData, *it);

                    if (escapeCR)
                    {
                        tagValue.replace(QLatin1Char('\n'), QLatin1String(" "));
                    }

                    values.append(tagValue);
                }
            }

            return values;
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Iptc key '%1' into image with Exiv2:")
                                    .arg(QLatin1String(iptcTagName)), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QStringList();
}

bool MetaEngine::setIptcTagsStringList(const char* iptcTagName, int maxSize,
                                       const QStringList& oldValues,
                                       const QStringList& newValues) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        QStringList oldvals = oldValues;
        QStringList newvals = newValues;

        qCDebug(DIGIKAM_METAENGINE_LOG) << d->filePath.toLatin1().constData()
                                        << " : " << iptcTagName
                                        << " => " << newvals.join(QString::fromLatin1(",")).toLatin1().constData();

        // Remove all old values.

        Exiv2::IptcData iptcData(d->iptcMetadata());
        Exiv2::IptcData::iterator it2 = iptcData.begin();

        while (it2 != iptcData.end())
        {
            QString key = QString::fromStdString(it2->key());
            QString val = QString::fromStdString(it2->toString());

            // Also remove new values to avoid duplicates. They will be added again below.

            if (
                (key == QLatin1String(iptcTagName)) &&
                (oldvals.contains(val) || newvals.contains(val))
               )
            {
                it2 = iptcData.erase(it2);
            }
            else
            {
                ++it2;
            }
        };

        // Add new values.

        Exiv2::IptcKey iptcTag(iptcTagName);

        for (QStringList::const_iterator it = newvals.constBegin() ; it != newvals.constEnd() ; ++it)
        {
            QString key = *it;
            key.truncate(maxSize);

            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::string);
            val->read(key.toUtf8().constData());
            iptcData.add(iptcTag, val.get());
        }

        d->iptcMetadata() = iptcData;

        // Make sure character set is UTF-8

        setIptcTagString("Iptc.Envelope.CharacterSet", QLatin1String("\33%G"));

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot set Iptc key '%1' into image with Exiv2:")
                                    .arg(QLatin1String(iptcTagName)), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QStringList MetaEngine::getIptcKeywords() const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        if (!d->iptcMetadata().empty())
        {
            QStringList keywords;
            Exiv2::IptcData iptcData(d->iptcMetadata());

            for (Exiv2::IptcData::const_iterator it = iptcData.begin() ; it != iptcData.end() ; ++it)
            {
                QString key = QString::fromStdString(it->key());

                if (key == QLatin1String("Iptc.Application2.Keywords"))
                {
                    QString val = d->extractIptcTagString(iptcData, *it);
                    keywords.append(val);
                }
            }
/*
            qCDebug(DIGIKAM_METAENGINE_LOG) << d->filePath << " ==> Read Iptc Keywords: " << keywords;
*/
            return keywords;
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot get Iptc Keywords from image with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QStringList();
}

bool MetaEngine::setIptcKeywords(const QStringList& oldKeywords, const QStringList& newKeywords) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        QStringList newkeys = newKeywords;

        qCDebug(DIGIKAM_METAENGINE_LOG) << d->filePath << " ==> New Iptc Keywords: " << newkeys;

        // Remove all old keywords.

        Exiv2::IptcData iptcData(d->iptcMetadata());
        Exiv2::IptcData::iterator it2 = iptcData.begin();

        while (it2 != iptcData.end())
        {
            QString key = QString::fromStdString(it2->key());
            QString val = QString::fromStdString(it2->toString());       // FIXME: check charset

            // Also remove new keywords to avoid duplicates. They will be added again below.

            if (
                (key == QLatin1String("Iptc.Application2.Keywords")) &&
                (oldKeywords.contains(val) || newKeywords.contains(val))
               )
            {
                it2 = iptcData.erase(it2);
            }
            else
            {
                ++it2;
            }
        };

        // Add new keywords. Note that Keywords Iptc tag is limited to 64 char but can be redondant.

        Exiv2::IptcKey iptcTag("Iptc.Application2.Keywords");

        for (QStringList::const_iterator it = newkeys.constBegin() ; it != newkeys.constEnd() ; ++it)
        {
            QString key = *it;
            key.truncate(64);

            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::string);
            val->read(key.toUtf8().constData());
            iptcData.add(iptcTag, val.get());
        }

        d->iptcMetadata() = iptcData;

        // Make sure character set is UTF-8

        setIptcTagString("Iptc.Envelope.CharacterSet", QLatin1String("\33%G"));

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Iptc Keywords into image with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QStringList MetaEngine::getIptcSubjects() const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        if (!d->iptcMetadata().empty())
        {
            QStringList subjects;
            Exiv2::IptcData iptcData(d->iptcMetadata());

            for (Exiv2::IptcData::const_iterator it = iptcData.begin() ; it != iptcData.end() ; ++it)
            {
                QString key = QString::fromStdString(it->key());

                if (key == QLatin1String("Iptc.Application2.Subject"))
                {
                    QString val = d->extractIptcTagString(iptcData, *it);
                    subjects.append(val);
                }
            }

            return subjects;
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot get Iptc Subjects from image with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QStringList();
}

bool MetaEngine::setIptcSubjects(const QStringList& oldSubjects, const QStringList& newSubjects) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        QStringList oldDef = oldSubjects;
        QStringList newDef = newSubjects;

        // Remove all old subjects.

        Exiv2::IptcData iptcData(d->iptcMetadata());
        Exiv2::IptcData::iterator it2 = iptcData.begin();

        while (it2 != iptcData.end())
        {
            QString key = QString::fromStdString(it2->key());
            QString val = QString::fromStdString(it2->toString());               // FIXME: check charset

            if (key == QLatin1String("Iptc.Application2.Subject") && oldDef.contains(val))
            {
                it2 = iptcData.erase(it2);
            }
            else
            {
                ++it2;
            }
        };

        // Add new subjects. Note that Keywords Iptc tag is limited to 236 char but can be redondant.

        Exiv2::IptcKey iptcTag("Iptc.Application2.Subject");

        for (QStringList::const_iterator it = newDef.constBegin() ; it != newDef.constEnd() ; ++it)
        {
            QString key = *it;
            key.truncate(236);

            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::string);
            val->read(key.toUtf8().constData());
            iptcData.add(iptcTag, val.get());
        }

        d->iptcMetadata() = iptcData;

        // Make sure character set is UTF-8

        setIptcTagString("Iptc.Envelope.CharacterSet", QLatin1String("\33%G"));

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Iptc Subjects into image with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QStringList MetaEngine::getIptcSubCategories() const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        if (!d->iptcMetadata().empty())
        {
            QStringList subCategories;
            Exiv2::IptcData iptcData(d->iptcMetadata());

            for (Exiv2::IptcData::const_iterator it = iptcData.begin() ; it != iptcData.end() ; ++it)
            {
                QString key = QString::fromStdString(it->key());

                if (key == QLatin1String("Iptc.Application2.SuppCategory"))
                {
                    QString val = d->extractIptcTagString(iptcData, *it);
                    subCategories.append(val);
                }
            }

            return subCategories;
        }
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot get Iptc Sub Categories from image with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QStringList();
}

bool MetaEngine::setIptcSubCategories(const QStringList& oldSubCategories, const QStringList& newSubCategories) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        QStringList newkeys = newSubCategories;

        // Remove all old Sub Categories.

        Exiv2::IptcData iptcData(d->iptcMetadata());
        Exiv2::IptcData::iterator it2 = iptcData.begin();

        while (it2 != iptcData.end())
        {
            QString key = QString::fromStdString(it2->key());
            QString val = QString::fromStdString(it2->toString());       // FIXME: check charset

            if ((key == QLatin1String("Iptc.Application2.SuppCategory")) && oldSubCategories.contains(val))
            {
                it2 = iptcData.erase(it2);
            }
            else
            {
                ++it2;
            }
        };

        // Add new Sub Categories. Note that SubCategories Iptc tag is limited to 32
        // characters but can be redondant.

        Exiv2::IptcKey iptcTag("Iptc.Application2.SuppCategory");

        for (QStringList::const_iterator it = newkeys.constBegin() ; it != newkeys.constEnd() ; ++it)
        {
            QString key               = *it;
            key.truncate(32);

            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::string);
            val->read(key.toUtf8().constData());
            iptcData.add(iptcTag, val.get());
        }

        d->iptcMetadata() = iptcData;

        // Make sure character set is UTF-8

        setIptcTagString("Iptc.Envelope.CharacterSet", QLatin1String("\33%G"));

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Iptc Sub Categories into image with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

MetaEngine::TagsMap MetaEngine::getIptcTagsList() const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        QList<const Exiv2::DataSet*> tags;
        tags << Exiv2::IptcDataSets::envelopeRecordList()
             << Exiv2::IptcDataSets::application2RecordList();

        TagsMap tagsMap;

        for (QList<const Exiv2::DataSet*>::iterator it = tags.begin() ; it != tags.end() ; ++it)
        {
            do
            {
                QString key = QLatin1String(Exiv2::IptcKey( (*it)->number_, (*it)->recordId_).key().c_str());
                QStringList values;
                values << QLatin1String((*it)->name_) << QLatin1String((*it)->title_) << QLatin1String((*it)->desc_);
                tagsMap.insert(key, values);
                ++(*it);
            }
            while ((*it)->number_ != 0xffff);
        }

        return tagsMap;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot get Iptc Tags list with Exiv2:"), e);
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
