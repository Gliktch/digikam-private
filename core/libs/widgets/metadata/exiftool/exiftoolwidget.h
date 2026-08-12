/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-04-18
 * Description : ExifTool metadata widget.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QStackedWidget>
#include <QString>
#include <QUrl>
#include <QAction>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ExifToolWidget : public QStackedWidget
{
    Q_OBJECT

public:

    enum TagFilters
    {
        NONE = 0,
        PHOTO,
        CUSTOM
    };

public:

    explicit ExifToolWidget(QWidget* const parent);
    ~ExifToolWidget() override;

    void loadFromUrl(const QUrl& url);

    QString getCurrentItemKey() const;
    void    setCurrentItemByKey(const QString& itemKey);

    QStringList getTagsFilter() const;
    void        setTagsFilter(const QStringList& list);

    int     getMode()           const;
    void    setMode(int mode);

Q_SIGNALS:

    void signalSetupExifTool();
    void signalSetupMetadataFilters();

private Q_SLOTS:

    void slotLoadingResult(bool ok);
    void slotPreLoadingTimerDone();
    void slotCopy2Clipboard();
    void slotPrintMetadata();
    void slotSaveMetadataToFile();
    void slotFilterChanged(QAction*);

private:

    void setup();
    QString metadataToText() const;
    void buildView();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
