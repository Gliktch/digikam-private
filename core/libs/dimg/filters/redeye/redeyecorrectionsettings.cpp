/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-09
 * Description : Red Eyes auto correction settings view.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2016      by Omar Amin <Omar dot moh dot amin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "redeyecorrectionsettings.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QCheckBox>
#include <QUrl>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dnuminput.h"
#include "digikam_debug.h"
#include "dexpanderbox.h"

namespace Digikam
{

class Q_DECL_HIDDEN RedEyeCorrectionSettings::Private
{
public:

    Private() = default;

    const QString configRedToAvgRatioAdjustmentEntry = QLatin1String("RedToAvgRatioAdjustment");

    DDoubleNumInput* redtoavgratioInput = nullptr;
};

// --------------------------------------------------------

RedEyeCorrectionSettings::RedEyeCorrectionSettings(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    const int spacing       = layoutSpacing();
    QGridLayout* const grid = new QGridLayout(parent);

    QLabel* const label2    = new QLabel(i18n("Red Level:"));
    d->redtoavgratioInput   = new DDoubleNumInput();
    d->redtoavgratioInput->setRange(1.5, 3.0, 0.1);
    d->redtoavgratioInput->setDefaultValue(2.1);
    d->redtoavgratioInput->setWhatsThis(i18n("Set here the reducing level of red to the average of blue and green."));

    // -------------------------------------------------------------

    grid->addWidget(label2,                0, 0, 1, 1);
    grid->addWidget(d->redtoavgratioInput, 1, 0, 1, 1);
    grid->setRowStretch(6, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // -------------------------------------------------------------

    connect(d->redtoavgratioInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));
}

RedEyeCorrectionSettings::~RedEyeCorrectionSettings()
{
    delete d;
}

RedEyeCorrectionContainer RedEyeCorrectionSettings::settings() const
{
    RedEyeCorrectionContainer prm;
    prm.m_redToAvgRatio = (double)d->redtoavgratioInput->value();

    return prm;
}

void RedEyeCorrectionSettings::setSettings(const RedEyeCorrectionContainer& settings)
{
    blockSignals(true);
    d->redtoavgratioInput->setValue(settings.m_redToAvgRatio);
    blockSignals(false);
}

void RedEyeCorrectionSettings::resetToDefault()
{
    blockSignals(true);
    d->redtoavgratioInput->slotReset();
    blockSignals(false);
}

RedEyeCorrectionContainer RedEyeCorrectionSettings::defaultSettings() const
{
    RedEyeCorrectionContainer prm;
    prm.m_redToAvgRatio = (double)(d->redtoavgratioInput->defaultValue());

    return prm;
}

void RedEyeCorrectionSettings::readSettings(const KConfigGroup& group)
{
    RedEyeCorrectionContainer prm;
    RedEyeCorrectionContainer defaultPrm = defaultSettings();
    prm.m_redToAvgRatio                  = group.readEntry(d->configRedToAvgRatioAdjustmentEntry,
                                                           defaultPrm.m_redToAvgRatio);

    setSettings(prm);
}

void RedEyeCorrectionSettings::writeSettings(KConfigGroup& group)
{
    RedEyeCorrectionContainer prm = settings();
    group.writeEntry(d->configRedToAvgRatioAdjustmentEntry, prm.m_redToAvgRatio);
}

} // namespace Digikam

#include "moc_redeyecorrectionsettings.cpp"
