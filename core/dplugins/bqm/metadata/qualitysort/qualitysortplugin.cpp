/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-09-22
 * Description : a BQM plugin to assign pick label metadata by image quality
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "qualitysortplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "qualitysort.h"

namespace DigikamBqmQualitySortPlugin
{

QualitySortPlugin::QualitySortPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString QualitySortPlugin::name() const
{
    return i18nc("@title", "Image Quality Scan");
}

QString QualitySortPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon QualitySortPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("flag-green"));
}

QString QualitySortPlugin::description() const
{
    return i18nc("@info", "A tool to assign pick label metadata by image quality");
}

QString QualitySortPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool will assign a pick label metadata value "
                  "by parsing image and estimate the quality.</para>");
}

QString QualitySortPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString QualitySortPlugin::handbookChapter() const
{
    return QLatin1String("metadata_tools");
}

QString QualitySortPlugin::handbookReference() const
{
    return QLatin1String("bqm-qualitysort");
}

QList<DPluginAuthor> QualitySortPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2021-2025"))
            ;
}

void QualitySortPlugin::setup(QObject* const parent)
{
    QualitySort* const tool = new QualitySort(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmQualitySortPlugin

#include "moc_qualitysortplugin.cpp"
