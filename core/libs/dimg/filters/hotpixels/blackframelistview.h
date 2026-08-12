/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-07-05
 * Description : a list view to display black frames
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2006 by Unai Garro <ugarro at users dot sourceforge dot net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QUrl>
#include <QTreeWidget>

// Local includes

#include "digikam_export.h"
#include "hotpixelprops.h"

namespace Digikam
{

class DIGIKAM_EXPORT BlackFrameListView : public QTreeWidget
{
    Q_OBJECT

public:

    explicit BlackFrameListView(QWidget* const parent = nullptr);
    ~BlackFrameListView()                                           override;

    bool contains(const QUrl& url);
    bool isSelected(const QUrl& url);
    QUrl currentUrl();

Q_SIGNALS:

    void signalBlackFrameSelected(const QList<HotPixelProps>&, const QUrl&);
    void signalBlackFrameRemoved(const QUrl&);
    void signalClearBlackFrameList();

private Q_SLOTS:

    void slotSelectionChanged();
    void slotHotPixelsParsed(const QList<HotPixelProps>&, const QUrl&);
    void slotToolTip();
    void slotContextMenu();

private:

    void hideToolTip();
    bool acceptToolTip(const QPoint& pos)                         const;

    void mouseMoveEvent(QMouseEvent*)                                   override;
    void wheelEvent(QWheelEvent*)                                       override;
    void keyPressEvent(QKeyEvent*)                                      override;
    void focusOutEvent(QFocusEvent*)                                    override;
    void leaveEvent(QEvent*)                                            override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
