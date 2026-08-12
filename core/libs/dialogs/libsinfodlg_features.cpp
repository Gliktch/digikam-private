/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-07-11
 * Description : shared libraries list dialog common to digiKam and Showfoto
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "libsinfodlg_p.h"

namespace Digikam
{

void LibsInfoDlg::populateFeatures()
{
    // NOTE: by default set a list of common components information used by Showfoto and digiKam.

    m_features = new QTreeWidgetItem(listView(), QStringList() << i18nc("@title", "Features"));
    listView()->addTopLevelItem(m_features);

    new QTreeWidgetItem(m_features, QStringList() <<
                        i18nc(Private::CONTEXT, "Parallelized demosaicing") <<        d->checkTriState(DRawDecoder::librawUseGomp()));

#ifdef HAVE_QTXMLPATTERNS

    new QTreeWidgetItem(m_features, QStringList() <<
                        i18nc(Private::CONTEXT, "Rajce support") <<                   d->supportedYes());

#else

    new QTreeWidgetItem(m_features, QStringList() <<
                        i18nc(Private::CONTEXT, "Rajce support") <<                   d->supportedNo());

#endif

    new QTreeWidgetItem(m_features, QStringList() <<
                        i18nc(Private::CONTEXT, "Exiv2 supports XMP metadata") <<     (MetaEngine::supportXmp() ?  d->supportedYes() : d->supportedNo()));

    new QTreeWidgetItem(m_features, QStringList() <<
                        i18nc(Private::CONTEXT, "Exiv2 supports Base Media") <<       (MetaEngine::supportBmff() ? d->supportedYes() : d->supportedNo()));

    new QTreeWidgetItem(m_features, QStringList() <<
                        i18nc(Private::CONTEXT, "Exiv2 supports JPEG-XL metadata") << (MetaEngine::supportJpegXL() ? d->supportedYes() : d->supportedNo()));

#ifdef HAVE_QMULTIMEDIA

    new QTreeWidgetItem(m_features, QStringList() <<
                        i18nc(Private::CONTEXT, "Qt Multimedia support") <<          d->supportedYes());

#else

    new QTreeWidgetItem(m_features, QStringList() <<
                        i18nc(Private::CONTEXT, "Qt Multimedia support") <<          d->supportedYes());

#endif

#ifdef HAVE_SONNET

    new QTreeWidgetItem(m_features, QStringList() <<
                        i18nc(Private::CONTEXT, "Spell-Checking support") <<         d->supportedYes());

#else

    new QTreeWidgetItem(m_features, QStringList() <<
                        i18nc(Private::CONTEXT, "Spell-Checking support") <<         d->supportedNo());

#endif

    int nbcore         = QThread::idealThreadCount();
    new QTreeWidgetItem(m_features, QStringList() <<
                        i18ncp(Private::CONTEXT, "CPU core", "CPU cores", nbcore) << QString::fromLatin1("%1").arg(nbcore));

    KMemoryInfo memInfo;

    if (!memInfo.isNull())
    {
        quint64 available = memInfo.totalPhysical();

        if (available > 0)
        {
            new QTreeWidgetItem(m_features, QStringList() <<
                                i18nc(Private::CONTEXT, "Memory available") << ItemPropertiesTab::humanReadableBytesCount(available));
        }
        else
        {
            new QTreeWidgetItem(m_features, QStringList() <<
                                i18nc(Private::CONTEXT, "Memory available") << i18nc("@item: information about memory", "Unknown"));
        }
    }

    quint64 cacheSize = LoadingCache::cache()->getCacheSize();

    new QTreeWidgetItem(m_features, QStringList() <<
                        i18nc(Private::CONTEXT, "Image cache size") << ItemPropertiesTab::humanReadableBytesCount(cacheSize));
}

} // namespace Digikam
