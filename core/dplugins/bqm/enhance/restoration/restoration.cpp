/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-19
 * Description : Restoration batch tool.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "restoration.h"

// Qt includes

#include <QLabel>
#include <QWidget>
#include <QComboBox>
#include <QStandardPaths>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dimg.h"
#include "dactivelabel.h"

namespace DigikamBqmRestorationPlugin
{

Restoration::Restoration(QObject* const parent)
    : BatchTool(QLatin1String("Restoration"), EnhanceTool, parent)
{
}

BatchTool* Restoration::clone(QObject* const parent) const
{
    return new Restoration(parent);
}

void Restoration::registerSettingsWidget()
{
    DVBox* const vbox                 = new DVBox;

    DActiveLabel* const cimgLogoLabel = new DActiveLabel(QUrl(QLatin1String("http://cimg.sourceforge.net")),    // krazy:exclude=insecurenet
                                                         QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/logo-cimg.webp")),
                                                         vbox);
    cimgLogoLabel->setToolTip(i18n("Visit CImg library website"));

    new QLabel(i18n("Filter:"), vbox);
    m_comboBox = new QComboBox(vbox);
    m_comboBox->insertItem(ReduceUniformNoise,  i18n("Reduce Uniform Noise"));
    m_comboBox->insertItem(ReduceJPEGArtefacts, i18n("Reduce JPEG Artifacts"));
    m_comboBox->insertItem(ReduceTexturing,     i18n("Reduce Texturing"));
    m_comboBox->setWhatsThis(i18n("<p>Select the filter preset to use for photograph restoration here:</p>"
                                  "<p><b>None</b>: Most common values. Puts settings to default.<br>"
                                  "<b>Reduce Uniform Noise</b>: reduce small image artifacts such as sensor noise.<br>"
                                  "<b>Reduce JPEG Artifacts</b>: reduce large image artifacts, such as a JPEG compression mosaic.<br>"
                                  "<b>Reduce Texturing</b>: reduce image artifacts, such as paper texture, or Moire patterns "
                                  "on scanned images.</p>"));

    QLabel* const space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    m_settingsWidget    = vbox;

    connect(m_comboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings Restoration::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert(QLatin1String("RestorationMethod"), ReduceUniformNoise);

    return settings;
}

void Restoration::slotAssignSettings2Widget()
{
    m_comboBox->setCurrentIndex(settings().value(QLatin1String("RestorationMethod")).toInt());
}

void Restoration::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert(QLatin1String("RestorationMethod"), (int)m_comboBox->currentIndex());
    BatchTool::slotSettingsChanged(settings);
}

bool Restoration::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    int type = settings().value(QLatin1String("RestorationMethod")).toInt();

    GreycstorationContainer settings;
    settings.setRestorationDefaultSettings();

    switch (type)
    {
        case ReduceUniformNoise:
        {
            settings.amplitude = 40.0;
            break;
        }

        case ReduceJPEGArtefacts:
        {
            settings.sharpness = 0.3F;
            settings.sigma     = 1.0;
            settings.amplitude = 100.0;
            settings.nbIter    = 2;
            break;
        }

        case ReduceTexturing:
        {
            settings.sharpness = 0.5F;
            settings.sigma     = 1.5;
            settings.amplitude = 100.0;
            settings.nbIter    = 2;
            break;
        }
    }

    m_cimgIface = new GreycstorationFilter(this);
    m_cimgIface->setMode(GreycstorationFilter::Restore);
    m_cimgIface->setOriginalImage(image());
    m_cimgIface->setSettings(settings);
    m_cimgIface->setup();

    applyFilter(m_cimgIface);

    delete m_cimgIface;
    m_cimgIface = nullptr;

    return (savefromDImg());
}

void Restoration::cancel()
{
    if (m_cimgIface)
    {
        m_cimgIface->cancelFilter();
    }

    BatchTool::cancel();
}

} // namespace DigikamBqmRestorationPlugin

#include "moc_restoration.cpp"
