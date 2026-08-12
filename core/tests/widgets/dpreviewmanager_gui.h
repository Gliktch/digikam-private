/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-10-17
 * Description : test for implementation of DPreviewManager api
 *
 * SPDX-FileCopyrightText: 2011-2012 by A Janardhan Reddy <annapareddyjanardhanreddy at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDialog>

class DPreviewManagerTest : public QDialog
{
    Q_OBJECT

public:

    explicit DPreviewManagerTest(QWidget* const parent = nullptr);
    ~DPreviewManagerTest() override;

private Q_SLOTS:

    void slotLoadImage();
    void slotSelectionChanged(const QRectF&);

private:

    class Private;
    Private* const d = nullptr;
};
