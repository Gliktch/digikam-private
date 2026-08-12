/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-11-07
 * Description : Directory watch interface
 *
 * SPDX-FileCopyrightText: 2011      by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QString>
#include <QUrl>

namespace Digikam
{

class Album;
class PAlbum;
class AlbumManager;
class DbEngineParameters;

class AlbumWatch : public QObject
{
    Q_OBJECT

public:

    explicit AlbumWatch(AlbumManager* const parent = nullptr);
    ~AlbumWatch() override;

    void clear();
    void removeWatchedPAlbums(const PAlbum* const album);
    void setDbEngineParameters(const DbEngineParameters& params);

protected Q_SLOTS:

    void slotAlbumAdded(Album* album);
    void slotAlbumAboutToBeDeleted(Album* album);
    void slotQFSWatcherDirty(const QString& path);

private:

    void rescanDirectory(const QString& dir);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
