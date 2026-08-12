/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-03-09
 * Description : Captions, Tags, and Rating properties editor
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2003-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemdescedittab_p.h"

namespace Digikam
{

ItemDescEditTab::ItemDescEditTab(QWidget* const parent)
    : DVBox(parent),
      d    (new Private(this))
{
    d->hub                 = new DisjointMetadata;

    setContentsMargins(QMargins());
    setSpacing(d->spacing);

    d->tabWidget           = new QTabWidget(this);

    d->metadataChangeTimer = new QTimer(this);
    d->metadataChangeTimer->setSingleShot(true);
    d->metadataChangeTimer->setInterval(250);

    // Captions/Date/Rating view -----------------------------------

    initDescriptionView();

    // Tags view ---------------------------------------------------

    initTagsView();

    // Information Management View ---------------------------------

    initInformationView();

    // Initialize --------------------------------------------------

    d->setupEventFilters();

    updateRecentTags();

    // Setup signals/slots connections -----------------------------

    d->setupConnections();
}

ItemDescEditTab::~ItemDescEditTab()
{
    delete d->hub;
    delete d;
}

void ItemDescEditTab::setCurrentTab(int index)
{
    d->tabWidget->setCurrentIndex(index);
}

int ItemDescEditTab::currentTab() const
{
    return d->tabWidget->currentIndex();
}

void ItemDescEditTab::readSettings(KConfigGroup& group)
{
    setCurrentTab(group.readEntry(QLatin1String("ItemDescEdit Tab"), (int)DESCRIPTIONS));
    d->titleEdit->setCurrentLanguageCode(group.readEntry(QLatin1String("ItemDescEditTab TitleLang"), QString()));
    d->captionsEdit->setCurrentLanguageCode(group.readEntry(QLatin1String("ItemDescEditTab CaptionsLang"), QString()));

    d->templateViewer->readSettings(group);

    d->tagCheckView->setConfigGroup(group);
    d->tagCheckView->setEntryPrefix(QLatin1String("ItemDescEditTab TagCheckView"));
    d->tagCheckView->loadState();
    d->tagsSearchBar->setConfigGroup(group);
    d->tagsSearchBar->setEntryPrefix(QLatin1String("ItemDescEditTab SearchBar"));
    d->tagsSearchBar->loadState();
}

void ItemDescEditTab::writeSettings(KConfigGroup& group)
{
    group.writeEntry(QLatin1String("ItemDescEdit Tab"),             currentTab());
    group.writeEntry(QLatin1String("ItemDescEditTab TitleLang"),    d->titleEdit->currentLanguageCode());
    group.writeEntry(QLatin1String("ItemDescEditTab CaptionsLang"), d->captionsEdit->currentLanguageCode());

    d->templateViewer->writeSettings(group);

    d->tagCheckView->saveState();
    d->tagsSearchBar->saveState();
}

void ItemDescEditTab::setItem(const ItemInfo& info)
{
    slotChangingItems();
    ItemInfoList list;

    if (!info.isNull())
    {
        list << info;
    }

    d->setInfos(list);
}

void ItemDescEditTab::setItems(const ItemInfoList& infos)
{
    slotChangingItems();
    d->setInfos(infos);
}

bool ItemDescEditTab::eventFilter(QObject* o, QEvent* e)
{
    if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent* const k = static_cast<QKeyEvent*>(e);

        if ((k->key() == Qt::Key_Enter) || (k->key() == Qt::Key_Return))
        {
            if      (k->modifiers() == Qt::ControlModifier)
            {
                d->lastSelectedWidget = qobject_cast<QWidget*>(o);

                Q_EMIT signalNextItem();

                return true;
            }
            else if (k->modifiers() == Qt::ShiftModifier)
            {
                d->lastSelectedWidget = qobject_cast<QWidget*>(o);

                Q_EMIT signalPrevItem();

                return true;
            }
        }

        if (k->key() == Qt::Key_PageUp)
        {
            d->lastSelectedWidget = qobject_cast<QWidget*>(o);

            Q_EMIT signalPrevItem();

            return true;
        }

        if (k->key() == Qt::Key_PageDown)
        {
            d->lastSelectedWidget = qobject_cast<QWidget*>(o);

            Q_EMIT signalNextItem();

            return true;
        }

        if ((d->newTagEdit == o) &&
            !d->newTagEdit->completer()->popup()->isVisible())
        {
            if (k->key() == Qt::Key_Up)
            {
                d->lastSelectedWidget = qobject_cast<QWidget*>(o);

                Q_EMIT signalPrevItem();

                return true;
            }

            if (k->key() == Qt::Key_Down)
            {
                d->lastSelectedWidget = qobject_cast<QWidget*>(o);

                Q_EMIT signalNextItem();

                return true;
            }
        }
    }

    return DVBox::eventFilter(o, e);
}

bool ItemDescEditTab::isModified() const
{
    return d->modified;
}

void ItemDescEditTab::slotChangingItems()
{
    if (!d->modified)
    {
        return;
    }

    if (d->currInfos.isEmpty())
    {
        return;
    }

    if (!ApplicationSettings::instance()->getApplySidebarChangesDirectly())
    {
        // Open dialog via queued connection out-of-scope, see bug 302311

        DisjointMetadata* const hub2 = new DisjointMetadata();
        hub2->setDataFields(d->hub->dataFields());

        Q_EMIT signalAskToApplyChanges(d->currInfos, hub2);

        d->reset();
    }
    else
    {
        slotApplyAllChanges();
    }
}

void ItemDescEditTab::slotAskToApplyChanges(const QList<ItemInfo>& infos, DisjointMetadata* hub)
{
    int changedFields = 0;

    if (hub->titlesChanged())
    {
        ++changedFields;
    }

    if (hub->commentsChanged())
    {
        ++changedFields;
    }

    if (hub->dateTimeChanged())
    {
        ++changedFields;
    }

    if (hub->ratingChanged())
    {
        ++changedFields;
    }

    if (hub->pickLabelChanged())
    {
        ++changedFields;
    }

    if (hub->colorLabelChanged())
    {
        ++changedFields;
    }

    if (hub->tagsChanged())
    {
        ++changedFields;
    }

    QString text;

    if (changedFields == 1)
    {
        if      (hub->commentsChanged())
        {
            text = i18ncp("@info", "You have edited the image caption. ",
                          "You have edited the captions of %1 images. ",
                          infos.count());
        }
        else if (hub->titlesChanged())
        {
            text = i18ncp("@info", "You have edited the image title. ",
                          "You have edited the titles of %1 images. ",
                          infos.count());
        }
        else if (hub->dateTimeChanged())
        {
            text = i18ncp("@info", "You have edited the date of the image. ",
                          "You have edited the date of %1 images. ",
                          infos.count());
        }
        else if (hub->pickLabelChanged())
        {
            text = i18ncp("@info", "You have edited the pick label of the image. ",
                          "You have edited the pick label of %1 images. ",
                          infos.count());
        }
        else if (hub->colorLabelChanged())
        {
            text = i18ncp("@info", "You have edited the color label of the image. ",
                          "You have edited the color label of %1 images. ",
                          infos.count());
        }
        else if (hub->ratingChanged())
        {
            text = i18ncp("@info", "You have edited the rating of the image. ",
                          "You have edited the rating of %1 images. ",
                          infos.count());
        }
        else if (hub->tagsChanged())
        {
            text = i18ncp("@info", "You have edited the tags of the image. ",
                          "You have edited the tags of %1 images. ",
                          infos.count());
        }

        text += i18nc("@info", "Do you want to apply your changes?");
    }
    else
    {
        text = i18ncp("@info", "<p>You have edited the metadata of the image: </p>",
                      "<p>You have edited the metadata of %1 images: </p>",
                      infos.count());

        text += QLatin1String("<p><ul>");

        if (hub->titlesChanged())
        {
            text += i18nc("@info", "<li>title</li>");
        }

        if (hub->commentsChanged())
        {
            text += i18nc("@info", "<li>caption</li>");
        }

        if (hub->dateTimeChanged())
        {
            text += i18nc("@info", "<li>date</li>");
        }

        if (hub->pickLabelChanged())
        {
            text += i18nc("@info", "<li>pick label</li>");
        }

        if (hub->colorLabelChanged())
        {
            text += i18nc("@info", "<li>color label</li>");
        }

        if (hub->ratingChanged())
        {
            text += i18nc("@info", "<li>rating</li>");
        }

        if (hub->tagsChanged())
        {
            text += i18nc("@info", "<li>tags</li>");
        }

        text += QLatin1String("</ul></p>");

        text += i18nc("@info", "<p>Do you want to apply your changes?</p>");
    }

    QCheckBox* const alwaysCBox  = new QCheckBox(i18nc("@action", "Always apply changes without confirmation"));

    QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Information,
                                                   i18nc("@title:window", "Apply Changes?"),
                                                   text,
                                                   QMessageBox::Yes | QMessageBox::No,
                                                   qApp->activeWindow());
    msgBox->setCheckBox(alwaysCBox);
    msgBox->setDefaultButton(QMessageBox::No);
    msgBox->setEscapeButton(QMessageBox::No);

    // Pop-up a message in desktop notification manager

    DNotificationWrapper(QString(), i18nc("@info", "Apply changes?"),
                         DigikamApp::instance(), DigikamApp::instance()->windowTitle());

    int returnCode   = msgBox->exec();
    bool alwaysApply = msgBox->checkBox()->isChecked();
    delete msgBox;

    if (alwaysApply)
    {
        ApplicationSettings::instance()->setApplySidebarChangesDirectly(true);
    }

    if (returnCode == QMessageBox::No)
    {
        delete hub;

        return;
    }

    // otherwise apply:

    FileActionMngr::instance()->applyMetadata(infos, hub);
}

void ItemDescEditTab::slotApplyAllChanges()
{
    if (!d->modified)
    {
        return;
    }

    if (d->currInfos.isEmpty())
    {
        return;
    }

    FileActionMngr::instance()->applyMetadata(d->currInfos, *d->hub);
    d->reset();
}

void ItemDescEditTab::slotRevertAllChanges()
{
    if (!d->modified)
    {
        return;
    }

    if (d->currInfos.isEmpty())
    {
        return;
    }

    d->setInfos(d->currInfos);

    Q_EMIT signalRightSideBarBusy(d->modified);
}

void ItemDescEditTab::slotReadFromFileMetadataToDatabase()
{
    d->initProgressIndicator();

    Q_EMIT signalProgressMessageChanged(i18nc("@info", "Reading metadata from files. Please wait..."));

    d->ignoreItemAttributesWatch = true;
    int i                        = 0;

    ScanController::instance()->suspendCollectionScan();

    CollectionScanner scanner;

    for (const ItemInfo& info : std::as_const(d->currInfos))
    {
        scanner.scanFile(info, CollectionScanner::CleanScan);

        Q_EMIT signalProgressValueChanged(i++ / (float)d->currInfos.count());

        qApp->processEvents();
    }

    ScanController::instance()->resumeCollectionScan();
    d->ignoreItemAttributesWatch = false;

    Q_EMIT signalProgressFinished();

    // reload everything

    d->setInfos(d->currInfos);
}

void ItemDescEditTab::slotWriteToFileMetadataFromDatabase()
{
    d->initProgressIndicator();

    Q_EMIT signalProgressMessageChanged(i18nc("@info", "Writing metadata to files. Please wait..."));

    int i = 0;

    for (const ItemInfo& info : std::as_const(d->currInfos))
    {
        MetadataHub hub;

        // read in from database

        hub.load(info);

        // write out to file DMetadata

        ScanController::FileMetadataWrite writeScope(info);
        writeScope.changed(hub.writeToMetadata(info, MetadataHub::WRITE_ALL));

        Q_EMIT signalProgressValueChanged(i++ / (float)d->currInfos.count());

        qApp->processEvents();
    }

    Q_EMIT signalProgressFinished();
}

void ItemDescEditTab::slotModified()
{
    d->modified = true;
    d->applyBtn->setEnabled(true);
    d->revertBtn->setEnabled(true);

    Q_EMIT signalRightSideBarBusy(d->modified);

    if (d->currInfos.size() == 1)
    {
        d->applyToAllVersionsButton->setEnabled(true);
    }
}

void ItemDescEditTab::slotMoreMenu()
{
    d->moreMenu->clear();

    if (d->singleSelection())
    {
        d->moreMenu->addAction(i18nc("@action", "Read metadata from file to database"),
                               this, SLOT(slotReadFromFileMetadataToDatabase()));
        QAction* const writeAction = d->moreMenu->addAction(i18nc("@action", "Write metadata to each file"),
                                                            this, SLOT(slotWriteToFileMetadataFromDatabase()));

        // we do not need a "Write to file" action here because the apply button will do just that
        // if selection is a single file.
        // Adding the option will confuse users: Does the apply button not write to file?
        // Removing the option will confuse users: There is not option to write to file! (not visible in single selection)
        // Disabling will confuse users: Why is it disabled?

        writeAction->setEnabled(false);
    }
    else
    {
        // We need to make clear that this action is different from the Apply button,
        // which saves the same changes to all files. These batch operations operate on each single file.

        d->moreMenu->addAction(i18nc("@action", "Read metadata from each file to database"),
                               this, SLOT(slotReadFromFileMetadataToDatabase()));

        d->moreMenu->addAction(i18nc("@action", "Write metadata to each file"),
                               this, SLOT(slotWriteToFileMetadataFromDatabase()));

        d->moreMenu->addSeparator();

        d->moreMenu->addAction(i18nc("@action", "Unify tags of selected items"),
                               this, SLOT(slotUnifyPartiallyTags()));
    }
}

void ItemDescEditTab::slotImagesChanged(int albumId)
{
    if (d->ignoreItemAttributesWatch || d->modified)
    {
        return;
    }

    const Album* const a = AlbumManager::instance()->findAlbum(albumId);

    if (d->currInfos.isEmpty() || !a || a->isRoot() || (a->type() != Album::TAG))
    {
        return;
    }

    d->setInfos(d->currInfos);
}

void ItemDescEditTab::slotReloadForMetadataChange()
{
    // NOTE: What to do if d->modified? Reloading is no option.
    // It may be a little change the user wants to ignore, or a large conflict.

    if (d->currInfos.isEmpty() || d->modified)
    {
        d->resetMetadataChangeInfo();

        return;
    }

    if (d->singleSelection())
    {
        if (d->metadataChangeIds.contains(d->currInfos.first().id()))
        {
            d->setInfos(d->currInfos);
        }
    }
    else
    {
        // if image id is in our list, update

        for (const ItemInfo& info : std::as_const(d->currInfos))
        {
            if (d->metadataChangeIds.contains(info.id()))
            {   // cppcheck-suppress useStlAlgorithm
                d->setInfos(d->currInfos);
                break;
            }
        }
    }
}

void ItemDescEditTab::slotApplyChangesToAllVersions()
{
    if (!d->modified)
    {
        return;
    }

    if (d->currInfos.isEmpty())
    {
        return;
    }

    QSet<qlonglong>                     tmpSet;
    QList<QPair<qlonglong, qlonglong> > relations;

    for (const ItemInfo& info : std::as_const(d->currInfos))
    {
        // Collect all ids in all image's relations.

        relations.append(info.relationCloud());
    }

    if (relations.isEmpty())
    {
        slotApplyAllChanges();
        return;
    }

    for (int i = 0 ; i < relations.size() ; ++i)
    {
        // Use QSet to prevent duplicates.

        tmpSet.insert(relations.at(i).first);
        tmpSet.insert(relations.at(i).second);
    }

    FileActionMngr::instance()->applyMetadata(ItemInfoList(tmpSet.values()), *d->hub);

    d->modified = false;
    d->hub->resetChanged();
    d->applyBtn->setEnabled(false);
    d->revertBtn->setEnabled(false);

    Q_EMIT signalRightSideBarBusy(d->modified);

    d->applyToAllVersionsButton->setEnabled(false);
}

} // namespace Digikam

#include "moc_itemdescedittab.cpp"
