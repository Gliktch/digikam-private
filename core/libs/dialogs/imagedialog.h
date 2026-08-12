/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-03-13
 * Description : Image files selection dialog.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QScrollArea>
#include <QFileIconProvider>
#include <QIcon>
#include <QImage>
#include <QModelIndex>
#include <QFileInfo>
#include <QAbstractItemView>

// Local includes

#include "digikam_export.h"
#include "ditemtooltip.h"

namespace Digikam
{

class LoadingDescription;
class ThumbnailImageCatcher;

class DIGIKAM_EXPORT ImageDialogPreview : public QScrollArea
{
    Q_OBJECT

public:

    explicit ImageDialogPreview(QWidget* const parent = nullptr);
    ~ImageDialogPreview() override;

    QSize sizeHint() const override;

    static QString identifyItem(const QUrl& url, const QImage& preview = QImage());

public Q_SLOTS:

    void slotShowPreview(const QUrl& url);

private Q_SLOTS:

    void showPreview();
    void slotClearPreview();
    void slotThumbnail(const LoadingDescription& desc, const QPixmap& pix);

private:

    void resizeEvent(QResizeEvent* e) override;

private:

    class Private;
    Private* const d = nullptr;
};

// ------------------------------------------------------------------------

class ImageDialogToolTip : public DItemToolTip
{
    Q_OBJECT

public:

    explicit ImageDialogToolTip();
    ~ImageDialogToolTip()                 override;

    void setData(QAbstractItemView* const view,
                 const QModelIndex& index,
                 const QUrl& url);

private:

    QRect   repositionRect()              override;
    QString tipContents()                 override;

    /// @note disabled
    explicit ImageDialogToolTip(QWidget*);

private:

    class Private;
    Private* const d = nullptr;
};

// ------------------------------------------------------------------------

class DIGIKAM_EXPORT ImageDialogIconProvider : public QObject,
                                               public QFileIconProvider
{
    Q_OBJECT

public:

    explicit ImageDialogIconProvider();
    ~ImageDialogIconProvider()                                 override;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    QIcon icon(QAbstractFileIconProvider::IconType type) const override;

#else

    QIcon icon(QFileIconProvider::IconType type)         const override;

#endif

    QIcon icon(const QFileInfo& info)                    const override;

Q_SIGNALS:

    void signalThumbnailRefresh();

private Q_SLOTS:

    void slotThumbnailLoaded(const LoadingDescription& desc, const QImage& img);

private:

    /// @note disabled
    explicit ImageDialogIconProvider(QObject*);

private:

    class Private;
    Private* const d = nullptr;
};

// ------------------------------------------------------------------------

class DIGIKAM_EXPORT ImageDialog : public QObject
{
    Q_OBJECT

public:

    explicit ImageDialog(QWidget* const parent,
                         const QUrl& url,
                         bool singleSelect = false,
                         const QString& caption = QString());
    ~ImageDialog();

    QUrl        url()                                           const;
    QList<QUrl> urls()                                          const;
    QStringList fileFormats()                                   const;

    void setEnableToolTips(bool val);

    static QUrl        getImageURL(QWidget* const parent,
                                   const QUrl& url,
                                   const QString& caption = QString());

    static QList<QUrl> getImageURLs(QWidget* const parent,
                                    const QUrl& url,
                                    const QString& caption = QString());

private:

    bool eventFilter(QObject* obj, QEvent* ev);

    void hideToolTip();
    bool acceptToolTip(const QUrl& url)                         const;

    /// @note disabled
    ImageDialog(const ImageDialog&)            = delete;
    ImageDialog& operator=(const ImageDialog&) = delete;

private Q_SLOTS:

    void slotToolTip();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
