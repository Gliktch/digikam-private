/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-08-03
 * Description : widget displaying all image versions in a list
 *
 * SPDX-FileCopyrightText: 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QModelIndex>
#include <QUrl>

// Local includes

#include "digikam_export.h"

class KConfigGroup;

namespace Digikam
{

class ActionVersionsOverlay;
class ItemInfo;
class ShowHideVersionsOverlay;
class VersionsDelegate;
class VersionsTreeView;

class VersionsWidget : public QWidget
{
    Q_OBJECT

public:

    explicit VersionsWidget(QWidget* const parent = nullptr);
    ~VersionsWidget() override;

    void readSettings(const KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    VersionsTreeView*        view()     const;
    VersionsDelegate*        delegate() const;

    ActionVersionsOverlay*   addActionOverlay(const QIcon& icon,
                                              const QString& text,
                                              const QString& tip = QString());
    ShowHideVersionsOverlay* addShowHideOverlay();

public Q_SLOTS:

    void setCurrentItem(const ItemInfo& info);

Q_SIGNALS:

    void imageSelected(const ItemInfo& info);

protected Q_SLOTS:

    void slotViewCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
    void slotViewModeChanged(int mode);
    void slotSetupChanged();

private:

    void applyViewMode();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
