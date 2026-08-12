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

#include "versionsoverlays.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "iteminfo.h"
#include "itemhistorygraphmodel.h"
#include "itemlistmodel.h"
#include "itemviewhoverbutton.h"
#include "tagscache.h"
#include "versionmanagersettings.h"

namespace Digikam
{

class Q_DECL_HIDDEN ShowHideVersionsOverlay::Button : public ItemViewHoverButton
{
    Q_OBJECT

public:

    explicit Button(QAbstractItemView* const parentView);
    QSize sizeHint() const override;

protected:

    QIcon icon() override;
    void updateToolTip() override;
};

ShowHideVersionsOverlay::Button::Button(QAbstractItemView* const parentView)
    : ItemViewHoverButton(parentView)
{
    setup();
}

QSize ShowHideVersionsOverlay::Button::sizeHint() const
{
    return QSize(24, 24);
}

QIcon ShowHideVersionsOverlay::Button::icon()
{
    QString icon = isChecked() ? QLatin1String("edit-bomb")
                               : QLatin1String("edit-clear-history");
    return QIcon::fromTheme(icon);
}

void ShowHideVersionsOverlay::Button::updateToolTip()
{
    setToolTip(isChecked() ?
               i18nc("@info:tooltip", "Hide item permanently") :
               i18nc("@info:tooltip", "Show item permanently"));
}

// ------------------------------------------------------------------------------------

ShowHideVersionsOverlay::ShowHideVersionsOverlay(QObject* const parent)
    : HoverButtonDelegateOverlay(parent)
{
}

void ShowHideVersionsOverlay::setSettings(const VersionManagerSettings& settings)
{
    m_filter = VersionItemFilterSettings(settings);
}

void ShowHideVersionsOverlay::setActive(bool active)
{
    HoverButtonDelegateOverlay::setActive(active);

    if (active)
    {
        connect(button(), SIGNAL(clicked(bool)),
                this, SLOT(slotClicked(bool)));
    }
    else
    {
        // button is deleted
    }
}

ItemViewHoverButton* ShowHideVersionsOverlay::createButton()
{
    return new Button(view());
}

void ShowHideVersionsOverlay::updateButton(const QModelIndex& index)
{
    const QRect rect = m_view->visualRect(index);
    const QSize size = button()->size();

    const int gap = 5;
    const int x   = rect.right()  - gap - size.width();
    const int y   = rect.bottom() - gap - size.height();
    button()->move(QPoint(x, y));

    ItemInfo info = ItemModel::retrieveItemInfo(index);
    button()->setChecked(m_filter.isExemptedBySettings(info));
}

void ShowHideVersionsOverlay::slotClicked(bool checked)
{
    QModelIndex index = button()->index();

    if (index.isValid())
    {
        ItemInfo info = ItemModel::retrieveItemInfo(index);
        int tagId     = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::versionAlwaysVisible());

        if (checked)
        {
            info.setTag(tagId);
        }
        else
        {
            info.removeTag(tagId);
        }
    }
}

bool ShowHideVersionsOverlay::checkIndex(const QModelIndex& index) const
{
    if (index.data(ItemHistoryGraphModel::IsImageItemRole).toBool())
    {
        ItemInfo info = ItemModel::retrieveItemInfo(index);
        return m_filter.isHiddenBySettings(info);
    }

    return false;
}

// --------------------------------------------------------------------

class Q_DECL_HIDDEN ActionVersionsOverlay::Button : public ItemViewHoverButton
{
    Q_OBJECT

public:

    Button(QAbstractItemView* const parentView, const QIcon& icon, const QString& text, const QString& tip);
    QSize sizeHint() const override;

protected:

    QIcon icon() override;
    void updateToolTip() override;

protected:

    QIcon   m_icon;
    QString m_text;
    QString m_tip;
};

ActionVersionsOverlay::Button::Button(QAbstractItemView* const parentView, const QIcon& icon, const QString& text, const QString& tip)
    : ItemViewHoverButton(parentView),
      m_icon             (icon),
      m_text             (text),
      m_tip              (tip)
{
    setup();
}

QSize ActionVersionsOverlay::Button::sizeHint() const
{
    return QSize(24, 24);
}

QIcon ActionVersionsOverlay::Button::icon()
{
    return m_icon;
}

void ActionVersionsOverlay::Button::updateToolTip()
{
    setToolTip(m_tip);
}

// ------------------------------------------------------------------------------------------

ActionVersionsOverlay::ActionVersionsOverlay(QObject* const parent, const QIcon& icon, const QString& text, const QString& tip)
    : HoverButtonDelegateOverlay(parent),
      m_icon                    (icon),
      m_text                    (text),
      m_tip                     (tip)
{
}

ActionVersionsOverlay::Button* ActionVersionsOverlay::overlayButton() const
{
    return static_cast<Button*>(HoverButtonDelegateOverlay::button());
}

void ActionVersionsOverlay::setReferenceModel(const ItemModel* model)
{
    m_referenceModel = model;
}

void ActionVersionsOverlay::setActive(bool active)
{
    HoverButtonDelegateOverlay::setActive(active);

    if (active)
    {
        connect(button(), SIGNAL(clicked(bool)),
                this, SLOT(slotClicked(bool)));
    }
    else
    {
        // button is deleted
    }
}

ItemViewHoverButton* ActionVersionsOverlay::createButton()
{
    return new Button(view(), m_icon, m_text, m_tip);
}

void ActionVersionsOverlay::updateButton(const QModelIndex& index)
{
    const QRect rect = m_view->visualRect(index);
    const QSize size = button()->size();

    const int gap    = 5;
    const int x      = rect.right() - gap - size.width();
    const int y      = rect.top()   + gap;
    button()->move(QPoint(x, y));
}

void ActionVersionsOverlay::slotClicked(bool checked)
{
    Q_UNUSED(checked);

    QModelIndex index = button()->index();

    if (index.isValid())
    {
        Q_EMIT activated(ItemModel::retrieveItemInfo(index));
    }
}

bool ActionVersionsOverlay::checkIndex(const QModelIndex& index) const
{
    if (index.data(ItemHistoryGraphModel::IsImageItemRole).toBool())
    {
        if (m_referenceModel)
        {
            ItemInfo info = ItemModel::retrieveItemInfo(index);

            // show overlay if image is not contained in reference model

            return (!m_referenceModel->hasImage(info));
        }

        return true;
    }

    return false;
}

} // namespace Digikam

#include "versionsoverlays.moc"

#include "moc_versionsoverlays.cpp"
