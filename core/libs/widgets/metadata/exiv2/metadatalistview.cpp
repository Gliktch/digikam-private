/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-21
 * Description : a generic list view widget to
 *               display metadata
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metadatalistview.h"

// Qt includes

#include <QHeaderView>
#include <QTimer>
#include <QPalette>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "mdkeylistviewitem.h"
#include "metadatalistviewitem.h"
#include "digikam_globals.h"

namespace Digikam
{

MetadataListView::MetadataListView(QWidget* const parent)
    : QTreeWidget(parent)
{
    setColumnCount(2);
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    setAllColumnsShowFocus(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setIndentation(layoutSpacing());

    header()->setSectionResizeMode(QHeaderView::Stretch);
    header()->hide();

    QStringList labels;
    labels.append(QLatin1String("Name"));        // no i18n here: hidden header
    labels.append(QLatin1String("Value"));       // no i18n here: hidden header
    setHeaderLabels(labels);

    setSortingEnabled(true);
    sortByColumn(0, Qt::AscendingOrder);

    m_parent = dynamic_cast<MetadataWidget*>(parent);

    connect(this, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotSelectionChanged(QTreeWidgetItem*,int)));
}

QString MetadataListView::getCurrentItemKey() const
{
    if (currentItem() && (currentItem()->flags() & Qt::ItemIsSelectable))
    {
        const MetadataListViewItem* const item = static_cast<MetadataListViewItem*>(currentItem());
        return item->getKey();
    }

    return QString();
}

void MetadataListView::setCurrentItemByKey(const QString& itemKey)
{
    if (itemKey.isNull())
    {
        return;
    }

    int i                 = 0;
    QTreeWidgetItem* item = nullptr;

    do
    {
        item = topLevelItem(i);

        if (item && (item->flags() & Qt::ItemIsSelectable))
        {
            const MetadataListViewItem* const lvItem = dynamic_cast<MetadataListViewItem*>(item);

            if (lvItem)
            {
                if (lvItem->getKey() == itemKey)
                {
                    setCurrentItem(item);
                    scrollToItem(item);
                    m_selectedItemKey = itemKey;

                    return;
                }
            }
        }

        ++i;
    }
    while (item);
}

void MetadataListView::slotSelectionChanged(QTreeWidgetItem* item, int)
{

    const MetadataListViewItem* const viewItem = dynamic_cast<MetadataListViewItem*>(item);

    if (!viewItem)
    {
        return;
    }

    m_selectedItemKey                    = viewItem->getKey();
    QString tagValue                     = viewItem->getValue().simplified();
    QString tagTitle                     = m_parent->getTagTitle(m_selectedItemKey);
    QString tagDesc                      = m_parent->getTagDescription(m_selectedItemKey);

    if (tagValue.length() > 128)
    {
        tagValue.truncate(128);
        tagValue.append(QLatin1String("..."));
    }

    this->setWhatsThis(i18n("<b>Title: </b><p>%1</p>"
                            "<b>Value: </b><p>%2</p>"
                            "<b>Description: </b><p>%3</p>",
                            tagTitle, tagValue, tagDesc));
}

void MetadataListView::setIfdList(const DMetadata::MetaDataMap& ifds, const QStringList& tagsFilter)
{
    clear();

    uint               subItems      = 0;
    MdKeyListViewItem* parentifDItem = nullptr;
    QStringList        filters       = tagsFilter;
    QString            ifDItemName;

    for (DMetadata::MetaDataMap::const_iterator it = ifds.constBegin() ; it != ifds.constEnd() ; ++it)
    {
        // We checking if we have changed of ifDName

        QString currentIfDName = it.key().section(QLatin1Char('.'), 1, 1);

        if (currentIfDName != ifDItemName)
        {
            ifDItemName = currentIfDName;

            // Check if the current IfD have any items. If no remove it before to toggle to the next IfD.

            if ((subItems == 0) && parentifDItem)
            {
                delete parentifDItem;
            }

            parentifDItem = new MdKeyListViewItem(this, currentIfDName);
            subItems      = 0;
        }

        if      (tagsFilter.isEmpty())
        {
            QString tagTitle = m_parent->getTagTitle(it.key());
            new MetadataListViewItem(parentifDItem, it.key(), tagTitle, it.value());
            ++subItems;
        }
        else if (!it.key().section(QLatin1Char('.'), 2, 2).startsWith(QLatin1String("0x")))
        {
            // We ignore all unknown tags if necessary.

            if      (filters.contains(QLatin1String("FULL")))
            {
                // We don't filter the output (Photo Mode)

                QString tagTitle = m_parent->getTagTitle(it.key());
                new MetadataListViewItem(parentifDItem, it.key(), tagTitle, it.value());
                ++subItems;
            }
            else if (!filters.isEmpty())
            {
                // We using the filter to make a more user friendly output (Custom Mode)

                // Filter is not a list of complete tag keys

                if      (!filters.at(0).contains(QLatin1Char('.')) && filters.contains(it.key().section(QLatin1Char('.'), 2, 2)))
                {
                    QString tagTitle = m_parent->getTagTitle(it.key());
                    new MetadataListViewItem(parentifDItem, it.key(), tagTitle, it.value());
                    ++subItems;
                    filters.removeAll(it.key());
                }
                else if (filters.contains(it.key()))
                {
                    QString tagTitle = m_parent->getTagTitle(it.key());
                    new MetadataListViewItem(parentifDItem, it.key(), tagTitle, it.value());
                    ++subItems;
                    filters.removeAll(it.key());
                }
            }
        }
    }

    // To check if the last IfD have any items...

    if ((subItems == 0) && parentifDItem)
    {
        delete parentifDItem;
    }

    // Add not found tags from filter as grey items.

    if (
        !filters.isEmpty()                       &&
        (filters.at(0) != QLatin1String("FULL")) &&
        filters.at(0).contains(QLatin1Char('.'))
       )
    {
        for (const QString& key : std::as_const(filters))
        {
            MdKeyListViewItem* pitem = findMdKeyItem(key);

            if (!pitem)
            {
                pitem = new MdKeyListViewItem(this, key.section(QLatin1Char('.'), 1, 1));
            }

            QString tagTitle = m_parent->getTagTitle(key);
            new MetadataListViewItem(pitem, key, tagTitle);
        }
    }

    setCurrentItemByKey(m_selectedItemKey);
    update();
}

void MetadataListView::setIfdList(const DMetadata::MetaDataMap& ifds, const QStringList& keysFilter,
                                  const QStringList& tagsFilter)
{
    clear();

    QStringList        filters       = tagsFilter;
    uint               subItems      = 0;
    MdKeyListViewItem* parentifDItem = nullptr;

    if (ifds.count() == 0)
    {
        return;
    }

    for (QStringList::const_iterator itKeysFilter = keysFilter.constBegin() ;
         itKeysFilter != keysFilter.constEnd() ; ++itKeysFilter)
    {
        subItems      = 0;
        parentifDItem = new MdKeyListViewItem(this, *itKeysFilter);

        DMetadata::MetaDataMap::const_iterator it = ifds.constEnd();

        while (it != ifds.constBegin())
        {
            --it;

            if (*itKeysFilter == it.key().section(QLatin1Char('.'), 1, 1))
            {
                if      (tagsFilter.isEmpty())
                {
                    QString tagTitle = m_parent->getTagTitle(it.key());
                    new MetadataListViewItem(parentifDItem, it.key(), tagTitle, it.value());
                    ++subItems;
                }
                else if (!it.key().section(QLatin1Char('.'), 2, 2).startsWith(QLatin1String("0x")))
                {
                    // We ignore all unknown tags if necessary.

                    if      (filters.contains(QLatin1String("FULL")))
                    {
                        // We don't filter the output (Photo Mode)

                        QString tagTitle = m_parent->getTagTitle(it.key());
                        new MetadataListViewItem(parentifDItem, it.key(), tagTitle, it.value());
                        ++subItems;
                    }
                    else if (!filters.isEmpty())
                    {
                        // We using the filter to make a more user friendly output (Custom Mode)

                        // Filter is not a list of complete tag keys

                        if      (!filters.at(0).contains(QLatin1Char('.')) &&
                                 filters.contains(it.key().section(QLatin1Char('.'), 2, 2)))
                        {
                            QString tagTitle = m_parent->getTagTitle(it.key());
                            new MetadataListViewItem(parentifDItem, it.key(), tagTitle, it.value());
                            ++subItems;
                            filters.removeAll(it.key());
                        }
                        else if (filters.contains(it.key()))
                        {
                            QString tagTitle = m_parent->getTagTitle(it.key());
                            new MetadataListViewItem(parentifDItem, it.key(), tagTitle, it.value());
                            ++subItems;
                            filters.removeAll(it.key());
                        }
                        else if (it.key().contains(QLatin1String("]/")))
                        {
                            // Special case to filter metadata tags in bag containers

                            int propIndex = it.key().lastIndexOf(QLatin1Char(':'));
                            int nameIndex = it.key().lastIndexOf(QLatin1Char('.'));

                            if ((propIndex != -1) && (nameIndex != -1))
                            {
                                QString property  = it.key().mid(propIndex + 1);
                                QString nameSpace = it.key().left(nameIndex + 1);

                                if (filters.contains(nameSpace + property))
                                {
                                    QString tagTitle = m_parent->getTagTitle(it.key());
                                    new MetadataListViewItem(parentifDItem, it.key(), tagTitle, it.value());
                                    ++subItems;

                                    if (it.key().contains(QLatin1String("[1]")))
                                    {
                                        filters.removeAll(nameSpace + property);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // We checking if the last IfD have any items. If no, we remove it.

        if ((subItems == 0) && parentifDItem)
        {
            delete parentifDItem;
        }
    }

    // Add not found tags from filter as grey items.

    if (
        !filters.isEmpty() &&
        (filters.at(0) != QLatin1String("FULL")) &&
        filters.at(0).contains(QLatin1Char('.'))
       )
    {
        for (const QString& key : std::as_const(filters))
        {
            MdKeyListViewItem* pitem = findMdKeyItem(key);

            if (!pitem)
            {
                pitem = new MdKeyListViewItem(this, key.section(QLatin1Char('.'), 1, 1));
            }

            QString tagTitle = m_parent->getTagTitle(key);
            new MetadataListViewItem(pitem, key, tagTitle);
        }
    }

    setCurrentItemByKey(m_selectedItemKey);
    update();
}

void MetadataListView::slotSearchTextChanged(const SearchTextSettings& settings)
{
    bool query     = false;
    QString search = settings.text;

    // Restore all MdKey items.

    QTreeWidgetItemIterator it2(this);

    while (*it2)
    {
        MdKeyListViewItem* const item = dynamic_cast<MdKeyListViewItem*>(*it2);

        if (item)
        {
            item->setHidden(false);
        }

        ++it2;
    }

    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        MetadataListViewItem* const item = dynamic_cast<MetadataListViewItem*>(*it);

        if (item)
        {
            if (
                item->text(0).contains(search, settings.caseSensitive) ||
                item->text(1).contains(search, settings.caseSensitive)
               )
            {
                query = true;
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

    Q_EMIT signalTextFilterMatch(query);
}

void MetadataListView::cleanUpMdKeyItem()
{
    QTreeWidgetItemIterator it(this);

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

MdKeyListViewItem* MetadataListView::findMdKeyItem(const QString& key)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        MdKeyListViewItem* const item = dynamic_cast<MdKeyListViewItem*>(*it);

        if (item)
        {
            if (key.section(QLatin1Char('.'), 1, 1) == item->getKey())
            {
                return item;
            }
        }

        ++it;
    }

    return nullptr;
}

} // namespace Digikam

#include "moc_metadatalistview.cpp"
