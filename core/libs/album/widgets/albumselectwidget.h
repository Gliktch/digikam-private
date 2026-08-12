/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-26-02
 * Description : a widget to select a physical album
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "albummanager.h"
#include "albumtreeview.h"
#include "searchtextbardb.h"

class QTreeWidget;

namespace Digikam
{

class PAlbum;
class AlbumModificationHelper;

/**
 * Enables a simple context menu only for creating a new album.
 *
 * @author jwienke
 */
class AlbumSelectTreeView: public AlbumTreeView
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param model album model to work with
     * @param albumModificationHelper helper object for modifying albums
     * @param parent the parent for Qt's parent child mechanism
     */
    AlbumSelectTreeView(AlbumModel* const model,
                        AlbumModificationHelper* const albumModificationHelper,
                        QWidget* const parent = nullptr);

    /**
     * Destructor.
     */
    ~AlbumSelectTreeView() override;

    void addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album) override;
    void handleCustomContextMenuAction(QAction* action, const AlbumPointer<Album>& album) override;

public Q_SLOTS:

    /**
     * Shows a dialog to create a new album under the selected album in this
     * view.
     */
    void slotNewAlbum();

private:

    class Private;
    Private* const d = nullptr;
};

// -----------------------------------------------------------------------------------------------

class AlbumSelectWidget : public QWidget
{
    Q_OBJECT

public:

    explicit AlbumSelectWidget(QWidget* const parent = nullptr,
                               PAlbum* const albumToSelect = nullptr,
                               bool completerSelect = false);
    ~AlbumSelectWidget() override;

    void setCurrentAlbumUrl(const QUrl& albumUrl);
    QUrl currentAlbumUrl() const;

    PAlbum* currentAlbum() const;
    void    setCurrentAlbum(PAlbum* const albumToSelect);

Q_SIGNALS:

    void itemSelectionChanged();
    void completerActivated();

private Q_SLOTS:

    void slotCompleterTimer();
    void slotSelectionChanged();
    void slotAlbumRenamed(Album*);
    void slotCompleterHighlighted(int albumId);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
