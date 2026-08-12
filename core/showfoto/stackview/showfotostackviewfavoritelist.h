/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-09-27
 * Description : Showfoto stack view favorites list
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QPoint>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QContextMenuEvent>

namespace ShowFoto
{

class ShowfotoStackViewFavorites;
class ShowfotoStackViewFavoriteItem;
class ShowfotoStackViewFavoriteItem;
class ShowfotoStackViewFavoriteItem;

class ShowfotoStackViewFavoriteList : public QTreeWidget
{
    Q_OBJECT

public:

    explicit ShowfotoStackViewFavoriteList(ShowfotoStackViewFavorites* const parent);
    ~ShowfotoStackViewFavoriteList()                                         override;

    ShowfotoStackViewFavoriteItem* findFavoriteByHierarchy(const QString& hierarchy);

    /**
     * Set the string used to filter the favorites list. signalSearchResult() is emitted when all is done.
     */
    void setFilter(const QString& filter, Qt::CaseSensitivity cs);

    /**
     * Return the current string used to filter the favorites list.
     */
    QString filter()                                                    const;

Q_SIGNALS:

    void signalAddFavorite();
    void signalAddFavoriteList(const QList<QUrl>&, const QUrl& current);
    void signalLoadContentsFromFiles(const QStringList& files, const QString& current);

    /**
     * Signal emitted when filtering is done through slotSetFilter().
     * Number of favorites found is sent when item relevant of filtering match the query.
     */
    void signalSearchResult(int);

public Q_SLOTS:

    void slotLoadContents();

private Q_SLOTS:

    void slotContextMenu(const QPoint& pos);
    void slotOpenInFileManager();

private:

    void dragEnterEvent(QDragEnterEvent*);
    void dragMoveEvent(QDragMoveEvent*);
    void dropEvent(QDropEvent*);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    QMimeData* mimeData(const QList<QTreeWidgetItem*>& items)           const override;

#else

    // cppcheck-suppress passedByValue
    QMimeData* mimeData(const QList<QTreeWidgetItem*> items)            const override;     // clazy:exclude=function-args-by-ref

#endif

    Qt::DropActions supportedDropActions()                              const override;
    void startDrag(Qt::DropActions supportedActions)                          override;
    QStringList mimeTypes()                                             const override;

    void rebaseHierarchy(ShowfotoStackViewFavoriteItem* const parent);

    ShowfotoStackViewFavoriteList(QWidget*);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace ShowFoto
