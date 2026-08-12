/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to resize images.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "resizeplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "resize.h"

namespace DigikamBqmResizePlugin
{

ResizePlugin::ResizePlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString ResizePlugin::name() const
{
    return i18nc("@title", "Resize");
}

QString ResizePlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ResizePlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("transform-scale"));
}

QString ResizePlugin::description() const
{
    return i18nc("@info", "A tool to resize images with a customized length");
}

QString ResizePlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can resize images.</para>");
}

QString ResizePlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString ResizePlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString ResizePlugin::handbookReference() const
{
    return QLatin1String("bqm-transformtools");
}

QList<DPluginAuthor> ResizePlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Sambhav Dusad"),
                             QString::fromUtf8("sambhavdusad24 at gmail dot com"),
                             QString::fromUtf8("(C) 2019"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2024"))
            ;
}

void ResizePlugin::setup(QObject* const parent)
{
    Resize* const tool = new Resize(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmResizePlugin

#include "moc_resizeplugin.cpp"
