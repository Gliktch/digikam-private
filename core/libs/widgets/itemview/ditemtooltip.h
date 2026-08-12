/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-12-10
 * Description : tool tip widget for iconview, thumbbar, and folderview items
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QFont>
#include <QLabel>
#include <QResizeEvent>
#include <QString>
#include <QImage>
#include <QEvent>
#include <QFontDatabase>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DToolTipStyleSheet
{

public:

    explicit DToolTipStyleSheet(const QFont& font = QFontDatabase::systemFont(QFontDatabase::GeneralFont));

    QString breakString(const QString& input)                        const;
    QString elidedText(const QString& input, Qt::TextElideMode mode) const;
    QString imageAsBase64(const QImage& img)                         const;

public:

    const int maxStringLength;

    QString   unavailable;
    QString   tipHeader;
    QString   tipFooter;
    QString   headBeg;
    QString   headEnd;
    QString   cellBeg;
    QString   cellMid;
    QString   cellEnd;
    QString   cellSpecBeg;
    QString   cellSpecMid;
    QString   cellSpecEnd;
};

// --------------------------------------------------------------------------------------

class DIGIKAM_EXPORT DItemToolTip : public QLabel
{
    Q_OBJECT

public:

    explicit DItemToolTip(QWidget* const parent = nullptr);
    ~DItemToolTip() override;

protected:

    bool    event(QEvent*)                    override;
    void    resizeEvent(QResizeEvent*)        override;
    void    paintEvent(QPaintEvent*)          override;
    void    updateToolTip();
    bool    toolTipIsEmpty()            const;
    void    reposition();
    void    renderArrows();

protected:

    virtual QRect   repositionRect() = 0;
    virtual QString tipContents()    = 0;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
