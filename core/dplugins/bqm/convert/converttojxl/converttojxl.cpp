/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-04-02
 * Description : JXL image Converter batch tool.
 *
 * SPDX-FileCopyrightText: 2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "converttojxl.h"

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

namespace DigikamBqmConvertToJxlPlugin
{

ConvertToJXL::ConvertToJXL(QObject* const parent)
    : BatchTool(QLatin1String("ConvertToJXL"), ConvertTool, parent)
{
}

BatchTool* ConvertToJXL::clone(QObject* const parent) const
{
    return new ConvertToJXL(parent);
}

void ConvertToJXL::registerSettingsWidget()
{
    DImgLoaderSettings* const JXLBox = DPluginLoader::instance()->exportWidget(QLatin1String("JXL"));

    if (JXLBox)
    {
        connect(JXLBox, SIGNAL(signalSettingsChanged()),
                this, SLOT(slotSettingsChanged()));
    }

    m_settingsWidget = JXLBox;

    BatchTool::registerSettingsWidget();
}

BatchToolSettings ConvertToJXL::defaultSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("ImageViewer Settings"));
    int compression           = group.readEntry(QLatin1String("JXLCompression"), 75);
    bool lossLessCompression  = group.readEntry(QLatin1String("JXLLossLess"),    true);
    BatchToolSettings settings;
    settings.insert(QLatin1String("quality"),  compression);
    settings.insert(QLatin1String("lossless"), lossLessCompression);

    return settings;
}

void ConvertToJXL::slotAssignSettings2Widget()
{
    m_changeSettings                 = false;
    DImgLoaderSettings* const JXLBox = dynamic_cast<DImgLoaderSettings*>(m_settingsWidget);

    if (JXLBox)
    {
        DImgLoaderPrms set;
        set.insert(QLatin1String("quality"),  settings().value(QLatin1String("quality")).toInt());
        set.insert(QLatin1String("lossless"), settings().value(QLatin1String("lossless")).toBool());
        JXLBox->setSettings(set);
    }

    m_changeSettings                 = true;
}

void ConvertToJXL::slotSettingsChanged()
{
    if (m_changeSettings)
    {
        const DImgLoaderSettings* const JXLBox = dynamic_cast<DImgLoaderSettings*>(m_settingsWidget);

        if (JXLBox)
        {
            BatchToolSettings settings;
            settings.insert(QLatin1String("quality"),  JXLBox->settings().value(QLatin1String("quality")).toInt());
            settings.insert(QLatin1String("lossless"), JXLBox->settings().value(QLatin1String("lossless")).toBool());
            BatchTool::slotSettingsChanged(settings);
        }
    }
}

QString ConvertToJXL::outputSuffix() const
{
    return QLatin1String("jxl");
}

bool ConvertToJXL::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    bool lossless = settings().value(QLatin1String("lossless")).toBool();
    image().setAttribute(QLatin1String("quality"), lossless ? 100 : settings().value(QLatin1String("quality")).toInt());

    return (savefromDImg());
}

} // namespace DigikamBqmConvertToJxlPlugin

#include "moc_converttojxl.cpp"
