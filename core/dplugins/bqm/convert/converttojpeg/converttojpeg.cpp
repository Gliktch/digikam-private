/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-24
 * Description : JPEG image Converter batch tool.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "converttojpeg.h"

// Qt includes

#include <QFileInfo>
#include <QWidget>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "dimg.h"
#include "dimgloader.h"
#include "dpluginloader.h"

namespace DigikamBqmConvertToJpegPlugin
{

ConvertToJPEG::ConvertToJPEG(QObject* const parent)
    : BatchTool(QLatin1String("ConvertToJPEG"), ConvertTool, parent)
{
}

BatchTool* ConvertToJPEG::clone(QObject* const parent) const
{
    return new ConvertToJPEG(parent);
}

void ConvertToJPEG::registerSettingsWidget()
{
    DImgLoaderSettings* const JPGBox = DPluginLoader::instance()->exportWidget(QLatin1String("JPEG"));

    connect(JPGBox, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    m_settingsWidget = JPGBox;

    BatchTool::registerSettingsWidget();
}

BatchToolSettings ConvertToJPEG::defaultSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("ImageViewer Settings"));
    int compression           = group.readEntry(QLatin1String("JPEGCompression"), 75);
    int subSampling           = group.readEntry(QLatin1String("JPEGSubSampling"), 1);  // Medium subsampling
    BatchToolSettings settings;
    settings.insert(QLatin1String("quality"),     compression);
    settings.insert(QLatin1String("subsampling"), subSampling);

    return settings;
}

void ConvertToJPEG::slotAssignSettings2Widget()
{
    m_changeSettings = false;

    DImgLoaderSettings* const JPGBox = dynamic_cast<DImgLoaderSettings*>(m_settingsWidget);

    if (JPGBox)
    {
        DImgLoaderPrms set;
        set.insert(QLatin1String("quality"),     settings().value(QLatin1String("quality")).toInt());
        set.insert(QLatin1String("subsampling"), settings().value(QLatin1String("subsampling")).toInt());
        JPGBox->setSettings(set);
    }

    m_changeSettings = true;
}

void ConvertToJPEG::slotSettingsChanged()
{
    if (m_changeSettings)
    {
        const DImgLoaderSettings* const JPGBox = dynamic_cast<DImgLoaderSettings*>(m_settingsWidget);

        if (JPGBox)
        {
            BatchToolSettings settings;
            settings.insert(QLatin1String("quality"),     JPGBox->settings().value(QLatin1String("quality")).toInt());
            settings.insert(QLatin1String("subsampling"), JPGBox->settings().value(QLatin1String("subsampling")).toInt());
            BatchTool::slotSettingsChanged(settings);
        }
    }
}

QString ConvertToJPEG::outputSuffix() const
{
    return QLatin1String("jpg");
}

bool ConvertToJPEG::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    int JPEGCompression = DImgLoader::convertCompressionForLibJpeg(settings().value(QLatin1String("quality")).toInt());
    image().setAttribute(QLatin1String("quality"),     JPEGCompression);
    image().setAttribute(QLatin1String("subsampling"), settings().value(QLatin1String("subsampling")).toInt());

    return (savefromDImg());
}

} // namespace DigikamBqmConvertToJpegPlugin

#include "moc_converttojpeg.cpp"
