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

SearchFieldRating::SearchFieldRating(QObject* const parent)
    : SearchField(parent)
{
    m_betweenLabel = new QLabel;
    m_firstBox     = new RatingComboBox;
    m_secondBox    = new RatingComboBox;
}

void SearchFieldRating::setupValueWidgets(QGridLayout* layout, int row, int column)
{
    layout->addWidget(m_firstBox,     row, column);
    layout->addWidget(m_betweenLabel, row, column + 1, Qt::AlignHCenter);
    layout->addWidget(m_secondBox,    row, column + 2);

    connect(m_firstBox, SIGNAL(ratingValueChanged(int)),
            this, SLOT(firstValueChanged()));

    connect(m_secondBox, SIGNAL(ratingValueChanged(int)),
            this, SLOT(secondValueChanged()));
}

void SearchFieldRating::read(SearchXmlCachingReader& reader)
{
    SearchXml::Relation relation = reader.fieldRelation();

    switch (relation)
    {
        case SearchXml::GreaterThanOrEqual:
        {
            m_firstBox->setRatingValue((RatingComboBox::RatingValue)reader.valueToInt());
            break;
        }

        case SearchXml::GreaterThan:
        {
            m_firstBox->setRatingValue((RatingComboBox::RatingValue)(reader.valueToInt() - 1));
            break;
        }

        case SearchXml::LessThanOrEqual:
        {
            m_secondBox->setRatingValue((RatingComboBox::RatingValue)reader.valueToInt());
            break;
        }

        case SearchXml::LessThan:
        {
            m_secondBox->setRatingValue((RatingComboBox::RatingValue)(reader.valueToInt() + 1));
            break;
        }

        case SearchXml::Equal:
        {
            m_firstBox->setRatingValue((RatingComboBox::RatingValue)reader.valueToInt());
            m_secondBox->setRatingValue((RatingComboBox::RatingValue)reader.valueToInt());
            break;
        }

        case SearchXml::Interval:
        case SearchXml::IntervalOpen:
        {
            QList<int> list = reader.valueToIntList();

            if (list.size() != 2)
            {
                return;
            }

            m_firstBox->setRatingValue((RatingComboBox::RatingValue)list.first());
            m_secondBox->setRatingValue((RatingComboBox::RatingValue)list.last());
            break;
        }

        default:
        {
            break;
        }
    }
}

void SearchFieldRating::write(SearchXmlWriter& writer)
{
    RatingComboBox::RatingValue first  = m_firstBox->ratingValue();
    RatingComboBox::RatingValue second = m_secondBox->ratingValue();

    if      (first == RatingComboBox::NoRating)
    {
        writer.writeField(m_name, SearchXml::Equal);
        writer.writeValue(-1);
        writer.finishField();
    }
    else if (first != RatingComboBox::Null && first == second)
    {
        writer.writeField(m_name, SearchXml::Equal);
        writer.writeValue(first);
        writer.finishField();
    }
    else if (first != RatingComboBox::Null && second != RatingComboBox::Null)
    {
        writer.writeField(m_name, SearchXml::Interval);
        writer.writeValue(QList<int>() << first << second);
        writer.finishField();
    }
    else
    {
        if (first != RatingComboBox::Null)
        {
            writer.writeField(m_name, SearchXml::GreaterThanOrEqual);
            writer.writeValue(first);
            writer.finishField();
        }

        if (second != RatingComboBox::Null)
        {
            writer.writeField(m_name, SearchXml::LessThanOrEqual);
            writer.writeValue(second);
            writer.finishField();
        }
    }
}

void SearchFieldRating::setBetweenText(const QString& text)
{
    m_betweenLabel->setText(text);
}

void SearchFieldRating::firstValueChanged()
{
    RatingComboBox::RatingValue first  = m_firstBox->ratingValue();
    RatingComboBox::RatingValue second = m_secondBox->ratingValue();

    if (first == RatingComboBox::NoRating)
    {
        m_secondBox->setRatingValue(RatingComboBox::Null);
        m_secondBox->setEnabled(false);
    }
    else
    {
        m_secondBox->setEnabled(true);
    }

    if ((first >= RatingComboBox::Rating0) && (first <= RatingComboBox::Rating5))
    {
        if (first > second)
        {
            m_secondBox->setRatingValue(RatingComboBox::Null);
        }
    }

    setValidValueState(first != RatingComboBox::Null || second != RatingComboBox::Null);
}

void SearchFieldRating::secondValueChanged()
{
    RatingComboBox::RatingValue first  = m_firstBox->ratingValue();
    RatingComboBox::RatingValue second = m_secondBox->ratingValue();

    // NoRating is not possible for the second box

    if (second >= RatingComboBox::Rating0 && second <= RatingComboBox::Rating5)
    {
        if (first > second)
        {
            m_firstBox->setRatingValue(second);
        }
    }

    setValidValueState(
                       (first  != RatingComboBox::Null) ||
                       (second != RatingComboBox::Null)
                      );
}

void SearchFieldRating::reset()
{
    m_firstBox->setRatingValue(RatingComboBox::Null);
    m_secondBox->setRatingValue(RatingComboBox::Null);
}

void SearchFieldRating::setValueWidgetsVisible(bool visible)
{
    m_firstBox->setVisible(visible);
    m_secondBox->setVisible(visible);
    m_betweenLabel->setVisible(visible);
}

QList<QRect> SearchFieldRating::valueWidgetRects() const
{
    QList<QRect> rects;
    rects << m_firstBox->geometry();
    rects << m_secondBox->geometry();

    return rects;
}

} // namespace Digikam
