/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-05-05
 * Description : file action manager
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "iteminfo.h"
#include "metaengine_rotation.h"
#include "digikam_export.h"

namespace Digikam
{

class MetadataHub;
class MetadataHubOnTheRoad;
class DisjointMetadata;

class DIGIKAM_GUI_EXPORT FileActionMngr : public QObject
{
    Q_OBJECT

public:

    enum GroupAction
    {
        AddToGroup,
        RemoveFromGroup,
        SplitGroup
    };

public:

    FileActionMngr();
    ~FileActionMngr() override;

    static FileActionMngr* instance();

    bool requestShutDown();
    void shutDown();
    bool isActive();

Q_SIGNALS:

    void signalImageChangeFailed(const QString& message, const QStringList& fileNames);

public Q_SLOTS:

    void assignTag(const ItemInfo& info, int tagID);
    void assignTag(const QList<ItemInfo>& infos, int tagID);
    void assignTags(const ItemInfo& info, const QList<int>& tagIDs);
    void assignTags(const QList<ItemInfo>& infos, const QList<int>& tagIDs);
    void assignTags(const QList<qlonglong>& imageIds, const QList<int>& tagIDs);

    void removeTag(const ItemInfo& info, int tagID);
    void removeTag(const QList<ItemInfo>& infos, int tagID);
    void removeTags(const ItemInfo& info, const QList<int>& tagIDs);
    void removeTags(const QList<ItemInfo>& infos, const QList<int>& tagIDs);

    void assignPickLabel(const ItemInfo& info, int pickId);
    void assignPickLabel(const QList<ItemInfo>& infos, int pickId);

    void assignColorLabel(const ItemInfo& info, int colorId);
    void assignColorLabel(const QList<ItemInfo>& infos, int colorId);

    void assignRating(const ItemInfo& info, int rating);
    void assignRating(const QList<ItemInfo>& infos, int rating);

    void addToGroup(const ItemInfo& stack, const QList<ItemInfo>& infos);
    void removeFromGroup(const ItemInfo& info);
    void removeFromGroup(const QList<ItemInfo>& infos);
    void ungroup(const ItemInfo& info);
    void ungroup(const QList<ItemInfo>& infos);

    void setExifOrientation(const QList<ItemInfo>& infos, int orientation);
/*
    void applyMetadata(const QList<ItemInfo>& infos, const MetadataHub& hub);
*/
    void applyMetadata(const QList<ItemInfo>& infos, const DisjointMetadata& hub);

    // ownership of the hub is passed, hub must be created without QObject parent
    void applyMetadata(const QList<ItemInfo>& infos, DisjointMetadata* hub);

    /**
     * Flip or rotate. Note: The NoTransformation action is interpreted as Exif auto-rotate
     */
    void transform(const QList<ItemInfo>& infos, MetaEngineRotation::TransformationAction action);

    void copyAttributes(const ItemInfo& source, const QStringList& derivedPaths);
    void copyAttributes(const ItemInfo& source, const QString& derivedPath);

public:

    // Declared public due to use by FileActionMngrWorker, FileActionMngrDatabaseWorker, and FileActionMngrFileWorker
    class Private;

private:

    /// @note disabled
    explicit FileActionMngr(QObject*);

    Private* const d = nullptr;
};

} // namespace Digikam
