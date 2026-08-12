/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface - Face Album helpers.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albummanager_p.h"

namespace Digikam
{

QHash<int, int> AlbumManager::getFaceCount() const
{
    return d->fAlbumsCount;
}

QHash<int, int> AlbumManager::getUnconfirmedFaceCount() const
{
    return d->uAlbumsCount;
}

void AlbumManager::personItemsCount()
{
    if (d->personListJob)
    {
        disconnect(d->personListJob, nullptr,
                   this, nullptr);

        d->personListJob->cancel();
        d->personListJob = nullptr;
    }

    TagsDBJobInfo jInfo;
    jInfo.setFaceFoldersJob();

    d->personListJob = DBJobsManager::instance()->startTagsJobThread(jInfo);

    connect(d->personListJob, SIGNAL(signalFinished()),
            this, SLOT(slotPeopleJobResult()));

    connect(d->personListJob, SIGNAL(faceFoldersData(QMap<QString,QHash<int,int> >)),    // krazy:exclude=normalize
            this, SLOT(slotPeopleJobData(QMap<QString,QHash<int,int> >)));               // krazy:exclude=normalize
}

void AlbumManager::slotPeopleJobResult()
{
    if (!d->personListJob)
    {
        return;
    }

    if (d->personListJob->hasErrors())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to list face tags";

        // Pop-up a message about the error.

        DNotificationWrapper(QString(), d->personListJob->errorsList().first(),
                             nullptr, i18n("digiKam"));
    }

    d->personListJob = nullptr;
}

void AlbumManager::slotPeopleJobData(const QMap<QString, QHash<int, int> >& facesStatHash)
{
    if (facesStatHash.isEmpty())
    {
        return;
    }

    d->uAlbumsCount.clear();

    // I think this is a bug
    // Why autodetectedFace have all autodetected tags?
    // They should be in autodetectedPerson

    if (facesStatHash.contains(ImageTagPropertyName::autodetectedFace()))                // autodetectedPerson
    {
        d->uAlbumsCount = *facesStatHash.find(ImageTagPropertyName::autodetectedFace()); // autodetectedPerson
    }

    d->fAlbumsCount.clear();

    if (ApplicationSettings::instance()->getShowFolderTreeViewItemsCount())
    {
        typedef QHash<int, int> IntIntHash;

        for (const IntIntHash& counts : std::as_const(facesStatHash))
        {
            QHash<int, int>::const_iterator it;

            for (it = counts.begin() ; it != counts.end() ; ++it)
            {
                d->fAlbumsCount[it.key()] += it.value();
            }
        }
    }

    Q_EMIT signalFaceCountsDirty(d->fAlbumsCount, d->uAlbumsCount, d->toUpdatedFaces);

    d->toUpdatedFaces.clear();
}

} // namespace Digikam
