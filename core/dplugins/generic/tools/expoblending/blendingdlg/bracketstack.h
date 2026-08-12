/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-13
 * Description : a tool to blend bracketed images.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTreeWidget>
#include <QString>
#include <QPixmap>
#include <QUrl>

// Local includes

#include "loadingdescription.h"

using namespace Digikam;

namespace DigikamGenericExpoBlendingPlugin
{

class BracketStackItem : public QTreeWidgetItem
{
public:

    explicit BracketStackItem(QTreeWidget* const parent);
    ~BracketStackItem()                                   override = default;

    void setUrl(const QUrl& _url);
    const QUrl& url()                               const;

    void setOn(bool b);
    bool isOn()                                     const;

    void setThumbnail(const QPixmap& pix);
    void setExposure(const QString& exp);

private:

    bool operator< (const QTreeWidgetItem& other)   const override;

private:

    QUrl m_url;

private:

    Q_DISABLE_COPY(BracketStackItem)
};

// ---------------------------------------------------------------------

class BracketStackList : public QTreeWidget
{
    Q_OBJECT

public:

    explicit BracketStackList(QWidget* const parent);
    ~BracketStackList() override = default;

    void addItems(const QList<QUrl>& list);

    QList<QUrl> urls();
    BracketStackItem* findItem(const QUrl& url);

Q_SIGNALS:

    void signalAddItems(const QList<QUrl>&);
    void signalItemClicked(const QUrl& url);

private Q_SLOTS:

    void slotThumbnail(const LoadingDescription&, const QPixmap& pix);
    void slotItemClicked(QTreeWidgetItem* item, int column);
};

} // namespace DigikamGenericExpoBlendingPlugin
