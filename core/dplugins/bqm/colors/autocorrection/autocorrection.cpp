/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-28
 * Description : auto colors correction batch tool.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "autocorrection.h"

// Qt includes

#include <QLabel>
#include <QWidget>
#include <QComboBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dimg.h"
#include "autolevelsfilter.h"
#include "equalizefilter.h"
#include "stretchfilter.h"
#include "autoexpofilter.h"
#include "normalizefilter.h"

namespace DigikamBqmAutoCorrectionPlugin
{

AutoCorrection::AutoCorrection(QObject* const parent)
    : BatchTool(QLatin1String("AutoCorrection"), ColorTool, parent)
{
}

BatchTool* AutoCorrection::clone(QObject* const parent) const
{
    return new AutoCorrection(parent);
}

void AutoCorrection::registerSettingsWidget()
{
    DVBox* const vbox   = new DVBox;
    QLabel* const label = new QLabel(vbox);
    m_comboBox          = new QComboBox(vbox);
    m_comboBox->insertItem(AutoLevelsCorrection,      i18n("Auto Levels"));
    m_comboBox->insertItem(NormalizeCorrection,       i18n("Normalize"));
    m_comboBox->insertItem(EqualizeCorrection,        i18n("Equalize"));
    m_comboBox->insertItem(StretchContrastCorrection, i18n("Stretch Contrast"));
    m_comboBox->insertItem(AutoExposureCorrection,    i18n("Auto Exposure"));
    label->setText(i18n("Filter:"));
    QLabel* const space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    m_settingsWidget = vbox;

    connect(m_comboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings AutoCorrection::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert(QLatin1String("AutoCorrectionFilter"), AutoLevelsCorrection);
    return settings;
}

void AutoCorrection::slotAssignSettings2Widget()
{
    m_comboBox->setCurrentIndex(settings().value(QLatin1String("AutoCorrectionFilter")).toInt());
}

void AutoCorrection::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert(QLatin1String("AutoCorrectionFilter"), (int)m_comboBox->currentIndex());
    BatchTool::slotSettingsChanged(settings);
}

bool AutoCorrection::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    int type = settings().value(QLatin1String("AutoCorrectionFilter")).toInt();

    switch (type)
    {
        case AutoLevelsCorrection:
        {
            AutoLevelsFilter autolevels(&image(), &image());
            applyFilter(&autolevels);
            break;
        }

        case NormalizeCorrection:
        {
            NormalizeFilter normalize(&image(), &image());
            applyFilter(&normalize);
            break;
        }

        case EqualizeCorrection:
        {
            EqualizeFilter equalize(&image(), &image());
            applyFilter(&equalize);
            break;
        }

        case StretchContrastCorrection:
        {
            StretchFilter stretch(&image(), &image());
            applyFilter(&stretch);
            break;
        }

        case AutoExposureCorrection:
        {
            AutoExpoFilter expo(&image(), &image());
            applyFilter(&expo);
            break;
        }
    }

    return (savefromDImg());
}

} // namespace DigikamBqmAutoCorrectionPlugin

#include "moc_autocorrection.cpp"
