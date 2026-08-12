/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-19
 * Description : A tab to display general item information
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itempropertiestab_p.h"

namespace Digikam
{

void ItemPropertiesTab::setCurrentUrl(const QUrl& url)
{
    if (url.isEmpty())
    {
        d->labelFile->setAdjustedText();
        d->labelFolder->clear();
        d->labelSymlink->setAdjustedText();
        d->labelFileModifiedDate->setAdjustedText();
        d->labelFileSize->setAdjustedText();
        d->labelFileOwner->setAdjustedText();
        d->labelFilePermissions->setAdjustedText();

        d->labelImageMime->setAdjustedText();
        d->labelImageDimensions->setAdjustedText();
        d->labelImageRatio->setAdjustedText();
        d->labelImageBitDepth->setAdjustedText();
        d->labelImageColorMode->setAdjustedText();
        d->labelHasSidecar->setAdjustedText();
        d->labelHasGPSInfo->setAdjustedText();

        d->labelPhotoMake->setAdjustedText();
        d->labelPhotoModel->setAdjustedText();
        d->labelPhotoLens->setAdjustedText();
        d->labelPhotoAperture->setAdjustedText();
        d->labelPhotoFocalLength->setAdjustedText();
        d->labelPhotoExposureTime->setAdjustedText();
        d->labelPhotoSensitivity->setAdjustedText();
        d->labelPhotoExposureMode->setAdjustedText();
        d->labelPhotoFlash->setAdjustedText();
        d->labelPhotoWhiteBalance->setAdjustedText();

        d->labelVideoAspectRatio->setAdjustedText();
        d->labelVideoDuration->setAdjustedText();
        d->labelVideoFrameRate->setAdjustedText();
        d->labelVideoVideoCodec->setAdjustedText();
        d->labelVideoAudioBitRate->setAdjustedText();
        d->labelVideoAudioChannelType->setAdjustedText();
        d->labelVideoAudioCodec->setAdjustedText();

        d->labelTitle->clear();
        d->labelCaption->clear();
        d->labelPick->clear();
        d->labelPickLabel->setAdjustedText();
        d->labelColor->clear();
        d->labelColorLabel->setAdjustedText();
        d->labelRating->clear();
        d->labelRating->setProperty("Rating", -1);
        d->labelVersionedInfo->setAdjustedText();
        d->labelGroupedInfo->setAdjustedText();

        d->labelTags->clear();
        d->labelPeoples->clear();
        d->labelPhotoDateTime->setAdjustedText();

        d->labelLocation->clear();
        d->labelRights->clear();

        setEnabled(false);
        return;
    }

    setEnabled(true);

    QFileInfo info(url.toLocalFile());

    d->labelFile->setAdjustedText(info.fileName());
    d->labelFolder->setText(QDir::toNativeSeparators(info.path()));
    d->labelSymlink->setAdjustedText(!info.isSymLink() ? i18nc("@info: item properties", "No")
                                                       : QDir::toNativeSeparators(info.canonicalPath()));
}

void ItemPropertiesTab::setPhotoInfoDisable(const bool b)
{
    if (b)
    {
        widget(ItemPropertiesTab::PhotoProperties)->hide();
    }
    else
    {
        widget(ItemPropertiesTab::PhotoProperties)->show();
    }
}

void ItemPropertiesTab::setVideoInfoDisable(const bool b)
{
    if (b)
    {
        widget(ItemPropertiesTab::VideoProperties)->hide();
    }
    else
    {
        widget(ItemPropertiesTab::VideoProperties)->show();
    }
}

void ItemPropertiesTab::setFileName(const QString& str)
{
    d->labelFile->setAdjustedText(str);
}

void ItemPropertiesTab::setFileFolder(const QString& str)
{
    d->labelFolder->setText(str);
}

void ItemPropertiesTab::setFileModifiedDate(const QString& str)
{
    d->labelFileModifiedDate->setAdjustedText(str);
}

void ItemPropertiesTab::setFileSize(const QString& str)
{
    d->labelFileSize->setAdjustedText(str);
}

void ItemPropertiesTab::setFileOwner(const QString& str)
{
    d->labelFileOwner->setAdjustedText(str);
}

void ItemPropertiesTab::setFilePermissions(const QString& str)
{
    d->labelFilePermissions->setAdjustedText(str);
}

void ItemPropertiesTab::setImageMime(const QString& str)
{
    d->labelImageMime->setAdjustedText(str);
}

void ItemPropertiesTab::setItemDimensions(const QString& str)
{
    d->labelImageDimensions->setAdjustedText(str);
}

void ItemPropertiesTab::setImageRatio(const QString& str)
{
    d->labelImageRatio->setAdjustedText(str);
}

void ItemPropertiesTab::setImageBitDepth(const QString& str)
{
    d->labelImageBitDepth->setAdjustedText(str);
}

void ItemPropertiesTab::setImageColorMode(const QString& str)
{
    d->labelImageColorMode->setAdjustedText(str);
}

void ItemPropertiesTab::setHasSidecar(const QString& str)
{
    d->labelHasSidecar->setAdjustedText(str);
}

void ItemPropertiesTab::setHasGPSInfo(const QString& str)
{
    d->labelHasGPSInfo->setAdjustedText(str);
}

void ItemPropertiesTab::setVersionedInfo(const QString& str)
{
    d->labelVersionedInfo->setAdjustedText(str);
}

void ItemPropertiesTab::setGroupedInfo(const QString& str)
{
    d->labelGroupedInfo->setAdjustedText(str);
}

void ItemPropertiesTab::setGroupedWidget(QWidget* const wdg)
{
    d->glay5->addWidget(wdg, 8, 0, 1, 2);
}

void ItemPropertiesTab::setPhotoMake(const QString& str)
{
    d->labelPhotoMake->setAdjustedText(str);
}

void ItemPropertiesTab::setPhotoModel(const QString& str)
{
    d->labelPhotoModel->setAdjustedText(str);
}

void ItemPropertiesTab::setPhotoDateTime(const QString& str)
{
    d->labelPhotoDateTime->setAdjustedText(str);
}

void ItemPropertiesTab::setPhotoLens(const QString& str)
{
    d->labelPhotoLens->setAdjustedText(str);
}

void ItemPropertiesTab::setPhotoAperture(const QString& str)
{
    d->labelPhotoAperture->setAdjustedText(str);
}

void ItemPropertiesTab::setPhotoFocalLength(const QString& str)
{
    d->labelPhotoFocalLength->setAdjustedText(str);
}

void ItemPropertiesTab::setPhotoExposureTime(const QString& str)
{
    d->labelPhotoExposureTime->setAdjustedText(str);
}

void ItemPropertiesTab::setPhotoSensitivity(const QString& str)
{
    d->labelPhotoSensitivity->setAdjustedText(str);
}

void ItemPropertiesTab::setPhotoExposureMode(const QString& str)
{
    d->labelPhotoExposureMode->setAdjustedText(str);
}

void ItemPropertiesTab::setPhotoFlash(const QString& str)
{
    d->labelPhotoFlash->setAdjustedText(str);
}

void ItemPropertiesTab::setPhotoWhiteBalance(const QString& str)
{
    d->labelPhotoWhiteBalance->setAdjustedText(str);
}

void ItemPropertiesTab::setTitle(const QString& str)
{
    // NOTE: special case for the title. See bug #460134

    d->labelTitle->setText((str.size() > 200) ? str.left(200) + QLatin1String("...") : str);
}

void ItemPropertiesTab::setCaption(const QString& str)
{
    // NOTE: special case for the caption. See bug #460134

    d->labelCaption->setText((str.size() > 200) ? str.left(200) + QLatin1String("...") : str);
}

void ItemPropertiesTab::setColorLabel(int colorId)
{
    if (colorId == NoColorLabel)
    {
        d->labelColor->clear();
        d->labelColorLabel->setAdjustedText(QString());
    }
    else
    {
        DXmlGuiWindow* const app = dynamic_cast<DXmlGuiWindow*>(qApp->activeWindow());
        d->labelColor->setPixmap(ColorLabelWidget::buildIcon((ColorLabel)colorId).pixmap(16, 16));

        if (app)
        {
            QAction* const ac = app->actionCollection()->action(QString::fromLatin1("colorshortcut-%1").arg(colorId));

            if (ac)
            {
                d->labelColorLabel->setAdjustedText(ac->toolTip());
            }
        }
        else
        {
            d->labelColorLabel->setAdjustedText(ColorLabelWidget::labelColorName((ColorLabel)colorId));
        }
    }
}

void ItemPropertiesTab::setPickLabel(int pickId)
{
    if (pickId == NoPickLabel)
    {
        d->labelPick->clear();
        d->labelPickLabel->setAdjustedText(QString());
    }
    else
    {
        d->labelPick->setPixmap(PickLabelWidget::buildIcon((PickLabel)pickId).pixmap(16, 16));
        d->labelPickLabel->setAdjustedText(PickLabelWidget::labelPickName((PickLabel)pickId));
    }
}

void ItemPropertiesTab::setRating(int rating)
{
    if ((rating > RatingMin) && (rating <= RatingMax))
    {
        double dpr   = devicePixelRatio();
        QPixmap star = RatingWidget::buildIcon(rating, 32).pixmap(16, 16);
        QPixmap pix(16 * rating * dpr, 16 * dpr);
        pix.setDevicePixelRatio(dpr);
        pix.fill(Qt::transparent);
        QPainter painter(&pix);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(palette().color(QPalette::Active, foregroundRole()));

        for (int i = 0 ; i < rating ; i++)
        {
            painter.drawPixmap(i * 16, 0, star);
        }

        painter.end();

        d->labelRating->setPixmap(pix);
    }
    else
    {
        d->labelRating->setPixmap(QPixmap());
    }

    d->labelRating->setProperty("Rating", rating);
}

void ItemPropertiesTab::setVideoAspectRatio(const QString& str)
{
    d->labelVideoAspectRatio->setAdjustedText(str);
}

void ItemPropertiesTab::setVideoAudioBitRate(const QString& str)
{
    // use string given as parameter by default because it contains the value for "unavailable" if needed

    QString audioBitRateString = str;
    bool ok                    = false;
    const int audioBitRateInt  = str.toInt(&ok);

    if (ok)
    {
        audioBitRateString = QLocale().toString(audioBitRateInt);
    }

    d->labelVideoAudioBitRate->setAdjustedText(audioBitRateString);
}

void ItemPropertiesTab::setVideoAudioChannelType(const QString& str)
{
    d->labelVideoAudioChannelType->setAdjustedText(str);
}

void ItemPropertiesTab::setVideoAudioCodec(const QString& str)
{
    d->labelVideoAudioCodec->setAdjustedText(str);
}

void ItemPropertiesTab::setVideoDuration(const QString& str)
{
    // duration is given as a string in milliseconds
    // use string given as parameter by default because it contains the value for "unavailable" if needed

    QString durationString          = str;
    bool ok                         = false;
    const unsigned int durationVal  = str.toUInt(&ok);

    if (ok)
    {
        unsigned int rr, dd, hh, mm, ss, ff;
        rr = durationVal;
        dd = rr / 86400000;
        rr = rr % 86400000;
        hh = rr / 3600000;
        rr = rr % 3600000;
        mm = rr / 60000;
        rr = rr % 60000;
        ss = rr / 1000;
        ff = rr % 1000;

        // cppcheck-suppress redundantInitialization
        durationString = QString().asprintf("%d.%02d:%02d:%02d.%03d", dd, hh, mm, ss, ff);
    }
    else
    {
        durationString = i18n("unavailable");
    }

    d->labelVideoDuration->setAdjustedText(durationString);
}

void ItemPropertiesTab::setVideoFrameRate(const QString& str)
{
    // use string given as parameter by default because it contains the value for "unavailable" if needed

    QString frameRateString      = str;
    bool ok;
    const double frameRateDouble = str.toDouble(&ok);

    if (ok)
    {
        frameRateString = QLocale().toString(frameRateDouble) + i18nc("@info: item properties", " fps");
    }

    d->labelVideoFrameRate->setAdjustedText(frameRateString);
}

void ItemPropertiesTab::setVideoVideoCodec(const QString& str)
{
    d->labelVideoVideoCodec->setAdjustedText(str);
}

void ItemPropertiesTab::setTags(const QStringList& regularTagPaths, const QStringList& regularTagNames,
                                const QStringList& peopleTagPaths, const QStringList& peopleTagNames)
{
    d->labelTags->setText(regularTagNames.join(QLatin1String(", ")));
    d->labelPeoples->setText(peopleTagNames.join(QLatin1String(", ")));
    d->labelTags->setToolTip(shortenedTagPaths(regularTagPaths).join(QLatin1Char('\n')));
    d->labelPeoples->setToolTip(shortenedTagPaths(peopleTagPaths).join(QLatin1Char('\n')));
}

void ItemPropertiesTab::setTemplate(const Template& t)
{
    IptcCoreLocationInfo l = t.locationInfo();
    QStringList places;
    QString info;

    if (!l.country.isEmpty())
    {
        places << l.country;
    }

    if (!l.provinceState.isEmpty())
    {
        places << l.provinceState;
    }

    if (!l.city.isEmpty())
    {
        places << l.city;
    }

    if (!l.location.isEmpty())
    {
        places << l.location;
    }

    if (!places.isEmpty())
    {
        info = places.join(QLatin1String(", "));
    }

    d->labelLocation->setText(info);

    // ---

    IptcCoreContactInfo c = t.contactInfo();
    QStringList rights;
    info.clear();

    if (!t.authors().isEmpty())
    {
        const auto auths = t.authors();

        for (const QString& s : auths)
        {
            if (!s.isEmpty())
            {
                rights << s;
            }
        }
    }

    if (!t.credit().isEmpty())
    {
        rights << t.credit();
    }

    if (!t.copyright().contains(QLatin1String("x-default")))
    {
        QString s = t.copyright().value(QLatin1String("x-default"));

        if (!s.isEmpty())
        {
            rights << s;
        }
    }

    if (!c.webUrl.isEmpty())
    {
        rights << QString::fromLatin1("<a href='%1'>%1</a>").arg(c.webUrl);
    }

    if (!rights.isEmpty())
    {
        info = rights.join(QLatin1String(", "));
    }

    d->labelRights->setText(info);
}

} // namespace Digikam
