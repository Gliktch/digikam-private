/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-08-31
 * Description : a widget to display free space for a mount-point.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "freespacewidget.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QPainter>
#include <QPixmap>
#include <QPalette>
#include <QColor>
#include <QTimer>
#include <QFont>
#include <QBoxLayout>
#include <QFontMetrics>
#include <QUrl>
#include <QIcon>
#include <QStyle>
#include <QStorageInfo>

// KDE includes

#include <KLocalizedString>

// Local includes

#include "digikam_debug.h"
#include "freespacetooltip.h"
#include "applicationsettings.h"
#include "itempropertiestab.h"

namespace Digikam
{

class Q_DECL_HIDDEN MountPointInfo
{
public:

    MountPointInfo() = default;

public:

    bool    isValid     = false;

    qint64  bytesSize   = 0;
    qint64  bytesUsed   = 0;
    qint64  bytesAvail  = 0;

    QString mountPoint;
};

// ---------------------------------------------------------------------------------

class Q_DECL_HIDDEN FreeSpaceWidget::Private
{
public:

    Private() = default;

public:

    bool                            isValid     = false;

    int                             percentUsed = -1;

    qint64                          dSizeBytes  = 0;
    qint64                          bytesSize   = 0;
    qint64                          bytesUsed   = 0;
    qint64                          bytesAvail  = 0;

    QStringList                     paths;
    QHash<QString, MountPointInfo>  infos;

    QTimer*                         timer       = nullptr;

    QPixmap                         iconPix;

    FreeSpaceToolTip*               toolTip     = nullptr;

    FreeSpaceWidget::FreeSpaceMode  mode        = FreeSpaceWidget::AlbumLibrary;
};

FreeSpaceWidget::FreeSpaceWidget(QWidget* const parent, int width)
    : QWidget(parent),
      d      (new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedWidth(width);
    setMaximumHeight(fontMetrics().height() + 4);
    d->timer   = new QTimer(this);
    d->toolTip = new FreeSpaceToolTip(this);

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotTimeout()));
}

FreeSpaceWidget::~FreeSpaceWidget()
{
    d->timer->stop();

    delete d->timer;
    delete d->toolTip;
    delete d;
}

void FreeSpaceWidget::setMode(FreeSpaceMode mode)
{
    d->mode = mode;

    if (d->mode == FreeSpaceWidget::AlbumLibrary)
    {
        d->iconPix = QIcon::fromTheme(QLatin1String("folder-pictures")).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize));
    }
    else
    {
        d->iconPix = QIcon::fromTheme(QLatin1String("camera-photo")).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize));
    }

    update();
}

void FreeSpaceWidget::setPath(const QString& path)
{
    d->paths.clear();
    d->paths << path;
    refresh();
}

void FreeSpaceWidget::setPaths(const QStringList& paths)
{
    d->paths = paths;
    refresh();
}

void FreeSpaceWidget::refresh()
{
    d->timer->stop();
    slotTimeout();
    d->timer->start(10000);
}

void FreeSpaceWidget::addInformation(qint64 _bytesSize,
                                     qint64 _bytesUsed,
                                     qint64 _bytesAvail,
                                     const QString& _mountPoint)
{
    MountPointInfo inf;

    inf.mountPoint        = _mountPoint;
    inf.bytesSize         = _bytesSize;
    inf.bytesUsed         = _bytesUsed;
    inf.bytesAvail        = _bytesAvail;
    inf.isValid           = (_bytesSize > 0);

    d->infos[_mountPoint] = inf;

    // Update cumulative data

    d->bytesSize          = 0;
    d->bytesUsed          = 0;
    d->bytesAvail         = 0;
    d->isValid            = false;
    d->percentUsed        = -1;

    for (const MountPointInfo& info : std::as_const(d->infos))
    {
        if (info.isValid)
        {
            d->bytesSize  += info.bytesSize;
            d->bytesUsed  += info.bytesUsed;
            d->bytesAvail += info.bytesAvail;
            d->isValid     = true;
        }
    }

    if (_bytesSize > 0)
    {
        d->percentUsed = lround(100.0 - (100.0 * _bytesAvail / _bytesSize));
    }

    updateToolTip();
    update();
}

void FreeSpaceWidget::setEstimatedDSizeBytes(qint64 dSize)
{
    d->dSizeBytes = dSize;

    updateToolTip();
    update();
}

qint64 FreeSpaceWidget::estimatedDSizeBytes() const
{
    return d->dSizeBytes;
}

bool FreeSpaceWidget::isValid() const
{
    return d->isValid;
}

int FreeSpaceWidget::percentUsed() const
{
    return d->percentUsed;
}

qint64 FreeSpaceWidget::bytesSize() const
{
    return d->bytesSize;
}

qint64 FreeSpaceWidget::bytesUsed() const
{
    return d->bytesUsed;
}

qint64 FreeSpaceWidget::bytesAvail() const
{
    return d->bytesAvail;
}

qint64 FreeSpaceWidget::bytesAvail(const QString& path) const
{
    int mountPointMatch = 0;
    MountPointInfo selectedInfo;

    for (const MountPointInfo& info : std::as_const(d->infos))
    {
        if (info.isValid && !info.mountPoint.isEmpty() && path.startsWith(info.mountPoint))
        {
            int length = info.mountPoint.length();

            if (length > mountPointMatch)
            {
                mountPointMatch = info.mountPoint.length();
                selectedInfo    = info;
            }
        }
    }

    if (!mountPointMatch)
    {
        qCWarning(DIGIKAM_IMPORTUI_LOG) << "Did not identify a valid mount point for" << path;

        return -1;
    }

    return selectedInfo.bytesAvail;
}

void FreeSpaceWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    p.setPen(palette().mid().color());
    p.drawRect(0, 0, width() - 1, height() - 1);
    p.drawPixmap(2, height() / 2 - d->iconPix.height() / 2,
                 d->iconPix, 0, 0, d->iconPix.width(), d->iconPix.height());

    if (isValid())
    {
        // We will compute the estimated % of space size used to download and process.

        qint64 eUsedBytes = d->dSizeBytes + d->bytesUsed;
        int peUsed        = (int)(100.0 * ((double)eUsedBytes / (double)d->bytesSize));
        int pClamp        = (peUsed > 100) ? 100 : peUsed;
        QColor barcol     = QColor(62, 255, 62);       // Smooth Green.

        if (peUsed > 80)
        {
            barcol = QColor(240, 255, 62);             // Smooth Yellow.
        }

        if (peUsed > 95)
        {
            barcol = QColor(255, 62, 62);              // Smooth Red.
        }

        p.setBrush(barcol);
        p.setPen(palette().light().color());
        QRect gRect(d->iconPix.height() + 3, 2,
                    (int)(((double)width() - 3.0 - d->iconPix.width() - 2.0) * (pClamp / 100.0)),
                    height() - 5);
        p.drawRect(gRect);

        QRect tRect(d->iconPix.height() + 3, 2, width() - 3 - d->iconPix.width() - 2, height() - 5);
        QString text        = i18nc("%1 is the percent value, % is the percent sign", "%1%", peUsed);
/*
        QFontMetrics fontMt = p.fontMetrics();
        QRect fontRect      = fontMt.boundingRect(tRect.x(), tRect.y(),
                                                  tRect.width(), tRect.height(), 0, text);
*/
        p.setPen(Qt::black);
        p.drawText(tRect, Qt::AlignCenter, text);
    }
}

void FreeSpaceWidget::updateToolTip()
{
    if (isValid())
    {
        QString header = i18nc("@title", "Camera Media");

        if (d->mode == FreeSpaceWidget::AlbumLibrary)
        {
            header = i18nc("@title", "Album Library");
        }

        DToolTipStyleSheet cnt(ApplicationSettings::instance()->getToolTipsFont());
        QString tip = cnt.tipHeader;

        tip        += cnt.headBeg + header + cnt.headEnd;

        if (d->dSizeBytes > 0)
        {
            tip += cnt.cellBeg + i18nc("@info Storage", "Capacity:") + cnt.cellMid;
            tip += ItemPropertiesTab::humanReadableBytesCount(d->bytesSize) + cnt.cellEnd;

            tip += cnt.cellBeg + i18nc("@info Storage", "Available:") + cnt.cellMid;
            tip += ItemPropertiesTab::humanReadableBytesCount(d->bytesAvail) + cnt.cellEnd;

            tip += cnt.cellBeg + i18nc("@info Storage", "Require:") + cnt.cellMid;
            tip += ItemPropertiesTab::humanReadableBytesCount(d->dSizeBytes) + cnt.cellEnd;
        }
        else
        {
            tip += cnt.cellBeg + i18nc("@info Storage", "Capacity:") + cnt.cellMid;
            tip += ItemPropertiesTab::humanReadableBytesCount(d->bytesSize) + cnt.cellEnd;

            tip += cnt.cellBeg + i18nc("@info Storage", "Available:") + cnt.cellMid;
            tip += ItemPropertiesTab::humanReadableBytesCount(d->bytesAvail) + cnt.cellEnd;
        }

        tip += cnt.tipFooter;

        d->toolTip->setToolTip(tip);
    }
    else
    {
        d->toolTip->setToolTip(QString());
    }
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

void FreeSpaceWidget::enterEvent(QEnterEvent* e)

#else

void FreeSpaceWidget::enterEvent(QEvent* e)

#endif

{
    Q_UNUSED(e)

    d->toolTip->show();
}

void FreeSpaceWidget::leaveEvent(QEvent* e)
{
    Q_UNUSED(e)

    d->toolTip->hide();
}

void FreeSpaceWidget::slotTimeout()
{
    for (const QString& path : std::as_const(d->paths))
    {
        QStorageInfo info(path);

        if (info.isValid())
        {
            addInformation((qint64)(info.bytesTotal()),
                           (qint64)(info.bytesTotal() - info.bytesAvailable()),
                           (qint64)(info.bytesAvailable()),
                           info.rootPath());
        }
    }
}

} // namespace Digikam

#include "moc_freespacewidget.cpp"
