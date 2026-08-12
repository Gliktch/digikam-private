/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-07-16
 * Description : metadata selector.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metadataselector.h"

// Qt includes

#include <QHeaderView>
#include <QGridLayout>
#include <QApplication>
#include <QPushButton>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "ditemtooltip.h"
#include "mdkeylistviewitem.h"
#include "digikam_globals.h"

namespace Digikam
{

MetadataSelectorItem::MetadataSelectorItem(MdKeyListViewItem* const parent,
                                           const QString& key,
                                           const QString& title,
                                           const QString& desc)
    : QTreeWidgetItem(parent),
      m_key          (key),
      m_parent       (parent)
{
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    setCheckState(0, Qt::Unchecked);
    setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

    setText(0, title);
    setToolTip(0, key);

    QString descVal = desc.simplified();

    if (descVal.length() > 512)
    {
        descVal.truncate(512);
        descVal.append(QLatin1String("..."));
    }

    setText(1, descVal);

    DToolTipStyleSheet cnt;
    setToolTip(1, QLatin1String
               ("<qt><p>") + cnt.breakString(descVal) + QLatin1String("</p></qt>"));
}

QString MetadataSelectorItem::key() const
{
    return m_key;
}

QString MetadataSelectorItem::mdKeyTitle() const
{
    return (m_parent ? m_parent->text(0) : QString());
}

// ------------------------------------------------------------------------------------

MetadataSelector::MetadataSelector(MetadataSelectorView* const parent)
    : QTreeWidget(parent),
      m_parent   (parent)
{
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAllColumnsShowFocus(true);
    setUniformRowHeights(true);
    setColumnCount(2);

    QStringList labels;
    labels.append(i18nc("@title: metadata properties", "Name"));
    labels.append(i18nc("@title: metadata properties", "Description"));
    setHeaderLabels(labels);
    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(1, QHeaderView::Stretch);

    setSortingEnabled(true);
    sortByColumn(0, Qt::AscendingOrder);
}

void MetadataSelector::setTagsMap(const DMetadata::TagsMap& map)
{
    clear();

    uint                    subItems      = 0;
    QString                 ifDItemName, currentIfDName;
    MdKeyListViewItem*      parentifDItem = nullptr;
    QList<QTreeWidgetItem*> toplevelItems;

    for (DMetadata::TagsMap::const_iterator it = map.constBegin();
         it != map.constEnd(); ++it)
    {
        // Check if we have changed group.

        if (m_parent->backend() == MetadataSelectorView::Exiv2Backend)
        {
            currentIfDName = it.key().section(QLatin1Char('.'), 1, 1);
        }
        else
        {
            currentIfDName = it.key().section(QLatin1Char('.'), 0, 0);
        }

        if (currentIfDName != ifDItemName)
        {
            ifDItemName = currentIfDName;

            // Remove the group header if it has no items.

            if ((subItems == 0) && parentifDItem)
            {
                delete parentifDItem;
            }

            parentifDItem = new MdKeyListViewItem(nullptr, currentIfDName);
            toplevelItems << parentifDItem;
            subItems      = 0;
        }

        // Ignore all unknown Exif tags.

        if (!it.key().section(QLatin1Char('.'), 2, 2).startsWith(QLatin1String("0x")))
        {
            new MetadataSelectorItem(parentifDItem, it.key(),
                                     it.value().at(0),          // Name
                                     it.value().at(2));         // Description
            ++subItems;
        }
    }

    addTopLevelItems(toplevelItems);

    // We need to call setFirstColumnSpanned() in here again because the
    // widgets were added parentless and therefore no layout information was
    // present at construction time. Now that all items have a parent, we need
    // to trigger the method again.

    for (QList<QTreeWidgetItem*>::const_iterator it = toplevelItems.constBegin() ;
         it != toplevelItems.constEnd() ; ++it)
    {
        if (*it)
        {
            (*it)->setFirstColumnSpanned(true);
        }
    }

    expandAll();
}

void MetadataSelector::setcheckedTagsList(const QStringList& list)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        MetadataSelectorItem* const item = dynamic_cast<MetadataSelectorItem*>(*it);

        if (item && list.contains(item->key()))
        {
            item->setCheckState(0, Qt::Checked);
        }

        ++it;
    }
}

QStringList MetadataSelector::checkedTagsList()
{
    QStringList list;
    QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Checked);

    while (*it)
    {
        const MetadataSelectorItem* const item = dynamic_cast<MetadataSelectorItem*>(*it);

        if (item)
        {
            list.append(item->key());
        }

        ++it;
    }

    return list;
}

void MetadataSelector::clearSelection()
{
    collapseAll();

    QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Checked);

    while (*it)
    {
        MetadataSelectorItem* const item = dynamic_cast<MetadataSelectorItem*>(*it);

        if (item)
        {
            item->setCheckState(0, Qt::Unchecked);
        }

        ++it;
    }

    expandAll();
}

void MetadataSelector::selectAll()
{
    collapseAll();

    QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::NotChecked);

    while (*it)
    {
        MetadataSelectorItem* const item = dynamic_cast<MetadataSelectorItem*>(*it);

        if (item)
        {
            item->setCheckState(0, Qt::Checked);
        }

        ++it;
    }

    expandAll();
}

// ------------------------------------------------------------------------------------

class Q_DECL_HIDDEN MetadataSelectorView::Private
{
public:

    Private() = default;

    QStringList       defaultFilter;

    QPushButton*      selectAllBtn          = nullptr;
    QPushButton*      clearSelectionBtn     = nullptr;
    QPushButton*      defaultSelectionBtn   = nullptr;

    MetadataSelector* selector              = nullptr;

    SearchTextBar*    searchBar             = nullptr;
    Backend           backend               = Exiv2Backend;
};

MetadataSelectorView::MetadataSelectorView(QWidget* const parent, Backend be)
    : QWidget(parent),
      d      (new Private)
{
    d->backend              = be;
    const int spacing       = layoutSpacing();


    QGridLayout* const grid = new QGridLayout(this);
    d->selector             = new MetadataSelector(this);
    d->searchBar            = new SearchTextBar(this, QLatin1String("MetadataSelectorView"));
    d->selectAllBtn         = new QPushButton(i18nc("@action: metadata selector", "Select All"),this);
    d->clearSelectionBtn    = new QPushButton(i18nc("@action: metadata selector", "Clear"),this);
    d->defaultSelectionBtn  = new QPushButton(i18nc("@action: metadata selector", "Default"),this);

    grid->addWidget(d->selector,            0, 0, 1, 5);
    grid->addWidget(d->searchBar,           1, 0, 1, 1);
    grid->addWidget(d->selectAllBtn,        1, 2, 1, 1);
    grid->addWidget(d->clearSelectionBtn,   1, 3, 1, 1);
    grid->addWidget(d->defaultSelectionBtn, 1, 4, 1, 1);
    grid->setColumnStretch(0, 10);
    grid->setRowStretch(0, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    setControlElements(SearchBar | SelectAllBtn | DefaultBtn | ClearBtn);

    connect(d->searchBar, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            this, SLOT(slotSearchTextChanged(SearchTextSettings)));

    connect(d->selectAllBtn, SIGNAL(clicked()),
            this, SLOT(slotSelectAll()));

    connect(d->defaultSelectionBtn, SIGNAL(clicked()),
            this, SLOT(slotDeflautSelection()));

    connect(d->clearSelectionBtn, SIGNAL(clicked()),
            this, SLOT(slotClearSelection()));
}

MetadataSelectorView::~MetadataSelectorView()
{
    delete d;
}

void MetadataSelectorView::setTagsMap(const DMetadata::TagsMap& map)
{
    d->selector->setTagsMap(map);
}

void MetadataSelectorView::setcheckedTagsList(const QStringList& list)
{
    d->selector->setcheckedTagsList(list);
}

void MetadataSelectorView::setDefaultFilter(const QStringList& list)
{
    d->defaultFilter = list;
}

QStringList MetadataSelectorView::defaultFilter() const
{
    return d->defaultFilter;
}

int MetadataSelectorView::itemsCount() const
{
    return d->selector->model()->rowCount();
}

MetadataSelectorView::Backend MetadataSelectorView::backend() const
{
    return d->backend;
}

QStringList MetadataSelectorView::checkedTagsList() const
{
    d->searchBar->clear();
    return d->selector->checkedTagsList();
}

void MetadataSelectorView::slotSearchTextChanged(const SearchTextSettings& settings)
{
    QString search       = settings.text;
    bool atleastOneMatch = false;

    // Restore all MdKey items.

    QTreeWidgetItemIterator it2(d->selector);

    while (*it2)
    {
        MdKeyListViewItem* const item = dynamic_cast<MdKeyListViewItem*>(*it2);

        if (item)
        {
            item->setHidden(false);
        }

        ++it2;
    }

    QTreeWidgetItemIterator it(d->selector);

    while (*it)
    {
        MetadataSelectorItem* const item = dynamic_cast<MetadataSelectorItem*>(*it);

        if (item)
        {
            bool match = item->text(0).contains(search, settings.caseSensitive) ||
                         item->mdKeyTitle().contains(search, settings.caseSensitive);

            if (match)
            {
                atleastOneMatch = true;
                item->setHidden(false);
            }
            else
            {
                item->setHidden(true);
            }
        }

        ++it;
    }

    // If we found MdKey items alone, we hide it...

    cleanUpMdKeyItem();

    d->searchBar->slotSearchResult(atleastOneMatch);
}

void MetadataSelectorView::cleanUpMdKeyItem()
{
    QTreeWidgetItemIterator it(d->selector);

    while (*it)
    {
        MdKeyListViewItem* const item = dynamic_cast<MdKeyListViewItem*>(*it);

        if (item)
        {
            int children = item->childCount();
            int visibles = 0;

            for (int i = 0 ; i < children ; ++i)
            {
                QTreeWidgetItem* const citem = (*it)->child(i);

                if (!citem->isHidden())
                {
                    ++visibles;
                }
            }

            if (!children || !visibles)
            {
                item->setHidden(true);
            }
        }

        ++it;
    }
}

void MetadataSelectorView::slotDeflautSelection()
{
    slotClearSelection();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    d->selector->collapseAll();

    QTreeWidgetItemIterator it(d->selector);

    while (*it)
    {
        MetadataSelectorItem* const item = dynamic_cast<MetadataSelectorItem*>(*it);

        if (item)
        {
            if (d->defaultFilter.contains(item->text(0)))
            {
                item->setCheckState(0, Qt::Checked);
            }
        }

        ++it;
    }

    d->selector->expandAll();
    QApplication::restoreOverrideCursor();
}

void MetadataSelectorView::slotSelectAll()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    d->selector->selectAll();
    QApplication::restoreOverrideCursor();
}

void MetadataSelectorView::slotClearSelection()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    d->selector->clearSelection();
    QApplication::restoreOverrideCursor();
}

void MetadataSelectorView::setControlElements(ControlElements controllerMask)
{
    d->searchBar->setVisible(controllerMask & SearchBar);
    d->selectAllBtn->setVisible(controllerMask & SelectAllBtn);
    d->clearSelectionBtn->setVisible(controllerMask & ClearBtn);
    d->defaultSelectionBtn->setVisible(controllerMask & DefaultBtn);
}

void MetadataSelectorView::clearSelection()
{
    slotClearSelection();
}

void MetadataSelectorView::selectAll()
{
    slotSelectAll();
}

void MetadataSelectorView::selectDefault()
{
    slotDeflautSelection();
}

} // namespace Digikam

#include "moc_metadataselector.cpp"
