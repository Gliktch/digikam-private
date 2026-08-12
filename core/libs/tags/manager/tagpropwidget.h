/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 20013-07-03
 * Description : Tag Properties widget to display tag properties
 *               when a tag or multiple tags are selected
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

// Local includes

#include "digikam_config.h"

namespace Digikam
{

class Album;

class TagPropWidget : public QWidget
{
    Q_OBJECT

public:

    enum ItemsEnable
    {
        DisabledAll,
        EnabledAll,
        IconOnly
    };
    Q_ENUM(ItemsEnable);

public:

    explicit TagPropWidget(QWidget* const parent);
    ~TagPropWidget() override;

Q_SIGNALS:

    void signalTitleEditReady();

public Q_SLOTS:

    void slotSelectionChanged(const QList<Album*>& albums);
    void slotFocusTitleEdit();

private Q_SLOTS:

    void slotIconResetClicked();
    void slotIconChanged();
    void slotDataChanged();
    void slotSaveChanges();
    void slotDiscardChanges();
    void slotReturnPressed();

private:

    /**
     * @brief enable items based on selection.
     * If no item is selected, disable all.
     * If one item selected, enable all.
     * If multiple selected, enable icon & icon button.
     */
    void enableItems(ItemsEnable value);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
