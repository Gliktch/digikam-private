/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-05-11
 * Description : setup Survey tab.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupsurvey.h"

// Qt includes

#include <QColor>
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

class Q_DECL_HIDDEN SetupSurvey::Private
{
public:

    Private() = default;

public:

    const QString configGroupName          = QLatin1String("Survey Settings");

    FullScreenSettings* fullScreenSettings = nullptr;
};

// --------------------------------------------------------

SetupSurvey::SetupSurvey(QWidget* const parent)
    : QScrollArea(parent),
      d          (new Private)
{
    const int spacing         = layoutSpacing();

    QWidget* const panel      = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    d->fullScreenSettings     = new FullScreenSettings(FS_SURVEY, panel);

    QVBoxLayout* const layout = new QVBoxLayout(panel);
    layout->addWidget(d->fullScreenSettings);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(spacing);
    layout->addStretch();

    // --------------------------------------------------------

    readSettings();
}

SetupSurvey::~SetupSurvey()
{
    delete d;
}

void SetupSurvey::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->fullScreenSettings->readSettings(group);
}

void SetupSurvey::applySettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    d->fullScreenSettings->saveSettings(group);
    config->sync();
}

} // namespace Digikam

#include "moc_setupsurvey.cpp"
