/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-12-25
 * Description : On Screen Display preview overlay.
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itempreviewosd.h"

// Qt includes

#include <QTextDocument>
#include <QApplication>
#include <QDateTime>
#include <QPixmap>
#include <QLocale>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "itempropertiestab.h"
#include "albummanager.h"
#include "photoinfocontainer.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemPreviewOsd::Private
{
public:

    Private() = default;

public:

    ItemInfo            info;

    bool                enabled      = false;

    PreviewOsdSettings* settings     = nullptr;
};

ItemPreviewOsd::ItemPreviewOsd(PreviewOsdSettings* const settings, QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setFocusPolicy(Qt::NoFocus);

    d->settings = settings;
}

ItemPreviewOsd::~ItemPreviewOsd()
{
    delete d;
}

void ItemPreviewOsd::setItemInfo(const ItemInfo& inf)
{
    d->info = inf;

    update();
}

void ItemPreviewOsd::setOsdEnabled(bool val)
{
    d->enabled = val;

    update();
}

void ItemPreviewOsd::paintEvent(QPaintEvent*)
{
    if (!d->enabled || d->info.isNull())
    {
        return;
    }

    QPainter p(this);
    p.setFont(d->settings->captionFont);

    QString str;

    PhotoInfoContainer photoInfo = d->info.photoInfoContainer();
    QString comment              = d->info.comment();
    QString title                = d->info.title();
    QStringList tags             = AlbumManager::instance()->tagNames(d->info.tagIds());
    int offset                   = 0;

    // Display tag names.

    if (d->settings->printKeywords)
    {
        PreviewOsdSettings::printTags(p, offset, tags);
    }

    // Display Titles.

    if (d->settings->printTitle)
    {
        str.clear();

        if (!title.isEmpty())
        {
            str += title;
            PreviewOsdSettings::printInfoText(p, offset, str);
        }
    }

    // Display Captions if no Titles.

    if (d->settings->printCapIfNoTitle)
    {
        str.clear();

        if (title.isEmpty())
        {
            str += comment;
            PreviewOsdSettings::printComments(p, offset, str);
        }
    }

    // Display Comments.

    if (d->settings->printCaption)
    {
        str = comment;
        PreviewOsdSettings::printComments(p, offset, str);
    }

    // Display Make and Model.

    if (d->settings->printMakeModel)
    {
        str.clear();

        QString make  = photoInfo.make;
        QString model = photoInfo.model;

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

    if (d->settings->printLensModel)
    {
        str.clear();

        QString lens = photoInfo.lens;

        if (!lens.isEmpty())
        {
            str = lens;
            PreviewOsdSettings::printInfoText(p, offset, str);
        }
    }

    // Display Exposure and Sensitivity.

    if (d->settings->printExpoSensitivity)
    {
        str.clear();

        QString exposureTime = photoInfo.exposureTime;
        QString sensitivity  = photoInfo.sensitivity;

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

    if (d->settings->printApertureFocal)
    {
        str.clear();

        QString aperture        = photoInfo.aperture;
        QString focalLength     = photoInfo.focalLength;
        QString focalLength35mm = photoInfo.focalLength35mm;

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

    if (d->settings->printDate)
    {
        QDateTime dateTime = d->info.dateTime();

        if (dateTime.isValid())
        {
            str = asUserDateTime(dateTime);
            PreviewOsdSettings::printInfoText(p, offset, str);
        }
    }

    // Display image File Name.

    if (d->settings->printName)
    {
        PreviewOsdSettings::printInfoText(p, offset, d->info.name());
    }
}

} // namespace Digikam

#include "moc_itempreviewosd.cpp"
