/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-13
 * Description : slide show settings container.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "slideshowsettings.h"

// Qt includes

#include <QRandomGenerator>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Local includes

#include "metaenginesettings.h"

using namespace Digikam;

namespace DigikamGenericSlideShowPlugin
{

void SlideShowSettings::readFromConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName);

    startWithCurrent          = group.readEntry(configSlideShowStartCurrentEntry,         false);
    delay                     = group.readEntry(configSlideShowDelayEntry,                5);
    loop                      = group.readEntry(configSlideShowLoopEntry,                 false);
    suffle                    = group.readEntry(configSlideShowSuffleEntry,               false);
    printLabels               = group.readEntry(configSlideShowPrintLabelsEntry,          false);
    printRating               = group.readEntry(configSlideShowPrintRatingEntry,          false);
    showProgressIndicator     = group.readEntry(configSlideShowProgressIndicatorEntry,    true);
    bgColor                   = group.readEntry(configSlideShowBgColorEntry,              QColor(Qt::black));
    slideScreen               = group.readEntry(configSlideScreenEntry,                   -2);
    exifRotate                = MetaEngineSettings::instance()->settings().exifRotate;

    osdSettings.readFromConfig(configGroupName);
}

void SlideShowSettings::writeToConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName);

    group.writeEntry(configSlideShowStartCurrentEntry,         startWithCurrent);
    group.writeEntry(configSlideShowDelayEntry,                delay);
    group.writeEntry(configSlideShowLoopEntry,                 loop);
    group.writeEntry(configSlideShowSuffleEntry,               suffle);
    group.writeEntry(configSlideShowPrintLabelsEntry,          printLabels);
    group.writeEntry(configSlideShowPrintRatingEntry,          printRating);
    group.writeEntry(configSlideShowProgressIndicatorEntry,    showProgressIndicator);
    group.writeEntry(configSlideShowBgColorEntry,              bgColor);
    group.writeEntry(configSlideScreenEntry,                   slideScreen);
    group.sync();

    osdSettings.writeToConfig(configGroupName);
}

int SlideShowSettings::indexOf(const QUrl& url) const
{
    return fileList.indexOf(url);
}

int SlideShowSettings::count() const
{
    return fileList.count();
}

void SlideShowSettings::suffleImages()
{
    if (suffle && autoPlayEnabled)
    {
        if (originalFileList.isEmpty())
        {
            // keep a backup of original file list at the first suffle

            originalFileList         = fileList;

            // suffle


            QList<QUrl>::iterator it = fileList.begin();
            QList<QUrl>::iterator it1;

            for (uint i = 0 ; i < (uint)count() ; ++i)
            {
                int inc = QRandomGenerator::global()->bounded(count());

                it1     = fileList.begin();
                it1    += inc;

                std::swap(*(it++), *(it1));
             }
        }
    }
    else
    {
        if (!originalFileList.isEmpty())
        {
            // return to original list

            fileList = originalFileList;
            originalFileList.clear();
        }
    }
}

} // namespace DigikamGenericSlideShowPlugin
