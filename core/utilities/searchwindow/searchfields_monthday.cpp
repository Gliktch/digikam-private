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

SearchFieldMonthDay::SearchFieldMonthDay(QObject* const parent)
    : SearchField(parent)
{
}

void SearchFieldMonthDay::setupValueWidgets(QGridLayout* layout, int row, int column)
{
    QHBoxLayout* const hbox = new QHBoxLayout;
    m_monthBox              = new QComboBox;
    m_monthBox->setEditable(false);
    m_dayLabel              = new QLabel(i18n("Day:"));
    m_dayBox                = new QComboBox;
    m_dayBox->setEditable(false);

    m_monthBox->addItem(i18n("Every Month"), 0);

    for (int i = 1 ; i <= 12 ; ++i)
    {
        m_monthBox->addItem(QLocale().standaloneMonthName(i, QLocale::LongFormat), i);
    }

    m_dayBox->addItem(i18n("Every Day"), 0);

    for (int i = 1 ; i <= 31 ; ++i)
    {
        m_dayBox->addItem(QString::number(i), i);
    }

    hbox->addWidget(m_monthBox);
    hbox->addStretch(10);
    hbox->addWidget(m_dayLabel);
    hbox->addWidget(m_dayBox);

    connect(m_monthBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotIndexChanged()));

    connect(m_dayBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotIndexChanged()));

    layout->addLayout(hbox, row, column, 1, 3);
}

void SearchFieldMonthDay::read(SearchXmlCachingReader& reader)
{
    SearchXml::Relation relation = reader.fieldRelation();

    if (relation == SearchXml::Equal)
    {
        QList<int> list = reader.valueToIntList();

        if (list.size() == 2)
        {
            m_monthBox->setCurrentIndex(m_monthBox->findData(list.at(0)));
            m_dayBox->setCurrentIndex(m_dayBox->findData(list.at(1)));
        }
    }
}

void SearchFieldMonthDay::write(SearchXmlWriter& writer)
{
    int month = m_monthBox->currentData().toInt();
    int day   = m_dayBox->currentData().toInt();

    if ((month > 0) || (day > 0))
    {
        writer.writeField(m_name, SearchXml::Equal);
        writer.writeValue(QList<int>() << month << day);
        writer.finishField();
    }
}

void SearchFieldMonthDay::reset()
{
    m_monthBox->setCurrentIndex(0);
    m_dayBox->setCurrentIndex(0);
}

void SearchFieldMonthDay::setValueWidgetsVisible(bool visible)
{
    m_monthBox->setVisible(visible);
    m_dayLabel->setVisible(visible);
    m_dayBox->setVisible(visible);
}

QList<QRect> SearchFieldMonthDay::valueWidgetRects() const
{
    QList<QRect> rects;
    rects << m_monthBox->geometry();
    rects << m_dayBox->geometry();

    return rects;
}

void SearchFieldMonthDay::slotIndexChanged()
{
    int month  = m_monthBox->currentData().toInt();
    int day    = m_dayBox->currentData().toInt();
    bool valid = ((month > 0) || (day > 0));

    setValidValueState(valid);
}

} // namespace Digikam
