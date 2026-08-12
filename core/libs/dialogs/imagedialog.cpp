/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-03-13
 * Description : Image files selection dialog - Improved Qt based file dialog.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "imagedialog_p.h"

namespace Digikam
{

ImageDialog::ImageDialog(QWidget* const parent, const QUrl& url,
                         bool singleSelect, const QString& caption)
    : QObject(parent),
      d      (new Private)
{
    QString all;
    d->fileFormats  = supportedImageMimeTypes(QIODevice::ReadOnly, all);
    qCDebug(DIGIKAM_GENERAL_LOG) << "file formats=" << d->fileFormats;

    d->toolTip      = new ImageDialogToolTip();
    d->toolTipTimer = new QTimer(this);

    connect(d->toolTipTimer, SIGNAL(timeout()),
            this, SLOT(slotToolTip()));

    d->dlg          = new DFileDialog(parent);
    d->dlg->setWindowTitle(caption);
    d->dlg->setDirectoryUrl(url);
    d->dlg->setNameFilters(d->fileFormats);
    d->dlg->selectNameFilter(d->fileFormats.last());
    d->dlg->setAcceptMode(QFileDialog::AcceptOpen);
    d->dlg->setFileMode(singleSelect ? QFileDialog::ExistingFile
                                     : QFileDialog::ExistingFiles);

    if ((d->dlg->options() & QFileDialog::DontUseNativeDialog) || !isRunningOnNativeKDE())
    {
        d->provider = new ImageDialogIconProvider();
        d->dlg->setIconProvider(d->provider);

        connect(d->provider, &ImageDialogIconProvider::signalThumbnailRefresh,
                this, [this]()
            {
                d->dlg->setIconProvider(d->provider);
            }
        );

        const auto items = d->dlg->findChildren<QAbstractItemView*>();

        for (auto* item : items)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << item;
            item->installEventFilter(this);
            item->setMouseTracking(true);
        }
    }

    d->dlg->exec();

    if (d->dlg && d->dlg->hasAcceptedUrls())
    {
        d->urls = d->dlg->selectedUrls();
    }
}

ImageDialog::~ImageDialog()
{
    delete d->toolTip;
    delete d->dlg;
    delete d->provider;
    delete d;
}

void ImageDialog::setEnableToolTips(bool val)
{
    d->showToolTips = val;

    if (!val)
    {
        hideToolTip();
    }
}

void ImageDialog::hideToolTip()
{
    d->toolTipIndex = QModelIndex();
    d->toolTipTimer->stop();
    slotToolTip();
}

void ImageDialog::slotToolTip()
{
    d->toolTip->setData(d->toolTipView, d->toolTipIndex, d->toolTipUrl);
}

bool ImageDialog::acceptToolTip(const QUrl& url) const
{
    if (url.isValid())
    {
        QFileInfo info(url.toLocalFile());

        if (info.isFile() && !info.isSymLink() && !info.isRoot())
        {
            return (!info.absoluteFilePath().isEmpty());
        }
    }

    return false;
}

bool ImageDialog::eventFilter(QObject* obj, QEvent* ev)
{
    if (d->dlg)
    {
        QAbstractItemView* const view = dynamic_cast<QAbstractItemView*>(obj);

        if (view)
        {
            if      ((ev->type() == QEvent::HoverMove) && (qApp->mouseButtons() == Qt::NoButton))
            {
                const QHoverEvent* const hev = dynamic_cast<QHoverEvent*>(ev);

                if (hev)
                {
                    QModelIndex index = view->indexAt(view->viewport()->mapFromGlobal(QCursor::pos()));

                    if (index.isValid())
                    {
                        QString name = index.data(Qt::DisplayRole).toString();

                        if (!name.isEmpty())
                        {
                            QUrl localUrl = QUrl::fromLocalFile(QDir::fromNativeSeparators(d->dlg->directoryUrl().toLocalFile() +
                                                                                           QLatin1Char('/') + name));

                            if (d->showToolTips)
                            {
                                if (!d->dlg->isActiveWindow())
                                {
                                    hideToolTip();

                                    return false;
                                }

                                if (index != d->toolTipIndex)
                                {
                                    hideToolTip();

                                    if (acceptToolTip(localUrl))
                                    {
                                        d->toolTipView  = view;
                                        d->toolTipIndex = index;
                                        d->toolTipUrl   = localUrl;
                                        d->toolTipTimer->setSingleShot(true);
                                        d->toolTipTimer->start(500);
                                    }
                                }

                                if ((index == d->toolTipIndex) && !acceptToolTip(localUrl))
                                {
                                    hideToolTip();
                                }
                            }

                            return false;
                        }
                    }
                    else
                    {
                         hideToolTip();

                         return false;
                    }
                }
            }
            else if (
                     (ev->type() == QEvent::HoverLeave) ||
                     (ev->type() == QEvent::FocusOut)   ||
                     (ev->type() == QEvent::Wheel)      ||
                     (ev->type() == QEvent::KeyPress)   ||
                     (ev->type() == QEvent::Paint)
                    )
            {
                hideToolTip();

                return false;
            }
        }
    }

    // Pass the event on to the parent class.

    return QObject::eventFilter(obj, ev);
}

QStringList ImageDialog::fileFormats() const
{
    return d->fileFormats;
}

QUrl ImageDialog::url() const
{
    if (d->urls.isEmpty())
    {
        return QUrl();
    }

    return d->urls.first();
}

QList<QUrl> ImageDialog::urls() const
{
    return d->urls;
}

QUrl ImageDialog::getImageURL(QWidget* const parent, const QUrl& url, const QString& caption)
{
    ImageDialog dlg(parent, url, true, caption.isEmpty() ? i18n("Select an Item") : caption);

    if (dlg.url() != QUrl())
    {
        return dlg.url();
    }
    else
    {
        return QUrl();
    }
}

QList<QUrl> ImageDialog::getImageURLs(QWidget* const parent, const QUrl& url, const QString& caption)
{
    ImageDialog dlg(parent, url, false, caption.isEmpty() ? i18n("Select Items") : caption);

    if (!dlg.urls().isEmpty())
    {
        return dlg.urls();
    }
    else
    {
        return QList<QUrl>();
    }
}

} // namespace Digikam

#include "moc_imagedialog.cpp"
