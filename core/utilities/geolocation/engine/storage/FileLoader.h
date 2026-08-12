/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QThread>

// Local includes

#include "GeoDataDocument.h"

class QString;

namespace Marble
{

class FileLoaderPrivate;
class PluginManager;
class GeoDataStyle;

class FileLoader : public QThread
{
    Q_OBJECT

public:

    FileLoader(QObject* parent,
               const PluginManager* pluginManager,
               bool recenter,
               const QString& file,
               const QString& property,
               const GeoDataStyle::Ptr& style,
               DocumentRole role,
               int renderOrder);
    FileLoader(QObject* parent,
               const PluginManager* pluginManager,
               const QString& contents,
               const QString& name,
               DocumentRole role);
    ~FileLoader()                         override;

    void run()                            override;
    bool recenter()                 const;
    QString path()                  const;
    GeoDataDocument* document();
    QString error()                 const;

Q_SIGNALS:

    void loaderFinished(FileLoader*);
    void newGeoDataDocumentAdded(GeoDataDocument*);

private:

    Q_PRIVATE_SLOT(d, void documentParsed(GeoDataDocument*, QString))

private:

    friend class FileLoaderPrivate;

    FileLoaderPrivate* d = nullptr;
};

} // namespace Marble
