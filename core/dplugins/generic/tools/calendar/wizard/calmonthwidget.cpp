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

#include "calmonthwidget.h"

// Qt includes

#include <QDragEnterEvent>
#include <QFileInfo>
#include <QImageReader>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QStandardPaths>
#include <QMimeData>
#include <QUrl>
#include <QSize>
#include <QLocale>

// Local includes

#include "calsettings.h"
#include "imagedialog.h"
#include "digikam_debug.h"

namespace DigikamGenericCalendarPlugin
{

class Q_DECL_HIDDEN CalMonthWidget::Private
{
public:

    Private() = default;

    const QSize          thumbSize          = QSize(64, 64);
    QPixmap              thumb;
    int                  month              = 0;
    QUrl                 imagePath;
    ThumbnailLoadThread* thumbLoadThread    = ThumbnailLoadThread::defaultThread();
};

CalMonthWidget::CalMonthWidget(QWidget* const parent, int month)
    : QPushButton(parent),
      d          (new Private)
{
    setAcceptDrops(true);
    setFixedSize(QSize(74, 94));
    d->month     = month;
    d->imagePath = QUrl();
    setThumb(QPixmap(QIcon::fromTheme(QLatin1String("view-preview"))
             .pixmap(32, QIcon::Disabled)));

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnail(LoadingDescription,QPixmap)));

    connect(this, SIGNAL(pressed()),
            this, SLOT(slotMonthSelected()));
}

CalMonthWidget::~CalMonthWidget()
{
    delete d;
}

QUrl CalMonthWidget::imagePath() const
{
    return d->imagePath;
}

void CalMonthWidget::paintEvent(QPaintEvent* e)
{
    QPushButton::paintEvent(e);

    QPainter painter(this);
    QString name = QLocale().standaloneMonthName(d->month, QLocale::ShortFormat);
    QRect cr     = contentsRect();
    cr.setBottom(70);
    painter.drawPixmap(cr.width()  / 2 - d->thumb.width()  / 2,
                       cr.height() / 2 - d->thumb.height() / 2,
                       d->thumb);

    cr           = contentsRect();
    cr.setTop(70);
    painter.drawText(cr, Qt::AlignHCenter, name);
}

void CalMonthWidget::dragEnterEvent(QDragEnterEvent* e)
{
    if (e->mimeData()->hasImage())
    {
        e->acceptProposedAction();
    }
}

QPixmap CalMonthWidget::thumb() const
{
    return d->thumb;
}

int CalMonthWidget::month()
{
  return d->month;
}

void CalMonthWidget::setThumb(const QPixmap& pic)
{
    d->thumb = pic.scaled(d->thumbSize, Qt::KeepAspectRatio);
    update();
}

void CalMonthWidget::setImage(const QUrl& url)
{
    if (!url.isValid())
    {
        return;
    }

    d->imagePath = url;
    CalSettings::instance()->setImage(d->month, d->imagePath);

    d->thumbLoadThread->find(ThumbnailIdentifier(url.toLocalFile()), d->thumbSize.width());
}

void CalMonthWidget::dropEvent(QDropEvent* e)
{
    QList<QUrl> srcURLs = e->mimeData()->urls();

    if (srcURLs.isEmpty())
    {
        return;
    }

    QUrl url = srcURLs.first();
    setImage(url);
}

void CalMonthWidget::slotMonthSelected()
{
    Q_EMIT monthSelected(d->month);
}

void CalMonthWidget::slotThumbnail(const LoadingDescription& desc, const QPixmap& pix)
{
    if (QUrl::fromLocalFile(desc.filePath) != d->imagePath)
    {
        return;
    }

    setThumb(pix);
}

void CalMonthWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (!contentsRect().contains(e->pos()))
    {
        return;
    }

    if      (e->button() == Qt::LeftButton)
    {
        ImageDialog dlg(this,
                        QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)),
                        true);
        setImage(dlg.url());
    }
    else if (e->button() == Qt::RightButton)
    {
        d->imagePath = QUrl();
        CalSettings::instance()->setImage(d->month, d->imagePath);
        setThumb(QPixmap(QIcon::fromTheme(QLatin1String("view-preview")).pixmap(32, QIcon::Disabled)));
    }
}

} // Namespace Digikam

#include "moc_calmonthwidget.cpp"
