/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : abstract class to define digiKam plugin
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dplugin.h"

// Local includes

#include "digikam_version.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN DPlugin::Private
{
public:

    Private() = default;

    bool    shouldLoaded = false;
    QString libraryFileName;
};

DPlugin::DPlugin(QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
}

DPlugin::~DPlugin()
{
    delete d;
}

QString DPlugin::libraryFileName() const
{
    return d->libraryFileName;
}

void DPlugin::setLibraryFileName(const QString& name)
{
    d->libraryFileName = name;
}

QString DPlugin::version() const
{
    return QLatin1String(digikam_version_short);
}

QIcon DPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("plugins"));
}

QStringList DPlugin::pluginAuthors() const
{
    QStringList list;
    const auto auths = authors();

    for (const DPluginAuthor& au : auths)
    {
        if (!list.contains(au.name))
        {
            list << au.name;
        }
    }

    list.sort();

    return list;
}

bool DPlugin::shouldLoaded() const
{
    return d->shouldLoaded;
}

void DPlugin::setShouldLoaded(bool b)
{
    d->shouldLoaded = b;
}

bool DPlugin::hasVisibilityProperty() const
{
    return true;
}

QString DPlugin::handbookSection() const
{
    return QString();
}

QString DPlugin::handbookChapter() const
{
    return QString();
}

QString DPlugin::handbookReference() const
{
    return QString();
}

} // namespace Digikam

#include "moc_dplugin.cpp"
