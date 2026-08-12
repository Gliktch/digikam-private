/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Scanning a single item - baloo helper.
 *
 * SPDX-FileCopyrightText: 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemscanner_p.h"

#ifdef HAVE_KFILEMETADATA
#   include "baloowrap.h"
#endif

namespace Digikam
{

void ItemScanner::scanBalooInfo()
{

#ifdef HAVE_KFILEMETADATA

    BalooWrap* const baloo = BalooWrap::instance();

    if (!baloo->getSyncToDigikam())
    {
        return;
    }

    BalooInfo bInfo = baloo->getSemanticInfo(QUrl::fromLocalFile(d->fileInfo.absoluteFilePath()));

    if (!bInfo.tags.isEmpty())
    {
        // get tag ids, create if necessary

        QList<int> tagIds = TagsCache::instance()->getOrCreateTags(bInfo.tags);
        d->commit.tagIds += tagIds;
    }

    if (bInfo.rating != -1)
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Baloo rating" << bInfo.rating;

        if (!d->commit.imageInformationFields.testFlag(DatabaseFields::Rating))
        {
            d->commit.imageInformationFields |= DatabaseFields::Rating;
        }
        else
        {
            d->commit.imageInformationInfos.removeAt(0);
        }

        d->commit.imageInformationInfos.insert(0, bInfo.rating);
    }

    if (!bInfo.comment.isEmpty())
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Baloo comment" << bInfo.comment;

        CaptionValues val;
        val.caption                  = bInfo.comment;
        d->commit.commitItemComments = true;
        d->commit.captions.insert(QLatin1String("x-default"), val);
    }

#endif // HAVE_KFILEMETADATA

}

} // namespace Digikam
