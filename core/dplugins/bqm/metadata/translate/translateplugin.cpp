/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-08-30
 * Description : a BQM plugin to translate metadata
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "translateplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "translate.h"

namespace DigikamBqmTranslatePlugin
{

TranslatePlugin::TranslatePlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString TranslatePlugin::name() const
{
    return i18nc("@title", "Translate Metadata");
}

QString TranslatePlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon TranslatePlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("language-chooser"));
}

QString TranslatePlugin::description() const
{
    return i18nc("@info", "A tool to translate strings from image metadata");
}

QString TranslatePlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can translate alternative language strings in Xmp metadata from images.</para>");
}

QString TranslatePlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString TranslatePlugin::handbookChapter() const
{
    return QLatin1String("metadata_tools");
}

QString TranslatePlugin::handbookReference() const
{
    return QLatin1String("bqm-translatemetadata");
}

QList<DPluginAuthor> TranslatePlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2021-2025"))
            ;
}

void TranslatePlugin::setup(QObject* const parent)
{
    Translate* const tool = new Translate(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmTranslatePlugin

#include "moc_translateplugin.cpp"
