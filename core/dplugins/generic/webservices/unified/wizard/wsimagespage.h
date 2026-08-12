/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : page visualizing photos user choosing to upload and
 *               user albums list to upload photos to. Creating new album
 *               is also available on this page.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QList>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QTreeWidget>
#include <QTreeWidgetItem>

// Local includes

#include "dwizardpage.h"
#include "wsitem.h"

using namespace Digikam;

namespace DigikamGenericUnifiedPlugin
{

class WSImagesPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit WSImagesPage(QWizard* const dialog, const QString& title);
    ~WSImagesPage();

    void    initializePage();
    bool    validatePage();
    bool    isComplete() const;

    void    setItemsList(const QList<QUrl>& urls);

private:

    /*
     * Get a structure from albums list and add it recursively to albums view
     */
    void addChildToTreeView(QTreeWidgetItem* const parent,
                            const QMap<QString, AlbumSimplified>& albumTree,
                            const QStringList& childrenAlbums);

    /*
     * Set id for album chosen to upload photos.
     *
     * This method should be called in validatePage(), so that talker can get it
     * from d->wizard later.
     */
    void setCurrentAlbumId(const QString& currentAlbumId);

Q_SIGNALS:

    /*
     * Signal to inform talker to list albums.
     */
    void signalListAlbumsRequest();

private Q_SLOTS:

    /*
     * Connected to signal signalListAlbumsDone of WSAuthentication to visualize albums list
     */
    void slotListAlbumsDone(const QMap<QString, AlbumSimplified>& albumTree,
                            const QStringList& rootAlbums,
                            const QString& currentAlbumId);

    /*
     * Connected to signalCreatAlbumDone of WSAuthentication to refresh album list and point
     * pre-selected album to new album
     */
    void slotCreateAlbumDone(int errCode, const QString& errMsg, const QString& newAlbumId);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericUnifiedPlugin
