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

SearchFieldRangeInt::SearchFieldRangeInt(QObject* const parent)
    : SearchField(parent)
{
    m_betweenLabel = new QLabel;
    m_firstBox     = new CustomStepsIntSpinBox;
    m_secondBox    = new CustomStepsIntSpinBox;
}

void SearchFieldRangeInt::setupValueWidgets(QGridLayout* layout, int row, int column)
{
/*
    QHBoxLayout *hbox = new QHBoxLayout;
    layout->addLayout(hbox, row, column);
*/
    m_firstBox->setSpecialValueText(QLatin1String(" "));
    m_secondBox->setSpecialValueText(QLatin1String(" "));
/*
    hbox->addWidget(m_firstBox);
    hbox->addWidget(m_betweenLabel);
    hbox->addWidget(m_secondBox);
    hbox->addStretch(1);
*/
    layout->addWidget(m_firstBox, row, column);
    layout->addWidget(m_betweenLabel, row, column + 1, Qt::AlignHCenter);
    layout->addWidget(m_secondBox, row, column + 2);

    connect(m_firstBox, SIGNAL(valueChanged(int)),
            this, SLOT(valueChanged()));

    connect(m_secondBox, SIGNAL(valueChanged(int)),
            this, SLOT(valueChanged()));
}

void SearchFieldRangeInt::read(SearchXmlCachingReader& reader)
{
    SearchXml::Relation relation = reader.fieldRelation();

    if (m_reciprocal)
    {
        switch (relation)
        {
            case SearchXml::LessThanOrEqual:
            case SearchXml::LessThan:
            {
                m_firstBox->setFractionMagicValue(reader.valueToDouble());
                break;
            }

            case SearchXml::GreaterThanOrEqual:
            case SearchXml::GreaterThan:
            {
                m_secondBox->setFractionMagicValue(reader.valueToDouble());
                break;
            }

            case SearchXml::Equal:
            {
                m_firstBox->setFractionMagicValue(reader.valueToDouble());
                m_secondBox->setFractionMagicValue(reader.valueToDouble());
                break;
            }

            case SearchXml::Interval:
            case SearchXml::IntervalOpen:
            {
                QList<double> list = reader.valueToDoubleList();

                if (list.size() != 2)
                {
                    return;
                }

                m_secondBox->setFractionMagicValue(list.first());
                m_firstBox->setFractionMagicValue(list.last());
                break;
            }

            default:
            {
                break;
            }
        }
    }
    else
    {
        switch (relation)
        {
            case SearchXml::GreaterThanOrEqual:
            {
                m_firstBox->setValue(reader.valueToInt());
                break;
            }

            case SearchXml::GreaterThan:
            {
                m_firstBox->setValue(reader.valueToInt() - 1);
                break;
            }

            case SearchXml::LessThanOrEqual:
            {
                m_secondBox->setValue(reader.valueToInt());
                break;
            }

            case SearchXml::LessThan:
            {
                m_secondBox->setValue(reader.valueToInt() + 1);
                break;
            }

            case SearchXml::Equal:
            {
                m_firstBox->setValue(reader.valueToInt());
                m_secondBox->setValue(reader.valueToInt());
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

                m_firstBox->setValue(list.first());
                m_secondBox->setValue(list.last());
                break;
            }

            default:
            {
                break;
            }
        }
    }
}

void SearchFieldRangeInt::write(SearchXmlWriter& writer)
{
    if (
        (m_firstBox->value()  != m_firstBox->minimum()) &&
        (m_secondBox->value() != m_secondBox->minimum())
       )
    {
        if (m_firstBox->value() != m_secondBox->value())
        {
            writer.writeField(m_name, SearchXml::Interval);

            if (m_reciprocal)
            {
                writer.writeValue(QList<float>() << m_secondBox->fractionMagicValue() << m_firstBox->fractionMagicValue());
            }
            else
            {
                writer.writeValue(QList<int>() << m_firstBox->value() << m_secondBox->value());
            }

            writer.finishField();
        }
        else
        {
            /**
             * @todo : This condition is never met.
             * Right value is either displayed empty (minimum, greater than left)
             * or one step larger than left
             */

            writer.writeField(m_name, SearchXml::Equal);

            if (m_reciprocal)
            {
                writer.writeValue(m_firstBox->fractionMagicValue());
            }
            else
            {
                writer.writeValue(m_firstBox->value());
            }

            writer.finishField();
        }
    }
    else
    {
        if (m_firstBox->value() != m_firstBox->minimum())
        {
            if (m_reciprocal)
            {
                writer.writeField(m_name, SearchXml::LessThanOrEqual);
                writer.writeValue(m_firstBox->fractionMagicValue());
            }
            else
            {
                writer.writeField(m_name, SearchXml::GreaterThanOrEqual);
                writer.writeValue(m_firstBox->value());
            }

            writer.finishField();
        }

        if (m_secondBox->value() != m_secondBox->minimum())
        {
            if (m_reciprocal)
            {
                writer.writeField(m_name, SearchXml::GreaterThanOrEqual);
                writer.writeValue(m_secondBox->fractionMagicValue());
            }
            else
            {
                writer.writeField(m_name, SearchXml::LessThanOrEqual);
                writer.writeValue(m_secondBox->value());
            }

            writer.finishField();
        }
    }
}

void SearchFieldRangeInt::setBetweenText(const QString& text)
{
    m_betweenLabel->setText(text);
}

void SearchFieldRangeInt::setNumberPrefixAndSuffix(const QString& prefix, const QString& suffix)
{
    m_firstBox->setPrefix(prefix);
    m_secondBox->setPrefix(prefix);
    m_firstBox->setSuffix(suffix);
    m_secondBox->setSuffix(suffix);
}

void SearchFieldRangeInt::setBoundary(int min, int max, int step)
{
    if (m_reciprocal)
    {
        m_min = max;
        m_max = min;
    }
    else
    {
        m_min = min;
        m_max = max;
    }

    m_firstBox->setRange(m_min, m_max);
    m_firstBox->setSingleStep(step);
    m_firstBox->setValue(m_min);

    m_secondBox->setRange(m_min, m_max);
    m_secondBox->setSingleStep(step);
    m_secondBox->setValue(m_min);
}

void SearchFieldRangeInt::enableFractionMagic(const QString& prefix)
{
    m_reciprocal = true;

    m_firstBox->enableFractionMagic(prefix);
    m_firstBox->setInvertStepping(true);

    m_secondBox->enableFractionMagic(prefix);
    m_secondBox->setInvertStepping(true);
}

void SearchFieldRangeInt::setSuggestedValues(const QList<int>& values)
{
    m_firstBox->setSuggestedValues(values);
    m_secondBox->setSuggestedValues(values);
}

void SearchFieldRangeInt::setSuggestedInitialValue(int value)
{
    m_firstBox->setSuggestedInitialValue(value);
    m_secondBox->setSuggestedInitialValue(value);
}

void SearchFieldRangeInt::setSingleSteps(int smaller, int larger)
{
    m_firstBox->setSingleSteps(smaller, larger);
    m_secondBox->setSingleSteps(smaller, larger);
}

void SearchFieldRangeInt::setInvertStepping(bool invert)
{
    m_firstBox->setInvertStepping(invert);
    m_secondBox->setInvertStepping(invert);
}

void SearchFieldRangeInt::valueChanged()
{
    bool validValue = false;

    if (m_reciprocal)
    {
        bool firstAtMinimum  = (m_firstBox->value()  == m_firstBox->minimum());
        bool secondAtMinimum = (m_secondBox->value() == m_secondBox->minimum());

        if (!secondAtMinimum)
        {
            m_firstBox->setRange(m_secondBox->value() - 1, m_max);
            validValue = true;
        }

        if (!firstAtMinimum)
        {
            m_secondBox->setRange(m_min - 1, m_firstBox->value());

            if (secondAtMinimum)
            {
                m_firstBox->setRange(m_min, m_max);
                m_secondBox->setValue(m_secondBox->minimum());
            }

            validValue = true;
        }

        if (firstAtMinimum && secondAtMinimum)
        {
            m_firstBox->setRange(m_min, m_max);
            m_secondBox->setRange(m_min, m_max);
        }
    }
    else
    {
        bool firstAtMinimum  = (m_firstBox->value()  == m_firstBox->minimum());
        bool secondAtMinimum = (m_secondBox->value() == m_secondBox->minimum());

        if (!secondAtMinimum)
        {
            m_firstBox->setRange(m_min, m_secondBox->value());
            validValue = true;
        }

        if (!firstAtMinimum)
        {
            m_secondBox->setRange(m_firstBox->value() - 1, m_max);

            if (secondAtMinimum)
            {
                m_firstBox->setRange(m_min, m_max);
                m_secondBox->setValue(m_secondBox->minimum());
            }

            validValue = true;
        }

        if (firstAtMinimum && secondAtMinimum)
        {
            m_firstBox->setRange(m_min, m_max);
            m_secondBox->setRange(m_min, m_max);
        }
    }

    setValidValueState(validValue);
}

void SearchFieldRangeInt::reset()
{
    m_firstBox->setRange(m_min, m_max);
    m_secondBox->setRange(m_min, m_max);
    m_firstBox->reset();
    m_secondBox->reset();
}

void SearchFieldRangeInt::setValueWidgetsVisible(bool visible)
{
    m_firstBox->setVisible(visible);
    m_secondBox->setVisible(visible);
    m_betweenLabel->setVisible(visible);
}

QList<QRect> SearchFieldRangeInt::valueWidgetRects() const
{
    QList<QRect> rects;
    rects << m_firstBox->geometry();
    rects << m_secondBox->geometry();

    return rects;
}

} // namespace Digikam
