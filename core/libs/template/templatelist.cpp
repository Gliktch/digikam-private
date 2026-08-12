/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-20
 * Description : identity list view.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "templatelist.h"

// Qt includes

#include <QHeaderView>
#include <QTreeWidgetItemIterator>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "templatemanager.h"

namespace Digikam
{

TemplateListItem::TemplateListItem(QTreeWidget* const parent, const Template& t)
    : QTreeWidgetItem(parent)
{
    setTemplate(t);
}

TemplateListItem::~TemplateListItem()
{
}

void TemplateListItem::setTemplate(const Template& t)
{
    m_template = t;

    if (!m_template.isNull())
    {
        setText(0, m_template.templateTitle());
        setText(1, m_template.authors().join(QLatin1Char(';')));
    }
}

Template TemplateListItem::getTemplate() const
{
    return m_template;
}

// -------------------------------------------------------------------

TemplateList::TemplateList(QWidget* const parent)
    : QTreeWidget(parent)
{
    setColumnCount(2);
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    setAllColumnsShowFocus(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setWhatsThis(i18nc("@info", "Here you can see the metadata template list managed by digiKam."));

    QStringList labels;
    labels.append(i18nc("@title: template properties", "Title"));
    labels.append(i18nc("@title: template properties", "Authors"));
    setHeaderLabels(labels);
    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(1, QHeaderView::Stretch);
}

TemplateList::~TemplateList()
{
}

void TemplateList::readSettings()
{
    const TemplateManager* const tm = TemplateManager::defaultManager();

    if (tm)
    {
        QList<Template> list = tm->templateList();

        for (const Template& t : std::as_const(list))
        {
            new TemplateListItem(this, t);
        }
    }
}

void TemplateList::applySettings()
{
    TemplateManager* const tm = TemplateManager::defaultManager();

    if (tm)
    {
        tm->clear();

        QTreeWidgetItemIterator it(this);

        while (*it)
        {
            const TemplateListItem* const item = dynamic_cast<TemplateListItem*>(*it);

            if (item)
            {
                Template t = item->getTemplate();

                if (!t.isNull())
                {
                    tm->insert(t);
                }
            }

            ++it;
        }

        tm->save();
    }
}

TemplateListItem* TemplateList::find(const QString& title)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        TemplateListItem* const item = dynamic_cast<TemplateListItem*>(*it);

        if (item)
        {
            Template t = item->getTemplate();

            if (!t.isNull())
            {
                if (t.templateTitle() == title)
                {
                    return item;
                }
            }
        }

        ++it;
    }

    return nullptr;
}

} // namespace Digikam

#include "moc_templatelist.cpp"
