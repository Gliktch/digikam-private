/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-01-20
 * Description : User interface for searches
 *
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "searchfields_p.h"

namespace Digikam
{

SearchFieldAlbum::SearchFieldAlbum(QObject* const parent, Type type)
    : SearchField(parent),
      m_type     (type)
{
}

void SearchFieldAlbum::setupValueWidgets(QGridLayout* layout, int row, int column)
{
    if      (m_type == TypeAlbum)
    {
        m_albumComboBox = new AlbumTreeViewSelectComboBox;
        m_wrapperBox    = m_albumComboBox;

        m_albumComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        m_albumComboBox->setDefaultModel();
        m_albumComboBox->setNoSelectionText(i18nc("@item", "Any Album"));
        m_albumComboBox->addCheckUncheckContextMenuActions();

        m_model = m_albumComboBox->model();
        layout->addWidget(m_wrapperBox, row, column, 1, 3);
    }
    else if (m_type == TypeTag)
    {
        m_wrapperBox  = new DHBox(nullptr);
        m_tagComboBox = new TagTreeViewSelectComboBox(m_wrapperBox);

        m_operation   = new SqueezedComboBox(m_wrapperBox);
        m_operation->addSqueezedItem(i18nc("@label:listbox", "In All"),    Operation::All);
        m_operation->addSqueezedItem(i18nc("@label:listbox", "In One of"), Operation::OneOf);
        m_operation->addSqueezedItem(i18nc("@label:listbox", "In Tree"),   Operation::InTree);

        m_tagComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        m_tagComboBox->setDefaultModel();
        m_tagComboBox->setNoSelectionText(i18nc("@item", "Any Tag"));
        m_tagComboBox->addCheckUncheckContextMenuActions();

        m_model       = m_tagComboBox->model();
        layout->addWidget(m_wrapperBox, row, column, 1, 3);
    }

    connect(m_model, SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
            this, SLOT(updateState()));

    updateState();
}

void SearchFieldAlbum::updateState()
{
    setValidValueState(!m_model->checkedAlbums().isEmpty());
}

void SearchFieldAlbum::read(SearchXmlCachingReader& reader)
{
    QList<int> ids = reader.valueToIntOrIntList();
    Album* a       = nullptr;

    if      (m_type == TypeAlbum)
    {
        for (int id : std::as_const(ids))
        {
            a = AlbumManager::instance()->findPAlbum(id);

            if (!a)
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Search: Did not find album for ID" << id << "given in Search XML";
                return;
            }

            m_model->setChecked(a, true);
        }
    }
    else if (m_type == TypeTag)
    {
        if      (reader.fieldRelation() == SearchXml::AllOf)
        {
            m_operation->setCurrentIndex(Operation::All);
        }
        else if (reader.fieldRelation() == SearchXml::OneOf)
        {
            m_operation->setCurrentIndex(Operation::OneOf);
        }
        else if (reader.fieldRelation() == SearchXml::InTree)
        {
            m_operation->setCurrentIndex(Operation::InTree);
        }

        for (int id : std::as_const(ids))
        {
            a = AlbumManager::instance()->findTAlbum(id);

            // Ignore internal tags here.

            if (a && TagsCache::instance()->isInternalTag(a->id()))
            {
                a = nullptr;
            }

            if (!a)
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Search: Did not find album for ID" << id << "given in Search XML";
                return;
            }

            m_model->setChecked(a, true);
        }
    }
}

void SearchFieldAlbum::write(SearchXmlWriter& writer)
{
    AlbumList checkedAlbums = m_model->checkedAlbums();

    if (checkedAlbums.isEmpty())
    {
        return;
    }

    QList<int> albumIds;

    for (const Album* const album : std::as_const(checkedAlbums))
    {
        albumIds << album->id();
    }

    SearchXml::Relation relation = SearchXml::OneOf;

    if (m_operation)
    {
        int operation = m_operation->currentData().toInt();

        if      (operation == Operation::All)
        {
            relation = SearchXml::AllOf;
        }
        else if (operation == Operation::InTree)
        {
            relation = SearchXml::InTree;
        }
    }

    if (albumIds.size() > 1)
    {
        writer.writeField(m_name, relation);
        writer.writeValue(albumIds);
    }
    else
    {
        if (relation != SearchXml::InTree)
        {
            relation = SearchXml::Equal;
        }

        writer.writeField(m_name, relation);
        writer.writeValue(albumIds.first());
    }

    writer.finishField();
}

void SearchFieldAlbum::reset()
{
    m_model->resetCheckedAlbums();

    if (m_operation)
    {
        m_operation->setCurrentIndex(Operation::All);
    }
}

void SearchFieldAlbum::setValueWidgetsVisible(bool visible)
{
    m_wrapperBox->setVisible(visible);
}

QList<QRect> SearchFieldAlbum::valueWidgetRects() const
{
    QList<QRect> rects;
    rects << m_wrapperBox->geometry();

    return rects;
}

} // namespace Digikam
