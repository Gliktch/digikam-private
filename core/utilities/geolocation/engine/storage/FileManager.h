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

#include <QObject>

// Local includes

#include "GeoDataDocument.h"

class QString;

namespace Marble
{

class FileManagerPrivate;
class FileLoader;
class GeoDataLatLonBox;
class GeoDataTreeModel;
class PluginManager;

/**
 * This class is responsible for loading the
 * different files into Geodata model.
 *
 * The loaded data are accessible via
 * various models in MarbleModel.
 */
class DIGIKAM_EXPORT FileManager : public QObject
{
    Q_OBJECT

public:

    /**
     * Creates a new file manager.
     *
     * @param treeModel The GeoDataTreeModel model.
     * @param pluginManager The plugin manager.
     * @param parent The parent object.
     */
    explicit FileManager(GeoDataTreeModel* treeModel, const PluginManager* pluginManager, QObject* parent = nullptr);

    /**
     * Destroys the file manager.
     */
    ~FileManager() override;

    /**
     * Loads a new file into the manager.
     */
    void addFile(const QString& fileName, const QString& property, const GeoDataStyle::Ptr& style, DocumentRole role, int renderOrder = 0, bool recenter = false);


    /**
    * removes an existing file from the manager
    */
    void removeFile(const QString& fileName);

    /**
    * add Data containing KML code as string
    */
    void addData(const QString& name, const QString& data, DocumentRole role);

    void closeFile(const GeoDataDocument* document);

    int size() const;
    GeoDataDocument* at(const QString& key);

    /** Returns the number of files being opened at the moment */
    int pendingFiles() const;

Q_SIGNALS:

    void fileAdded(const QString& key);
    void fileRemoved(const QString& key);
    void centeredDocument(const GeoDataLatLonBox&);
    void fileError(const QString& key, const QString& error);

private:

    Q_PRIVATE_SLOT(d, void cleanupLoader(FileLoader* loader))

    Q_DISABLE_COPY(FileManager)

private:

    friend class FileManagerPrivate;
    FileManagerPrivate* const d = nullptr;
};

} // namespace Marble
