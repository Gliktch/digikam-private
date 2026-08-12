/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 20013-07-03
 * Description : Tag Manager main class
 *
 * SPDX-FileCopyrightText:      2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText:      2014 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagsmanager_p.h"

namespace Digikam
{

void TagsManager::slotWriteToImage()
{
    int result = QMessageBox::warning(this, qApp->applicationName(),
                                      i18n("<qt>digiKam will clean up tag metadata before setting "
                                           "tags from database.<br> You may <b>lose tags</b> if you did not "
                                           "read tags before (by calling Read Tags from Image).<br> "
                                           "Do you want to continue?</qt>"),
                                      QMessageBox::Yes | QMessageBox::Cancel);

    if (result != QMessageBox::Yes)
    {
        return;
    }

    result = QMessageBox::warning(this, qApp->applicationName(),
                                  i18n("This operation can take long time "
                                       "depending on collection size.\n"
                                       "Do you want to continue?"),
                                  QMessageBox::Yes | QMessageBox::Cancel);

    if (result != QMessageBox::Yes)
    {
        return;
    }

    MetadataSynchronizer* const tool = new MetadataSynchronizer(AlbumList(),
                                                                MetadataSynchronizer::WriteFromDatabaseToFile);
    tool->setUseMultiCoreCPU(false);
    tool->setTagsOnly(true);
    tool->start();
}

void TagsManager::slotReadFromImage()
{
    int result = QMessageBox::warning(this, qApp->applicationName(),
                                      i18n("This operation can take long time "
                                           "depending on collection size.\n"
                                           "Do you want to continue?"),
                                      QMessageBox::Yes | QMessageBox::Cancel);

    if (result != QMessageBox::Yes)
    {
        return;
    }

    MetadataSynchronizer* const tool = new MetadataSynchronizer(AlbumList(),
                                                                MetadataSynchronizer::ReadFromFileToDatabase);
    tool->setUseMultiCoreCPU(false);
    tool->setTagsOnly(true);
    tool->start();
}

void TagsManager::slotWipeAll()
{
    const int result = QMessageBox::warning(this, qApp->applicationName(),
                                            i18n("This operation will wipe all tags from database only.\n"
                                                 "To apply changes to files, "
                                                 "you must choose write metadata to file later.\n"
                                                 "Do you want to continue?"),
                                            QMessageBox::Yes | QMessageBox::Cancel);

    if (result != QMessageBox::Yes)
    {
        return;
    }

    /**
     * Disable writing tags to images
     */
    MetaEngineSettings* const metaSettings      = MetaEngineSettings::instance();
    MetaEngineSettingsContainer backUpContainer = metaSettings->settings();
    MetaEngineSettingsContainer newContainer    = backUpContainer;
    bool settingsChanged                        = false;

    if ((backUpContainer.saveTags == true) || (backUpContainer.saveFaceTags == true))
    {
        settingsChanged           = true;
        newContainer.saveTags     = false;
        newContainer.saveFaceTags = false;
        metaSettings->setSettings(newContainer);
    }

    AlbumPointerList<TAlbum> tagList;
    const QModelIndex root  = d->tagMngrView->model()->index(0, 0);
    int iter                = 0;
    QModelIndex child       = root.model()->index(iter++, 0, root);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    while (child.isValid())
    {
        TAlbum* const rtag = static_cast<TAlbum*>(d->tagMngrView->albumForIndex(child));

        if (FaceTags::personParentTag() == rtag->id())
        {
            int citer          = 0;
            QModelIndex person = child;
            child              = root.model()->index(citer++, 0, person);

            while (child.isValid())
            {
                TAlbum* const ctag = static_cast<TAlbum*>(d->tagMngrView->albumForIndex(child));

                if (!FaceTags::isSystemPersonTagId(ctag->id()))
                {
                    tagList << AlbumPointer<TAlbum>(ctag);
                }

                child = root.model()->index(citer++, 0, person);
            }
        }
        else
        {
            tagList << AlbumPointer<TAlbum>(rtag);
        }

        child = root.model()->index(iter++, 0, root);
    }

    AlbumPointerList<TAlbum>::iterator it;

    for (it = tagList.begin() ; it != tagList.end() ; ++it)
    {
        QString errMsg;

        if (!AlbumManager::instance()->deleteTAlbum(*it, errMsg))
        {
            QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(), errMsg);
        }
    }

    QApplication::restoreOverrideCursor();

    /**
     * Restore settings after tag deletion
     */
    if (settingsChanged)
    {
        metaSettings->setSettings(backUpContainer);
    }
}

void TagsManager::slotSaveTags()
{
    QList<Album*> selectedTags = d->tagMngrView->selectedTags();

    if ((selectedTags.size() != 1) || !selectedTags.constFirst())
    {
        QMessageBox::information(this, qApp->applicationName(),
                                 i18n("Select a base tag from which the tags should be exported."));

        return;
    }

    QString baseTag = static_cast<TAlbum*>(selectedTags.constFirst())->tagPath();

    if (baseTag.count(QLatin1Char('/')) > 1)
    {
        QMessageBox::warning(this, qApp->applicationName(),
                                 i18n("Selected tag is not a base tag at the top of the hierarchy."));

        return;
    }

    QString tagsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    QString savePath = DFileDialog::getSaveFileName(qApp->activeWindow(),
                                                    i18nc("@title:window", "Export Controlled Vocabulary Tag File"),
                                                    tagsPath, i18n("Text File (*.txt)"), nullptr,
                                                    QFileDialog::DontConfirmOverwrite);

    if (savePath.isEmpty())
    {
        return;
    }

    if (!savePath.endsWith(QLatin1String(".txt"), Qt::CaseInsensitive))
    {
        savePath.append(QLatin1String(".txt"));
    }

    QFile tagFile(savePath);

    if (!tagFile.open(QIODevice::WriteOnly))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot open file to export tags" << savePath;

        return;
    }

    QTextStream tagStream(&tagFile);

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)

    // In Qt5 only. Qt6 uses UTF-8 by default.

    tagStream.setCodec(QTextCodec::codecForName("UTF-8"));

#endif

    QApplication::setOverrideCursor(Qt::WaitCursor);

    tagStream.setAutoDetectUnicode(true);

    QStringList sortedTagList;
    AlbumList::const_iterator it;
    AlbumList tList = AlbumManager::instance()->allTAlbums();

    for (it = tList.constBegin() ; it != tList.constEnd() ; ++it)
    {
        const TAlbum* const tag = static_cast<TAlbum*>(*it);

        if (
            tag                                    &&
            !tag->isRoot()                         &&
            !tag->isInternalTag()                  &&
            !FaceTags::isSystemPersonTagId(tag->id())
           )
        {
            if (
                (baseTag == tag->tagPath())                        ||
                (baseTag == QLatin1String("/"))                    ||
                tag->tagPath().startsWith(baseTag + QLatin1Char('/'))
               )
            {
                sortedTagList << tag->tagPath();
            }
        }
    }

    sortedTagList.sort();

    for (const QString& stag : std::as_const(sortedTagList))
    {
        QStringList tagPathList = stag.split(QLatin1Char('/'),
                                             Qt::SkipEmptyParts);

        if (tagPathList.isEmpty())
        {
            continue;
        }

        QString ctag = tagPathList.takeLast();
        int tabs     = tagPathList.size();

        while (tabs > 0)
        {
            tagStream << QLatin1Char('\t');
            --tabs;
        }

        tagStream << ctag << Qt::endl;
    }

    tagFile.close();

    QApplication::restoreOverrideCursor();
}

void TagsManager::slotLoadTags()
{
    QList<Album*> selectedTags = d->tagMngrView->selectedTags();

    if ((selectedTags.size() != 1) || !selectedTags.constFirst())
    {
        QMessageBox::information(this, qApp->applicationName(),
                                 i18n("Select a tag into which the tags should be imported."));

        return;
    }

    const TAlbum* const selTag = static_cast<TAlbum*>(selectedTags.constFirst());

    if (FaceTags::isSystemPersonTagId(selTag->id()))
    {
        QMessageBox::warning(this, qApp->applicationName(),
                             i18n("No tags can be imported to the selected face tag."));

        return;
    }

    QString baseTag  = selTag->tagPath();
    QString tagsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    QString loadPath = DFileDialog::getOpenFileName(qApp->activeWindow(),
                                                    i18nc("@title:window", "Import Controlled Vocabulary Tag File"),
                                                    tagsPath, i18n("Text File (*.txt)"));

    if (loadPath.isEmpty())
    {
        return;
    }

    QFile tagFile(loadPath);

    if (!tagFile.open(QIODevice::ReadOnly))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot open file to import tags" << loadPath;

        return;
    }

    QTextStream tagStream(&tagFile);

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)

    // In Qt5 only. Qt6 uses UTF-8 by default.

    tagStream.setCodec(QTextCodec::codecForName("UTF-8"));

#endif

    QApplication::setOverrideCursor(Qt::WaitCursor);

    tagStream.setAutoDetectUnicode(true);

    QStringList tagPathList;
    int startPos = -1;
    QString line;

    while (tagStream.readLineInto(&line))
    {
        QString currTag;
        int tabPos = 0;

        for (int i = 0 ; i < line.size() ; ++i)
        {
            if (
                (line.at(i) == QLatin1Char('[')) ||
                (line.at(i) == QLatin1Char('{'))
               )
            {
                break;
            }

            if (line.at(i) == QLatin1Char('\t'))
            {
                ++tabPos;
                continue;
            }

            if (startPos == -1)
            {
                startPos = i;
            }

            currTag = line.mid(i);
            break;
        }

        if (!currTag.isEmpty())
        {
            const int count = tabPos - startPos;

            while (!tagPathList.isEmpty() && (tagPathList.size() > count))
            {
                tagPathList.removeLast();
            }

            QString tagPath = baseTag;

            tagPathList.append(currTag);

            if (baseTag != QLatin1String("/"))
            {
                tagPath.append(QLatin1Char('/'));
            }

            tagPath.append(tagPathList.join(QLatin1Char('/')));

            TagsCache::instance()->getOrCreateTag(tagPath);
        }
    }

    tagFile.close();

    QApplication::restoreOverrideCursor();
}

void TagsManager::slotRemoveTagsFromImages()
{
    const QModelIndexList selList = d->tagMngrView->selectionModel()->selectedIndexes();

    const int result = QMessageBox::warning(this, qApp->applicationName(),
                                            i18np("Do you really want to remove the selected tag from all images?",
                                                  "Do you really want to remove the selected tags from all images?",
                                                  selList.count()),
                                            QMessageBox::Yes | QMessageBox::Cancel);

    if (result != QMessageBox::Yes)
    {
        return;
    }

    for (const QModelIndex& index : std::as_const(selList))
    {
        TAlbum* const t = static_cast<TAlbum*>(d->tagMngrView->albumForIndex(index));

        AlbumPointer<TAlbum> tag(t);

        if (tag->isRoot())
        {
            continue;
        }

        QList<qlonglong> assignedItems = CoreDbAccess().db()->getItemIDsInTag(tag->id());
        ItemInfoList imgList(assignedItems);
        FileActionMngr::instance()->removeTag(imgList, tag->id());
    }
}

} // namespace Digikam
