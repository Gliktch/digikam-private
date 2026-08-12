/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 20013-07-31
 * Description : Tag List implementation as Quick Access for various
 *               subtrees in Tag Manager
 *
 * SPDX-FileCopyrightText: 2013      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

namespace Digikam
{

class TagMngrTreeView;
class Album;

class TagList : public QWidget
{
    Q_OBJECT

public:

    explicit TagList(TagMngrTreeView* const treeView, QWidget* const parent);
    ~TagList() override;

    /**
     * @brief save settings to digiKam_tagsmanagerrc KConfig.
     */
    void saveSettings();

    /**
     * @brief read settings from digikam_tagsmanagerrc
     * config and populate model with data.
     */
    void restoreSettings();

    /**
     * @brief disable Add Button when selection is empty
     * or only root tag is selected.
     */
    void enableAddButton(bool value);

private Q_SLOTS:

    void slotAddPressed();

    void slotSelectionChanged();

    void slotTagDeleted(Album* album);

    void slotDeleteSelected();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
