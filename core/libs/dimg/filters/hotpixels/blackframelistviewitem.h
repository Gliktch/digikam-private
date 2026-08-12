/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-07-05
 * Description : a list view to display black frames - the item
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
#include <QString>
#include <QUrl>
#include <QTreeWidget>

// Local includes

#include "digikam_export.h"
#include "hotpixelprops.h"

namespace Digikam
{

class DIGIKAM_EXPORT BlackFrameListViewItem : public QObject,
                                              public QTreeWidgetItem
{
    Q_OBJECT

public:

    enum BlackFrameConst
    {
        // Columns
        PREVIEW     = 0,
        SIZE        = 1,
        HOTPIXELS   = 2,

        // Thumbnail properties
        THUMB_WIDTH = 150
    };

public:

    explicit BlackFrameListViewItem(QTreeWidget* const parent, const QUrl& url);
    ~BlackFrameListViewItem()     override;

    QUrl    frameUrl()      const;
    QString toolTipString() const;

    void emitHotPixelsParsed();

Q_SIGNALS:

    void signalHotPixelsParsed(const QList<HotPixelProps>&, const QUrl&);

private Q_SLOTS:

    void slotHotPixelsParsed(const QList<HotPixelProps>&);
    void slotLoadingProgress(float);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
