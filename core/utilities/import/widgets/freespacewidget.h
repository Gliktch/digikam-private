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

#pragma once

// Qt includes

#include <QString>
#include <QWidget>

namespace Digikam
{

class FreeSpaceWidget : public QWidget
{
    Q_OBJECT

public:

    enum FreeSpaceMode
    {
        AlbumLibrary = 0,
        UMSCamera,
        GPhotoCamera
    };

public:

    explicit FreeSpaceWidget(QWidget* const parent, int width);
    ~FreeSpaceWidget()                                override;

    void   setEstimatedDSizeBytes(qint64 dSize);
    qint64 estimatedDSizeBytes()                const;

    bool   isValid()                            const;
    int    percentUsed()                        const;
    qint64 bytesSize()                          const;
    qint64 bytesUsed()                          const;
    qint64 bytesAvail()                         const;
    qint64 bytesAvail(const QString& path)      const;

    void          refresh();

    void setMode(FreeSpaceMode mode);
    void setPath(const QString& path);
    void setPaths(const QStringList& paths);

    void addInformation(qint64 _bytesSize,
                        qint64 _bytesUsed,
                        qint64 _bytesAvail,
                        const QString& _mountPoint);

protected:

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    void enterEvent(QEnterEvent*)                     override;

#else

    void enterEvent(QEvent*)                          override;

#endif

    void leaveEvent(QEvent*)                          override;
    void paintEvent(QPaintEvent*)                     override;
    void updateToolTip();

private Q_SLOTS:

    void slotTimeout();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
