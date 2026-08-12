/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-05-11
 * Description : setup Light Table tab.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setuplighttable.h"

// Qt includes

#include <QCheckBox>
#include <QColor>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "fullscreensettings.h"
#include "dxmlguiwindow.h"

namespace Digikam
{

class Q_DECL_HIDDEN SetupLightTable::Private
{
public:

    Private() = default;

public:

    const QString configGroupName               = QLatin1String("LightTable Settings");
    const QString configAutoSyncPreviewEntry    = QLatin1String("Auto Sync Preview");
    const QString configAutoLoadRightPanelEntry = QLatin1String("Auto Load Right Panel");
    const QString configClearOnCloseEntry       = QLatin1String("Clear On Close");

    QCheckBox*           autoSyncPreview        = nullptr;
    QCheckBox*           autoLoadOnRightPanel   = nullptr;
    QCheckBox*           clearOnClose           = nullptr;

    FullScreenSettings*  fullScreenSettings     = nullptr;
};

// --------------------------------------------------------

SetupLightTable::SetupLightTable(QWidget* const parent)
    : QScrollArea(parent),
      d          (new Private)
{
    const int spacing         = layoutSpacing();

    QWidget* const panel      = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QVBoxLayout* const layout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QGroupBox* const interfaceOptionsGroup = new QGroupBox(i18n("Interface Options"), panel);
    QVBoxLayout* const gLayout             = new QVBoxLayout(interfaceOptionsGroup);

    d->autoSyncPreview      = new QCheckBox(i18n("Synchronize panels automatically"), interfaceOptionsGroup);
    d->autoSyncPreview->setWhatsThis(i18n("Set this option to automatically synchronize "
                                          "zooming and panning between left and right panels if the "
                                          "images have the same size."));

    d->autoLoadOnRightPanel = new QCheckBox(i18n("Selecting a thumbbar item loads the image to the right panel"),
                                            interfaceOptionsGroup);
    d->autoLoadOnRightPanel->setWhatsThis(i18n("Set this option to automatically load an image "
                                               "into the right panel when the corresponding item is selected on the thumbbar."));

    d->clearOnClose = new QCheckBox(i18n("Clear the light table on close"));
    d->clearOnClose->setWhatsThis(i18n("Set this option to remove all images "
                                       "from the light table when you close it, "
                                       "or unset it to preserve the images "
                                       "currently on the light table."));

    gLayout->addWidget(d->autoSyncPreview);
    gLayout->addWidget(d->autoLoadOnRightPanel);
    gLayout->addWidget(d->clearOnClose);
    gLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    gLayout->setSpacing(0);

    // --------------------------------------------------------

    d->fullScreenSettings = new FullScreenSettings(FS_LIGHTTABLE, panel);

    // --------------------------------------------------------

    layout->addWidget(interfaceOptionsGroup);
    layout->addWidget(d->fullScreenSettings);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(spacing);
    layout->addStretch();

    // --------------------------------------------------------

    readSettings();
}

SetupLightTable::~SetupLightTable()
{
    delete d;
}

void SetupLightTable::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->fullScreenSettings->readSettings(group);
    d->autoSyncPreview->setChecked(group.readEntry(d->configAutoSyncPreviewEntry,         true));
    d->autoLoadOnRightPanel->setChecked(group.readEntry(d->configAutoLoadRightPanelEntry, true));
    d->clearOnClose->setChecked(group.readEntry(d->configClearOnCloseEntry,               false));
}

void SetupLightTable::applySettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    d->fullScreenSettings->saveSettings(group);
    group.writeEntry(d->configAutoSyncPreviewEntry,       d->autoSyncPreview->isChecked());
    group.writeEntry(d->configAutoLoadRightPanelEntry,    d->autoLoadOnRightPanel->isChecked());
    group.writeEntry(d->configClearOnCloseEntry,          d->clearOnClose->isChecked());
    config->sync();
}

} // namespace Digikam

#include "moc_setuplighttable.cpp"
