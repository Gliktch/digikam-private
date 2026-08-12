/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to crop images.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "cropplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "crop.h"

namespace DigikamBqmCropPlugin
{

CropPlugin::CropPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString CropPlugin::name() const
{
    return i18nc("@title", "Crop");
}

QString CropPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon CropPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("transform-crop"));
}

QString CropPlugin::description() const
{
    return i18nc("@info", "A tool to crop images to a region");
}

QString CropPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can crop images to a region.</para>");
}

QString CropPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString CropPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString CropPlugin::handbookReference() const
{
    return QLatin1String("bqm-transformtools");
}

QList<DPluginAuthor> CropPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2012-2024"))
            ;
}

void CropPlugin::setup(QObject* const parent)
{
    Crop* const tool = new Crop(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmCropPlugin

#include "moc_cropplugin.cpp"
