/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-18
 * Description : PGF image Converter batch tool.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "converttopgf.h"

// Qt includes

#include <QFileInfo>
#include <QWidget>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "dimg.h"
#include "dpluginloader.h"

namespace DigikamBqmConvertToPgfPlugin
{

ConvertToPGF::ConvertToPGF(QObject* const parent)
    : BatchTool(QLatin1String("ConvertToPGF"), ConvertTool, parent)
{
}

BatchTool* ConvertToPGF::clone(QObject* const parent) const
{
    return new ConvertToPGF(parent);
}

void ConvertToPGF::registerSettingsWidget()
{
    DImgLoaderSettings* const PGFBox = DPluginLoader::instance()->exportWidget(QLatin1String("PGF"));

    connect(PGFBox, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    m_settingsWidget          = PGFBox;

    BatchTool::registerSettingsWidget();
}

BatchToolSettings ConvertToPGF::defaultSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("ImageViewer Settings"));
    int compression           = group.readEntry(QLatin1String("PGFCompression"), 3);
    bool lossLessCompression  = group.readEntry(QLatin1String("PGFLossLess"),    true);
    BatchToolSettings settings;
    settings.insert(QLatin1String("quality"),  compression);
    settings.insert(QLatin1String("lossless"), lossLessCompression);

    return settings;
}

void ConvertToPGF::slotAssignSettings2Widget()
{
    m_changeSettings = false;

    DImgLoaderSettings* const PGFBox = dynamic_cast<DImgLoaderSettings*>(m_settingsWidget);

    if (PGFBox)
    {
        DImgLoaderPrms set;
        set.insert(QLatin1String("quality"),  settings().value(QLatin1String("quality")).toInt());
        set.insert(QLatin1String("lossless"), settings().value(QLatin1String("lossless")).toBool());
        PGFBox->setSettings(set);
    }

    m_changeSettings = true;
}

void ConvertToPGF::slotSettingsChanged()
{
    if (m_changeSettings)
    {
        const DImgLoaderSettings* const PGFBox = dynamic_cast<DImgLoaderSettings*>(m_settingsWidget);

        if (PGFBox)
        {
            BatchToolSettings settings;
            settings.insert(QLatin1String("quality"),  PGFBox->settings().value(QLatin1String("quality")).toInt());
            settings.insert(QLatin1String("lossless"), PGFBox->settings().value(QLatin1String("lossless")).toBool());
            BatchTool::slotSettingsChanged(settings);
        }
    }
}

QString ConvertToPGF::outputSuffix() const
{
    return QLatin1String("pgf");
}

bool ConvertToPGF::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    bool lossless = settings().value(QLatin1String("lossless")).toBool();
    image().setAttribute(QLatin1String("quality"), lossless ? 0 : settings().value(QLatin1String("quality")).toInt());

    return (savefromDImg());
}

} // namespace DigikamBqmConvertToPgfPlugin

#include "moc_converttopgf.cpp"
