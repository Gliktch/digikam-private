/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-04-02
 * Description : WEBP image Converter batch tool.
 *
 * SPDX-FileCopyrightText: 2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "converttowebp.h"

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

namespace DigikamBqmConvertToWebpPlugin
{

ConvertToWEBP::ConvertToWEBP(QObject* const parent)
    : BatchTool(QLatin1String("ConvertToWEBP"), ConvertTool, parent)
{
}

BatchTool* ConvertToWEBP::clone(QObject* const parent) const
{
    return new ConvertToWEBP(parent);
}

void ConvertToWEBP::registerSettingsWidget()
{
    DImgLoaderSettings* const WEBPBox = DPluginLoader::instance()->exportWidget(QLatin1String("WEBP"));

    if (WEBPBox)
    {
        connect(WEBPBox, SIGNAL(signalSettingsChanged()),
                this, SLOT(slotSettingsChanged()));
    }

    m_settingsWidget = WEBPBox;

    BatchTool::registerSettingsWidget();
}

BatchToolSettings ConvertToWEBP::defaultSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("ImageViewer Settings"));
    int compression           = group.readEntry(QLatin1String("WEBPCompression"), 75);
    bool lossLessCompression  = group.readEntry(QLatin1String("WEBPLossLess"),    true);
    BatchToolSettings settings;
    settings.insert(QLatin1String("quality"),  compression);
    settings.insert(QLatin1String("lossless"), lossLessCompression);

    return settings;
}

void ConvertToWEBP::slotAssignSettings2Widget()
{
    m_changeSettings                  = false;
    DImgLoaderSettings* const WEBPBox = dynamic_cast<DImgLoaderSettings*>(m_settingsWidget);

    if (WEBPBox)
    {
        DImgLoaderPrms set;
        set.insert(QLatin1String("quality"),  settings().value(QLatin1String("quality")).toInt());
        set.insert(QLatin1String("lossless"), settings().value(QLatin1String("lossless")).toBool());
        WEBPBox->setSettings(set);
    }

    m_changeSettings                  = true;
}

void ConvertToWEBP::slotSettingsChanged()
{
    if (m_changeSettings)
    {
        const DImgLoaderSettings* const WEBPBox = dynamic_cast<DImgLoaderSettings*>(m_settingsWidget);

        if (WEBPBox)
        {
            BatchToolSettings settings;
            settings.insert(QLatin1String("quality"),  WEBPBox->settings().value(QLatin1String("quality")).toInt());
            settings.insert(QLatin1String("lossless"), WEBPBox->settings().value(QLatin1String("lossless")).toBool());
            BatchTool::slotSettingsChanged(settings);
        }
    }
}

QString ConvertToWEBP::outputSuffix() const
{
    return QLatin1String("webp");
}

bool ConvertToWEBP::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    bool lossless = settings().value(QLatin1String("lossless")).toBool();
    image().setAttribute(QLatin1String("quality"), lossless ? 100 : settings().value(QLatin1String("quality")).toInt());

    return (savefromDImg());
}

} // namespace DigikamBqmConvertToWebpPlugin

#include "moc_converttowebp.cpp"
