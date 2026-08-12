/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-21-06
 * Description : Thread actions task for metadata remover.
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2023      by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metadataremovetask.h"

// Local includes

#include "digikam_debug.h"
#include "coredboperationgroup.h"
#include "maintenancedata.h"
#include "scancontroller.h"
#include "facetagseditor.h"
#include "metadatahub.h"
#include "faceutils.h"
#include "tagscache.h"
#include "iteminfo.h"

namespace Digikam
{

class Q_DECL_HIDDEN MetadataRemoveTask::Private
{
public:

    Private() = default;

public:

    MetadataRemover::RemoveAction removeAction  = MetadataRemover::None;

    MaintenanceData*              data          = nullptr;
};

// -------------------------------------------------------

MetadataRemoveTask::MetadataRemoveTask()
    : ActionJob(),
      d        (new Private)
{
}

MetadataRemoveTask::~MetadataRemoveTask()
{
    cancel();
    delete d;
}

void MetadataRemoveTask::setRemoveAction(MetadataRemover::RemoveAction action)
{
    d->removeAction = action;
}

void MetadataRemoveTask::setMaintenanceData(MaintenanceData* const data)
{
    d->data = data;
}

void MetadataRemoveTask::run()
{
    ActionJob::run();       // To customize thread name

    // While we have data (using this as check for non-null).

    while (d->data)
    {
        if (m_cancel)
        {
            return;
        }

        ItemInfo item = d->data->getItemInfo();

        // If the item is null, we are done.

        if (item.isNull())
        {
            break;
        }

        if      (d->removeAction == MetadataRemover::Faces)
        {
            if (FaceTagsEditor().databaseFaces(item.id()).size() > 0)
            {
                FaceUtils().removeAllFaces(item.id());
            }
        }
        else if (d->removeAction == MetadataRemover::Tags)
        {
            const QList<int>& tagIds = TagsCache::instance()->publicTags(item.tagIds());

            if (!tagIds.isEmpty())
            {
                bool tagRemoved  = false;
                QList<int> confirmedFaceTags;
                const auto faces = FaceTagsEditor().confirmedFaceTagsIfaces(item.id());

                for (const FaceTagsIface& face : faces)
                {
                    confirmedFaceTags << face.tagId();
                }

                {
                    CoreDbOperationGroup group;
                    group.setMaximumTime(200);

                    for (int tag : std::as_const(tagIds))
                    {
                        if (
                            !confirmedFaceTags.contains(tag)  &&
                            !FaceTags::isSystemPersonTagId(tag)
                           )
                        {
                            item.removeTag(tag);
                            group.allowLift();

                            tagRemoved = true;
                        }
                    }
                }

                if (tagRemoved)
                {
                    MetadataHub hub;
                    hub.load(item);

                    ScanController::FileMetadataWrite writeScope(item);
                    writeScope.changed(hub.writeToMetadata(item, MetadataHub::WRITE_TAGS));
                }
            }
        }

        Q_EMIT signalFinished(item, QImage());
    }

    Q_EMIT signalDone();
}

} // namespace Digikam

#include "moc_metadataremovetask.cpp"
