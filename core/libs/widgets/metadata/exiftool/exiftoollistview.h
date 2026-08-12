/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-04-18
 * Description : ExifTool metadata list view.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTreeWidget>
#include <QWidget>
#include <QString>
#include <QStringList>
#include <QUrl>

// Local includes

#include "exiftoolparser.h"
#include "searchtextbar.h"

namespace Digikam
{

class ExifToolListViewGroup;

class ExifToolListView : public QTreeWidget
{
    Q_OBJECT

public:

    explicit ExifToolListView(QWidget* const parent);
    ~ExifToolListView() override;

    void loadFromUrl(const QUrl& url);
    QString errorString() const;

    QString getCurrentItemKey() const;
    void    setCurrentItemByKey(const QString& itemKey);

    void setGroupList(const QStringList& tagsFilter,
                      const QStringList& keysFilter = QStringList());

Q_SIGNALS:

    void signalTextFilterMatch(bool);
    void signalLoadingResult(bool ok);

public Q_SLOTS:

    void slotSearchTextChanged(const SearchTextSettings&);

private Q_SLOTS:

    void slotExifToolAsyncData(const ExifToolParser::ExifToolData& map);
    void slotSelectionChanged(QTreeWidgetItem*, int);

private:

    ExifToolListViewGroup* findGroup(const QString& group);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
