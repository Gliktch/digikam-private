/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-04-12
 * Description : A tool to export items to Rajce web service
 *
 * SPDX-FileCopyrightText: 2011      by Lukas Krejci <krejci.l at centrum dot cz>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "rajcewindow.h"

// Qt includes

#include <QAction>
#include <QMenu>
#include <QCloseEvent>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "rajcewidget.h"

namespace DigikamGenericRajcePlugin
{

RajceWindow::RajceWindow(DInfoInterface* const iface, QWidget* const /*parent*/)
    : WSToolDialog(nullptr, QLatin1String("RajceExport Dialog"))
{
    m_widget = new RajceWidget(iface, this);
    m_widget->readSettings();

    setMainWidget(m_widget);
    setModal(false);
    setWindowTitle(i18nc("@title:window", "Export to Rajce.net"));

    startButton()->setText(i18nc("@action:button", "Start Upload"));
    startButton()->setToolTip(i18nc("@info:tooltip, button", "Start upload to Rajce.net"));

    m_widget->setMinimumSize(700, 500);

    connect(startButton(), SIGNAL(clicked()),
            m_widget, SLOT(slotStartUpload()));

    connect(this, SIGNAL(finished(int)),
            this, SLOT(slotFinished()));

    connect(m_widget, SIGNAL(signalLoginStatusChanged(bool)),
            this, SLOT(slotSetUploadButtonEnabled(bool)));

    startButton()->setEnabled(false);
}

RajceWindow::~RajceWindow()
{
}

void RajceWindow::reactivate()
{
    m_widget->reactivate();
    show();
}

void RajceWindow::slotSetUploadButtonEnabled(bool enabled)
{
    startButton()->setEnabled(enabled);
}

void RajceWindow::slotFinished()
{
    m_widget->cancelUpload();
    m_widget->writeSettings();
}

void RajceWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

} // namespace DigikamGenericRajcePlugin

#include "moc_rajcewindow.cpp"
