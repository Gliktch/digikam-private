/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to rotate images.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "rotateplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "rotate.h"

namespace DigikamBqmRotatePlugin
{

RotatePlugin::RotatePlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString RotatePlugin::name() const
{
    return i18nc("@title", "Rotate");
}

QString RotatePlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon RotatePlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("object-rotate-right"));
}

QString RotatePlugin::description() const
{
    return i18nc("@info", "A tool to rotate images");
}

QString RotatePlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can rotate images.</para>");
}

QString RotatePlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString RotatePlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString RotatePlugin::handbookReference() const
{
    return QLatin1String("bqm-transformtools");
}

QList<DPluginAuthor> RotatePlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2025"))
            ;
}

void RotatePlugin::setup(QObject* const parent)
{
    Rotate* const tool = new Rotate(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmRotatePlugin

#include "moc_rotateplugin.cpp"
