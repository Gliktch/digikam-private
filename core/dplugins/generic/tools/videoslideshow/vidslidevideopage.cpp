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

#include "vidslidevideopage.h"

// Qt includes

#include <QIcon>
#include <QLabel>
#include <QSpinBox>
#include <QUrl>
#include <QWidget>
#include <QApplication>
#include <QStyle>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "vidslidewizard.h"
#include "transitionpreview.h"
#include "effectpreview.h"
#include "digikam_debug.h"

namespace DigikamGenericVideoSlideShowPlugin
{

class Q_DECL_HIDDEN VidSlideVideoPage::Private
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

    QSpinBox*          framesVal    = nullptr;
    QComboBox*         typeVal      = nullptr;
    QComboBox*         bitrateVal   = nullptr;
    QComboBox*         stdVal       = nullptr;
    QComboBox*         codecVal     = nullptr;
    QComboBox*         transVal     = nullptr;
    QComboBox*         effVal       = nullptr;
    QLabel*            duration     = nullptr;
    VidSlideWizard*    wizard       = nullptr;
    VidSlideSettings*  settings     = nullptr;
    TransitionPreview* transPreview = nullptr;
    EffectPreview*     effPreview   = nullptr;
};

VidSlideVideoPage::VidSlideVideoPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d          (new Private(dialog))
{
    setObjectName(QLatin1String("VideoPage"));

    QWidget* const main       = new QWidget(this);

    // --------------------

    QLabel* const framesLabel = new QLabel(main);
    framesLabel->setWordWrap(false);
    framesLabel->setText(i18n("Number of Frames by Image:"));
    d->framesVal              = new QSpinBox(main);
    d->framesVal->setRange(1, 15000);
    d->framesVal->setToolTip(i18n("This value repeat one image in the sequence.\nWhile creating timelapse; it is usual to use 1.\n"
                                  "Typically, the number of frames will determine\nthe time to show the same frame in the video.\n"
                                  "The result will depend of the number of frames\nper second used to encode the media."));
    framesLabel->setBuddy(d->framesVal);

    // --------------------

    QLabel* const stdLabel = new QLabel(main);
    stdLabel->setWordWrap(false);
    stdLabel->setText(i18n("Video Standard:"));
    d->stdVal              = new QComboBox(main);
    d->stdVal->setEditable(false);

    QMap<VidSlideSettings::VidStd, QString> map3                = VidSlideSettings::videoStdNames();
    QMap<VidSlideSettings::VidStd, QString>::const_iterator it3 = map3.constBegin();

    while (it3 != map3.constEnd())
    {
        d->stdVal->addItem(it3.value(), (int)it3.key());
        ++it3;
    }

    stdLabel->setBuddy(d->stdVal);

    // --------------------

    QLabel* const typeLabel = new QLabel(main);
    typeLabel->setWordWrap(false);
    typeLabel->setText(i18n("Video Type:"));
    d->typeVal              = new QComboBox(main);
    d->typeVal->setEditable(false);

    QMap<VidSlideSettings::VidType, QString> map                = VidSlideSettings::videoTypeNames();
    QMap<VidSlideSettings::VidType, QString>::const_iterator it = map.constBegin();

    while (it != map.constEnd())
    {
        if (VidSlideSettings::isVideoTVFormat(it.key()))
        {
            d->typeVal->addItem(it.value(), (int)it.key());
        }

        ++it;
    }

    typeLabel->setBuddy(d->typeVal);

    // --------------------

    QLabel* const bitrateLabel = new QLabel(main);
    bitrateLabel->setWordWrap(false);
    bitrateLabel->setText(i18n("Video Bit Rate:"));
    d->bitrateVal              = new QComboBox(main);
    d->bitrateVal->setEditable(false);

    QMap<VidSlideSettings::VidBitRate, QString> map2                = VidSlideSettings::videoBitRateNames();
    QMap<VidSlideSettings::VidBitRate, QString>::const_iterator it2 = map2.constBegin();

    while (it2 != map2.constEnd())
    {
        d->bitrateVal->addItem(it2.value(), (int)it2.key());
        ++it2;
    }

    bitrateLabel->setBuddy(d->bitrateVal);

    // --------------------

    QLabel* const codecLabel = new QLabel(main);
    codecLabel->setWordWrap(false);
    codecLabel->setText(i18n("Video Codec:"));
    d->codecVal              = new QComboBox(main);
    d->codecVal->setToolTip(i18n("This control allows to choose the codec used to encode the video stream."
                                 "The entries in this list are populated corresponding to the codec supported by FFMpeg."));
    d->codecVal->setEditable(false);
    codecLabel->setBuddy(d->codecVal);

    // --------------------

    QGroupBox* const effGrp = new QGroupBox(i18n("Effect While Displaying Images"), main);
    QLabel* const effLabel  = new QLabel(effGrp);
    effLabel->setWordWrap(false);
    effLabel->setText(i18n("Type:"));
    d->effVal               = new QComboBox(effGrp);
    d->effVal->setEditable(false);

    QMap<EffectMngr::EffectType, QString> map6                = EffectMngr::effectNames();
    QMap<EffectMngr::EffectType, QString>::const_iterator it6 = map6.constBegin();

    while (it6 != map6.constEnd())
    {
        d->effVal->insertItem((int)it6.key(), it6.value(), (int)it6.key());
        ++it6;
    }

    effLabel->setBuddy(d->effVal);

    QLabel* const effNote  = new QLabel(effGrp);
    effNote->setWordWrap(true);
    effNote->setText(i18n("<i>An effect is an visual panning or zooming applied while an image "
                          "is displayed. The effect duration will follow the number of frames used "
                          "to render the image on video stream. For a timelapse, let this setting to None.</i>"));

    d->effPreview              = new EffectPreview(effGrp);
    QGridLayout* const effGrid = new QGridLayout(effGrp);
    effGrid->setSpacing(layoutSpacing());
    effGrid->addWidget(effLabel,      0, 0, 1, 1);
    effGrid->addWidget(d->effVal,     0, 1, 1, 1);
    effGrid->addWidget(effNote,       1, 0, 1, 2);
    effGrid->addWidget(d->effPreview, 0, 2, 2, 1);
    effGrid->setColumnStretch(1, 10);
    effGrid->setRowStretch(1, 10);

    // --------------------

    QGroupBox* const transGrp = new QGroupBox(i18n("Transition Between Images"), main);
    QLabel* const transLabel  = new QLabel(transGrp);
    transLabel->setWordWrap(false);
    transLabel->setText(i18n("Type:"));
    d->transVal               = new QComboBox(transGrp);
    d->transVal->setEditable(false);

    QMap<TransitionMngr::TransType, QString> map4                = TransitionMngr::transitionNames();
    QMap<TransitionMngr::TransType, QString>::const_iterator it4 = map4.constBegin();

    while (it4 != map4.constEnd())
    {
        d->transVal->addItem(it4.value(), (int)it4.key());
        ++it4;
    }

    transLabel->setBuddy(d->transVal);

    QLabel* const transNote  = new QLabel(transGrp);
    transNote->setWordWrap(true);
    transNote->setText(i18n("<i>A transition is an visual effect applied between two images. "
                            "For some effects, the duration can depend of random values and "
                            "can change while the slideshow. For a timelapse, let this setting to None.</i>"));

    d->transPreview              = new TransitionPreview(transGrp);
    QGridLayout* const transGrid = new QGridLayout(transGrp);
    transGrid->setSpacing(layoutSpacing());

    transGrid->addWidget(transLabel,      0, 0, 1, 1);
    transGrid->addWidget(d->transVal,     0, 1, 1, 1);
    transGrid->addWidget(transNote,       1, 0, 1, 2);
    transGrid->addWidget(d->transPreview, 0, 2, 2, 1);
    transGrid->setColumnStretch(1, 10);
    transGrid->setRowStretch(1, 10);

    // --------------------

    d->duration     = new QLabel(main);

    // --------------------

    QGridLayout* const grid = new QGridLayout(main);
    grid->setSpacing(layoutSpacing());

    grid->addWidget(framesLabel,     0, 0, 1, 1);
    grid->addWidget(d->framesVal,    0, 1, 1, 1);
    grid->addWidget(stdLabel,        1, 0, 1, 1);
    grid->addWidget(d->stdVal,       1, 1, 1, 1);
    grid->addWidget(typeLabel,       2, 0, 1, 1);
    grid->addWidget(d->typeVal,      2, 1, 1, 1);
    grid->addWidget(bitrateLabel,    3, 0, 1, 1);
    grid->addWidget(d->bitrateVal,   3, 1, 1, 1);
    grid->addWidget(codecLabel,      4, 0, 1, 1);
    grid->addWidget(d->codecVal,     4, 1, 1, 1);
    grid->addWidget(effGrp,          5, 0, 1, 2);
    grid->addWidget(transGrp,        6, 0, 1, 2);
    grid->addWidget(d->duration,     7, 0, 1, 2);
    grid->setRowStretch(8, 10);

    setPageWidget(main);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("video-mp4")));

    // --------------------

    connect(d->framesVal, SIGNAL(valueChanged(int)),
            this, SLOT(slotSlideDuration()));

    connect(d->stdVal, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotSlideDuration()));

    connect(d->transVal, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotTransitionChanged()));

    connect(d->effVal, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotEffectChanged()));
}

VidSlideVideoPage::~VidSlideVideoPage()
{
    delete d;
}

void VidSlideVideoPage::slotTransitionChanged()
{
    d->transPreview->stopPreview();
    d->transPreview->startPreview((TransitionMngr::TransType)d->transVal->currentIndex());
}

void VidSlideVideoPage::slotEffectChanged()
{
    d->effPreview->stopPreview();
    d->effPreview->startPreview((EffectMngr::EffectType)d->effVal->currentIndex());
}

void VidSlideVideoPage::slotSlideDuration()
{
    VidSlideSettings tmp;
    tmp.imgFrames = d->framesVal->value();
    tmp.vStandard = (VidSlideSettings::VidStd)d->stdVal->currentIndex();
    qreal titem   = tmp.imgFrames / tmp.videoFrameRate();
    qreal ttotal  = titem * d->settings->inputImages.count();
    d->duration->setText(i18n("Duration : %1 seconds by item, total %2 seconds (without transitions)",
                              titem, ttotal));
}

void VidSlideVideoPage::initializePage()
{
    // Populate Codecs List

    d->codecVal->clear();
    QMap<VidSlideSettings::VidCodec, QString> map5                = VidSlideSettings::videoCodecNames();
    QMap<VidSlideSettings::VidCodec, QString>::const_iterator it5 = map5.constBegin();

    QStringList codecs = d->settings->ffmpegCodecs.keys();
    int currentCodec   = d->settings->vCodec;

    while (it5 != map5.constEnd())
    {
        d->codecVal->insertItem((int)it5.key(), it5.value(), (int)it5.key());

        // Disable entry if FFmpeg codec is not available.

        VidSlideSettings tmp;
        tmp.vCodec = (VidSlideSettings::VidCodec)it5.key();

        if (!codecs.contains(tmp.videoCodec()))
        {
            d->codecVal->setItemData((int)it5.key(), false, Qt::UserRole - 1);
        }
        else
        {
            if ((int)it5.key() == currentCodec)
            {
                d->codecVal->setCurrentIndex(currentCodec);
            }
        }

        ++it5;
    }

    d->framesVal->setValue(d->settings->imgFrames);
    d->typeVal->setCurrentIndex(d->typeVal->findData(d->settings->vType));
    d->bitrateVal->setCurrentIndex(d->settings->vbitRate);
    d->stdVal->setCurrentIndex(d->settings->vStandard);
    d->effVal->setCurrentIndex(d->settings->vEffect);
    d->transVal->setCurrentIndex(d->settings->transition);
    d->transPreview->setImagesList(d->settings->inputImages);
    d->effPreview->setImagesList(d->settings->inputImages);
    slotSlideDuration();
}

bool VidSlideVideoPage::validatePage()
{
    d->transPreview->stopPreview();
    d->effPreview->stopPreview();
    d->settings->imgFrames  = d->framesVal->value();
    d->settings->vType      = (VidSlideSettings::VidType)d->typeVal->currentData().toInt();
    d->settings->vbitRate   = (VidSlideSettings::VidBitRate)d->bitrateVal->currentIndex();
    d->settings->vStandard  = (VidSlideSettings::VidStd)d->stdVal->currentIndex();
    d->settings->vCodec     = (VidSlideSettings::VidCodec)d->codecVal->currentData().toInt();
    d->settings->vEffect    = (EffectMngr::EffectType)d->effVal->currentIndex();
    d->settings->transition = (TransitionMngr::TransType)d->transVal->currentIndex();

    return true;
}

} // namespace DigikamGenericVideoSlideShowPlugin

#include "moc_vidslidevideopage.cpp"
