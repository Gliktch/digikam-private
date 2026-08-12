/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-09
 * Description : a presentation tool.
 *
 * SPDX-FileCopyrightText: 2008      by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "presentation_advpage.h"

// Local includes

#include "digikam_config.h"
#include "presentationcontainer.h"
#include "presentationwidget.h"

#ifdef HAVE_OPENGL
#   include "presentationgl.h"
#   include "presentationkb.h"
#endif

using namespace Digikam;

namespace DigikamGenericPresentationPlugin
{

PresentationAdvPage::PresentationAdvPage(QWidget* const parent,
                                         PresentationContainer* const sharedData)
    : QWidget(parent)
{
    setupUi(this);

    m_sharedData = sharedData;

    connect(m_useMillisecondsCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotUseMillisecondsToggled()));

#ifdef HAVE_OPENGL

    m_openGlFullScale->setEnabled(true);

#else

    m_openGlFullScale->setEnabled(false);

#endif

}

void PresentationAdvPage::readSettings()
{
    m_enableMouseWheelCheckBox->setChecked(m_sharedData->enableMouseWheel);
    m_useMillisecondsCheckBox->setChecked(m_sharedData->useMilliseconds);
    m_kbDisableFadeCheckBox->setChecked(m_sharedData->kbDisableFadeInOut);
    m_kbDisableCrossfadeCheckBox->setChecked(m_sharedData->kbDisableCrossFade);
    m_enableKenBurnsSameSpeed->setChecked(m_sharedData->kbEnableSameSpeed);
    m_openGlFullScale->setChecked(m_sharedData->openGlFullScale);
    m_openGlFullScale->setEnabled(m_sharedData->opengl);
    m_useMillisecondsCheckBox->setEnabled(!m_sharedData->offAutoDelay);         // MilliSeconds Checkbox in advanced page off when AutoDelay is On

    slotUseMillisecondsToggled();
}

void PresentationAdvPage::saveSettings()
{

#ifdef HAVE_OPENGL

    m_sharedData->openGlFullScale    = m_openGlFullScale->isChecked();

#endif

    m_sharedData->useMilliseconds    = m_useMillisecondsCheckBox->isChecked();
    m_sharedData->enableMouseWheel   = m_enableMouseWheelCheckBox->isChecked();
    m_sharedData->kbDisableFadeInOut = m_kbDisableFadeCheckBox->isChecked();
    m_sharedData->kbDisableCrossFade = m_kbDisableCrossfadeCheckBox->isChecked();
    m_sharedData->kbEnableSameSpeed  = m_enableKenBurnsSameSpeed->isChecked();
}

void PresentationAdvPage::slotUseMillisecondsToggled()
{
    m_sharedData->useMilliseconds = m_useMillisecondsCheckBox->isChecked();

    Q_EMIT useMillisecondsToggled();
}

} // namespace DigikamGenericPresentationPlugin

#include "moc_presentation_advpage.cpp"
