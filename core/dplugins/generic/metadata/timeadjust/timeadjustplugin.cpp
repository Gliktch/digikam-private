/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to adjust items date-time.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "timeadjustplugin.h"

// Qt includes

#include <QPointer>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "timeadjustdialog.h"

namespace DigikamGenericTimeAdjustPlugin
{

TimeAdjustPlugin::TimeAdjustPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

void TimeAdjustPlugin::cleanUp()
{
    delete m_toolDlg;
}

QString TimeAdjustPlugin::name() const
{
    return i18nc("@title", "Time Adjust");
}

QString TimeAdjustPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon TimeAdjustPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("appointment-new"));
}

QString TimeAdjustPlugin::description() const
{
    return i18nc("@info", "A tool to adjust items date-time");
}

QString TimeAdjustPlugin::details() const
{
    return i18nc("@info", "This tool allows users to adjust date time-stamp of items in batch.<br><br>"
                 "Many source of original time-stamp can be selected from original items, or a from a common file.<br><br>"
                 "Many metadata time-stamp can be adjusted or left untouched. The adjustment can be an offset of time or a specific date.");
}

QString TimeAdjustPlugin::handbookSection() const
{
    return QLatin1String("post_processing");
}

QString TimeAdjustPlugin::handbookChapter() const
{
    return QLatin1String("time_adjust");
}

QList<DPluginAuthor> TimeAdjustPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Jesper K. Pedersen"),
                             QString::fromUtf8("blackie at kde dot org"),
                             QString::fromUtf8("(C) 2003-2005"))
            << DPluginAuthor(QString::fromUtf8("Smit Mehta"),
                             QString::fromUtf8("smit dot meh at gmail dot com"),
                             QString::fromUtf8("(C) 2012"))
            << DPluginAuthor(QString::fromUtf8("Pieter Edelman"),
                             QString::fromUtf8("p dot edelman at gmx dot net"),
                             QString::fromUtf8("(C) 2008"))
            << DPluginAuthor(QString::fromUtf8("Maik Qualmann"),
                             QString::fromUtf8("metzpinguin at gmail dot com"),
                             QString::fromUtf8("(C) 2018-2025"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2006-2025"))
            ;
}

void TimeAdjustPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Adjust Time && Date..."));
    ac->setObjectName(QLatin1String("timeadjust_edit"));
    ac->setActionCategory(DPluginAction::GenericMetadata);
    ac->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_D);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotTimeAdjust()));

    addAction(ac);
}

void TimeAdjustPlugin::slotTimeAdjust()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new TimeAdjustDialog(nullptr, infoIface(sender()));
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace DigikamGenericTimeAdjustPlugin

#include "moc_timeadjustplugin.cpp"
