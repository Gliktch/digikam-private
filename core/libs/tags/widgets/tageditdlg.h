/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-07-01
 * Description : dialog to edit and create digiKam Tags
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMap>
#include <QString>
#include <QKeySequence>
#include <QDialog>

// Local includes

#include "albummanager.h"
#include "digikam_config.h"

namespace Digikam
{

class TagEditDlg : public QDialog
{
    Q_OBJECT

public:

    explicit TagEditDlg(QWidget* const parent, TAlbum* const album, bool create = false);
    ~TagEditDlg() override;

    QString      title()    const;
    QString      icon()     const;
    QKeySequence shortcut() const;

    static bool tagEdit(QWidget* const parent, TAlbum* const album, QString& title, QString& icon, QKeySequence& ks);
    static bool tagCreate(QWidget* const parent, TAlbum* const album, QString& title, QString& icon, QKeySequence& ks);

    /**
     * @brief Create a list of new Tag album using a list of tags hierarchies separated by ",".
     * A hierarchy of tags is a string path of tags name separated by "/".
     * If a hierarchy start by "/" or if mainRootAlbum is null, it will be created from
     * root tag album, else it will be created from mainRootAlbum as parent album.
     * 'errMap' is Map of TAlbum path and error message if tag creation failed.
     * @return the list of created Albums.
     */
    static AlbumList createTAlbum(TAlbum* const mainRootAlbum, const QString& tagStr, const QString& icon,
                                  const QKeySequence& ks, QMap<QString, QString>& errMap);

    static void showtagsListCreationError(QWidget* const parent, const QMap<QString, QString>& errMap);

private Q_SLOTS:

    void slotIconChanged();
    void slotIconResetClicked();
    void slotTitleChanged(const QString& newtitle);
    void slotHelp();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
