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

ItemPropertiesTab::ItemPropertiesTab(QWidget* const parent)
    : DExpanderBox(parent),
      d           (new Private)
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setLineWidth(style()->pixelMetric(QStyle::PM_DefaultFrameWidth));

    // --------------------------------------------------

    const int spacing                  = layoutSpacing();

    QWidget* const w1                  = new QWidget(this);
    QGridLayout* const glay1           = new QGridLayout(w1);

    DTextLabelName* const folder       = new DTextLabelName(i18nc("@label: item properties", "Folder: "),      w1);
    DTextLabelName* const symlink      = new DTextLabelName(i18nc("@label: item properties", "Symlink: "),     w1);
    DTextLabelName* const modifiedDate = new DTextLabelName(i18nc("@label: item properties", "Date: "),        w1);
    DTextLabelName* const size         = new DTextLabelName(i18nc("@label: item properties", "Size: "),        w1);
    DTextLabelName* const owner        = new DTextLabelName(i18nc("@label: item properties", "Owner: "),       w1);
    DTextLabelName* const permissions  = new DTextLabelName(i18nc("@label: item properties", "Permissions: "), w1);

    d->labelFile                       = new DTextLabelValue(QString(), w1);
    QFont fnt = d->labelFile->font();
    fnt.setBold(true);
    fnt.setPointSize(fnt.pointSize() + 4);
    d->labelFile->setFont(fnt);
    d->labelFile->setAlignment(Qt::AlignCenter);

    d->labelFolder                     = new QLabel(QString(), w1);
    d->labelFolder->setWordWrap(true);
    d->labelFolder->setAlignment(Qt::AlignRight);
    d->labelSymlink                    = new DTextLabelValue(QString(), w1);
    d->labelFileModifiedDate           = new DTextLabelValue(QString(), w1);
    d->labelFileSize                   = new DTextLabelValue(QString(), w1);
    d->labelFileOwner                  = new DTextLabelValue(QString(), w1);
    d->labelFilePermissions            = new DTextLabelValue(QString(), w1);

    d->labelFile->setTextInteractionFlags(Qt::TextSelectableByMouse);
    d->labelFolder->setTextInteractionFlags(Qt::TextSelectableByMouse);

    glay1->addWidget(d->labelFile,             0, 0, 1, 2);
    glay1->addWidget(folder,                   1, 0, 1, 1);
    glay1->addWidget(d->labelFolder,           1, 1, 1, 1);
    glay1->addWidget(symlink,                  2, 0, 1, 1);
    glay1->addWidget(d->labelSymlink,          2, 1, 1, 1);
    glay1->addWidget(modifiedDate,             3, 0, 1, 1);
    glay1->addWidget(d->labelFileModifiedDate, 3, 1, 1, 1);
    glay1->addWidget(size,                     4, 0, 1, 1);
    glay1->addWidget(d->labelFileSize,         4, 1, 1, 1);
    glay1->addWidget(owner,                    5, 0, 1, 1);
    glay1->addWidget(d->labelFileOwner,        5, 1, 1, 1);
    glay1->addWidget(permissions,              6, 0, 1, 1);
    glay1->addWidget(d->labelFilePermissions,  6, 1, 1, 1);
    glay1->setContentsMargins(spacing, spacing, spacing, spacing);
    glay1->setColumnStretch(0, 10);
    glay1->setColumnStretch(1, 25);
    glay1->setSpacing(0);

    insertItem(ItemPropertiesTab::FileProperties,
               w1, QIcon::fromTheme(QLatin1String("dialog-information")),
               i18nc("@title: item properties", "File Properties"), QLatin1String("FileProperties"), true);

    // --------------------------------------------------

    QWidget* const w2                       = new QWidget(this);
    QGridLayout* const glay2                = new QGridLayout(w2);

    DTextLabelName* const mime              = new DTextLabelName(i18nc("@label: item properties", "Type: "),         w2);
    DTextLabelName* const dimensions        = new DTextLabelName(i18nc("@label: item properties", "Dimensions: "),   w2);
    DTextLabelName* const ratio             = new DTextLabelName(i18nc("@label: item properties", "Aspect Ratio: "), w2);
    DTextLabelName* const bitDepth          = new DTextLabelName(i18nc("@label: item properties", "Bit depth: "),    w2);
    DTextLabelName* const colorMode         = new DTextLabelName(i18nc("@label: item properties", "Color mode: "),   w2);
    DTextLabelName* const hasSidecar        = new DTextLabelName(i18nc("@label: item properties", "Sidecar: "),      w2);
    DTextLabelName* const hasGPSInfo        = new DTextLabelName(i18nc("@label: item properties", "GPS: "),          w2);

    d->labelImageMime                       = new DTextLabelValue(QString(), w2);
    d->labelImageDimensions                 = new DTextLabelValue(QString(), w2);
    d->labelImageRatio                      = new DTextLabelValue(QString(), w2);
    d->labelImageBitDepth                   = new DTextLabelValue(QString(), w2);
    d->labelImageColorMode                  = new DTextLabelValue(QString(), w2);
    d->labelHasSidecar                      = new DTextLabelValue(QString(), w2);
    d->labelHasGPSInfo                      = new DTextLabelValue(QString(), w2);

    glay2->addWidget(mime,                      0, 0, 1, 1);
    glay2->addWidget(d->labelImageMime,         0, 1, 1, 1);
    glay2->addWidget(dimensions,                1, 0, 1, 1);
    glay2->addWidget(d->labelImageDimensions,   1, 1, 1, 1);
    glay2->addWidget(ratio,                     2, 0, 1, 1);
    glay2->addWidget(d->labelImageRatio,        2, 1, 1, 1);
    glay2->addWidget(bitDepth,                  3, 0, 1, 1);
    glay2->addWidget(d->labelImageBitDepth,     3, 1, 1, 1);
    glay2->addWidget(colorMode,                 4, 0, 1, 1);
    glay2->addWidget(d->labelImageColorMode,    4, 1, 1, 1);
    glay2->addWidget(hasSidecar,                5, 0, 1, 1);
    glay2->addWidget(d->labelHasSidecar,        5, 1, 1, 1);
    glay2->addWidget(hasGPSInfo,                6, 0, 1, 1);
    glay2->addWidget(d->labelHasGPSInfo,        6, 1, 1, 1);
    glay2->setContentsMargins(spacing, spacing, spacing, spacing);
    glay2->setColumnStretch(0, 10);
    glay2->setColumnStretch(1, 25);
    glay2->setSpacing(0);

    insertItem(ItemPropertiesTab::ImageProperties,
               w2, QIcon::fromTheme(QLatin1String("view-preview")),
               i18nc("@title: item properties", "Item Properties"), QLatin1String("ItemProperties"), true);

    // --------------------------------------------------

    QWidget* const w3                  = new QWidget(this);
    QGridLayout* const glay3           = new QGridLayout(w3);

    DTextLabelName* const make         = new DTextLabelName(i18nc("@label: item properties", "Make: "),          w3);
    DTextLabelName* const model        = new DTextLabelName(i18nc("@label: item properties", "Model: "),         w3);
    DTextLabelName* const lens         = new DTextLabelName(i18nc("@label: item properties", "Lens: "),          w3);
    DTextLabelName* const aperture     = new DTextLabelName(i18nc("@label: item properties", "Aperture: "),      w3);
    DTextLabelName* const focalLength  = new DTextLabelName(i18nc("@label: item properties", "Focal: "),         w3);
    DTextLabelName* const exposureTime = new DTextLabelName(i18nc("@label: item properties", "Exposure: "),      w3);
    DTextLabelName* const sensitivity  = new DTextLabelName(i18nc("@label: item properties", "Sensitivity: "),   w3);
    DTextLabelName* const exposureMode = new DTextLabelName(i18nc("@label: item properties", "Mode/Program: "),  w3);
    DTextLabelName* const flash        = new DTextLabelName(i18nc("@label: item properties", "Flash: "),         w3);
    DTextLabelName* const whiteBalance = new DTextLabelName(i18nc("@label: item properties", "White balance: "), w3);

    d->labelPhotoMake                  = new DTextLabelValue(QString(), w3);
    d->labelPhotoModel                 = new DTextLabelValue(QString(), w3);
    d->labelPhotoLens                  = new DTextLabelValue(QString(), w3);
    d->labelPhotoAperture              = new DTextLabelValue(QString(), w3);
    d->labelPhotoFocalLength           = new DTextLabelValue(QString(), w3);
    d->labelPhotoExposureTime          = new DTextLabelValue(QString(), w3);
    d->labelPhotoSensitivity           = new DTextLabelValue(QString(), w3);
    d->labelPhotoExposureMode          = new DTextLabelValue(QString(), w3);
    d->labelPhotoFlash                 = new DTextLabelValue(QString(), w3);
    d->labelPhotoWhiteBalance          = new DTextLabelValue(QString(), w3);

    glay3->addWidget(make,                      0, 0, 1, 1);
    glay3->addWidget(d->labelPhotoMake,         0, 1, 1, 1);
    glay3->addWidget(model,                     1, 0, 1, 1);
    glay3->addWidget(d->labelPhotoModel,        1, 1, 1, 1);
    glay3->addWidget(lens,                      2, 0, 1, 1);
    glay3->addWidget(d->labelPhotoLens,         2, 1, 1, 1);
    glay3->addWidget(aperture,                  3, 0, 1, 1);
    glay3->addWidget(d->labelPhotoAperture,     3, 1, 1, 1);
    glay3->addWidget(focalLength,               4, 0, 1, 1);
    glay3->addWidget(d->labelPhotoFocalLength,  4, 1, 1, 1);
    glay3->addWidget(exposureTime,              5, 0, 1, 1);
    glay3->addWidget(d->labelPhotoExposureTime, 5, 1, 1, 1);
    glay3->addWidget(sensitivity,               6, 0, 1, 1);
    glay3->addWidget(d->labelPhotoSensitivity,  6, 1, 1, 1);
    glay3->addWidget(exposureMode,              7, 0, 1, 1);
    glay3->addWidget(d->labelPhotoExposureMode, 7, 1, 1, 1);
    glay3->addWidget(flash,                     8, 0, 1, 1);
    glay3->addWidget(d->labelPhotoFlash,        8, 1, 1, 1);
    glay3->addWidget(whiteBalance,              9, 0, 1, 1);
    glay3->addWidget(d->labelPhotoWhiteBalance, 9, 1, 1, 1);
    glay3->setContentsMargins(spacing, spacing, spacing, spacing);
    glay3->setColumnStretch(0, 10);
    glay3->setColumnStretch(1, 25);
    glay3->setSpacing(0);

    insertItem(ItemPropertiesTab::PhotoProperties,
               w3, QIcon::fromTheme(QLatin1String("camera-photo")),
               i18nc("@title: item properties", "Photograph Properties"), QLatin1String("PhotographProperties"), true);

    // --------------------------------------------------

    QWidget* const w4                      = new QWidget(this);
    QGridLayout* const glay4               = new QGridLayout(w4);

    DTextLabelName* const aspectRatio      = new DTextLabelName(i18nc("@label: item properties", "Aspect Ratio: "),       w4);
    DTextLabelName* const duration         = new DTextLabelName(i18nc("@label: item properties", "Duration: "),           w4);
    DTextLabelName* const frameRate        = new DTextLabelName(i18nc("@label: item properties", "Frame Rate: "),         w4);
    DTextLabelName* const videoCodec       = new DTextLabelName(i18nc("@label: item properties", "Video Codec: "),        w4);
    DTextLabelName* const audioBitRate     = new DTextLabelName(i18nc("@label: item properties", "Audio Bit Rate: "),     w4);
    DTextLabelName* const audioChannelType = new DTextLabelName(i18nc("@label: item properties", "Audio Channel Type: "), w4);
    DTextLabelName* const audioCodec       = new DTextLabelName(i18nc("@label: item properties", "Audio Codec: "),        w4);

    d->labelVideoAspectRatio               = new DTextLabelValue(QString(), w4);
    d->labelVideoDuration                  = new DTextLabelValue(QString(), w4);
    d->labelVideoFrameRate                 = new DTextLabelValue(QString(), w4);
    d->labelVideoVideoCodec                = new DTextLabelValue(QString(), w4);
    d->labelVideoAudioBitRate              = new DTextLabelValue(QString(), w4);
    d->labelVideoAudioChannelType          = new DTextLabelValue(QString(), w4);
    d->labelVideoAudioCodec                = new DTextLabelValue(QString(), w4);

    glay4->addWidget(aspectRatio,                   0, 0, 1, 1);
    glay4->addWidget(d->labelVideoAspectRatio,      0, 1, 1, 1);
    glay4->addWidget(duration,                      1, 0, 1, 1);
    glay4->addWidget(d->labelVideoDuration,         1, 1, 1, 1);
    glay4->addWidget(frameRate,                     2, 0, 1, 1);
    glay4->addWidget(d->labelVideoFrameRate,        2, 1, 1, 1);
    glay4->addWidget(videoCodec,                    3, 0, 1, 1);
    glay4->addWidget(d->labelVideoVideoCodec,       3, 1, 1, 1);
    glay4->addWidget(audioBitRate,                  4, 0, 1, 1);
    glay4->addWidget(d->labelVideoAudioBitRate,     4, 1, 1, 1);
    glay4->addWidget(audioChannelType,              5, 0, 1, 1);
    glay4->addWidget(d->labelVideoAudioChannelType, 5, 1, 1, 1);
    glay4->addWidget(audioCodec,                    6, 0, 1, 1);
    glay4->addWidget(d->labelVideoAudioCodec,       6, 1, 1, 1);
    glay4->setContentsMargins(spacing, spacing, spacing, spacing);
    glay4->setColumnStretch(0, 10);
    glay4->setColumnStretch(1, 25);
    glay4->setSpacing(0);

    insertItem(ItemPropertiesTab::VideoProperties,
               w4, QIcon::fromTheme(QLatin1String("video-x-generic")),
               i18nc("@title: item properties", "Audio/Video Properties"), QLatin1String("VideoProperties"), true);

    // --------------------------------------------------

    QWidget* const w5                = new QWidget(this);
    d->glay5                         = new QGridLayout(w5);

    d->labelPhotoDateTime            = new DTextLabelValue(QString(), w5);
    fnt                              = d->labelPhotoDateTime->font();
    fnt.setPointSize(fnt.pointSize() + 4);
    fnt.setUnderline(true);
    d->labelPhotoDateTime->setFont(fnt);
    d->labelPhotoDateTime->setAlignment(Qt::AlignCenter);

    d->title                         = new DTextLabelName(i18nc("@label: item properties", "Title: "),       w5);
    d->caption                       = new DTextLabelName(i18nc("@label: item properties", "Caption: "),     w5);
    d->pickLabel                     = new DTextLabelName(i18nc("@label: item properties", "Pick label: "),  w5);
    d->colorLabel                    = new DTextLabelName(i18nc("@label: item properties", "Color label: "), w5);
    d->rating                        = new DTextLabelName(i18nc("@label: item properties", "Rating: "),      w5);
    d->versioned                     = new DTextLabelName(i18nc("@label: item properties", "Versioned: "),   w5);
    d->grouped                       = new DTextLabelName(i18nc("@label: item properties", "Grouped: "),     w5);

    d->labelTitle                    = new QLabel(QString(), w5);
    d->labelTitle->setWordWrap(true);
    d->labelTitle->setAlignment(Qt::AlignRight);
    d->labelCaption                  = new QLabel(QString(), w5);
    d->labelCaption->setWordWrap(true);
    d->labelCaption->setAlignment(Qt::AlignRight);

    QWidget* const labelPickBox      = new QWidget(w5);
    QHBoxLayout* const labelPickLay  = new QHBoxLayout(labelPickBox);
    d->labelPick                     = new QLabel(w5);
    d->labelPickLabel                = new DTextLabelValue(QString(), w5);
    labelPickLay->setContentsMargins(0, 0, 0, 0);
    labelPickLay->addWidget(d->labelPickLabel);
    labelPickLay->addWidget(d->labelPick);

    QWidget* const labelColorBox     = new QWidget(w5);
    QHBoxLayout* const labelColorLay = new QHBoxLayout(labelColorBox);
    d->labelColor                    = new QLabel(w5);
    d->labelColorLabel               = new DTextLabelValue(QString(), w5);
    labelColorLay->setContentsMargins(0, 0, 0, 0);
    labelColorLay->addWidget(d->labelColorLabel);
    labelColorLay->addWidget(d->labelColor);

    d->labelRating                   = new QLabel(QString(), w5);
    d->labelRating->setAlignment(Qt::AlignRight);
    d->labelVersionedInfo            = new DTextLabelValue(QString(), w5);
    d->labelGroupedInfo              = new DTextLabelValue(QString(), w5);

    d->glay5->addWidget(d->labelPhotoDateTime, 0, 0, 1, 2);
    d->glay5->addWidget(d->title,              1, 0, 1, 1);
    d->glay5->addWidget(d->labelTitle,         1, 1, 1, 1);
    d->glay5->addWidget(d->caption,            2, 0, 1, 1);
    d->glay5->addWidget(d->labelCaption,       2, 1, 1, 1);
    d->glay5->addWidget(d->pickLabel,          3, 0, 1, 1);
    d->glay5->addWidget(labelPickBox,          3, 1, 1, 1);
    d->glay5->addWidget(d->colorLabel,         4, 0, 1, 1);
    d->glay5->addWidget(labelColorBox,         4, 1, 1, 1);
    d->glay5->addWidget(d->rating,             5, 0, 1, 1);
    d->glay5->addWidget(d->labelRating,        5, 1, 1, 1);
    d->glay5->addWidget(d->versioned,          6, 0, 1, 1);
    d->glay5->addWidget(d->labelVersionedInfo, 6, 1, 1, 1);
    d->glay5->addWidget(d->grouped,            7, 0, 1, 1);
    d->glay5->addWidget(d->labelGroupedInfo,   7, 1, 1, 1);
    d->glay5->setContentsMargins(spacing, spacing, spacing, spacing);
    d->glay5->setColumnStretch(0, 10);
    d->glay5->setColumnStretch(1, 25);
    d->glay5->setSpacing(0);

    insertItem(ItemPropertiesTab::digiKamProperties,
               w5, QIcon::fromTheme(QLatin1String("edit-text-frame-update")),
               i18nc("@title: item properties", "digiKam Properties"), QLatin1String("DigikamProperties"), true);

    // --------------------------------------------------

    QWidget* const w6        = new QWidget(this);
    QGridLayout* const glay6 = new QGridLayout(w6);

    d->tags                  = new DTextLabelName(i18nc("@label: item properties", "Keywords: "),      w6);
    d->labelTags             = new QLabel(QString(), w6);
    d->labelTags->setWordWrap(true);
    d->labelTags->setAlignment(Qt::AlignRight);
    QPalette pal             = d->labelTags->palette();
    pal.setColor(QPalette::WindowText, qApp->palette().color(QPalette::Link));
    d->labelTags->setPalette(pal);

    d->peoples               = new DTextLabelName(i18nc("@label: item properties", "People: "),      w6);
    d->labelPeoples          = new QLabel(QString(), w6);
    d->labelPeoples->setWordWrap(true);
    d->labelPeoples->setAlignment(Qt::AlignRight);

    glay6->addWidget(d->tags,         0, 0, 1, 1);
    glay6->addWidget(d->labelTags,    0, 1, 1, 1);
    glay6->addWidget(d->peoples,      1, 0, 1, 1);
    glay6->addWidget(d->labelPeoples, 1, 1, 1, 1);
    glay6->setContentsMargins(spacing, spacing, spacing, spacing);
    glay6->setColumnStretch(0, 10);
    glay6->setColumnStretch(1, 25);
    glay6->setSpacing(0);

    insertItem(ItemPropertiesTab::TagsProperties,
               w6, QIcon::fromTheme(QLatin1String("tag")),
               i18nc("@title: item properties", "Tags"), QLatin1String("TagsProperties"), true);

    // --------------------------------------------------

    QWidget* const w7        = new QWidget(this);
    QGridLayout* const glay7 = new QGridLayout(w7);

    d->labelLocation         = new QLabel(QString(), w7);
    d->labelLocation->setWordWrap(true);
    d->labelLocation->setAlignment(Qt::AlignRight);

    glay7->addWidget(d->labelLocation, 0, 0, 1, 2);
    glay7->setContentsMargins(spacing, spacing, spacing, spacing);
    glay7->setColumnStretch(0, 10);
    glay7->setColumnStretch(1, 25);
    glay7->setSpacing(0);

    insertItem(ItemPropertiesTab::LocationProperties,
               w7, QIcon::fromTheme(QLatin1String("globe")),
               i18nc("@title: item properties", "Location"), QLatin1String("LocationProperties"), true);

    // --------------------------------------------------

    QWidget* const w8        = new QWidget(this);
    QGridLayout* const glay8 = new QGridLayout(w8);

    d->labelRights           = new QLabel(QString(), w8);
    d->labelRights->setWordWrap(true);
    d->labelRights->setAlignment(Qt::AlignRight);
    d->labelRights->setOpenExternalLinks(true);
    d->labelRights->setTextFormat(Qt::RichText);
    d->labelRights->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

    glay8->addWidget(d->labelRights, 0, 0, 1, 2);
    glay8->setContentsMargins(spacing, spacing, spacing, spacing);
    glay8->setColumnStretch(0, 10);
    glay8->setColumnStretch(1, 25);
    glay8->setSpacing(0);

    insertItem(ItemPropertiesTab::RightProperties,
               w8, QIcon::fromTheme(QLatin1String("flag")),
               i18nc("@title: item properties", "Rights"), QLatin1String("RightsProperties"), true);

    // --------------------------------------------------

    addStretch();
}

ItemPropertiesTab::~ItemPropertiesTab()
{
    delete d;
}

void ItemPropertiesTab::showOrHideCachedProperties()
{
    bool hasDate       = !d->labelPhotoDateTime->adjustedText().isEmpty();
    d->labelPhotoDateTime->setVisible(hasDate);

    bool hasTitle      = !d->labelTitle->text().isEmpty();
    d->labelTitle->setVisible(hasTitle);
    d->title->setVisible(hasTitle);

    bool hasCaption    = !d->labelCaption->text().isEmpty();
    d->labelCaption->setVisible(hasCaption);
    d->caption->setVisible(hasCaption);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    bool hasPickLabel  = !d->labelPick->pixmap().isNull();

#else

    bool hasPickLabel  = !d->labelPick->pixmap(Qt::ReturnByValue).isNull();

#endif

    d->pickLabel->setVisible(hasPickLabel);
    d->labelPick->setVisible(hasPickLabel);
    d->labelPickLabel->setVisible(hasPickLabel);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    bool hasColorLabel = !d->labelColor->pixmap().isNull();

#else

    bool hasColorLabel = !d->labelColor->pixmap(Qt::ReturnByValue).isNull();

#endif

    d->colorLabel->setVisible(hasColorLabel);
    d->labelColor->setVisible(hasColorLabel);
    d->labelColorLabel->setVisible(hasColorLabel);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    bool hasRating     = !d->labelRating->pixmap().isNull();


#else

    bool hasRating     = !d->labelRating->pixmap(Qt::ReturnByValue).isNull();

#endif

    d->rating->setVisible(hasRating);
    d->labelRating->setVisible(hasRating);

    bool hasVersioned  = !d->labelVersionedInfo->adjustedText().isEmpty();
    d->versioned->setVisible(hasVersioned);
    d->labelVersionedInfo->setVisible(hasVersioned);

    bool hasGrouped    = !d->labelGroupedInfo->adjustedText().isEmpty();
    d->grouped->setVisible(hasGrouped);
    d->labelGroupedInfo->setVisible(hasGrouped);

    widget(ItemPropertiesTab::digiKamProperties)->setVisible(hasTitle      ||
                                                             hasCaption    ||
                                                             hasRating     ||
                                                             hasPickLabel  ||
                                                             hasDate       ||
                                                             hasColorLabel ||
                                                             hasVersioned  ||
                                                             hasGrouped);

    bool hasTags       = !d->labelTags->text().isEmpty();
    bool hasPeoples    = !d->labelPeoples->text().isEmpty();

    d->tags->setVisible(hasTags);
    d->labelTags->setVisible(hasTags);
    d->peoples->setVisible(hasPeoples);
    d->labelPeoples->setVisible(hasPeoples);

    widget(ItemPropertiesTab::TagsProperties)->setVisible(hasTags || hasPeoples);

    bool hasLocation = !d->labelLocation->text().isEmpty();

    widget(ItemPropertiesTab::LocationProperties)->setVisible(hasLocation);

    bool hasRights = !d->labelRights->text().isEmpty();

    widget(ItemPropertiesTab::RightProperties)->setVisible(hasRights);
}

void ItemPropertiesTab::appendProperty(QString& data, const QString& key, const QString& value) const
{
    QString stripped = value;
    stripped.remove(QLatin1String("<i>"));
    stripped.remove(QLatin1String("</i>"));
    stripped.replace(QLatin1Char('\n'), QLatin1Char(' '));
    data.append(key + QLatin1String(": ") + stripped + QLatin1Char('\n'));
}

QString ItemPropertiesTab::propertiesToText() const
{
    QString data;

    appendProperty(data, i18n("File")                    , d->labelFile->adjustedText()                 );
    appendProperty(data, i18n("Folder")                  , d->labelFolder->text()                       );
    appendProperty(data, i18n("Symlink")                 , d->labelSymlink->adjustedText()              );
    appendProperty(data, i18n("Modified Date")           , d->labelFileModifiedDate->adjustedText()     );
    appendProperty(data, i18n("Size")                    , d->labelFileSize->adjustedText()             );
    appendProperty(data, i18n("Owner")                   , d->labelFileOwner->adjustedText()            );
    appendProperty(data, i18n("Permissions")             , d->labelFilePermissions->adjustedText()      );

    appendProperty(data, i18n("Mime")                    , d->labelImageMime->adjustedText()            );
    appendProperty(data, i18n("Dimensions")              , d->labelImageDimensions->adjustedText()      );
    appendProperty(data, i18n("Ratio")                   , d->labelImageRatio->adjustedText()           );
    appendProperty(data, i18n("Bit Depth")               , d->labelImageBitDepth->adjustedText()        );
    appendProperty(data, i18n("Color Mode")              , d->labelImageColorMode->adjustedText()       );
    appendProperty(data, i18n("Has Sidecar")             , d->labelHasSidecar->adjustedText()           );
    appendProperty(data, i18n("Has GPS")                 , d->labelHasGPSInfo->adjustedText()           );
    appendProperty(data, i18n("Versioned")               , d->labelVersionedInfo->adjustedText()        );
    appendProperty(data, i18n("Grouped")                 , d->labelGroupedInfo->adjustedText()          );

    appendProperty(data, i18n("Make")                    , d->labelPhotoMake->adjustedText()            );
    appendProperty(data, i18n("Model")                   , d->labelPhotoModel->adjustedText()           );
    appendProperty(data, i18n("Date Time")               , d->labelPhotoDateTime->adjustedText()        );
    appendProperty(data, i18n("Lens")                    , d->labelPhotoLens->adjustedText()            );
    appendProperty(data, i18n("Aperture")                , d->labelPhotoAperture->adjustedText()        );
    appendProperty(data, i18n("Focal Length")            , d->labelPhotoFocalLength->adjustedText()     );
    appendProperty(data, i18n("Exposure Time")           , d->labelPhotoExposureTime->adjustedText()    );
    appendProperty(data, i18n("Sensitivity")             , d->labelPhotoSensitivity->adjustedText()     );
    appendProperty(data, i18n("Exposure Mode")           , d->labelPhotoExposureMode->adjustedText()    );
    appendProperty(data, i18n("Flash")                   , d->labelPhotoFlash->adjustedText()           );
    appendProperty(data, i18n("White Balance")           , d->labelPhotoWhiteBalance->adjustedText()    );

    appendProperty(data, i18n("Video Aspect Ratio")      , d->labelVideoAspectRatio->adjustedText()     );
    appendProperty(data, i18n("Video Duration")          , d->labelVideoDuration->adjustedText()        );
    appendProperty(data, i18n("Video Frame Rate")        , d->labelVideoFrameRate->adjustedText()       );
    appendProperty(data, i18n("Video Video Codec")       , d->labelVideoVideoCodec->adjustedText()      );
    appendProperty(data, i18n("Video Audio Bit Rate")    , d->labelVideoAudioBitRate->adjustedText()    );
    appendProperty(data, i18n("Video Audio Channel Type"), d->labelVideoAudioChannelType->adjustedText());
    appendProperty(data, i18n("Video Audio Codec")       , d->labelVideoAudioCodec->adjustedText()      );

    appendProperty(data, i18n("Caption")                 , d->labelCaption->text()                      );
    appendProperty(data, i18n("Title")                   , d->labelTitle->text()                        );
    appendProperty(data, i18n("Tags")                    , d->labelTags->text()                         );
    appendProperty(data, i18n("Peoples")                 , d->labelPeoples->text()                      );
    appendProperty(data, i18n("Location")                , d->labelLocation->text()                     );
    appendProperty(data, i18n("Rights")                  , d->labelRights->text()                       );
    appendProperty(data, i18n("Pick")                    , d->labelPickLabel->adjustedText()            );
    appendProperty(data, i18n("Color")                   , d->labelColorLabel->adjustedText()           );
    appendProperty(data, i18n("Rating")                  , d->labelRating->property("Rating").toString());

    return data;
}

} // namespace Digikam

#include "moc_itempropertiestab.cpp"
