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
#include <QPoint>
#include <QList>
#include <QIcon>

// Local includes

#include "enfusesettings.h"

namespace DigikamGenericExpoBlendingPlugin
{

class EnfuseStackItem : public QTreeWidgetItem
{
public:

    explicit EnfuseStackItem(QTreeWidget* const parent);
    ~EnfuseStackItem()                    override;

    /**
     * Return the preview image url assigned to item.
     */
    const QUrl& url()               const;

    void setEnfuseSettings(const EnfuseSettings& settings);
    EnfuseSettings enfuseSettings() const;

    void setOn(bool b);
    bool isOn()                     const;

    void setProgressAnimation(const QPixmap& pix);
    void setThumbnail(const QPixmap& pix);
    void setProcessedIcon(const QIcon& icon);
    bool asValidThumb()             const;

private:

    class Private;
    Private* const d = nullptr;

private:

    Q_DISABLE_COPY(EnfuseStackItem)
};

// ---------------------------------------------------------------------

class EnfuseStackList : public QTreeWidget
{
    Q_OBJECT

public:

    explicit EnfuseStackList(QWidget* const parent);
    ~EnfuseStackList() override;

    void setTemplateFileName(DSaveSettingsWidget::OutputFormat, const QString&);

    void setThumbnail(const QUrl& url, const QImage& img);
    void setOnItem(const QUrl& url, bool on);
    void removeItem(const QUrl& url);
    void clearSelected();
    void addItem(const QUrl& url, const EnfuseSettings& settings);
    void processingItem(const QUrl& url, bool run);
    void processedItem(const QUrl& url, bool success);

    QList<EnfuseSettings> settingsList();

Q_SIGNALS:

    void signalItemClicked(const QUrl&);

private:

    EnfuseStackItem* findItemByUrl(const QUrl& url);

private Q_SLOTS:

    void slotItemClicked(QTreeWidgetItem*);
    void slotContextMenu(const QPoint&);
    void slotRemoveItem();
    void slotProgressTimerDone();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericExpoBlendingPlugin
