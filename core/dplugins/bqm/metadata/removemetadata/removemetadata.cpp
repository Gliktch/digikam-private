/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-14
 * Description : remove metadata batch tool.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "removemetadata.h"

// Qt includes

#include <QGridLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QWidget>
#include <QLabel>
#include <QFile>
#include <QScopedPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "dmetadata.h"
#include "dfileoperations.h"

namespace DigikamBqmRemoveMetadataPlugin
{

class Q_DECL_HIDDEN RemoveMetadata::Private
{
public:

    enum RemoveAction
    {
        ALL = 0,
        GPS,
        DATE,
        EXIF,
        VIDEO,
        DUBLIN,
        COMMENT,
        DIGIKAM,
        HISTORY,
        PREVIEW,
        XPKEYWORDS

    };

public:

    Private() = default;

public:

    QCheckBox* removeExif       = nullptr;
    QCheckBox* removeIptc       = nullptr;
    QCheckBox* removeXmp        = nullptr;

    QComboBox* exifComboBox     = nullptr;
    QComboBox* iptcComboBox     = nullptr;
    QComboBox* xmpComboBox      = nullptr;

    bool       changeSettings   = true;
};

RemoveMetadata::RemoveMetadata(QObject* const parent)
    : BatchTool(QLatin1String("RemoveMetadata"), MetadataTool, parent),
      d        (new Private)
{
}

RemoveMetadata::~RemoveMetadata()
{
    delete d;
}

BatchTool* RemoveMetadata::clone(QObject* const parent) const
{
    return new RemoveMetadata(parent);
}

void RemoveMetadata::registerSettingsWidget()
{
    QWidget* const panel     = new QWidget;
    QGridLayout* const grid  = new QGridLayout(panel);

    d->removeExif            = new QCheckBox(i18nc("@title", "Exif:"), panel);
    d->exifComboBox          = new QComboBox(panel);
    d->exifComboBox->addItem(i18nc("@item: exif namespace", "Completely"),                     Private::ALL);
    d->exifComboBox->addItem(i18nc("@item: exif namespace", "Date"),                           Private::DATE);
    d->exifComboBox->addItem(i18nc("@item: exif namespace", "GPS"),                            Private::GPS);
    d->exifComboBox->addItem(i18nc("@item: exif namespace", "XPKeywords"),                     Private::XPKEYWORDS);
    d->exifComboBox->addItem(i18nc("@item: exif namespace", "Comment and description"),        Private::COMMENT);

    d->removeIptc            = new QCheckBox(i18nc("@title", "Iptc:"), panel);
    d->iptcComboBox          = new QComboBox(panel);
    d->iptcComboBox->addItem(i18nc("@item: iptc namespace", "Completely"),                     Private::ALL);
    d->iptcComboBox->addItem(i18nc("@item: iptc namespace", "Date"),                           Private::DATE);
    d->iptcComboBox->addItem(i18nc("@item: iptc namespace", "Caption"),                        Private::COMMENT);
    d->iptcComboBox->addItem(i18nc("@item: iptc namespace", "Preview image"),                  Private::PREVIEW);

    d->removeXmp             = new QCheckBox(i18nc("@title", "Xmp:"), panel);
    d->xmpComboBox           = new QComboBox(panel);
    d->xmpComboBox->addItem(i18nc("@item: xmp namespace", "Completely"),                       Private::ALL);
    d->xmpComboBox->addItem(i18nc("@item: xmp namespace", "Date"),                             Private::DATE);
    d->xmpComboBox->addItem(i18nc("@item: xmp namespace", "digiKam completely"),               Private::DIGIKAM);
    d->xmpComboBox->addItem(i18nc("@item: xmp namespace", "digiKam image history"),            Private::HISTORY);
    d->xmpComboBox->addItem(i18nc("@item: xmp namespace", "digiKam preview image"),            Private::PREVIEW);
    d->xmpComboBox->addItem(i18nc("@item: xmp namespace", "Dublin Core"),                      Private::DUBLIN);
    d->xmpComboBox->addItem(i18nc("@item: xmp namespace", "Exif"),                             Private::EXIF);
    d->xmpComboBox->addItem(i18nc("@item: xmp namespace", "Video"),                            Private::VIDEO);
    d->xmpComboBox->addItem(i18nc("@item: xmp namespace", "Caption, comment and description"), Private::COMMENT);


    grid->addWidget(d->removeExif,   0, 0, 1, 1);
    grid->addWidget(d->exifComboBox, 0, 1, 1, 2);
    grid->addWidget(d->removeIptc,   1, 0, 1, 1);
    grid->addWidget(d->iptcComboBox, 1, 1, 1, 2);
    grid->addWidget(d->removeXmp,    2, 0, 1, 1);
    grid->addWidget(d->xmpComboBox,  2, 1, 1, 2);
    grid->setColumnStretch(2, 10);
    grid->setRowStretch(3, 10);

    m_settingsWidget = panel;

    connect(d->removeExif, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->removeIptc, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->removeXmp, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->exifComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->iptcComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->xmpComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings RemoveMetadata::defaultSettings()
{
    BatchToolSettings settings;

    settings.insert(QLatin1String("RemoveExif"), false);
    settings.insert(QLatin1String("RemoveIptc"), false);
    settings.insert(QLatin1String("RemoveXmp"),  false);
    settings.insert(QLatin1String("ExifData"),   Private::ALL);
    settings.insert(QLatin1String("IptcData"),   Private::ALL);
    settings.insert(QLatin1String("XmpData"),    Private::ALL);

    return settings;
}

void RemoveMetadata::slotAssignSettings2Widget()
{
    d->changeSettings = false;

    d->removeExif->setChecked(settings().value(QLatin1String("RemoveExif")).toBool());
    d->removeIptc->setChecked(settings().value(QLatin1String("RemoveIptc")).toBool());
    d->removeXmp->setChecked(settings().value(QLatin1String("RemoveXmp")).toBool());

    int exifData = settings().value(QLatin1String("ExifData")).toInt();
    d->exifComboBox->setCurrentIndex(d->exifComboBox->findData(exifData));

    int iptcData = settings().value(QLatin1String("IptcData")).toInt();
    d->iptcComboBox->setCurrentIndex(d->iptcComboBox->findData(iptcData));

    int xmpData  = settings().value(QLatin1String("XmpData")).toInt();
    d->xmpComboBox->setCurrentIndex(d->xmpComboBox->findData(xmpData));

    d->exifComboBox->setEnabled(d->removeExif->isChecked());
    d->iptcComboBox->setEnabled(d->removeIptc->isChecked());
    d->xmpComboBox->setEnabled(d->removeXmp->isChecked());

    d->changeSettings = true;
}

void RemoveMetadata::slotSettingsChanged()
{
    if (d->changeSettings)
    {
        BatchToolSettings settings;

        settings.insert(QLatin1String("RemoveExif"), d->removeExif->isChecked());
        settings.insert(QLatin1String("RemoveIptc"), d->removeIptc->isChecked());
        settings.insert(QLatin1String("RemoveXmp"),  d->removeXmp->isChecked());
        settings.insert(QLatin1String("ExifData"),   d->exifComboBox->currentData().toInt());
        settings.insert(QLatin1String("IptcData"),   d->iptcComboBox->currentData().toInt());
        settings.insert(QLatin1String("XmpData" ),   d->xmpComboBox->currentData().toInt());

        BatchTool::slotSettingsChanged(settings);
    }
}

bool RemoveMetadata::toolOperations()
{
    if (!isLastChainedTool())
    {
        setErrorDescription(i18nc("@info", "Remove Metadata: Not the last tool in the list."));
        return false;
    }

    bool ret = true;
    QScopedPointer<DMetadata> meta(new DMetadata);

    if (image().isNull())
    {
        QFile::remove(outputUrl().toLocalFile());
        ret = DFileOperations::copyFile(inputUrl().toLocalFile(), outputUrl().toLocalFile());

        if (!ret || !meta->load(outputUrl().toLocalFile()))
        {
            return ret;
        }
    }
    else
    {
        ret = savefromDImg();
        meta->setData(image().getMetadata());
    }

    bool removeExif = settings().value(QLatin1String("RemoveExif")).toBool();
    bool removeIptc = settings().value(QLatin1String("RemoveIptc")).toBool();
    bool removeXmp  = settings().value(QLatin1String("RemoveXmp")).toBool();

    int exifData    = settings().value(QLatin1String("ExifData")).toInt();
    int iptcData    = settings().value(QLatin1String("IptcData")).toInt();
    int xmpData     = settings().value(QLatin1String("XmpData")).toInt();

    if (removeExif)
    {
        if      (exifData == Private::ALL)
        {
            meta->clearExif();
            meta->clearComments();
        }
        else if (exifData == Private::DATE)
        {
            meta->removeExifTag("Exif.Image.DateTime");
            meta->removeExifTag("Exif.Image.PreviewDateTime");
            meta->removeExifTag("Exif.Photo.DateTimeOriginal");
            meta->removeExifTag("Exif.Photo.DateTimeDigitized");
        }
        else if (exifData == Private::GPS)
        {
            meta->removeExifTags(QStringList() << QLatin1String("GPSInfo"));
        }
        else if (exifData == Private::XPKEYWORDS)
        {
            meta->removeExifTag("Exif.Image.XPKeywords");
        }
        else if (exifData == Private::COMMENT)
        {
            meta->removeExifTag("Exif.Image.ImageDescription");
            meta->removeExifTag("Exif.Photo.UserComment");
            meta->clearComments();
        }
    }

    if (removeIptc)
    {
        if      (iptcData == Private::ALL)
        {
            meta->clearIptc();
        }
        else if (iptcData == Private::DATE)
        {
            meta->removeIptcTag("Iptc.Application2.DateCreated");
            meta->removeIptcTag("Iptc.Application2.TimeCreated");
        }
        else if (iptcData == Private::COMMENT)
        {
            meta->removeIptcTag("Iptc.Application2.Caption");
            meta->clearComments();
        }
        else if (iptcData == Private::PREVIEW)
        {
            meta->removeIptcTag("Iptc.Application2.Preview");
            meta->removeIptcTag("Iptc.Application2.PreviewFormat");
            meta->removeIptcTag("Iptc.Application2.PreviewVersion");
        }
    }

    if (removeXmp)
    {
        if      (xmpData == Private::ALL)
        {
            meta->clearXmp();
        }
        else if (xmpData == Private::DATE)
        {
            meta->removeXmpTag("Xmp.photoshop.DateCreated");
            meta->removeXmpTag("Xmp.exif.DateTimeOriginal");
            meta->removeXmpTag("Xmp.xmp.MetadataDate");
            meta->removeXmpTag("Xmp.xmp.CreateDate");
            meta->removeXmpTag("Xmp.xmp.ModifyDate");
            meta->removeXmpTag("Xmp.tiff.DateTime");
            meta->removeXmpTag("Xmp.video.DateTimeDigitized");
            meta->removeXmpTag("Xmp.video.DateTimeOriginal");
            meta->removeXmpTag("Xmp.video.ModificationDate");
            meta->removeXmpTag("Xmp.video.DateUTC");
        }
        else if (xmpData == Private::DIGIKAM)
        {
            meta->removeXmpTags(QStringList() << QLatin1String("digiKam"));
        }
        else if (xmpData == Private::HISTORY)
        {
            meta->removeXmpTag("Xmp.digiKam.ImageHistory");
        }
        else if (xmpData == Private::PREVIEW)
        {
            meta->removeXmpTag("Xmp.digiKam.Preview");
        }
        else if (xmpData == Private::DUBLIN)
        {
            meta->removeXmpTags(QStringList() << QLatin1String("dc"));
        }
        else if (xmpData == Private::EXIF)
        {
            meta->removeXmpTags(QStringList() << QLatin1String("exif"));
        }
        else if (xmpData == Private::VIDEO)
        {
            meta->removeXmpTags(QStringList() << QLatin1String("video"));
        }
        else if (xmpData == Private::COMMENT)
        {
            meta->removeXmpTag("Xmp.acdsee.Caption");
            meta->removeXmpTag("Xmp.dc.Description");
            meta->removeXmpTag("Xmp.crs.Description");
            meta->removeXmpTag("Xmp.exif.UserComment");
            meta->removeXmpTag("Xmp.tiff.ImageDescription");
            meta->removeXmpTag("Xmp.xmp.Description");
            meta->removeXmpTag("Xmp.xmpDM.DMComment");
            meta->clearComments();
        }
    }

    if (ret && (removeExif || removeIptc || removeXmp))
    {
        ret = meta->save(outputUrl().toLocalFile());
    }

    return ret;
}

} // namespace DigikamBqmRemoveMetadataPlugin

#include "moc_removemetadata.cpp"
