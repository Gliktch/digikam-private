/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-07-18
 * Description : DNG Raw Converter batch tool.
 *
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "converttodng.h"

// Qt includes

#include <QFileInfo>
#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dinfointerface.h"
#include "dpluginbqm.h"

namespace DigikamBqmConvertToDngPlugin
{

ConvertToDNG::ConvertToDNG(QObject* const parent)
    : BatchTool(QLatin1String("ConvertToDNG"), ConvertTool, parent)
{
}

BatchTool* ConvertToDNG::clone(QObject* const parent) const
{
    return new ConvertToDNG(parent);
}

void ConvertToDNG::registerSettingsWidget()
{
    DNGSettings* const DNGBox = new DNGSettings;

    connect(DNGBox, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(DNGBox, SIGNAL(signalSetupExifTool()),
            this, SLOT(slotSetupExifTool()));

    m_settingsWidget = DNGBox;

    BatchTool::registerSettingsWidget();
}

BatchToolSettings ConvertToDNG::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert(QLatin1String("CompressLossLess"),      true);
    settings.insert(QLatin1String("PreviewMode"),           DNGWriter::FULL_SIZE);
    settings.insert(QLatin1String("BackupOriginalRawFile"), false);

    return settings;
}

void ConvertToDNG::slotAssignSettings2Widget()
{
    m_changeSettings          = false;
    DNGSettings* const DNGBox = dynamic_cast<DNGSettings*>(m_settingsWidget);

    if (DNGBox)
    {
        DNGBox->setCompressLossLess(settings().value(QLatin1String("CompressLossLess")).toBool());
        DNGBox->setPreviewMode(settings().value(QLatin1String("PreviewMode")).toInt());
        DNGBox->setBackupOriginalRawFile(settings().value(QLatin1String("BackupOriginalRawFile")).toBool());
    }

    m_changeSettings          = true;
}

void ConvertToDNG::slotSettingsChanged()
{
    if (m_changeSettings)
    {
        BatchToolSettings settings;
        const DNGSettings* const DNGBox = dynamic_cast<DNGSettings*>(m_settingsWidget);

        if (DNGBox)
        {
            settings.insert(QLatin1String("CompressLossLess"),      DNGBox->compressLossLess());
            settings.insert(QLatin1String("PreviewMode"),           DNGBox->previewMode());
            settings.insert(QLatin1String("BackupOriginalRawFile"), DNGBox->backupOriginalRawFile());
            BatchTool::slotSettingsChanged(settings);
        }
    }
}

void ConvertToDNG::slotSetupExifTool()
{
    DInfoInterface* const iface = plugin()->infoIface();

    if (iface)
    {
        DNGSettings* const DNGBox = dynamic_cast<DNGSettings*>(m_settingsWidget);

        if (DNGBox)
        {
            connect(iface, SIGNAL(signalSetupChanged()),
                    DNGBox, SLOT(slotSetupChanged()));
        }

        iface->openSetupPage(DInfoInterface::ExifToolPage);
    }
}

QString ConvertToDNG::outputSuffix() const
{
    return QLatin1String("dng");
}

void ConvertToDNG::cancel()
{
    m_dngProcessor.cancel();
    BatchTool::cancel();
}

bool ConvertToDNG::toolOperations()
{
    if (!isRawFile(inputUrl()))
    {
        return false;
    }

    m_dngProcessor.reset();
    m_dngProcessor.setInputFile(inputUrl().toLocalFile());
    m_dngProcessor.setOutputFile(outputUrl().toLocalFile());
    m_dngProcessor.setBackupOriginalRawFile(settings().value(QLatin1String("BackupOriginalRawFile")).toBool());
    m_dngProcessor.setCompressLossLess(settings().value(QLatin1String("CompressLossLess")).toBool());
    m_dngProcessor.setPreviewMode(settings().value(QLatin1String("PreviewMode")).toInt());

    int ret = m_dngProcessor.convert();

    return (ret == DNGWriter::PROCESS_COMPLETE);
}

} // namespace DigikamBqmConvertToDngPlugin

#include "moc_converttodng.cpp"
