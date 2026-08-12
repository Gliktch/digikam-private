/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-07-13
 * Description : Model for item versions
 *
 * SPDX-FileCopyrightText: 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QModelIndex>
#include <QPixmap>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT ItemVersionsModel : public QAbstractListModel
{
    Q_OBJECT

public:

    explicit ItemVersionsModel(QObject* const parent = nullptr);
    ~ItemVersionsModel()                                                           override;

    Qt::ItemFlags flags(const QModelIndex& index)                            const override;
    QVariant      data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int           rowCount(const QModelIndex& parent = QModelIndex())        const override;

    void setupModelData(QList<QPair<QString, int> >& data);
    void clearModelData();

    QString     currentSelectedImage()                                       const;
    void        setCurrentSelectedImage(const QString& path);
    QModelIndex currentSelectedImageIndex()                                  const;

    bool paintTree()                                                         const;
    int  listIndexOf(const QString& item)                                    const;

public Q_SLOTS:

    void slotAnimationStep();
    void setPaintTree(bool paint);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
