/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-01-24
 * Description : Tags Action Manager - Keyboard shortcuts
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagsactionmngr_p.h"

namespace Digikam
{

QString TagsActionMngr::ratingShortcutPrefix() const
{
    return d->ratingShortcutPrefix;
}

QString TagsActionMngr::tagShortcutPrefix() const
{
    return d->tagShortcutPrefix;
}

QString TagsActionMngr::pickShortcutPrefix() const
{
    return d->pickShortcutPrefix;
}

bool TagsActionMngr::createRatingActionShortcut(KActionCollection* const ac, int rating)
{
    if (ac)
    {
        QAction* const action = ac->addAction(QString::fromUtf8("%1-%2")
                                                  .arg(d->ratingShortcutPrefix).arg(rating));

        action->setText(i18n("Assign Rating \"%1 Star\"", rating));

#ifdef Q_OS_MACOS

        // NOTE: Force to use Control Key under macOS, not the CMD translated by Qt map.

        ac->setDefaultShortcut(action, QKeySequence(QString::fromUtf8("CTRL+%1").arg(rating), QKeySequence::PortableText));

#else

        ac->setDefaultShortcut(action, QKeySequence(QString::fromUtf8("CTRL+%1").arg(rating)));

#endif

        action->setIcon(RatingWidget::buildIcon(rating, 32));
        action->setData(rating);

        connect(action, SIGNAL(triggered()),
                this, SLOT(slotAssignFromShortcut()));

        // Create shortcuts without toggling rating.

        if (rating > 0)
        {
            QAction* const actionNT = ac->addAction(QString::fromUtf8("%1%3-%2")
                                                        .arg(d->ratingShortcutPrefix).arg(rating)
                                                        .arg(d->noToggleShortcutPrefix));

            actionNT->setText(i18n("Assign Rating \"%1 Star\" (no toggle)", rating));
            actionNT->setIcon(RatingWidget::buildIcon(rating, 32));
            actionNT->setData(rating);

            connect(actionNT, SIGNAL(triggered()),
                    this, SLOT(slotAssignFromShortcut()));
        }

        return true;
    }

    return false;
}

bool TagsActionMngr::createPickLabelActionShortcut(KActionCollection* const ac, int pickId)
{
    if (ac)
    {
        QAction* const action = ac->addAction(QString::fromUtf8("%1-%2")
                                                  .arg(d->pickShortcutPrefix).arg(pickId));

        action->setText(i18n("Assign Pick Label \"%1\"",
                             PickLabelWidget::labelPickName((PickLabel)pickId)));
        ac->setDefaultShortcut(action, QKeySequence(QString::fromUtf8("ALT+%1").arg(pickId)));
        action->setIcon(PickLabelWidget::buildIcon((PickLabel)pickId));
        action->setData(pickId);

        connect(action, SIGNAL(triggered()),
                this, SLOT(slotAssignFromShortcut()));

        return true;
    }

    return false;
}

bool TagsActionMngr::createTagActionShortcut(int tagId)
{
    if (!tagId)
    {
        return false;
    }

    TAlbum* const talbum = AlbumManager::instance()->findTAlbum(tagId);

    if (!talbum)
    {
        return false;
    }

    QString value = TagsCache::instance()->propertyValue(tagId, TagPropertyName::tagKeyboardShortcut());

    if (value.isEmpty())
    {
        return false;
    }

    QKeySequence ks(value);

    // FIXME: tag icons can be files on disk, or system icon names. Only the latter will work here.

    QIcon     icon(SyncJob::getTagThumbnail(talbum));

    qCDebug(DIGIKAM_GENERAL_LOG) << "Create Shortcut " << ks.toString()
                                 << " to Tag " << talbum->title()
                                 << " (" << tagId << ")";

    for (KActionCollection* const ac : std::as_const(d->actionCollectionList))
    {
        QString actionName    = QString::fromUtf8("%1-%2").arg(d->tagShortcutPrefix).arg(tagId);
        QAction* const action = new QAction(i18n("Assign Tag \"%1\"", talbum->title()), this);
        action->setShortcut(ks);
        ac->addAction(actionName, action);
        ac->writeSettings(nullptr, true, action);
        action->setIcon(icon);
        action->setData(tagId);

        connect(action, SIGNAL(triggered()),
                this, SLOT(slotAssignFromShortcut()));

        connect(action, SIGNAL(changed()),
                this, SLOT(slotTagActionChanged()));

        d->tagsActionMap.insert(tagId, action);
    }

    return true;
}

void TagsActionMngr::updateTagShortcut(int tagId, const QKeySequence& ks, bool delAction)
{
    if (!tagId)
    {
        return;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Tag Shortcut " << tagId << "Changed to " << ks;

    QString value = TagsCache::instance()->propertyValue(tagId, TagPropertyName::tagKeyboardShortcut());

    if (value == ks.toString())
    {
        return;
    }

    TagProperties tprop(tagId);

    if (ks.isEmpty())
    {
        for (KActionCollection* const ac : std::as_const(d->actionCollectionList))
        {
            QString actionName    = QString::fromUtf8("%1-%2").arg(d->tagShortcutPrefix).arg(tagId);
            QAction* const action = ac->action(actionName);

            if (action)
            {
                action->setShortcut(QKeySequence());
                ac->writeSettings(nullptr, true, action);
            }
        }

        removeTagActionShortcut(tagId, delAction);
        tprop.removeProperties(TagPropertyName::tagKeyboardShortcut());
    }
    else
    {
        removeTagActionShortcut(tagId, delAction);
        tprop.setProperty(TagPropertyName::tagKeyboardShortcut(), ks.toString());
        createTagActionShortcut(tagId);
    }
}

bool TagsActionMngr::removeTagActionShortcut(int tagId, bool delAction)
{
    if (!d->tagsActionMap.contains(tagId))
    {
        return false;
    }

    const auto acs = d->tagsActionMap.values(tagId);

    for (QAction* const act : acs)
    {
        if (act)
        {
            KActionCollection* const ac = dynamic_cast<KActionCollection*>(act->parent());

            if (ac)
            {
                ac->takeAction(act);
            }

            if (delAction)
            {
                delete act;
            }
        }
    }

    d->tagsActionMap.remove(tagId);

    return true;
}

void TagsActionMngr::slotAssignFromShortcut()
{
    QAction* const action = dynamic_cast<QAction*>(sender());

    if (!action)
    {
        return;
    }

    int val                      = action->data().toInt();
    qCDebug(DIGIKAM_GENERAL_LOG) << "Shortcut value: " << val;

    QWidget* const w             = qApp->activeWindow();
    const DigikamApp* const dkw  = dynamic_cast<DigikamApp*>(w);

    if (dkw)
    {
        //qCDebug(DIGIKAM_GENERAL_LOG) << "Handling by DigikamApp";

        if      (action->objectName().startsWith(d->ratingShortcutPrefix))
        {
            bool toggle = !action->objectName().contains(d->noToggleShortcutPrefix);
            dkw->view()->slotAssignRating(val, toggle);
        }
        else if (action->objectName().startsWith(d->pickShortcutPrefix))
        {
            dkw->view()->slotAssignPickLabel(val);
        }
        else if (action->objectName().startsWith(d->colorShortcutPrefix))
        {
            dkw->view()->slotAssignColorLabel(val);
        }
        else if (action->objectName().startsWith(d->tagShortcutPrefix))
        {
            dkw->view()->toggleTag(val);
        }

        return;
    }

    ImageWindow* const imw = dynamic_cast<ImageWindow*>(w);

    if (imw)
    {
        //qCDebug(DIGIKAM_GENERAL_LOG) << "Handling by ImageWindow";

        if      (action->objectName().startsWith(d->ratingShortcutPrefix))
        {
            imw->slotAssignRating(val);
        }
        else if (action->objectName().startsWith(d->pickShortcutPrefix))
        {
            imw->slotAssignPickLabel(val);
        }
        else if (action->objectName().startsWith(d->colorShortcutPrefix))
        {
            imw->slotAssignColorLabel(val);
        }
        else if (action->objectName().startsWith(d->tagShortcutPrefix))
        {
            imw->toggleTag(val);
        }

        return;
    }

    LightTableWindow* const ltw = dynamic_cast<LightTableWindow*>(w);

    if (ltw)
    {
        //qCDebug(DIGIKAM_GENERAL_LOG) << "Handling by LightTableWindow";

        if      (action->objectName().startsWith(d->ratingShortcutPrefix))
        {
            ltw->slotAssignRating(val);
        }
        else if (action->objectName().startsWith(d->pickShortcutPrefix))
        {
            ltw->slotAssignPickLabel(val);
        }
        else if (action->objectName().startsWith(d->colorShortcutPrefix))
        {
            ltw->slotAssignColorLabel(val);
        }
        else if (action->objectName().startsWith(d->tagShortcutPrefix))
        {
            ltw->toggleTag(val);
        }

        return;
    }

    SurveyWindow* const svw = dynamic_cast<SurveyWindow*>(w);

    if (svw)
    {
        //qCDebug(DIGIKAM_GENERAL_LOG) << "Handling by SurveyWindow";

        if      (action->objectName().startsWith(d->ratingShortcutPrefix))
        {
            bool toggle = !action->objectName().contains(d->noToggleShortcutPrefix);
            svw->slotAssignRating(val, toggle);
        }
        else if (action->objectName().startsWith(d->pickShortcutPrefix))
        {
            svw->slotAssignPickLabel(val);
        }
        else if (action->objectName().startsWith(d->colorShortcutPrefix))
        {
            svw->slotAssignColorLabel(val);
        }
        else if (action->objectName().startsWith(d->tagShortcutPrefix))
        {
            svw->toggleTag(val);
        }

        return;
    }

    // Emit signal to DInfoInterface to broadcast to another component:

    Q_EMIT signalShortcutPressed(action->objectName(), val);
}

} // namespace Digikam
