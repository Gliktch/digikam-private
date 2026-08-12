/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to flip images.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "flipplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "flip.h"

namespace DigikamBqmFlipPlugin
{

FlipPlugin::FlipPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString FlipPlugin::name() const
{
    return i18nc("@title", "Flip");
}

QString FlipPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon FlipPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("object-flip-vertical"));
}

QString FlipPlugin::description() const
{
    return i18nc("@info", "A tool to flip images horizontally or vertically");
}

QString FlipPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can run user shell script as workflow stage.</para>");
}

QString FlipPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString FlipPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString FlipPlugin::handbookReference() const
{
    return QLatin1String("bqm-transformtools");
}

QList<DPluginAuthor> FlipPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2024"))
            ;
}

void FlipPlugin::setup(QObject* const parent)
{
    Flip* const tool = new Flip(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmFlipPlugin

#include "moc_flipplugin.cpp"
