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

SearchFieldComboBox::SearchFieldComboBox(QObject* const parent)
    : SearchField(parent)
{
}

void SearchFieldComboBox::setupValueWidgets(QGridLayout* layout, int row, int column)
{
    m_comboBox = new QComboBox;
    m_comboBox->setEditable(false);
    layout->addWidget(m_comboBox, row, column, 1, 3);

    connect(m_comboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(indexChanged(int)));
}

void SearchFieldComboBox::write(SearchXmlWriter& writer)
{
    int index = m_comboBox->currentIndex();

    if (index != -1)
    {
        QVariant bits = m_comboBox->itemData(index);

        if (!bits.isNull())
        {
            writer.writeField(m_name, SearchXml::Equal);
            writer.writeValue(bits.toInt());
            writer.finishField();
        }
    }
}

void SearchFieldComboBox::setValueWidgetsVisible(bool visible)
{
    m_comboBox->setVisible(visible);
}

void SearchFieldComboBox::reset()
{
    m_comboBox->setCurrentIndex(0);
}

QList<QRect> SearchFieldComboBox::valueWidgetRects() const
{
    QList<QRect> rects;
    rects << m_comboBox->geometry();

    return rects;
}

void SearchFieldComboBox::indexChanged(int index)
{
    setValidValueState(index != 0);
}

} // namespace Digikam
