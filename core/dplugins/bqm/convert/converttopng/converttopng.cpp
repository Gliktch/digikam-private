/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-28
 * Description : PNG image Converter batch tool.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "converttopng.h"

// Qt includes

#include <QFileInfo>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "dimgloader.h"
#include "dpluginloader.h"

namespace DigikamBqmConvertToPngPlugin
{

ConvertToPNG::ConvertToPNG(QObject* const parent)
    : BatchTool(QLatin1String("ConvertToPNG"), ConvertTool, parent)
{
}

BatchTool* ConvertToPNG::clone(QObject* const parent) const
{
    return new ConvertToPNG(parent);
}

void ConvertToPNG::registerSettingsWidget()
{
    QWidget* const box               = new QWidget();
    QVBoxLayout* const vlay          = new QVBoxLayout(box);
    DImgLoaderSettings* const PNGBox = DPluginLoader::instance()->exportWidget(QLatin1String("PNG"));
    QLabel* const note               = new QLabel(i18n("<b>If conversion to PNG fails, this may be due to the "
                                                       "color profile check. Simply insert the tool for "
                                                       "color profile conversion before this tool and "
                                                       "select the desired color profile.</b>"));
    note->setWordWrap(true);
    note->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    vlay->addWidget(PNGBox);
    vlay->addWidget(note);
    vlay->addStretch(10);

    connect(PNGBox, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    m_settingsWidget = box;

    BatchTool::registerSettingsWidget();
}

BatchToolSettings ConvertToPNG::defaultSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("ImageViewer Settings"));
    int compression           = group.readEntry(QLatin1String("PNGCompression"), 9);
    BatchToolSettings settings;
    settings.insert(QLatin1String("quality"), compression);

    return settings;
}

void ConvertToPNG::slotAssignSettings2Widget()
{
    m_changeSettings = false;

    DImgLoaderSettings* const PNGBox = dynamic_cast<DImgLoaderSettings*>(m_settingsWidget);

    if (PNGBox)
    {
        DImgLoaderPrms set;
        set.insert(QLatin1String("quality"),  settings().value(QLatin1String("quality")).toInt());
        PNGBox->setSettings(set);
    }

    m_changeSettings = true;
}

void ConvertToPNG::slotSettingsChanged()
{
    if (m_changeSettings)
    {
        const DImgLoaderSettings* const PNGBox = dynamic_cast<DImgLoaderSettings*>(m_settingsWidget);

        if (PNGBox)
        {
            BatchToolSettings settings;
            settings.insert(QLatin1String("quality"),  PNGBox->settings().value(QLatin1String("quality")).toInt());
            BatchTool::slotSettingsChanged(settings);
        }
    }
}

QString ConvertToPNG::outputSuffix() const
{
    return QLatin1String("png");
}

bool ConvertToPNG::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    int PNGCompression = DImgLoader::convertCompressionForLibPng(settings().value(QLatin1String("quality")).toInt());
    image().setAttribute(QLatin1String("quality"), PNGCompression);

    return (savefromDImg());
}

} // namespace DigikamBqmConvertToPngPlugin

#include "moc_converttopng.cpp"
