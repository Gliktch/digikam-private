/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-01-20
 * Description : action categorized view
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_export.h"
#include "dcategorizedview.h"

class QWidget;
class QEvent;
class QScrollBar;
class QMouseEvent;
class QPropertyAnimation;

namespace Digikam
{

class DIGIKAM_EXPORT ActionCategorizedView : public DCategorizedView
{
    Q_OBJECT

public:

    explicit ActionCategorizedView(QWidget* const parent = nullptr, bool autoScroll = false);
    ~ActionCategorizedView()            override = default;

    void setupIconMode();
    void adjustGridSize();

protected:

    int  autoScrollDuration(float relativeDifference, QPropertyAnimation* animation);
    void autoScroll(float relativePos, QScrollBar* scrollBar, QPropertyAnimation* animation);
    void mouseMoveEvent(QMouseEvent* e) override;
    void leaveEvent(QEvent* e)          override;

protected:

    QPropertyAnimation* m_verticalScrollAnimation   = nullptr;
    QPropertyAnimation* m_horizontalScrollAnimation = nullptr;
    bool                m_autoScroll                = false;
};

} // namespace Digikam
