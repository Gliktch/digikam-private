/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-02-11
 * Description : a BQM plugin to apply metadata with ExifTool
 *
 * SPDX-FileCopyrightText: 2024 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "applymetadataplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "applymetadata.h"

namespace DigikamBqmApplyMetadataPlugin
{

ApplyMetadataPlugin::ApplyMetadataPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString ApplyMetadataPlugin::name() const
{
    return i18nc("@title", "Apply Metadata");
}

QString ApplyMetadataPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ApplyMetadataPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("format-text-code"));
}

QString ApplyMetadataPlugin::description() const
{
    return i18nc("@info", "A tool to apply metadata from a image or JSON file with ExifTool");
}

QString ApplyMetadataPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can apply metadata "
                           "from a image or JSON file with ExifTool.</para>");
}

QString ApplyMetadataPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString ApplyMetadataPlugin::handbookChapter() const
{
    return QLatin1String("metadata_tools");
}

QString ApplyMetadataPlugin::handbookReference() const
{
    return QLatin1String("");
}

QList<DPluginAuthor> ApplyMetadataPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Maik Qualmann"),
                             QString::fromUtf8("metzpinguin at gmail dot com"),
                             QString::fromUtf8("(C) 2024-2025"))
            ;
}

void ApplyMetadataPlugin::setup(QObject* const parent)
{
    ApplyMetadata* const tool = new ApplyMetadata(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmApplyMetadataPlugin

#include "moc_applymetadataplugin.cpp"
