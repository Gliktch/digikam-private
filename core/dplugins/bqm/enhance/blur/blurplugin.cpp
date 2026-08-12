/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to blur images
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "blurplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "blur.h"

namespace DigikamBqmBlurPlugin
{

BlurPlugin::BlurPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString BlurPlugin::name() const
{
    return i18nc("@title", "Blur Image");
}

QString BlurPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon BlurPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("blurimage"));
}

QString BlurPlugin::description() const
{
    return i18nc("@info", "A tool to blur images");
}

QString BlurPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can blur images.</para>");
}

QString BlurPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString BlurPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString BlurPlugin::handbookReference() const
{
    return QLatin1String("bqm-enhancetools");
}

QList<DPluginAuthor> BlurPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2009-2024"))
            ;
}

void BlurPlugin::setup(QObject* const parent)
{
    Blur* const tool = new Blur(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmBlurPlugin

#include "moc_blurplugin.cpp"
