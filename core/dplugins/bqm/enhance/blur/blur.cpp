/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-03
 * Description : blur image batch tool.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "blur.h"

// Qt includes

#include <QLabel>
#include <QGridLayout>
#include <QWidget>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "blurfilter.h"

namespace DigikamBqmBlurPlugin
{

Blur::Blur(QObject* const parent)
    : BatchTool(QLatin1String("Blur"), EnhanceTool, parent)
{
}

BatchTool* Blur::clone(QObject* const parent) const
{
    return new Blur(parent);
}

void Blur::registerSettingsWidget()
{
    const int spacing = layoutSpacing();


    m_settingsWidget = new QWidget;
    QLabel* label    = new QLabel(i18n("Smoothness:"));
    m_radiusInput    = new DIntNumInput();
    m_radiusInput->setRange(0, 100, 1);
    m_radiusInput->setDefaultValue(0);
    m_radiusInput->setWhatsThis(i18n("A smoothness of 0 has no effect, "
                                     "1 and above determine the Gaussian blur matrix radius "
                                     "that determines how much to blur the image."));

    QGridLayout* grid = new QGridLayout(m_settingsWidget);
    grid->addWidget(label,         0, 0, 1, 2);
    grid->addWidget(m_radiusInput, 1, 0, 1, 2);
    grid->setRowStretch(2, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    connect(m_radiusInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings Blur::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert(QLatin1String("Radius"), (int)m_radiusInput->defaultValue());
    return settings;
}

void Blur::slotAssignSettings2Widget()
{
    m_changeSettings = false;
    m_radiusInput->setValue(settings().value(QLatin1String("Radius")).toInt());
    m_changeSettings = true;
}

void Blur::slotSettingsChanged()
{
    if (m_changeSettings)
    {
        BatchToolSettings settings;
        settings.insert(QLatin1String("Radius"), (int)m_radiusInput->value());
        BatchTool::slotSettingsChanged(settings);
    }
}

bool Blur::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    double radius = settings().value(QLatin1String("Radius")).toInt();

    BlurFilter blur(&image(), nullptr, radius);
    applyFilter(&blur);

    return savefromDImg();
}

} // namespace DigikamBqmBlurPlugin

#include "moc_blur.cpp"
