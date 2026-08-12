/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-01-24
 * Description : Tags Action Manager - Actions
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagsactionmngr_p.h"

namespace Digikam
{

void TagsActionMngr::registerTagsActionCollections()
{
    d->actionCollectionList.append(DigikamApp::instance()->actionCollection());
    d->actionCollectionList.append(ImageWindow::imageWindow()->actionCollection());
    d->actionCollectionList.append(LightTableWindow::lightTableWindow()->actionCollection());

    // Create Tags shortcuts.

    QList<int> tagIds = TagsCache::instance()->tagsWithProperty(TagPropertyName::tagKeyboardShortcut());

    for (int tagId : std::as_const(tagIds))
    {
        createTagActionShortcut(tagId);
    }

    Q_EMIT signalTagsActionCollectionsRegistered();
}

QList<KActionCollection*> TagsActionMngr::actionCollections() const
{
    return d->actionCollectionList;
}

void TagsActionMngr::registerLabelsActions(KActionCollection* const ac)
{
    // Create Rating shortcuts.

    for (int i = RatingMin ; i <= RatingMax ; ++i)
    {
        createRatingActionShortcut(ac, i);
    }

    // Create Color Label shortcuts.

    for (int i = NoColorLabel ; i <= WhiteLabel ; ++i)
    {
        createColorLabelActionShortcut(ac, i);
    }

    // Create Pick Label shortcuts.

    for (int i = NoPickLabel ; i <= AcceptedLabel ; ++i)
    {
        createPickLabelActionShortcut(ac, i);
    }
}

void TagsActionMngr::registerActionsToWidget(QWidget* const wdg)
{
    DXmlGuiWindow* const win = dynamic_cast<DXmlGuiWindow*>(qApp->activeWindow());

    if (win)
    {
        const auto acs = win->actionCollection()->actions();

        for (QAction* const ac : acs)
        {
            if (
                ac->objectName().startsWith(d->ratingShortcutPrefix) ||
                ac->objectName().startsWith(d->tagShortcutPrefix)    ||
                ac->objectName().startsWith(d->pickShortcutPrefix)   ||
                ac->objectName().startsWith(d->colorShortcutPrefix)
               )
            {
                wdg->addAction(ac);
            }
        }
    }
}

void TagsActionMngr::slotTagActionChanged()
{
    QAction* const action = dynamic_cast<QAction*>(sender());

    if (!action)
    {
        return;
    }

    int tagId       = action->data().toInt();

    QKeySequence ks;
    QStringList lst = action->shortcut().toString().split(QLatin1Char(','));

    if (!lst.isEmpty())
    {
        ks = QKeySequence(lst.first());
    }

    updateTagShortcut(tagId, ks, false);
}

} // namespace Digikam
