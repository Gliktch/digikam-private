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

SearchFieldCheckBox::SearchFieldCheckBox(QObject* const parent)
    : SearchField(parent)
{
}

void SearchFieldCheckBox::setupValueWidgets(QGridLayout* layout, int row, int column)
{
    m_checkBox = new QCheckBox(m_text);
    layout->addWidget(m_checkBox, row, column, 1, 3);

    connect(m_checkBox, SIGNAL(toggled(bool)),
            this, SLOT(slotToggled(bool)));
}

void SearchFieldCheckBox::setLabel(const QString& text)
{
    m_text = text;

    if (m_checkBox)
    {
        m_checkBox->setText(m_text);
    }
}

void SearchFieldCheckBox::write(SearchXmlWriter& writer)
{
    if (m_checkBox->isChecked())
    {
        writer.writeField(m_name, SearchXml::Equal);
        writer.finishField();
    }
}

void SearchFieldCheckBox::read(SearchXmlCachingReader& reader)
{
    SearchXml::Relation relation = reader.fieldRelation();
    reader.readToEndOfElement();

    if (relation == SearchXml::Equal)
    {
        m_checkBox->setChecked(true);
    }
}

void SearchFieldCheckBox::setValueWidgetsVisible(bool visible)
{
    m_checkBox->setVisible(visible);
}

void SearchFieldCheckBox::reset()
{
    m_checkBox->setChecked(false);
}

QList<QRect> SearchFieldCheckBox::valueWidgetRects() const
{
    QList<QRect> rects;
    rects << m_checkBox->geometry();

    return rects;
}

void SearchFieldCheckBox::slotToggled(bool checked)
{
    setValidValueState(checked);
}

} // namespace Digikam
