/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-11-10
 * Description : Performs face detection and recognition
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QElapsedTimer>

// Local includes

#include "facepipelinebase.h"
#include "facetagsiface.h"
#include "faceutils.h"

namespace Digikam
{

class FacePipelineEdit : public FacePipelineBase
{
    Q_OBJECT

public:

    explicit FacePipelineEdit();
    ~FacePipelineEdit()                                         override = default;

    static FacePipelineEdit* instance();

public:

    bool start()                                                override;

    FaceTagsIface confirmFace(const ItemInfo& info,
                              const FaceTagsIface& face,
                              const TagRegion& region,
                              int tagId,
                              bool retrain = true);

    void removeFace(const ItemInfo& info,
                    const FaceTagsIface& face);

    void removeAllFaces(const ItemInfo& info);

    FaceTagsIface editTag(const ItemInfo& info,
                          const FaceTagsIface& face,
                          int newTagId);

    FaceTagsIface editRegion(const ItemInfo& info,
                             const FaceTagsIface& face,
                             const TagRegion& region,
                             int tagId = -1,
                             bool retrain = true);

    FaceTagsIface addManually(const ItemInfo& info,
                              const DImg& image,
                              const TagRegion& region,
                              bool retrain = true);

    /**
     * Delete all saved rejected face tag lists for faces in the given image.
     */
    QList<FaceTagsIface> deleteRejectedFaceTagLists(const ItemInfo& info);

    /**
     * Delete all saved rejected face tags for the given face.
     */
    FaceTagsIface deleteRejectedFaceTagList(const FaceTagsIface& face);

protected:

    bool finder()                                               override
    {
        return false;
    }

    bool loader()                                               override;
    bool extractor()                                            override;

    bool classifier()                                           override
    {
        return false;
    }

    bool trainer()                                              override
    {
        return false;
    }

    bool writer()                                               override;

    void addMoreWorkers()                                       override;

    /**
     * returns a copy of FaceTagsIface object with the face tags that were rejected
     */
    FaceTagsIface getRejectedFaceTagList(const FaceTagsIface& face) const;

private:

    bool            isStarted = false;

    // TODO: Remove DEBUG timer
    QElapsedTimer   debugConfirmTimer;

private:

    /// @note disabled
    FacePipelineEdit(QObject* const)                            = delete;
    FacePipelineEdit(const FacePipelineEdit&)                   = delete;
    FacePipelineEdit& operator=(const FacePipelineEdit&)        = delete;
};

} // namespace Digikam
