/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-03-09
 * Description : a BQM plugin to automatically rotate images.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2025      by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "autorotateplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "autorotate.h"

namespace DigikamBqmAutoRotatePlugin
{

AutoRotatePlugin::AutoRotatePlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString AutoRotatePlugin::name() const
{
    return i18nc("@title", "Auto-Rotate");
}

QString AutoRotatePlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon AutoRotatePlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-rotate-right-symbolic"));
}

QString AutoRotatePlugin::description() const
{
    return i18nc("@info", "A tool to automatically rotate images");
}

QString AutoRotatePlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can automatically detect the correct "
                           "orientation of an image and rotate the image.</para>");
}

QString AutoRotatePlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString AutoRotatePlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString AutoRotatePlugin::handbookReference() const
{
    return QLatin1String("bqm-transformtools");
}

QList<DPluginAuthor> AutoRotatePlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Michael Miller"),
                             QString::fromUtf8("michael dot miller at msn dot com"),
                             QString::fromUtf8("(C) 2025"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2026"));
}

void AutoRotatePlugin::setup(QObject* const parent)
{
    AutoRotate* const tool = new AutoRotate(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmAutoRotatePlugin

#include "moc_autorotateplugin.cpp"
