/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-28
 * Description : JPEG2000 image Converter batch tool.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "converttojp2.h"

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

namespace DigikamBqmConvertToJp2Plugin
{

ConvertToJP2::ConvertToJP2(QObject* const parent)
    : BatchTool(QLatin1String("ConvertToJP2"), ConvertTool, parent)
{
}

BatchTool* ConvertToJP2::clone(QObject* const parent) const
{
    return new ConvertToJP2(parent);
}

void ConvertToJP2::registerSettingsWidget()
{
    DImgLoaderSettings* const JP2Box = DPluginLoader::instance()->exportWidget(QLatin1String("JP2"));

    if (JP2Box)
    {
        connect(JP2Box, SIGNAL(signalSettingsChanged()),
                this, SLOT(slotSettingsChanged()));
    }

    m_settingsWidget = JP2Box;

    BatchTool::registerSettingsWidget();
}

BatchToolSettings ConvertToJP2::defaultSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("ImageViewer Settings"));
    int compression           = group.readEntry(QLatin1String("JPEG2000Compression"), 75);
    bool lossLessCompression  = group.readEntry(QLatin1String("JPEG2000LossLess"),    true);
    BatchToolSettings settings;
    settings.insert(QLatin1String("quality"),  compression);
    settings.insert(QLatin1String("lossless"), lossLessCompression);

    return settings;
}

void ConvertToJP2::slotAssignSettings2Widget()
{
    m_changeSettings                 = false;
    DImgLoaderSettings* const JP2Box = dynamic_cast<DImgLoaderSettings*>(m_settingsWidget);

    if (JP2Box)
    {
        DImgLoaderPrms set;
        set.insert(QLatin1String("quality"),  settings().value(QLatin1String("quality")).toInt());
        set.insert(QLatin1String("lossless"), settings().value(QLatin1String("lossless")).toBool());
        JP2Box->setSettings(set);
    }

    m_changeSettings = true;
}

void ConvertToJP2::slotSettingsChanged()
{
    if (m_changeSettings)
    {
        BatchToolSettings settings;
        const DImgLoaderSettings* const JP2Box = dynamic_cast<DImgLoaderSettings*>(m_settingsWidget);

        if (JP2Box)
        {
            settings.insert(QLatin1String("quality"),  JP2Box->settings().value(QLatin1String("quality")).toInt());
            settings.insert(QLatin1String("lossless"), JP2Box->settings().value(QLatin1String("lossless")).toBool());
            BatchTool::slotSettingsChanged(settings);
        }
    }
}

QString ConvertToJP2::outputSuffix() const
{
    return QLatin1String("jp2");
}

bool ConvertToJP2::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    bool lossless = settings().value(QLatin1String("lossless")).toBool();
    image().setAttribute(QLatin1String("quality"), lossless ? 100 : settings().value(QLatin1String("quality")).toInt());

    return (savefromDImg());
}

} // namespace DigikamBqmConvertToJp2Plugin

#include "moc_converttojp2.cpp"
