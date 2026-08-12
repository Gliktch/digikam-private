/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to reduce lens artifacts
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "lensautofixplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "lensautofix.h"

namespace DigikamBqmLensAutoFixPlugin
{

LensAutoFixPlugin::LensAutoFixPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString LensAutoFixPlugin::name() const
{
    return i18nc("@title", "Lens Auto-Correction");
}

QString LensAutoFixPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon LensAutoFixPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("lensautofix"));
}

QString LensAutoFixPlugin::description() const
{
    return i18nc("@info", "A tool to fix automatically lens artifacts");
}

QString LensAutoFixPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can fix automatically lens artifacts over images.</para>");
}

QString LensAutoFixPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString LensAutoFixPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString LensAutoFixPlugin::handbookReference() const
{
    return QLatin1String("bqm-enhancetools");
}

QList<DPluginAuthor> LensAutoFixPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2024"))
            ;
}

void LensAutoFixPlugin::setup(QObject* const parent)
{
    LensAutoFix* const tool = new LensAutoFix(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmLensAutoFixPlugin

#include "moc_lensautofixplugin.cpp"
