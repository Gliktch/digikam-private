/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-07-24
 * Description : frame on screen display settings widget
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021      by Quoc Hưng Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "frameosdwidget.h"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dfontselect.h"

namespace Digikam
{

class Q_DECL_HIDDEN FrameOsdWidget::Private
{
public:

    Private() = default;

    QCheckBox*   showName            = nullptr;
    QCheckBox*   showDate            = nullptr;
    QCheckBox*   showApertureFocal   = nullptr;
    QCheckBox*   showExpoSensitivity = nullptr;
    QCheckBox*   showMakeModel       = nullptr;
    QCheckBox*   showLensModel       = nullptr;
    QCheckBox*   showComment         = nullptr;
    QCheckBox*   showTitle           = nullptr;
    QCheckBox*   showTags            = nullptr;
    QCheckBox*   showRating          = nullptr;
    QCheckBox*   showCapIfNoTitle    = nullptr;
    DFontSelect* osdFont             = nullptr;
};

FrameOsdWidget::FrameOsdWidget(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    d->showName                = new QCheckBox(i18nc("@option:check", "Show image file name"), this);
    d->showName->setWhatsThis(i18nc("@info", "Show the image file name at the bottom of the screen."));

    d->showDate                = new QCheckBox(i18nc("@option:check", "Show image creation date"), this);
    d->showDate->setWhatsThis(i18nc("@info", "Show the image creation time/date at the bottom of the screen."));

    d->showApertureFocal       = new QCheckBox(i18nc("@option:check", "Show camera aperture and focal length"), this);
    d->showApertureFocal->setWhatsThis(i18nc("@info", "Show the camera aperture and focal length at the bottom of the screen."));

    d->showExpoSensitivity     = new QCheckBox(i18nc("@option:check", "Show camera exposure and sensitivity"), this);
    d->showExpoSensitivity->setWhatsThis(i18nc("@info", "Show the camera exposure and sensitivity at the bottom of the screen."));

    d->showMakeModel           = new QCheckBox(i18nc("@option:check", "Show camera make and model"), this);
    d->showMakeModel->setWhatsThis(i18nc("@info", "Show the camera make and model at the bottom of the screen."));

    d->showLensModel           = new QCheckBox(i18nc("@option:check", "Show camera lens model"), this);
    d->showLensModel->setWhatsThis(i18nc("@info", "Show the camera lens model at the bottom of the screen."));

    d->showComment             = new QCheckBox(i18nc("@option:check", "Show image caption"), this);
    d->showComment->setWhatsThis(i18nc("@info", "Show the image caption at the bottom of the screen."));

    d->showTitle               = new QCheckBox(i18nc("@option:check", "Show image title"), this);
    d->showTitle->setWhatsThis(i18nc("@info", "Show the image title at the bottom of the screen."));

    d->showCapIfNoTitle        = new QCheckBox(i18nc("@option:check", "Show image caption if it has not title"), this);
    d->showCapIfNoTitle->setWhatsThis(i18nc("@info", "Show the image caption at the bottom of the screen if no titles existed."));

    d->showRating              = new QCheckBox(i18nc("@option:check", "Show image rating"), this);
    d->showRating->setWhatsThis(i18nc("@info", "Show the digiKam image rating at the bottom of the screen."));

    d->showTags                = new QCheckBox(i18nc("@option:check", "Show image tags"), this);
    d->showTags->setWhatsThis(i18nc("@info", "Show the digiKam image tag names at the bottom of the screen."));

    d->osdFont                 = new DFontSelect(i18nc("@option", "On Screen Display Font:"), this);
    d->osdFont->setToolTip(i18nc("@info", "Select here the font used to display text on the screen display ."));

    QGridLayout* const grid    = new QGridLayout(this);
    grid->addWidget(d->showName,             1, 0, 1, 1);
    grid->addWidget(d->showRating,           1, 1, 1, 1);
    grid->addWidget(d->showApertureFocal,    2, 0, 1, 1);
    grid->addWidget(d->showDate,             2, 1, 1, 1);
    grid->addWidget(d->showMakeModel,        3, 0, 1, 1);
    grid->addWidget(d->showExpoSensitivity,  3, 1, 1, 1);
    grid->addWidget(d->showLensModel,        4, 0, 1, 1);
    grid->addWidget(d->showComment,          4, 1, 1, 1);
    grid->addWidget(d->showTitle,            5, 0, 1, 1);
    grid->addWidget(d->showCapIfNoTitle,     5, 1, 1, 1);
    grid->addWidget(d->showTags,             6, 0, 1, 1);
    grid->addWidget(d->osdFont,              7, 0, 1, 2);

    connect(d->showName, SIGNAL(stateChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->showApertureFocal, SIGNAL(stateChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->showDate, SIGNAL(stateChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->showMakeModel, SIGNAL(stateChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->showExpoSensitivity, SIGNAL(stateChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->showLensModel, SIGNAL(stateChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->showComment, SIGNAL(stateChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

   // Disable and uncheck the "Show captions if no title" checkbox if the "Show comment" checkbox enabled

    connect(d->showComment, SIGNAL(stateChanged(int)),
            this, SLOT(slotSetUnchecked(int)));

    connect(d->showComment, SIGNAL(toggled(bool)),
            d->showCapIfNoTitle, SLOT(setDisabled(bool)));

    connect(d->showTitle, SIGNAL(stateChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->showCapIfNoTitle, SIGNAL(stateChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->showTags, SIGNAL(stateChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->showRating, SIGNAL(stateChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->osdFont, SIGNAL(signalFontChanged()),
            this, SIGNAL(signalSettingsChanged()));
}

FrameOsdWidget::~FrameOsdWidget()
{
    delete d;
}

void FrameOsdWidget::slotSetUnchecked(int)
{
    d->showCapIfNoTitle->setCheckState(Qt::Unchecked);
}

void FrameOsdWidget::setSettings(const FrameOsdSettings& settings)
{
    d->showDate->blockSignals(true);
    d->showApertureFocal->blockSignals(true);
    d->showCapIfNoTitle->blockSignals(true);
    d->showComment->blockSignals(true);
    d->showExpoSensitivity->blockSignals(true);
    d->showLensModel->blockSignals(true);
    d->showMakeModel->blockSignals(true);
    d->showName->blockSignals(true);
    d->showTags->blockSignals(true);
    d->showRating->blockSignals(true);
    d->showTitle->blockSignals(true);
    d->osdFont->blockSignals(true);

    d->showDate->setChecked(settings.printDate);
    d->showApertureFocal->setChecked(settings.printApertureFocal);
    d->showCapIfNoTitle->setChecked(settings.printCapIfNoTitle);
    d->showComment->setChecked(settings.printComment);
    d->showExpoSensitivity->setChecked(settings.printExpoSensitivity);
    d->showLensModel->setChecked(settings.printLensModel);
    d->showMakeModel->setChecked(settings.printMakeModel);
    d->showName->setChecked(settings.printName);
    d->showTags->setChecked(settings.printTags);
    d->showRating->setChecked(settings.printRating);
    d->showTitle->setChecked(settings.printTitle);
    d->osdFont->setFont(settings.osdFont);

    d->showDate->blockSignals(false);
    d->showApertureFocal->blockSignals(false);
    d->showCapIfNoTitle->blockSignals(false);
    d->showComment->blockSignals(false);
    d->showExpoSensitivity->blockSignals(false);
    d->showLensModel->blockSignals(false);
    d->showMakeModel->blockSignals(false);
    d->showName->blockSignals(false);
    d->showTags->blockSignals(false);
    d->showRating->blockSignals(false);
    d->showTitle->blockSignals(false);
    d->osdFont->blockSignals(false);
}

FrameOsdSettings FrameOsdWidget::settings() const
{
    FrameOsdSettings settings;

    settings.printName             = d->showName->isChecked();
    settings.printDate             = d->showDate->isChecked();
    settings.printApertureFocal    = d->showApertureFocal->isChecked();
    settings.printExpoSensitivity  = d->showExpoSensitivity->isChecked();
    settings.printMakeModel        = d->showMakeModel->isChecked();
    settings.printLensModel        = d->showLensModel->isChecked();
    settings.printComment          = d->showComment->isChecked();
    settings.printTitle            = d->showTitle->isChecked();
    settings.printCapIfNoTitle     = d->showCapIfNoTitle->isChecked();
    settings.printTags             = d->showTags->isChecked();
    settings.printRating           = d->showRating->isChecked();
    settings.osdFont               = d->osdFont->font();

    return settings;
}

} // namespace Digikam

#include "moc_frameosdwidget.cpp"
