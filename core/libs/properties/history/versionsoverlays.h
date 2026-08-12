/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-12-26
 * Description : images versions tree view overlays
 *
 * SPDX-FileCopyrightText: 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
 * SPDX-FileCopyrightText: 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include <QString>
#include <QIcon>

// Local includes

#include "digikam_export.h"
#include "itemdelegateoverlay.h"
#include "itemfiltersettings.h"

namespace Digikam
{

class ItemInfo;
class ItemModel;
class VersionManagerSettings;

class ShowHideVersionsOverlay : public HoverButtonDelegateOverlay
{
    Q_OBJECT

public:

    explicit ShowHideVersionsOverlay(QObject* const parent);
    void setActive(bool active)                     override;

    void setSettings(const VersionManagerSettings& settings);

protected:

    ItemViewHoverButton* createButton()             override;
    void updateButton(const QModelIndex& index)     override;
    bool checkIndex(const QModelIndex& index) const override;

protected Q_SLOTS:

    void slotClicked(bool checked);

protected:

    VersionItemFilterSettings m_filter;
    class Button;
};

// -------------------------------------------------------------------

class ActionVersionsOverlay : public HoverButtonDelegateOverlay
{
    Q_OBJECT

public:

    explicit ActionVersionsOverlay(QObject* const parent,
                                   const QIcon& icon,
                                   const QString& text,
                                   const QString& tip = QString());
    void setActive(bool active)                     override;

    void setReferenceModel(const ItemModel* model);

Q_SIGNALS:

    void activated(const ItemInfo& info);

protected:

    ItemViewHoverButton* createButton()             override;
    void updateButton(const QModelIndex& index)     override;
    bool checkIndex(const QModelIndex& index) const override;

protected Q_SLOTS:

    void slotClicked(bool checked);

protected:

    class Button;
    Button* overlayButton()                   const;

protected:

    QIcon            m_icon;
    QString          m_text;
    QString          m_tip;
    const ItemModel* m_referenceModel = nullptr;
};

} // namespace Digikam
