/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-21
 * Description : setup tab for slideshow options.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupslideshow_dialog.h"

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QStyle>
#include <QGroupBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dlayoutbox.h"
#include "dnuminput.h"
#include "dfontselect.h"
#include "dcolorselector.h"
#include "digikam_debug.h"
#include "slideshowsettings.h"
#include "previewosdwidget.h"

using namespace Digikam;

namespace DigikamGenericSlideShowPlugin
{

class Q_DECL_HIDDEN SetupSlideShowDialog::Private
{
public:

    Private() = default;

public:

    QCheckBox*         startWithCurrent     = nullptr;
    QCheckBox*         loopMode             = nullptr;
    QCheckBox*         suffleMode           = nullptr;
    QCheckBox*         showName             = nullptr;
    QCheckBox*         showDate             = nullptr;
    QCheckBox*         showApertureFocal    = nullptr;
    QCheckBox*         showExpoSensitivity  = nullptr;
    QCheckBox*         showMakeModel        = nullptr;
    QCheckBox*         showLensModel        = nullptr;
    QCheckBox*         showLabels           = nullptr;
    QCheckBox*         showRating           = nullptr;
    QCheckBox*         showComment          = nullptr;
    QCheckBox*         showTitle            = nullptr;
    QCheckBox*         showTags             = nullptr;
    QCheckBox*         showCapIfNoTitle     = nullptr;
    QCheckBox*         showProgress         = nullptr;

    QLabel*            itemBgColorLbl       = nullptr;

    QComboBox*         screenPlacement      = nullptr;

    DFontSelect*       textFont             = nullptr;
    DIntNumInput*      delayInput           = nullptr;
    DColorSelector*    itemBgColorSel       = nullptr;

    PreviewOsdWidget*  osd                  = nullptr;
    SlideShowSettings* settings             = nullptr;
};

// --------------------------------------------------------

SetupSlideShowDialog::SetupSlideShowDialog(SlideShowSettings* const settings, QWidget* const parent)
    : DPluginDialog(parent, QLatin1String("Slideshow Settings")),
      d            (new Private)
{
    d->settings               = settings;

    setModal(true);
    setPlugin(settings->plugin);
    setWindowTitle(i18nc("@title:window", "Slideshow Settings"));

    m_buttons->addButton(QDialogButtonBox::Close);
    m_buttons->addButton(QDialogButtonBox::Ok);
    m_buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    QWidget* const panel      = new QWidget(this);
    const int spacing         = layoutSpacing();

    // ---

    QGroupBox* const behavior = new QGroupBox(i18n("Behavior"), panel);
    QGridLayout* const bgrid  = new QGridLayout(behavior);

    QLabel* const delayLbl    = new QLabel(i18n("Delay between items: "), panel);
    d->delayInput             = new DIntNumInput(panel);
    d->delayInput->setDefaultValue(5);
    d->delayInput->setRange(1, 3600, 1);
    d->delayInput->setWhatsThis(i18n("The delay, in seconds, between items."));

    d->startWithCurrent       = new QCheckBox(i18n("Start with current item"), panel);
    d->startWithCurrent->setWhatsThis(i18n("If this option is enabled, the Slideshow will be started "
                                           "with the current item selected in the items list."));

    d->loopMode               = new QCheckBox(i18n("Slideshow runs in a loop"), panel);
    d->loopMode->setWhatsThis(i18n("Run the slideshow in a loop."));

    d->suffleMode             = new QCheckBox(i18n("Shuffle items"), panel);
    d->suffleMode->setWhatsThis(i18n("If this option is enabled, the Slideshow will run in random order"));

    d->showProgress           = new QCheckBox(i18n("Show progress indicator"), panel);
    d->showProgress->setWhatsThis(i18n("Show a progress indicator with pending items to show and time progression."));

    d->showLabels             = new QCheckBox(i18n("Show labels editor"), panel);
    d->showLabels->setWhatsThis(i18n("Show the digiKam item color label and pick label editors at the bottom of the screen."));

    d->showRating             = new QCheckBox(i18n("Show rating editor"), panel);
    d->showRating->setWhatsThis(i18n("Show the digiKam item rating editor at the bottom of the screen."));

    d->itemBgColorLbl         = new QLabel(i18n("Use background color for items:"), panel);
    d->itemBgColorSel         = new DColorSelector(panel);
    d->itemBgColorSel->setWhatsThis(i18n("Define a color to use as background to render items on screen."));

    QLabel* const screenLbl   = new QLabel(i18n("Screen placement:"), panel);
    d->screenPlacement        = new QComboBox(panel);
    d->screenPlacement->setToolTip(i18n("In case of multi-screen computer, select here the monitor to slide contents."));

    QStringList choices;
    choices.append(i18nc("@label:listbox The current screen, for the presentation mode", "Current Screen"));
    choices.append(i18nc("@label:listbox The default screen for the presentation mode",  "Default Screen"));

    for (int i = 0 ; i < qApp->screens().count() ; ++i)
    {
        QString model = qApp->screens().at(i)->model();
        choices.append(i18nc("@label:listbox %1 is the screen number (0, 1, ...)", "Screen %1", i) +
                             QString::fromUtf8(" (%1)").arg(model.left(model.length() - 1)));
    }

    d->screenPlacement->addItems(choices);

    bgrid->addWidget(delayLbl,            0, 0, 1, 1);
    bgrid->addWidget(d->delayInput,       0, 2, 1, 1);
    bgrid->addWidget(d->startWithCurrent, 1, 0, 1, 1);
    bgrid->addWidget(d->loopMode,         1, 2, 1, 1);
    bgrid->addWidget(d->suffleMode,       2, 0, 1, 1);
    bgrid->addWidget(d->showProgress,     2, 2, 1, 1);
    bgrid->addWidget(d->showLabels,       3, 0, 1, 1);
    bgrid->addWidget(d->showRating,       3, 2, 1, 1);
    bgrid->addWidget(d->itemBgColorLbl,   4, 0, 1, 1);
    bgrid->addWidget(d->itemBgColorSel,   4, 2, 1, 1);
    bgrid->addWidget(screenLbl,           5, 0, 1, 1);
    bgrid->addWidget(d->screenPlacement,  5, 2, 1, 1);
    bgrid->setColumnStretch(1, 10);

    // ---

    d->osd                = new PreviewOsdWidget(&d->settings->osdSettings, panel);

    // ---

    QLabel* const keyNote = new QLabel(i18n("<b>Note: This dialog for the Slideshow Settings "
                                                   "can be activated at any time with the Alt+P key.</b>"), panel);
    keyNote->setWordWrap(true);
    keyNote->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    // Only digiKam support this feature, showFoto do not support digiKam database information.

    if (qApp->applicationName() == QLatin1String("showfoto"))
    {
        d->showLabels->hide();
        d->showRating->hide();
    }

    // ---

    QGridLayout* const grid   = new QGridLayout(panel);
    grid->addWidget(behavior, 0, 0, 1, 1);
    grid->addWidget(d->osd,   1, 0, 1, 1);
    grid->addWidget(keyNote,  2, 0, 1, 1);
    grid->setRowStretch(3, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    QVBoxLayout* const mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(panel);
    mainLayout->addWidget(m_buttons);

    setLayout(mainLayout);

    connect(m_buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(slotApplySettings()));

    connect(m_buttons->button(QDialogButtonBox::Close), SIGNAL(clicked()),
            this, SLOT(reject()));

    readSettings();
}

SetupSlideShowDialog::~SetupSlideShowDialog()
{
    delete d;
}

void SetupSlideShowDialog::slotSetUnchecked(int)
{
    d->showCapIfNoTitle->setCheckState(Qt::Unchecked);
}

void SetupSlideShowDialog::slotApplySettings()
{
    d->settings->delay                 = d->delayInput->value();
    d->settings->startWithCurrent      = d->startWithCurrent->isChecked();
    d->settings->loop                  = d->loopMode->isChecked();
    d->settings->suffle                = d->suffleMode->isChecked();
    d->settings->printLabels           = d->showLabels->isChecked();
    d->settings->printRating           = d->showRating->isChecked();
    d->settings->showProgressIndicator = d->showProgress->isChecked();
    d->settings->bgColor               = d->itemBgColorSel->color();
    d->settings->slideScreen           = d->screenPlacement->currentIndex() - 2;

    d->osd->writeSettings();

    d->settings->writeToConfig();

    accept();
}

void SetupSlideShowDialog::readSettings()
{
    d->delayInput->setValue(d->settings->delay);
    d->startWithCurrent->setChecked(d->settings->startWithCurrent);
    d->loopMode->setChecked(d->settings->loop);
    d->suffleMode->setChecked(d->settings->suffle);
    d->showLabels->setChecked(d->settings->printLabels);
    d->showRating->setChecked(d->settings->printRating);
    d->showProgress->setChecked(d->settings->showProgressIndicator);
    d->itemBgColorSel->setColor(d->settings->bgColor);

    const int screen = d->settings->slideScreen;

    if ((screen >= -2) && (screen < qApp->screens().count()))
    {
        d->screenPlacement->setCurrentIndex(screen + 2);
    }
    else
    {
        d->screenPlacement->setCurrentIndex(0);
        d->settings->slideScreen = -2;
        d->settings->writeToConfig();
    }

    d->osd->readSettings();
}

} // namespace DigikamGenericSlideShowPlugin

#include "moc_setupslideshow_dialog.cpp"
