/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-05-17
 * Description : Album Labels Search Tree View.
 *
 * SPDX-FileCopyrightText: 2014-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albumlabelssearchhandler.h"

// QT includes

#include <QApplication>
#include <QTreeWidget>
#include <QPainter>
#include <QUrl>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "coredbsearchxml.h"
#include "searchtabheader.h"
#include "albummanager.h"
#include "albumtreeview.h"
#include "coredbconstants.h"
#include "itemlister.h"
#include "statesavingobject.h"
#include "coredbaccess.h"
#include "coredb.h"
#include "colorlabelfilter.h"
#include "picklabelfilter.h"
#include "tagscache.h"
#include "applicationsettings.h"
#include "dnotificationwrapper.h"
#include "digikamapp.h"
#include "ratingwidget.h"
#include "dbjobsmanager.h"

namespace Digikam
{

class Q_DECL_HIDDEN AlbumLabelsSearchHandler::Private
{
public:

    Private() = default;

    LabelsTreeView*       treeWidget                    = nullptr;
    SearchesDBJobsThread* dbJobThread                   = nullptr;
    bool                  restoringSelectionFromHistory = false;
    bool                  currentXmlIsEmpty             = false;
    QString               oldXml;
    Album*                albumForSelectedItems         = nullptr;
    QString               generatedAlbumName;
    QList<QUrl>           urlListForSelectedAlbum;
};

AlbumLabelsSearchHandler::AlbumLabelsSearchHandler(LabelsTreeView* const treeWidget)
    : d(new Private)
{
    d->treeWidget = treeWidget;

    if (!d->treeWidget->isCheckable())
    {
        connect(d->treeWidget, SIGNAL(itemSelectionChanged()),
                this, SLOT(slotSelectionChanged()));

        connect(d->treeWidget, SIGNAL(signalSetCurrentAlbum()),
                this, SLOT(slotSetCurrentAlbum()));
    }
    else
    {
        connect(d->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
                this, SLOT(slotCheckStateChanged()));
    }
}

AlbumLabelsSearchHandler::~AlbumLabelsSearchHandler()
{
    delete d;
}

Album *AlbumLabelsSearchHandler::albumForSelectedItems() const
{
    return d->albumForSelectedItems;
}

QList<QUrl> AlbumLabelsSearchHandler::imagesUrls() const
{
    return d->urlListForSelectedAlbum;
}

QString AlbumLabelsSearchHandler::generatedName() const
{
    return d->generatedAlbumName;
}

void AlbumLabelsSearchHandler::restoreSelectionFromHistory(const QHash<LabelsTreeView::Labels,
                                                           QList<int> >& neededLabels)
{
    d->restoringSelectionFromHistory = true;
    d->treeWidget->restoreSelectionFromHistory(neededLabels);
    d->restoringSelectionFromHistory = false;
    slotSelectionChanged();
}

bool AlbumLabelsSearchHandler::isRestoringSelectionFromHistory() const
{
    return d->restoringSelectionFromHistory;
}

QString AlbumLabelsSearchHandler::createXMLForCurrentSelection(const QHash<LabelsTreeView::Labels,
                                                               QList<int> >& selectedLabels)
{
    SearchXmlWriter writer;
    writer.setFieldOperator(SearchXml::standardFieldOperator());
    QList<int>      ratings;
    QList<int>      colorsAndPicks;
    const auto rates = selectedLabels[LabelsTreeView::Ratings];

    for (int rate : rates)
    {
        if (rate == RatingMin)
        {
            ratings << NoRating;
        }

        ratings << rate;
    }

    const auto cols = selectedLabels[LabelsTreeView::Colors];

    for (int color : cols)
    {
        colorsAndPicks << TagsCache::instance()->tagForColorLabel(color);
    }

    const auto picks = selectedLabels[LabelsTreeView::Picks];

    for (int pick : std::as_const(picks))
    {
        colorsAndPicks << TagsCache::instance()->tagForPickLabel(pick);
    }

    d->currentXmlIsEmpty = ((ratings.isEmpty() && colorsAndPicks.isEmpty()) ? true : false);

    int noColor          = TagsCache::instance()->tagForColorLabel(NoColorLabel);
    int noPick           = TagsCache::instance()->tagForPickLabel(NoPickLabel);
    QList<int> allColors = TagsCache::instance()->colorLabelTags().toList();
    QList<int> allPicks  = TagsCache::instance()->pickLabelTags().toList();

    if (!ratings.isEmpty())
    {
        for (int rate : std::as_const(ratings))
        {
            writer.writeGroup();
            writer.writeField(QLatin1String("rating"), SearchXml::Equal);
            writer.writeValue(rate);
            writer.finishField();
            writer.finishGroup();
        }
    }

    if (!colorsAndPicks.isEmpty())
    {
        writer.writeGroup();
        writer.writeField(QLatin1String("tagid"), SearchXml::InTree);
        writer.writeValue(colorsAndPicks);
        writer.finishField();
        writer.finishGroup();

        if (colorsAndPicks.contains(noColor))
        {
            writer.writeGroup();
            writer.writeField(QLatin1String("tagid"), SearchXml::NotInTree);
            writer.writeValue(allColors);
            writer.finishField();
            writer.finishGroup();
        }

        if (colorsAndPicks.contains(noPick))
        {
            writer.writeGroup();
            writer.writeField(QLatin1String("tagid"), SearchXml::NotInTree);
            writer.writeValue(allPicks);
            writer.finishField();
            writer.finishGroup();
        }
    }

    if (ratings.isEmpty() && colorsAndPicks.isEmpty())
    {
        writer.writeGroup();
        writer.finishGroup();
    }

    writer.finish();

    generateAlbumNameForExporting(selectedLabels[LabelsTreeView::Ratings],
                                  selectedLabels[LabelsTreeView::Colors],
                                  selectedLabels[LabelsTreeView::Picks]);
    return writer.xml();
}

SAlbum* AlbumLabelsSearchHandler::search(const QString& xml) const
{
    SAlbum* album = nullptr;
    int     id    = 0;

    if (!d->treeWidget->isCheckable())
    {
        album = AlbumManager::instance()->findSAlbum(SAlbum::getTemporaryTitle(DatabaseSearch::AdvancedSearch));

        if (album)
        {
            id = album->id();
            CoreDbAccess().db()->updateSearch(id, DatabaseSearch::AdvancedSearch,
                                              SAlbum::getTemporaryTitle(DatabaseSearch::AdvancedSearch), xml);
        }
        else
        {
            id = CoreDbAccess().db()->addSearch(DatabaseSearch::AdvancedSearch,
                                                SAlbum::getTemporaryTitle(DatabaseSearch::AdvancedSearch), xml);
        }

        album = new SAlbum(getDefaultTitle(), id);
    }
    else
    {
        album = AlbumManager::instance()->findSAlbum(getDefaultTitle());

        if (album)
        {
            id = album->id();
            CoreDbAccess().db()->updateSearch(id, DatabaseSearch::AdvancedSearch,
                                              getDefaultTitle(), xml);
        }
        else
        {
            id = CoreDbAccess().db()->addSearch(DatabaseSearch::AdvancedSearch,
                                                getDefaultTitle(), xml);
        }

        album = new SAlbum(d->generatedAlbumName, id);
    }

    if (!album->isUsedByLabelsTree())
    {
        album->setUsedByLabelsTree(true);
    }

    return album;
}

void AlbumLabelsSearchHandler::generateAlbumNameForExporting(const QList<int>& ratings,
                                                             const QList<int>& colorsList,
                                                             const QList<int>& picksList)
{
    QString name;
    QString ratingsString;
    QString picksString;
    QString colorsString;

    if (!ratings.isEmpty())
    {
        ratingsString += i18nc("@info: generate album name for exporting", "Rating: ");

        QListIterator<int> it(ratings);

        while (it.hasNext())
        {
            int rating = it.next();

            if (rating == -1)
            {
                ratingsString += i18nc("@info: rating label", "No Rating");
            }
            else
            {
                ratingsString += QString::number(rating);
            }

            if (it.hasNext())
            {
                ratingsString += QLatin1String(", ");
            }
        }
    }

    if (!colorsList.isEmpty())
    {
        colorsString += i18nc("@info: generate album name for exporting", "Colors: ");

        QListIterator<int> it(colorsList);

        while (it.hasNext())
        {
            switch (it.next())
            {
                case NoColorLabel:
                {
                    colorsString += i18nc("@info: color label name", "No Color");
                    break;
                }

                case RedLabel:
                {
                    colorsString += i18nc("@info: color label name", "Red");
                    break;
                }

                case OrangeLabel:
                {
                    colorsString += i18nc("@info: color label name", "Orange");
                    break;
                }

                case YellowLabel:
                {
                    colorsString += i18nc("@info: color label name", "Yellow");
                    break;
                }

                case GreenLabel:
                {
                    colorsString += i18nc("@info: color label name", "Green");
                    break;
                }

                case BlueLabel:
                {
                    colorsString += i18nc("@info: color label name", "Blue");
                    break;
                }

                case MagentaLabel:
                {
                    colorsString += i18nc("@info: color label name", "Magenta");
                    break;
                }

                case GrayLabel:
                {
                    colorsString += i18nc("@info: color label name", "Gray");
                    break;
                }

                case BlackLabel:
                {
                    colorsString += i18nc("@info: color label name", "Black");
                    break;
                }

                case WhiteLabel:
                {
                    colorsString += i18nc("@info: color label name", "White");
                    break;
                }

                default:
                {
                    break;
                }
            }

            if (it.hasNext())
            {
                colorsString += QLatin1String(", ");
            }
        }
    }

    if (!picksList.isEmpty())
    {
        picksString += i18nc("@info: generate album name for exporting", "Picks: ");

        QListIterator<int> it(picksList);

        while (it.hasNext())
        {
            switch (it.next())
            {
                case NoPickLabel:
                {
                    picksString += i18nc("@info: pick label name", "No Pick");
                    break;
                }

                case RejectedLabel:
                {
                    picksString += i18nc("@info: pick label name", "Rejected");
                    break;
                }

                case PendingLabel:
                {
                    picksString += i18nc("@info: pick label name", "Pending");
                    break;
                }

                case AcceptedLabel:
                {
                    picksString += i18nc("@info: pick label name", "Accepted");
                    break;
                }

                default:
                {
                    break;
                }
            }

            if (it.hasNext())
            {
                picksString += QLatin1String(", ");
            }
        }
    }

    if      (ratingsString.isEmpty() && picksString.isEmpty())
    {
        name = colorsString;
    }
    else if (ratingsString.isEmpty() && colorsString.isEmpty())
    {
        name = picksString;
    }
    else if (colorsString.isEmpty() && picksString.isEmpty())
    {
        name = ratingsString;
    }
    else if (ratingsString.isEmpty())
    {
        name = picksString + QLatin1String(" | ") + colorsString;
    }
    else if (picksString.isEmpty())
    {
        name = ratingsString + QLatin1String(" | ") + colorsString;
    }
    else if (colorsString.isEmpty())
    {
        name = ratingsString + QLatin1String(" | ") + picksString;
    }
    else
    {
        name = ratingsString + QLatin1String(" | ") + picksString + QLatin1String(" | ") + colorsString;
    }

    d->generatedAlbumName = name;
}

void AlbumLabelsSearchHandler::imagesUrlsForCurrentAlbum()
{
    QList<int> searchIds = QList<int>() << d->albumForSelectedItems->id();
    SearchesDBJobInfo jobInfo(std::move(searchIds));
    jobInfo.setRecursive();

    d->dbJobThread = DBJobsManager::instance()->startSearchesJobThread(jobInfo);

    connect(d->dbJobThread, SIGNAL(signalFinished()),
            this, SLOT(slotResult()));

    connect(d->dbJobThread, SIGNAL(data(QList<ItemListerRecord>)),
            this, SLOT(slotData(QList<ItemListerRecord>)));
}

QString AlbumLabelsSearchHandler::getDefaultTitle() const
{
    if (d->treeWidget->isCheckable())
    {
        return i18nc("@info: search label default title", "Exported Labels");
    }
    else
    {
        return i18nc("@info: search label default title", "Labels Album");
    }
}

void AlbumLabelsSearchHandler::slotSelectionChanged()
{
    if (d->treeWidget->isLoadingState() || d->restoringSelectionFromHistory)
    {
        return;
    }

    QString xml         = createXMLForCurrentSelection(d->treeWidget->selectedLabels());
    SAlbum* const album = search(xml);

    if (album)
    {
        AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << album);
        d->albumForSelectedItems = album;
        d->oldXml                = xml;
    }
}

void AlbumLabelsSearchHandler::slotCheckStateChanged()
{
    QString currentXml = createXMLForCurrentSelection(d->treeWidget->selectedLabels());

    if (currentXml == d->oldXml)
    {
        return;
    }

    if (d->albumForSelectedItems)
    {
        Q_EMIT checkStateChanged(d->albumForSelectedItems, Qt::Unchecked);
    }

    SAlbum* const album = search(currentXml);

    if (album)
    {
        if (!d->currentXmlIsEmpty)
        {
            d->albumForSelectedItems = album;
            imagesUrlsForCurrentAlbum();
        }
        else
        {
            d->albumForSelectedItems = nullptr;
        }

        Q_EMIT checkStateChanged(album, Qt::Checked);
    }

    d->oldXml = currentXml;
}

void AlbumLabelsSearchHandler::slotSetCurrentAlbum()
{
    slotSelectionChanged();
}

void AlbumLabelsSearchHandler::slotResult()
{
    if (d->dbJobThread != sender())
    {
        return;
    }

    if (d->dbJobThread->hasErrors())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to list urls: " << d->dbJobThread->errorsList().first();

        // Pop-up a message about the error.

        DNotificationWrapper(QString(),
                             d->dbJobThread->errorsList().first(),
                             DigikamApp::instance(),
                             DigikamApp::instance()->windowTitle());
    }
}

void AlbumLabelsSearchHandler::slotData(const QList<ItemListerRecord>& data)
{
    if ((d->dbJobThread != sender()) || data.isEmpty())
    {
        return;
    }

    QList<QUrl> urlList;

    for (const ItemListerRecord& record : std::as_const(data))
    {
        ItemInfo info(record);
        urlList << info.fileUrl();
    }

    d->urlListForSelectedAlbum = urlList;
}

} // namespace Digikam

#include "moc_albumlabelssearchhandler.cpp"
