/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-22-07
 * Description : Icon view for import tool items
 *
 * SPDX-FileCopyrightText: 2012      by Islam Wazery <wazery at ubuntu dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "importcategorizedview.h"

namespace Digikam
{

class ItemViewUtilities;

class ImportIconView : public ImportCategorizedView
{
    Q_OBJECT

public:

    explicit ImportIconView(QWidget* const parent = nullptr);
    ~ImportIconView()                                                             override;

    ItemViewUtilities* utilities() const;

    int fitToWidthIcons();

    CamItemInfo  camItemInfo(const QString& folder, const QString& file);
    CamItemInfo& camItemInfoRef(const QString& folder, const QString& file);

public Q_SLOTS:

    void deleteSelected(bool permanently = false);
    void deleteSelectedDirectly(bool permanently = false);

    void createGroupFromSelection();
    void createGroupByTimeFromSelection();
    void ungroupSelected();
    void removeSelectedFromGroup();
    void rename();

    void assignTagToSelected(int tagID);
    void removeTagFromSelected(int tagID);

    void assignPickLabel(const QModelIndex& index, int pickId);
    void assignPickLabelToSelected(int pickId);

    void assignColorLabel(const QModelIndex& index, int colorId);
    void assignColorLabelToSelected(int colorId);

    void assignRating(const QList<QModelIndex>& index, int rating);
    void assignRatingToSelected(int rating);

Q_SIGNALS:

    void previewRequested(const CamItemInfo& info, bool downloadPreview);
/*
    void signalPopupTagsView();
*/
private Q_SLOTS:

    void slotRotateLeft(const QList<QModelIndex>&);
    void slotRotateRight(const QList<QModelIndex>&);
/*
    void slotInitProgressIndicator();
*/

protected:

    void activated(const CamItemInfo& info, Qt::KeyboardModifiers modifiers)      override;
    void showContextMenuOnInfo(QContextMenuEvent* event, const CamItemInfo& info) override;
    void showContextMenu(QContextMenuEvent* event)                                override;
    void slotSetupChanged()                                                       override;

private:

    /// @note disabled
    ImportIconView(const ImportIconView&)            = delete;
    ImportIconView& operator=(const ImportIconView&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
