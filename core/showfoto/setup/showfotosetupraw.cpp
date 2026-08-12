/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : setup RAW decoding settings.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotosetupraw.h"

// Qt includes

#include <QTabWidget>

// Local includes

#include "setupraw.h"

using namespace Digikam;

namespace ShowFoto
{

class Q_DECL_HIDDEN ShowfotoSetupRaw::Private
{
public:

    Private() = default;

    QTabWidget* tab = nullptr;
    SetupRaw*   raw = nullptr;
};

ShowfotoSetupRaw::ShowfotoSetupRaw(QWidget* const parent)
    : QScrollArea(parent),
      d          (new Private)
{
    d->tab = new QTabWidget;
    d->raw = new SetupRaw(d->tab);

    setWidget(d->tab);
    setWidgetResizable(true);

    readSettings();
}

ShowfotoSetupRaw::~ShowfotoSetupRaw()
{
    delete d;
}

void ShowfotoSetupRaw::setActiveTab(RAWTab tab)
{
    d->tab->setCurrentIndex(tab);
}

ShowfotoSetupRaw::RAWTab ShowfotoSetupRaw::activeTab() const
{
    return (RAWTab)d->tab->currentIndex();
}

void ShowfotoSetupRaw::applySettings()
{
    d->raw->applySettings();
}

void ShowfotoSetupRaw::readSettings()
{
    d->raw->readSettings();
}

} // namespace ShowFoto

#include "moc_showfotosetupraw.cpp"
