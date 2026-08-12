/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-21
 * Description : a widget to setup the preview OSD.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "previewosdwidget.h"

// Qt includes

#include <QLabel>
#include <QApplication>
#include <QGridLayout>
#include <QCheckBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dfontselect.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN PreviewOsdWidget::Private
{
public:

    Private() = default;

public:

    QCheckBox*          showName             = nullptr;
    QCheckBox*          showDate             = nullptr;
    QCheckBox*          showApertureFocal    = nullptr;
    QCheckBox*          showExpoSensitivity  = nullptr;
    QCheckBox*          showMakeModel        = nullptr;
    QCheckBox*          showLensModel        = nullptr;
    QCheckBox*          showComment          = nullptr;
    QCheckBox*          showTitle            = nullptr;
    QCheckBox*          showTags             = nullptr;
    QCheckBox*          showCapIfNoTitle     = nullptr;

    DFontSelect*        textFont             = nullptr;

    PreviewOsdSettings* settings             = nullptr;
};

// --------------------------------------------------------

PreviewOsdWidget::PreviewOsdWidget(PreviewOsdSettings* const settings, QWidget* const parent)
    : QGroupBox(i18n("On Screen Display"), parent),
      d      (new Private)
{
    d->settings               = settings;

    QGridLayout* const igrid  = new QGridLayout(this);

    d->showName               = new QCheckBox(i18n("Show item file name"), this);
    d->showName->setWhatsThis(i18n("Show the item file name at the bottom of the screen."));

    d->showDate               = new QCheckBox(i18n("Show item creation date"), this);
    d->showDate->setWhatsThis(i18n("Show the item creation time/date at the bottom of the screen."));

    d->showApertureFocal      = new QCheckBox(i18n("Show camera aperture and focal length"), this);
    d->showApertureFocal->setWhatsThis(i18n("Show the camera aperture and focal length at the bottom of the screen."));

    d->showExpoSensitivity    = new QCheckBox(i18n("Show camera exposure and sensitivity"), this);
    d->showExpoSensitivity->setWhatsThis(i18n("Show the camera exposure and sensitivity at the bottom of the screen."));

    d->showMakeModel          = new QCheckBox(i18n("Show camera make and model"), this);
    d->showMakeModel->setWhatsThis(i18n("Show the camera make and model at the bottom of the screen."));

    d->showLensModel          = new QCheckBox(i18n("Show camera lens model"), this);
    d->showLensModel->setWhatsThis(i18n("Show the camera lens model at the bottom of the screen."));

    d->showComment            = new QCheckBox(i18n("Show item caption"), this);
    d->showComment->setWhatsThis(i18n("Show the item caption at the bottom of the screen."));

    d->showTitle              = new QCheckBox(i18n("Show item title"), this);
    d->showTitle->setWhatsThis(i18n("Show the item title at the bottom of the screen."));

    d->showCapIfNoTitle       = new QCheckBox(i18n("Show item caption if it has not title"), this);
    d->showCapIfNoTitle->setWhatsThis(i18n("Show the item caption at the bottom of the screen if no titles existed."));

    d->showTags               = new QCheckBox(i18n("Show item tags"), this);
    d->showTags->setWhatsThis(i18n("Show the digiKam item tag names at the bottom of the screen."));

    d->textFont               = new DFontSelect(i18n("Text font:"), this);
    d->textFont->setToolTip(i18n("Select here the font used to display OSD text over the slideshow."));

    igrid->addWidget(d->showName,             0, 0, 1, 1);
    igrid->addWidget(d->showDate,             0, 2, 1, 1);
    igrid->addWidget(d->showApertureFocal,    1, 0, 1, 1);
    igrid->addWidget(d->showExpoSensitivity,  1, 2, 1, 1);
    igrid->addWidget(d->showMakeModel,        2, 0, 1, 1);
    igrid->addWidget(d->showLensModel,        2, 2, 1, 1);
    igrid->addWidget(d->showComment,          3, 0, 1, 1);
    igrid->addWidget(d->showTitle,            3, 2, 1, 1);
    igrid->addWidget(d->showCapIfNoTitle,     4, 0, 1, 1);
    igrid->addWidget(d->showTags,             4, 2, 1, 1);
    igrid->addWidget(d->textFont,             6, 0, 1, 3);
    igrid->setColumnStretch(1, 10);

    // Disable and uncheck the "Show texts if no title" checkbox if the "Show comment" checkbox enabled

    connect(d->showComment, SIGNAL(stateChanged(int)),
            this, SLOT(slotSetUnchecked(int)));

    connect(d->showComment, SIGNAL(toggled(bool)),
            d->showCapIfNoTitle, SLOT(setDisabled(bool)));

    // Only digiKam support this feature, showFoto do not support digiKam database information.

    if (qApp->applicationName() == QLatin1String("showfoto"))
    {
        d->showCapIfNoTitle->hide();
        d->showTitle->hide();
        d->showTags->hide();
    }

    readSettings();
}

PreviewOsdWidget::~PreviewOsdWidget()
{
    delete d;
}

void PreviewOsdWidget::slotSetUnchecked(int)
{
    d->showCapIfNoTitle->setCheckState(Qt::Unchecked);
}

void PreviewOsdWidget::writeSettings()
{
    d->settings->printName             = d->showName->isChecked();
    d->settings->printDate             = d->showDate->isChecked();
    d->settings->printApertureFocal    = d->showApertureFocal->isChecked();
    d->settings->printExpoSensitivity  = d->showExpoSensitivity->isChecked();
    d->settings->printMakeModel        = d->showMakeModel->isChecked();
    d->settings->printLensModel        = d->showLensModel->isChecked();
    d->settings->printCaption          = d->showComment->isChecked();
    d->settings->printTitle            = d->showTitle->isChecked();
    d->settings->printCapIfNoTitle     = d->showCapIfNoTitle->isChecked();
    d->settings->printKeywords         = d->showTags->isChecked();
    d->settings->captionFont           = d->textFont->font();
}

void PreviewOsdWidget::readSettings()
{
    d->showName->setChecked(d->settings->printName);
    d->showDate->setChecked(d->settings->printDate);
    d->showApertureFocal->setChecked(d->settings->printApertureFocal);
    d->showExpoSensitivity->setChecked(d->settings->printExpoSensitivity);
    d->showMakeModel->setChecked(d->settings->printMakeModel);
    d->showLensModel->setChecked(d->settings->printLensModel);
    d->showComment->setChecked(d->settings->printCaption);
    d->showTitle->setChecked(d->settings->printTitle);
    d->showCapIfNoTitle->setChecked(d->settings->printCapIfNoTitle);
    d->showTags->setChecked(d->settings->printKeywords);
    d->textFont->setFont(d->settings->captionFont);
}

} // namespace Digikam

#include "moc_previewosdwidget.cpp"
