/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-23
 * Description : a widget to manage preview.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QStackedWidget>
#include <QString>
#include <QColor>
#include <QImage>
#include <QUrl>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DPreviewManager : public QStackedWidget
{
    Q_OBJECT

public:

    enum DisplayMode
    {
        MessageMode = 0,
        PreviewMode
    };

public:

    explicit DPreviewManager(QWidget* const parent);
    ~DPreviewManager() override;

    bool load(const QUrl& file, bool fit = true);
    void setImage(const QImage& img, bool fit = true);
    void setText(const QString& text, const QColor& color = Qt::white);
    void setBusy(bool b, const QString& text = QString());
    void setThumbnail(const QPixmap& preview = QPixmap());
    void setButtonText(const QString& text);
    void setButtonVisible(bool b);
    void setSelectionAreaPossible(bool b);

    ///@{
    /**
     * Manage a selection area and show it
     *
     * @param rectangle This rectangle should have height and width of 1.0
     */
    void   setSelectionArea(const QRectF& rectangle);
    QRectF getSelectionArea() const;
    ///@}

Q_SIGNALS:

    void signalButtonClicked();
    void signalSelectionChanged(const QRectF&);

public Q_SLOTS:

    void slotLoad(const QUrl& url);

private Q_SLOTS:

    void slotProgressTimerDone();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
