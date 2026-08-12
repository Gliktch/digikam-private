/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to generate video slideshow from images.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "vidslideoutputpage.h"

// Qt includes

#include <QIcon>
#include <QLabel>
#include <QUrl>
#include <QTimer>
#include <QWidget>
#include <QApplication>
#include <QStyle>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QGridLayout>
#include <QStandardItemModel>
#include <QStandardPaths>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "frameosdwidget.h"
#include "vidslidewizard.h"
#include "dfileselector.h"
#include "filesaveconflictbox.h"
#include "ffmpeglauncher.h"
#include "dexpanderbox.h"

#ifdef HAVE_MEDIAPLAYER

#   include "audplayerwdg.h"

#endif

namespace DigikamGenericVideoSlideShowPlugin
{

class Q_DECL_HIDDEN VidSlideOutputPage::Private
{
public:

    explicit Private(QWizard* const dialog)
    {
        wizard = dynamic_cast<VidSlideWizard*>(dialog);

        if (wizard)
        {
            settings = wizard->settings();
        }
    }

public:

    DFileSelector*       audioUrl    = nullptr;
    DFileSelector*       destUrl     = nullptr;
    FileSaveConflictBox* conflictBox = nullptr;
    QComboBox*           playerVal   = nullptr;
    QComboBox*           formatVal   = nullptr;
    VidSlideWizard*      wizard      = nullptr;
    VidSlideSettings*    settings    = nullptr;
    QLabel*              duration    = nullptr;
    QTimer*              trigUpdate  = nullptr;
    QCheckBox*           equalize    = nullptr;
    QSpinBox*            strength    = nullptr;
    FrameOsdWidget*      frameOsd    = nullptr;
    DExpanderBox*        expanderBox = nullptr;

#ifdef HAVE_MEDIAPLAYER

    AudPlayerWdg*        audioPlayer = nullptr;

#endif

};

VidSlideOutputPage::VidSlideOutputPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d          (new Private(dialog))
{
    setObjectName(QLatin1String("OutputPage"));

    d->trigUpdate             = new QTimer(this);
    d->trigUpdate->setSingleShot(true);

    d->expanderBox   = new DExpanderBox;
    d->expanderBox->setObjectName(QLatin1String("VideoSlideShow Output Settings Expander"));

    // --------------------

    QWidget* const outputBox    = new QWidget(d->expanderBox);
    QGridLayout* const formGrid = new QGridLayout(outputBox);

    QLabel* const formatLabel   = new QLabel(outputBox);
    formatLabel->setWordWrap(false);
    formatLabel->setText(i18n("Media Container Format:"));
    d->formatVal                = new QComboBox(outputBox);
    d->formatVal->setEditable(false);
    d->formatVal->setToolTip(i18n("This control allows to choose the format of the media container to host video stream and soundtrack."
                                  "The entries in this list are populated corresponding to the format supported by FFMpeg."));
    formatLabel->setBuddy(d->formatVal);

    QLabel* const playerLabel = new QLabel(outputBox);
    playerLabel->setWordWrap(false);
    playerLabel->setText(i18n("Open in Player:"));
    d->playerVal              = new QComboBox(outputBox);
    d->playerVal->setEditable(false);

    QMap<VidSlideSettings::VidPlayer, QString> map2                = VidSlideSettings::videoPlayerNames();
    QMap<VidSlideSettings::VidPlayer, QString>::const_iterator it2 = map2.constBegin();

    while (it2 != map2.constEnd())
    {
        d->playerVal->addItem(it2.value(), (int)it2.key());
        ++it2;
    }

#ifndef HAVE_MEDIAPLAYER

    auto* const model = qobject_cast<QStandardItemModel*>(d->playerVal->model());

    if (model)
    {
        auto* const item = model->item(VidSlideSettings::INTERNAL);

        if (item)
        {
            item->setEnabled(false);
        }
    }

#endif

    playerLabel->setBuddy(d->playerVal);

    formGrid->addWidget(formatLabel,     0, 0, 1, 1);
    formGrid->addWidget(d->formatVal,    0, 1, 1, 1);
    formGrid->addWidget(playerLabel,     1, 0, 1, 1);
    formGrid->addWidget(d->playerVal,    1, 1, 1, 1);

    d->expanderBox->addItem(outputBox,
                            QIcon::fromTheme(QLatin1String("video-mp4")),
                            i18n("Output"),
                            QLatin1String("Output"), true);

    // --------------------

    QWidget* const videoBox     = new QWidget(d->expanderBox);
    QGridLayout* const normGrid = new QGridLayout(videoBox);

    d->equalize                 = new QCheckBox(i18n("Equalize Video Luminosity"), videoBox);

    QLabel* const strengthLabel = new QLabel(videoBox);
    strengthLabel->setWordWrap(false);
    strengthLabel->setEnabled(false);
    strengthLabel->setText(i18n("Equalization Strength:"));
    d->strength                 = new QSpinBox(videoBox);
    d->strength->setEnabled(false);
    d->strength->setRange(0, 10);
    d->strength->setToolTip(i18n("This controls strength of equalization. Setting this option to zero effectively does nothing."));
    strengthLabel->setBuddy(d->strength);

    QLabel* const equalNote     = new QLabel(videoBox);
    equalNote->setWordWrap(true);
    equalNote->setText(i18n("<i>These settings apply temporal midway video equalization effect while "
                            "encoding frames as video. This adjusts the sequence of frames to have "
                            "the same histograms, while maintaining their dynamics as much as possible. "
                            "It’s useful for e.g. matching exposures from a timelapse sequence.</i>"));

    normGrid->addWidget(d->equalize,     0, 0, 1, 2);
    normGrid->addWidget(strengthLabel,   1, 0, 1, 1);
    normGrid->addWidget(d->strength,     1, 1, 1, 1);
    normGrid->addWidget(equalNote,       2, 0, 1, 2);

    d->expanderBox->addItem(videoBox,
                            QIcon::fromTheme(QLatin1String("view-media-equalizer")),
                            i18n("Frames Normalization"),
                            QLatin1String("Normalization"), true);

    // --------------------

    QWidget* const audioBox      = new QWidget(d->expanderBox);
    QGridLayout* const audioGrid = new QGridLayout(audioBox);

    QLabel* const audioLabel = new QLabel(audioBox);
    audioLabel->setWordWrap(false);
    audioLabel->setText(i18n("Soundtrack File:"));

    d->audioUrl              = new DFileSelector(audioBox);
    d->audioUrl->setFileDlgMode(QFileDialog::ExistingFile);
    d->audioUrl->setFileDlgOptions(QFileDialog::ReadOnly);
    d->audioUrl->setFileDlgFilter(QLatin1String("*.mp3 *.ogg *.wav *.flac *.m4a"));
    d->audioUrl->setFileDlgTitle(i18nc("@title:window", "Select Audio Track"));
    audioLabel->setBuddy(d->audioUrl);

    QLabel* const durationLabel = new QLabel(i18n("Soundtrack Duration:"), audioBox);
    d->duration                 = new QLabel(QLatin1String("---"), audioBox);
    d->duration->setAlignment(Qt::AlignRight);

#ifdef HAVE_MEDIAPLAYER

    QLabel* const previewLabel = new QLabel(i18n("Soundtrack Preview:"), audioBox);
    d->audioPlayer             = new AudPlayerWdg(audioBox);

#endif

    QLabel* const audioNote     = new QLabel(audioBox);
    audioNote->setWordWrap(true);
    audioNote->setText(i18n("<i>Notes about soundtrack: if the audio length is smaller than video, it will be "
                            "played in loop. If the audio length is larger than video, it will be trimmed. "
                            "Leave this setting empty if you do not want a soundtrack to the media.</i>"));

    audioGrid->addWidget(audioLabel,      0, 0, 1, 1);
    audioGrid->addWidget(d->audioUrl,     0, 1, 1, 2);
    audioGrid->addWidget(durationLabel,   1, 0, 1, 1);
    audioGrid->addWidget(d->duration,     1, 2, 1, 1);

#ifdef HAVE_MEDIAPLAYER

    audioGrid->addWidget(previewLabel,    2, 0, 1, 1);
    audioGrid->addWidget(d->audioPlayer,  2, 2, 1, 1);

#endif

    audioGrid->addWidget(audioNote,       3, 0, 1, 3);
    audioGrid->setColumnStretch(1, 10);

    d->expanderBox->addItem(audioBox,
                            QIcon::fromTheme(QLatin1String("audio-mp3")),
                            i18n("Audio Track"),
                            QLatin1String("Soundtrack"), true);

    // --------------------

    d->frameOsd = new FrameOsdWidget(d->expanderBox);

    d->expanderBox->addItem(d->frameOsd,
                            QIcon::fromTheme(QLatin1String("draw-text")),
                            i18n("On Screen Display"),
                            QLatin1String("OSD"), true);

    // --------------------

    QWidget* const destBox      = new QWidget(d->expanderBox);
    QGridLayout* const destGrid = new QGridLayout(destBox);

    QLabel* const fileLabel = new QLabel(destBox);
    fileLabel->setWordWrap(false);
    fileLabel->setText(i18n("Destination folder:"));

    d->destUrl              = new DFileSelector(destBox);
    d->destUrl->setFileDlgMode(QFileDialog::Directory);
    d->destUrl->setFileDlgOptions(QFileDialog::ShowDirsOnly);
    d->destUrl->setFileDlgTitle(i18nc("@title:window", "Destination Folder"));
    d->destUrl->lineEdit()->setPlaceholderText(i18n("Output Destination Path"));
    fileLabel->setBuddy(d->destUrl);

    QLabel* const outputLbl = new QLabel(destBox);
    outputLbl->setText(i18n("The video output file name will be generated automatically."));
    d->conflictBox          = new FileSaveConflictBox(destBox);

    destGrid->addWidget(fileLabel,      0, 0, 1, 1);
    destGrid->addWidget(d->destUrl,     0, 1, 1, 1);
    destGrid->addWidget(outputLbl,      1, 0, 1, 2);
    destGrid->addWidget(d->conflictBox, 2, 0, 1, 2);

    d->expanderBox->addItem(destBox,
                            QIcon::fromTheme(QLatin1String("folder-new")),
                            i18n("Target File"),
                            QLatin1String("File"), true);

    d->expanderBox->addStretch();

    setPageWidget(d->expanderBox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("folder-video")));

    connect(d->trigUpdate, SIGNAL(timeout()),
            this, SIGNAL(completeChanged()));

    connect(d->destUrl->lineEdit(), &QLineEdit::textEdited,
            this, &VidSlideOutputPage::slotTriggerUpdate);

    connect(d->destUrl, SIGNAL(signalUrlSelected(QUrl)),
            this, SLOT(slotTriggerUpdate()));

    connect(d->audioUrl->lineEdit(), &QLineEdit::textEdited,
            this, &VidSlideOutputPage::slotTriggerUpdate);

    connect(d->audioUrl, SIGNAL(signalUrlSelected(QUrl)),
            this, SLOT(slotTriggerUpdate()));

    connect(d->equalize, SIGNAL(toggled(bool)),
            strengthLabel, SLOT(setEnabled(bool)));

    connect(d->equalize, SIGNAL(toggled(bool)),
            d->strength, SLOT(setEnabled(bool)));
}

VidSlideOutputPage::~VidSlideOutputPage()
{
    delete d;
}

void VidSlideOutputPage::initializePage()
{
    // Populate Formats List

    d->formatVal->clear();
    QMap<VidSlideSettings::VidFormat, QString> map                = VidSlideSettings::videoFormatNames();
    QMap<VidSlideSettings::VidFormat, QString>::const_iterator it = map.constBegin();

    QStringList formats = d->settings->ffmpegFormats.keys();
    int currentFormat   = d->settings->vFormat;

    while (it != map.constEnd())
    {
        d->formatVal->addItem(it.value(), (int)it.key());

        // Disable entry if FFmpeg format is not available.

        VidSlideSettings tmp;
        tmp.vFormat = (VidSlideSettings::VidFormat)it.key();

        if (!formats.contains(tmp.videoFormat()))
        {
            d->formatVal->setItemData((int)it.key(), false, Qt::UserRole-1);
        }
        else
        {
            if ((int)it.key() == currentFormat)
            {
                d->formatVal->setCurrentIndex(currentFormat);
            }
        }

        ++it;
    }

    d->audioUrl->setFileDlgPath(d->settings->audioTrack);

#ifdef HAVE_MEDIAPLAYER

    d->audioPlayer->setAudioFile(d->settings->audioTrack);

#endif

    d->destUrl->setFileDlgPath(d->settings->outputDir);
    d->conflictBox->setConflictRule(d->settings->conflictRule);
    d->playerVal->setCurrentIndex(d->settings->outputPlayer);
    d->equalize->setChecked(d->settings->equalize);
    d->strength->setValue(d->settings->strength);

    d->frameOsd->setSettings(d->settings->osdSettings);
}

bool VidSlideOutputPage::validatePage()
{
    if (d->destUrl->fileDlgPath().isEmpty())
    {
        return false;
    }

    d->settings->vFormat      = (VidSlideSettings::VidFormat)d->formatVal->currentIndex();
    d->settings->outputDir    = d->destUrl->fileDlgPath();
    d->settings->audioTrack   = d->audioUrl->fileDlgPath();
    d->settings->conflictRule = d->conflictBox->conflictRule();
    d->settings->outputPlayer = (VidSlideSettings::VidPlayer)d->playerVal->currentIndex();
    d->settings->equalize     = d->equalize->isChecked();
    d->settings->strength     = d->strength->value();

    d->settings->osdSettings  = d->frameOsd->settings();

    return true;
}

void VidSlideOutputPage::slotTriggerUpdate()
{
    d->trigUpdate->start(1000);
}

bool VidSlideOutputPage::isComplete() const
{
    d->duration->setText(QLatin1String("---"));

    QString apath = d->audioUrl->fileDlgPath();

#ifdef HAVE_MEDIAPLAYER

    d->audioPlayer->setAudioFile(apath);
    d->audioPlayer->setDisabled(apath.isEmpty());


#endif

    if (!apath.isEmpty())
    {
        FFmpegLauncher ffmpeg;
        ffmpeg.setSettings(d->wizard->settings());
        d->wizard->settings()->soundtrackLength = ffmpeg.soundTrackLength(apath);

        if (d->wizard->settings()->soundtrackLength.isValid())
        {
            d->duration->setText(d->wizard->settings()->soundtrackLength.toString());
        }
    }

    return (!d->destUrl->fileDlgPath().isEmpty());
}

} // namespace DigikamGenericVideoSlideShowPlugin

#include "moc_vidslideoutputpage.cpp"
