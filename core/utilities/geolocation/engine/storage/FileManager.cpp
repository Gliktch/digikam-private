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

#include "FileManager.h"

// Qt includes

#include <QFileInfo>
#include <QElapsedTimer>

// Local includes

#include "FileLoader.h"
#include "MarbleModel.h"
#include "GeoDataTreeModel.h"
#include "GeoDataLatLonAltBox.h"
#include "GeoDataStyle.h"
#include "digikam_debug.h"

namespace Marble
{

class Q_DECL_HIDDEN FileManagerPrivate
{
public:

    FileManagerPrivate(GeoDataTreeModel* treeModel,
                       const PluginManager* pluginManager,
                       FileManager* parent)
        : q(parent),
          m_treeModel(treeModel),
          m_pluginManager(pluginManager)
    {
    }

    ~FileManagerPrivate()
    {
        for (FileLoader* loader : m_loaderList)
        {
            if (loader)
            {
                loader->wait();
            }
        }
    }

    void appendLoader(FileLoader* loader);
    void closeFile(const QString& key);
    void cleanupLoader(FileLoader* loader);

public:

    FileManager* const                  q               = nullptr;
    GeoDataTreeModel* const             m_treeModel     = nullptr;
    const PluginManager* const          m_pluginManager = nullptr;

    QList<FileLoader*>                  m_loaderList;
    QHash < QString, GeoDataDocument* > m_fileItemHash;
    GeoDataLatLonBox                    m_latLonBox;
    QElapsedTimer                       m_timer;
};


FileManager::FileManager(GeoDataTreeModel* treeModel, const PluginManager* pluginManager, QObject* parent)
    : QObject(parent)
    , d(new FileManagerPrivate(treeModel, pluginManager, this))
{
}

FileManager::~FileManager()
{
    delete d;
}

void FileManager::addFile(const QString& filepath, const QString& property, const GeoDataStyle::Ptr& style, DocumentRole role, int renderOrder, bool recenter)
{
    if (d->m_fileItemHash.contains(filepath))
    {
        return;  // already loaded
    }

    for (const FileLoader* loader : d->m_loaderList)
    {
        if (loader->path() == filepath)
        {
            return;    // currently loading
        }
    }

    qCDebug(DIGIKAM_GEOENGINE_LOG) << "adding container:" << filepath;
    qCDebug(DIGIKAM_GEOENGINE_LOG) << "Starting placemark loading timer";
    d->m_timer.start();
    FileLoader* loader = new FileLoader(this, d->m_pluginManager, recenter, filepath, property, style, role, renderOrder);
    d->appendLoader(loader);
}

void FileManager::addData(const QString& name, const QString& data, DocumentRole role)
{
    FileLoader* loader = new FileLoader(this, d->m_pluginManager, data, name, role);
    d->appendLoader(loader);
}

void FileManagerPrivate::appendLoader(FileLoader* loader)
{
    QObject::connect(loader, SIGNAL(loaderFinished(FileLoader*)),
                     q, SLOT(cleanupLoader(FileLoader*)));

    m_loaderList.append(loader);
    loader->start();
}

void FileManager::removeFile(const QString& key)
{
    for (FileLoader* loader : d->m_loaderList)
    {
        if (loader->path() == key)
        {
            disconnect(loader, nullptr, this, nullptr);
            loader->wait();
            d->m_loaderList.removeAll(loader);
            delete loader->document();
            return;
        }
    }

    if (d->m_fileItemHash.contains(key))
    {
        d->closeFile(key);
    }

    qCDebug(DIGIKAM_GEOENGINE_LOG) << "could not identify " << key;
}

void FileManagerPrivate::closeFile(const QString& key)
{
    qCDebug(DIGIKAM_GEOENGINE_LOG) << "FileManager::closeFile " << key;

    if (m_fileItemHash.contains(key))
    {
        GeoDataDocument* doc = m_fileItemHash.value(key);
        m_treeModel->removeDocument(doc);
        Q_EMIT q->fileRemoved(key);
        delete doc;
        m_fileItemHash.remove(key);
    }
}

void FileManager::closeFile(const GeoDataDocument* document)
{
    QHash < QString, GeoDataDocument* >::iterator itpoint = d->m_fileItemHash.begin();
    QHash < QString, GeoDataDocument* >::iterator const endpoint = d->m_fileItemHash.end();

    for (; itpoint != endpoint; ++itpoint)
    {
        if (d->m_fileItemHash.value(itpoint.key()) == document)
        {
            d->closeFile(itpoint.key());
            return;
        }
    }
}

int FileManager::size() const
{
    return d->m_fileItemHash.size();
}

GeoDataDocument* FileManager::at(const QString& key)
{
    if (d->m_fileItemHash.contains(key))
    {
        return d->m_fileItemHash.value(key);
    }

    return nullptr;
}

int FileManager::pendingFiles() const
{
    return d->m_loaderList.size();
}

void FileManagerPrivate::cleanupLoader(FileLoader* loader)
{
    GeoDataDocument* doc = loader->document();
    m_loaderList.removeAll(loader);

    if (loader->isFinished())
    {
        if (doc)
        {
            if (doc->name().isEmpty() && !doc->fileName().isEmpty())
            {
                QFileInfo file(doc->fileName());
                doc->setName(file.baseName());
            }

            m_treeModel->addDocument(doc);
            m_fileItemHash.insert(loader->path(), doc);
            Q_EMIT q->fileAdded(loader->path());

            if (loader->recenter())
            {
                m_latLonBox |= doc->latLonAltBox();
            }
        }

        if (!loader->error().isEmpty())
        {
            qCWarning(DIGIKAM_GEOENGINE_LOG) << "Failed to parse" << loader->path() << loader->error();
            Q_EMIT q->fileError(loader->path(), loader->error());
        }

        delete loader;
    }

    if (m_loaderList.isEmpty())
    {
        qCDebug(DIGIKAM_GEOENGINE_LOG) << "Finished loading all placemarks " << m_timer.elapsed();

        if (!m_latLonBox.isEmpty())
        {
            Q_EMIT q->centeredDocument(m_latLonBox);
        }

        m_latLonBox.clear();
    }
}

} // namespace Marble

#include "moc_FileManager.cpp"
