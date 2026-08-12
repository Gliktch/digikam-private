/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-06
 * Description : interface to database information for shared tools.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 * SPDX-FileCopyrightText: 2017      by Mario Frank <mario dot frank at uni minus potsdam dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_config.h"
#include "digikam_globals.h"
#include "digikam_export.h"
#include "dinfointerface.h"

namespace Digikam
{

class Album;

class DIGIKAM_GUI_EXPORT DBInfoIface : public DInfoInterface
{
    Q_OBJECT

public:

    explicit DBInfoIface(QObject* const parent,
                         const QList<QUrl>& lst = QList<QUrl>(),
                         const OperationType type = UnspecifiedOps);
    ~DBInfoIface()                                                                 override;

public Q_SLOTS:

    void slotDateTimeForUrl(const QUrl& url, const QDateTime& dt, bool updModDate) override;
    void slotMetadataChangedForUrl(const QUrl& url)                                override;

public:

    QList<QUrl> currentSelectedItems()                                       const override;
    QList<QUrl> currentAlbumItems()                                          const override;
    void        parseAlbumItemsRecursive()                                         override;

    QList<QUrl> albumItems(Album* const album)                               const;
    QList<QUrl> albumItems(int id)                                           const override;
    QList<QUrl> albumsItems(const DAlbumIDs&)                                const override;
    QList<QUrl> allAlbumItems()                                              const override;

    DInfoMap    albumInfo(int)                                               const override;

    DInfoMap    itemInfo(const QUrl&)                                        const override;
    void        setItemInfo(const QUrl&, const DInfoMap&)                          override;

    QWidget*    albumChooser(QWidget* const parent)                          const override;
    DAlbumIDs   albumChooserItems()                                          const override;
    bool        supportAlbums()                                              const override;

    QWidget*    uploadWidget(QWidget* const parent)                          const override;
    QUrl        uploadUrl()                                                  const override;

    QUrl        defaultUploadUrl()                                           const override;

    QAbstractItemModel* tagFilterModel()                                           override;

    void        deleteImage(const QUrl& url)                                       override;

    void        openSetupPage(SetupPage page)                                      override;

#ifdef HAVE_GEOLOCATION

    QList<GPSItemContainer*> currentGPSItems()                               const override;

#endif

    QMap<QString, QString> passShortcutActionsToWidget(QWidget* const wdg)   const override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
