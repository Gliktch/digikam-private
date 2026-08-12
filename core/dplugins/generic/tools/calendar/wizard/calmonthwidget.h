/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-11-03
 * Description : Calendar month image selection widget.
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006      by Tom Albers <tomalbers at kde dot nl>
 * SPDX-FileCopyrightText: 2007-2008 by Orgad Shaneh <orgads at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Angelo Naselli <anaselli at linux dot it>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QPixmap>
#include <QPushButton>
#include <QUrl>

// Local includes

#include "loadingdescription.h"
#include "thumbnailloadthread.h"

class QDragEnterEvent;
class QDropEvent;
class QMouseEvent;
class QPaintEvent;

using namespace Digikam;

namespace DigikamGenericCalendarPlugin
{

class CalMonthWidget : public QPushButton
{
    Q_OBJECT

public:

    explicit CalMonthWidget(QWidget* const parent, int month);
    ~CalMonthWidget()                       override;

    QUrl imagePath() const;
    void setImage(const QUrl& url);
    int  month();

Q_SIGNALS:

    void monthSelected(int);

protected:

    void dragEnterEvent(QDragEnterEvent* e) override;
    void dropEvent(QDropEvent* e)           override;
    void paintEvent(QPaintEvent* e)         override;
    void mouseReleaseEvent(QMouseEvent* e)  override;

private Q_SLOTS:

    void slotThumbnail(const LoadingDescription&, const QPixmap&);
    void slotMonthSelected();

private:

    QPixmap thumb() const;
    void    setThumb(const QPixmap& pic);

    Q_PROPERTY(QPixmap thumb READ thumb WRITE setThumb)

private:

    class Private;
    Private* const d = nullptr;
};

} // Namespace Digikam
