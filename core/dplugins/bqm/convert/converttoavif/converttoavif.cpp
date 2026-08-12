/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-04-03
 * Description : AVIF image Converter batch tool.
 *
 * SPDX-FileCopyrightText: 2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "converttoavif.h"

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

namespace DigikamBqmConvertToAvifPlugin
{

ConvertToAVIF::ConvertToAVIF(QObject* const parent)
    : BatchTool(QLatin1String("ConvertToAVIF"), ConvertTool, parent)
{
}

BatchTool* ConvertToAVIF::clone(QObject* const parent) const
{
    return new ConvertToAVIF(parent);
}

void ConvertToAVIF::registerSettingsWidget()
{
    DImgLoaderSettings* const AVIFBox = DPluginLoader::instance()->exportWidget(QLatin1String("AVIF"));

    if (AVIFBox)
    {
        connect(AVIFBox, SIGNAL(signalSettingsChanged()),
                this, SLOT(slotSettingsChanged()));
    }

    m_settingsWidget = AVIFBox;

    BatchTool::registerSettingsWidget();
}

BatchToolSettings ConvertToAVIF::defaultSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("ImageViewer Settings"));
    int compression           = group.readEntry(QLatin1String("AVIFCompression"), 75);
    bool lossLessCompression  = group.readEntry(QLatin1String("AVIFLossLess"),    true);
    BatchToolSettings settings;
    settings.insert(QLatin1String("quality"),  compression);
    settings.insert(QLatin1String("lossless"), lossLessCompression);

    return settings;
}

void ConvertToAVIF::slotAssignSettings2Widget()
{
    m_changeSettings                  = false;
    DImgLoaderSettings* const AVIFBox = dynamic_cast<DImgLoaderSettings*>(m_settingsWidget);

    if (AVIFBox)
    {
        DImgLoaderPrms set;
        set.insert(QLatin1String("quality"),  settings().value(QLatin1String("quality")).toInt());
        set.insert(QLatin1String("lossless"), settings().value(QLatin1String("lossless")).toBool());
        AVIFBox->setSettings(set);
    }

    m_changeSettings                 = true;
}

void ConvertToAVIF::slotSettingsChanged()
{
    if (m_changeSettings)
    {
        const DImgLoaderSettings* const AVIFBox = dynamic_cast<DImgLoaderSettings*>(m_settingsWidget);

        if (AVIFBox)
        {
            BatchToolSettings settings;
            settings.insert(QLatin1String("quality"),  AVIFBox->settings().value(QLatin1String("quality")).toInt());
            settings.insert(QLatin1String("lossless"), AVIFBox->settings().value(QLatin1String("lossless")).toBool());
            BatchTool::slotSettingsChanged(settings);
        }
    }
}

QString ConvertToAVIF::outputSuffix() const
{
    return QLatin1String("avif");
}

bool ConvertToAVIF::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    bool lossless = settings().value(QLatin1String("lossless")).toBool();
    image().setAttribute(QLatin1String("quality"), lossless ? 100 : settings().value(QLatin1String("quality")).toInt());

    return (savefromDImg());
}

} // namespace DigikamBqmConvertToAvifPlugin

#include "moc_converttoavif.cpp"
