/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-07-24
 * Description : MJPEG Stream configuration dialog - Views methods
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021      by Quoc Hưng Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mjpegstreamdlg_p.h"

namespace DigikamGenericMjpegStreamPlugin
{

QWidget* MjpegStreamDlg::setupItemsView()
{
    d->albumSupport   = (d->settings.iface && d->settings.iface->supportAlbums());
    QWidget* itemsSel = nullptr;

    if (d->albumSupport)
    {
        d->albumSelector = d->settings.iface->albumChooser(this);
        itemsSel         = d->albumSelector;

        connect(d->settings.iface, SIGNAL(signalAlbumChooserSelectionChanged()),
                this, SLOT(slotSelectionChanged()));
    }
    else
    {
        d->listView = new DItemsList(this);
        d->listView->setObjectName(QLatin1String("MjpegStream ImagesList"));
        d->listView->setControlButtonsPlacement(DItemsList::ControlButtonsRight);
        d->listView->setIface(d->settings.iface);

        // Add all items currently loaded in application.

        d->listView->loadImagesFromCurrentSelection();

        // Replug the previous shared items list.

        d->listView->slotAddImages(d->mngr->itemsList());
        itemsSel    = d->listView;

        connect(d->listView, SIGNAL(signalImageListChanged()),
                this, SLOT(slotSelectionChanged()));
    }

    return itemsSel;
}

void MjpegStreamDlg::setupServerView()
{
    QWidget* const serverSettings = new QWidget(d->tabView);

    QLabel* const portLbl         = new QLabel(i18nc("@label", "Server Port:"), serverSettings);
    d->srvPort                    = new DIntNumInput(serverSettings);
    d->srvPort->setDefaultValue(8080);
    d->srvPort->setRange(1025, 65535, 1);
    d->srvPort->setWhatsThis(i18nc("@info", "The MJPEG server IP port."));
    portLbl->setBuddy(d->srvPort);

    d->startOnStartup             = new QCheckBox(i18nc("@option", "Start Server at Startup"));
    d->startOnStartup->setWhatsThis(i18nc("@info", "Set this option to turn-on the MJPEG server at application start-up automatically"));
    d->startOnStartup->setChecked(true);

    // ---

    QLabel* const explanation     = new QLabel(serverSettings);
    explanation->setOpenExternalLinks(true);
    explanation->setWordWrap(true);
    explanation->setFrameStyle(QFrame::Box | QFrame::Plain);

    explanation->setText(i18nc("@info",
        "The %1 server allows to share items through the local network using a web browser. "
        "Motion JPEG is a video compression format in which each video frame is compressed "
        "separately as a JPEG image. MJPEG stream is a standard which allows network clients to be "
        "connected without additional module. Most major web browsers and players support MJPEG stream. "
        "To access the stream from your browser, just use http://address:port as url, with address "       // krazy:exclude=insecurenet
        "the MJPEG server IP, and port the server port set in this config dialog.",
        QLatin1String("<a href='https://en.wikipedia.org/wiki/Motion_JPEG'>Motion JPEG</a>")));

    d->srvButton                  = new QPushButton(serverSettings);
    d->srvStatus                  = new QLabel(serverSettings);
    d->aStats                     = new QLabel(serverSettings);
    d->separator                  = new QLabel(QLatin1String(" / "), serverSettings);
    d->iStats                     = new QLabel(serverSettings);
    d->progress                   = new WorkingWidget(serverSettings);
    d->srvPreview                 = new QPushButton(i18nc("@action: button", "Preview..."), serverSettings);
    d->srvPreview->setWhatsThis(i18nc("@info", "Press this button to preview the stream on your computer with your browser."));

    QGridLayout* const grid3      = new QGridLayout(serverSettings);
    grid3->addWidget(portLbl,           0, 0, 1, 1);
    grid3->addWidget(d->srvPort,        0, 1, 1, 1);
    grid3->addWidget(d->startOnStartup, 0, 2, 1, 5);
    grid3->addWidget(d->srvButton,      1, 0, 1, 1);
    grid3->addWidget(d->srvStatus,      1, 1, 1, 1);
    grid3->addWidget(d->aStats,         1, 2, 1, 1);
    grid3->addWidget(d->separator,      1, 3, 1, 1);
    grid3->addWidget(d->iStats,         1, 4, 1, 1);
    grid3->addWidget(d->progress,       1, 5, 1, 1);
    grid3->addWidget(d->srvPreview,     1, 6, 1, 1);
    grid3->addWidget(explanation,       2, 0, 1, 7);
    grid3->setSpacing(d->spacing);

    d->tabView->insertTab(Private::Server, serverSettings, i18nc("@title", "Server"));

    connect(d->srvButton, SIGNAL(clicked()),
            this, SLOT(slotToggleMjpegServer()));

    connect(d->srvPort, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->srvPreview, SIGNAL(clicked()),
            this, SLOT(slotOpenPreview()));
}

void MjpegStreamDlg::setupStreamView()
{
    d->streamSettings         = new QWidget(d->tabView);

    QLabel* const qualityLbl  = new QLabel(i18nc("@label", "JPEG Quality:"), d->streamSettings);
    d->quality                = new DIntNumInput(d->streamSettings);
    d->quality->setDefaultValue(75);
    d->quality->setRange(50, 100, 1);
    d->quality->setWhatsThis(i18nc("@info", "The JPEG quality [50:lower - 100:higher]. "
                                            "Warning: better quality requires more network bandwidth"));

    qualityLbl->setBuddy(d->quality);

    // ---

    QLabel* const typeLabel   = new QLabel(d->streamSettings);
    typeLabel->setWordWrap(false);
    typeLabel->setText(i18nc("@label", "JPEG Size:"));
    d->typeVal                = new DComboBox(d->streamSettings);
    d->typeVal->combo()->setEditable(false);

    QMap<VidSlideSettings::VidType, QString> map                = VidSlideSettings::videoTypeNames();
    QMap<VidSlideSettings::VidType, QString>::const_iterator it = map.constBegin();

    while (it != map.constEnd())
    {
        d->typeVal->addItem(it.value(), (int)it.key());
        ++it;
    }

    d->typeVal->setDefaultIndex(VidSlideSettings::BLUERAY);
    d->typeVal->setWhatsThis(i18nc("@info", "The JPEG image size in pixels, using standard screen resolutions. "
                                            "Warning: larger size requires more network bandwidth"));
    typeLabel->setBuddy(d->typeVal);

    // ---

    QLabel* const delayLbl    = new QLabel(i18nc("@label", "Delay in Seconds:"), d->streamSettings);
    d->delay                  = new DIntNumInput(d->streamSettings);
    d->delay->setDefaultValue(5);
    d->delay->setRange(1, 3600, 1);
    d->delay->setWhatsThis(i18nc("@info", "The delay in seconds between images."));
    delayLbl->setBuddy(d->delay);

    QLabel* const rateLbl     = new QLabel(i18nc("@label", "Frames by Second:"), d->streamSettings);
    d->rate                   = new DIntNumInput(d->streamSettings);
    d->rate->setDefaultValue(10);
    d->rate->setRange(5, 24, 1);
    d->rate->setWhatsThis(i18nc("@info", "The number of frames by second to render the stream. "
                                         "Warning: larger rate requires more network bandwidth"));
    rateLbl->setBuddy(d->rate);

    d->streamLoop             = new QCheckBox(i18nc("@option:check", "Stream in Loop"), d->streamSettings);
    d->streamLoop->setChecked(true);
    d->streamLoop->setWhatsThis(i18nc("@info", "The MJPEG stream will be played in loop instead of playing once."));

    QGridLayout* const grid2  = new QGridLayout(d->streamSettings);
    grid2->addWidget(qualityLbl,    0, 0, 1, 1);
    grid2->addWidget(d->quality,    0, 1, 1, 1);
    grid2->addWidget(typeLabel,     1, 0, 1, 1);
    grid2->addWidget(d->typeVal,    1, 1, 1, 1);
    grid2->addWidget(delayLbl,      2, 0, 1, 1);
    grid2->addWidget(d->delay,      2, 1, 1, 1);
    grid2->addWidget(rateLbl,       3, 0, 1, 1);
    grid2->addWidget(d->rate,       3, 1, 1, 1);
    grid2->addWidget(d->streamLoop, 4, 0, 1, 2);
    grid2->setSpacing(d->spacing);

    d->tabView->insertTab(Private::Stream, d->streamSettings, i18nc("@title", "Stream"));

    connect(d->delay, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->rate, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->quality, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->streamLoop, SIGNAL(stateChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->typeVal, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotSettingsChanged()));
}

void MjpegStreamDlg::setupTransitionView()
{
    QWidget* const transitionSettings = new QWidget(d->tabView);

    QLabel* const transLabel          = new QLabel(transitionSettings);
    transLabel->setWordWrap(false);
    transLabel->setText(i18nc("@label", "Type:"));
    d->transVal                       = new DComboBox(transitionSettings);
    d->transVal->combo()->setEditable(false);

    QMap<TransitionMngr::TransType, QString> map4                = TransitionMngr::transitionNames();
    QMap<TransitionMngr::TransType, QString>::const_iterator it4 = map4.constBegin();

    while (it4 != map4.constEnd())
    {
        d->transVal->addItem(it4.value(), (int)it4.key());
        ++it4;
    }

    d->transVal->setDefaultIndex(TransitionMngr::None);
    transLabel->setBuddy(d->transVal);

    QLabel* const transNote  = new QLabel(transitionSettings);
    transNote->setWordWrap(true);
    transNote->setText(i18nc("@label", "A transition is a visual effect applied between two images. "
                                       "For some effects, the duration can depend on random values and "
                                       "can change while the stream is playing."));

    d->transPreview              = new TransitionPreview(transitionSettings);
    d->transPreview->setImagesList(QList<QUrl>());

    QGridLayout* const transGrid = new QGridLayout(transitionSettings);
    transGrid->setSpacing(d->spacing);
    transGrid->addWidget(transLabel,      0, 0, 1, 1);
    transGrid->addWidget(d->transVal,     0, 1, 1, 1);
    transGrid->addWidget(transNote,       1, 0, 1, 2);
    transGrid->addWidget(d->transPreview, 0, 2, 2, 1);
    transGrid->setColumnStretch(1, 10);
    transGrid->setRowStretch(1, 10);

    d->tabView->insertTab(Private::Transition, transitionSettings, i18nc("@title", "Transition"));

    connect(d->transVal, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotSettingsChanged()));
}

void MjpegStreamDlg::setupEffectView()
{
    QWidget* const effectSettings = new QWidget(d->tabView);

    QLabel* const effLabel        = new QLabel(effectSettings);
    effLabel->setWordWrap(false);
    effLabel->setText(i18nc("@label", "Type:"));
    d->effVal                     = new DComboBox(effectSettings);
    d->effVal->combo()->setEditable(false);

    QMap<EffectMngr::EffectType, QString> map6                = EffectMngr::effectNames();
    QMap<EffectMngr::EffectType, QString>::const_iterator it6 = map6.constBegin();

    while (it6 != map6.constEnd())
    {
        d->effVal->insertItem((int)it6.key(), it6.value(), (int)it6.key());
        ++it6;
    }

    d->effVal->setDefaultIndex(EffectMngr::None);
    effLabel->setBuddy(d->effVal);

    QLabel* const effNote      = new QLabel(effectSettings);
    effNote->setWordWrap(true);
    effNote->setText(i18nc("@label", "An effect is a visual panning or zooming applied while an image "
                                     "is displayed in MJPEG stream."));

    d->effPreview              = new EffectPreview(effectSettings);
    d->effPreview->setImagesList(QList<QUrl>());

    QGridLayout* const effGrid = new QGridLayout(effectSettings);
    effGrid->setSpacing(d->spacing);
    effGrid->addWidget(effLabel,      0, 0, 1, 1);
    effGrid->addWidget(d->effVal,     0, 1, 1, 1);
    effGrid->addWidget(effNote,       1, 0, 1, 2);
    effGrid->addWidget(d->effPreview, 0, 2, 2, 1);
    effGrid->setColumnStretch(1, 10);
    effGrid->setRowStretch(1, 10);

    d->tabView->insertTab(Private::Effect, effectSettings, i18nc("@title", "Effect"));

    connect(d->effVal, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotSettingsChanged()));
}

void MjpegStreamDlg::setupOSDView()
{
    d->frameOsdWidget = new FrameOsdWidget(d->tabView);

    d->tabView->insertTab(Private::OSD, d->frameOsdWidget,
                          i18nc("@title: On Screen Display", "OSD"));

    connect(d->frameOsdWidget, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));
}

} // namespace DigikamGenericMjpegStreamPlugin
