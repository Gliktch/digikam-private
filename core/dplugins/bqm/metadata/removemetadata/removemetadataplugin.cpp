/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to remove metadata
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "removemetadataplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "removemetadata.h"

namespace DigikamBqmRemoveMetadataPlugin
{

RemoveMetadataPlugin::RemoveMetadataPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString RemoveMetadataPlugin::name() const
{
    return i18nc("@title", "Remove Metadata");
}

QString RemoveMetadataPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon RemoveMetadataPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("format-text-code"));
}

QString RemoveMetadataPlugin::description() const
{
    return i18nc("@info", "A tool to remove Exif, Iptc, or Xmp metadata from images");
}

QString RemoveMetadataPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can remove metadata as Exif, Iptc, or Xmp from images.</para>");
}

QString RemoveMetadataPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString RemoveMetadataPlugin::handbookChapter() const
{
    return QLatin1String("metadata_tools");
}

QString RemoveMetadataPlugin::handbookReference() const
{
    return QLatin1String("bqm-removemetadata");
}

QList<DPluginAuthor> RemoveMetadataPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2010-2025"))
            ;
}

void RemoveMetadataPlugin::setup(QObject* const parent)
{
    RemoveMetadata* const tool = new RemoveMetadata(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmRemoveMetadataPlugin

#include "moc_removemetadataplugin.cpp"
