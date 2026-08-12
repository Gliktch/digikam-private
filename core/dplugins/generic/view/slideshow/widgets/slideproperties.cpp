/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-19
 * Description : slide properties widget
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "slideproperties.h"

// Qt includes

#include <QTextDocument>
#include <QApplication>
#include <QDateTime>
#include <QPixmap>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "dinfointerface.h"
#include "itempropertiestab.h"

using namespace Digikam;

namespace DigikamGenericSlideShowPlugin
{

class Q_DECL_HIDDEN SlideProperties::Private
{
public:

    Private() = default;

public:

    QUrl                     url;

    SlideShowSettings*       settings       = nullptr;

    DInfoInterface::DInfoMap infoMap;
};

SlideProperties::SlideProperties(SlideShowSettings* const settings, QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    setMouseTracking(true);
    d->settings = settings;
}

SlideProperties::~SlideProperties()
{
    delete d;
}

void SlideProperties::setCurrentUrl(const QUrl& url)
{
    d->infoMap = d->settings->iface->itemInfo(url);
    d->url     = url;

    update();
}

void SlideProperties::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setFont(d->settings->osdSettings.captionFont);

    DItemInfo item(d->infoMap);

    QString str;
/*
    PhotoInfoContainer photoInfo = d->info.photoInfo;
*/
    QString comment  = item.comment();
    QString title    = item.title();
    QStringList tags = item.keywords();
    int offset       = 0;

    // Display tag names.

    if (d->settings->osdSettings.printKeywords)
    {
        PreviewOsdSettings::printTags(p, offset, tags);
    }

    // Display Titles.

    if (d->settings->osdSettings.printTitle)
    {
        str.clear();

        if (!title.isEmpty())
        {
            str += title;
            PreviewOsdSettings::printInfoText(p, offset, str);
        }
    }

    // Display Captions if no Titles.

    if (d->settings->osdSettings.printCapIfNoTitle)
    {
        str.clear();

        if (title.isEmpty())
        {
            str += comment;
            PreviewOsdSettings::printComments(p, offset, str);
        }
    }

    // Display Comments.

    if (d->settings->osdSettings.printCaption)
    {
        str = comment;
        PreviewOsdSettings::printComments(p, offset, str);
    }

    // Display Make and Model.

    if (d->settings->osdSettings.printMakeModel)
    {
        str.clear();

        QString make  = item.make();
        QString model = item.model();

        if (!make.isEmpty())
        {
            ItemPropertiesTab::shortenedMakeInfo(make);
            str = make;
        }

        if (!model.isEmpty())
        {
            if (!make.isEmpty())
            {
                str += QLatin1String(" / ");
            }

            ItemPropertiesTab::shortenedModelInfo(model);
            str += model;
        }

        PreviewOsdSettings::printInfoText(p, offset, str);
    }

    // Display Lens model.

    if (d->settings->osdSettings.printLensModel)
    {
        str.clear();

        QString lens = item.lens();

        if (!lens.isEmpty())
        {
            str = lens;
            PreviewOsdSettings::printInfoText(p, offset, str);
        }
    }

    // Display Exposure and Sensitivity.

    if (d->settings->osdSettings.printExpoSensitivity)
    {
        str.clear();

        QString exposureTime = item.exposureTime();
        QString sensitivity  = item.sensitivity();

        if (!exposureTime.isEmpty())
        {
            str = exposureTime;
        }

        if (!sensitivity.isEmpty())
        {
            if (!exposureTime.isEmpty())
            {
                str += QLatin1String(" / ");
            }

            str += i18n("%1 ISO", sensitivity);
        }

        PreviewOsdSettings::printInfoText(p, offset, str);
    }

    // Display Aperture and Focal.

    if (d->settings->osdSettings.printApertureFocal)
    {
        str.clear();

        QString aperture        = item.aperture();
        QString focalLength     = item.focalLength();
        QString focalLength35mm = item.focalLength35mm();

        if (!aperture.isEmpty())
        {
            str = aperture;
        }

        if (focalLength35mm.isEmpty())
        {
            if (!focalLength.isEmpty())
            {
                if (!aperture.isEmpty())
                {
                    str += QLatin1String(" / ");
                }

                str += focalLength;
            }
        }
        else
        {
            if (!aperture.isEmpty())
            {
                str += QLatin1String(" / ");
            }

            if (!focalLength.isEmpty())
            {
                str += QString::fromUtf8("%1 (%2)").arg(focalLength).arg(focalLength35mm);
            }
            else
            {
                str += QString::fromUtf8("%1").arg(focalLength35mm);
            }
        }

        PreviewOsdSettings::printInfoText(p, offset, str);
    }

    // Display Creation Date.

    if (d->settings->osdSettings.printDate)
    {
        QDateTime dateTime = item.dateTime();

        if (dateTime.isValid())
        {
            str = asUserDateTime(dateTime);
            PreviewOsdSettings::printInfoText(p, offset, str);
        }
    }

    // Display image File Name.

    if (d->settings->osdSettings.printName)
    {
        PreviewOsdSettings::printInfoText(p, offset, d->url.fileName());
    }
}

} // namespace DigikamGenericSlideShowPlugin

#include "moc_slideproperties.cpp"
