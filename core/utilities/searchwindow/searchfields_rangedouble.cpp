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

SearchFieldRangeDouble::SearchFieldRangeDouble(QObject* const parent)
    : SearchField(parent)
{
    m_betweenLabel = new QLabel;
    m_firstBox     = new CustomStepsDoubleSpinBox;
    m_secondBox    = new CustomStepsDoubleSpinBox;
}

void SearchFieldRangeDouble::setupValueWidgets(QGridLayout* layout, int row, int column)
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
    layout->addWidget(m_firstBox,     row, column);
    layout->addWidget(m_betweenLabel, row, column + 1, Qt::AlignHCenter);
    layout->addWidget(m_secondBox,    row, column + 2);

    connect(m_firstBox, SIGNAL(valueChanged(double)),
            this, SLOT(valueChanged()));

    connect(m_secondBox, SIGNAL(valueChanged(double)),
            this, SLOT(valueChanged()));
}

void SearchFieldRangeDouble::read(SearchXmlCachingReader& reader)
{
    SearchXml::Relation relation = reader.fieldRelation();

    if      ((relation == SearchXml::GreaterThanOrEqual) || (relation == SearchXml::GreaterThan))
    {
        m_firstBox->setValue(reader.valueToDouble() / m_factor);
    }
    else if ((relation == SearchXml::LessThanOrEqual) || (relation == SearchXml::LessThan))
    {
        m_secondBox->setValue(reader.valueToDouble() / m_factor);
    }
    else if ((relation == SearchXml::Interval) || (relation == SearchXml::IntervalOpen))
    {
        QList<double> list = reader.valueToDoubleList();

        if (list.size() != 2)
        {
            return;
        }

        m_firstBox->setValue(list.first() / m_factor);
        m_secondBox->setValue(list.last() / m_factor);
    }
}

void SearchFieldRangeDouble::write(SearchXmlWriter& writer)
{
    if ((m_firstBox->value() != m_firstBox->minimum()) && (m_secondBox->value() != m_secondBox->minimum()))
    {
        if (m_firstBox->value() != m_secondBox->value())
        {
            writer.writeField(m_name, SearchXml::Interval);
            writer.writeValue(QList<double>() << (m_firstBox->value() * m_factor) << (m_secondBox->value() * m_factor));
            writer.finishField();
        }
        else
        {
            // TODO: See SearchFieldRangeInt

            writer.writeField(m_name, SearchXml::Equal);
            writer.writeValue(m_firstBox->value() * m_factor);
            writer.finishField();
        }
    }
    else
    {
        if (m_firstBox->value() != m_firstBox->minimum())
        {
            writer.writeField(m_name, SearchXml::GreaterThanOrEqual);
            writer.writeValue(m_firstBox->value() * m_factor);
            writer.finishField();
        }

        if (m_secondBox->value() != m_secondBox->minimum())
        {
            writer.writeField(m_name, SearchXml::LessThanOrEqual);
            writer.writeValue(m_secondBox->value() * m_factor);
            writer.finishField();
        }
    }
}

void SearchFieldRangeDouble::setBetweenText(const QString& text)
{
    m_betweenLabel->setText(text);
}

void SearchFieldRangeDouble::setNoValueText(const QString& text)
{
    m_firstBox->setSpecialValueText(text);
    m_secondBox->setSpecialValueText(text);
}

void SearchFieldRangeDouble::setNumberPrefixAndSuffix(const QString& prefix, const QString& suffix)
{
    m_firstBox->setPrefix(prefix);
    m_secondBox->setPrefix(prefix);
    m_firstBox->setSuffix(suffix);
    m_secondBox->setSuffix(suffix);
}

void SearchFieldRangeDouble::setBoundary(double min, double max, int decimals, double step)
{
    m_min = min;
    m_max = max;

    m_firstBox->setRange(min, max);
    m_firstBox->setSingleStep(step);
    m_firstBox->setDecimals(decimals);
    m_firstBox->setValue(min);

    m_secondBox->setRange(min, max);
    m_secondBox->setSingleStep(step);
    m_secondBox->setDecimals(decimals);
    m_secondBox->setValue(min);
}

void SearchFieldRangeDouble::setFactor(double factor)
{
    m_factor = factor;
}

void SearchFieldRangeDouble::setSuggestedValues(const QList<double>& values)
{
    m_firstBox->setSuggestedValues(values);
    m_secondBox->setSuggestedValues(values);
}

void SearchFieldRangeDouble::setSuggestedInitialValue(double value)
{
    m_firstBox->setSuggestedInitialValue(value);
    m_secondBox->setSuggestedInitialValue(value);
}

void SearchFieldRangeDouble::setSingleSteps(double smaller, double larger)
{
    m_firstBox->setSingleSteps(smaller, larger);
    m_secondBox->setSingleSteps(smaller, larger);
}

void SearchFieldRangeDouble::setInvertStepping(bool invert)
{
    m_firstBox->setInvertStepping(invert);
    m_secondBox->setInvertStepping(invert);
}

void SearchFieldRangeDouble::valueChanged()
{
    bool validValue      = false;
    bool firstAtMinimum  = (m_firstBox->value()  == m_firstBox->minimum());
    bool secondAtMinimum = (m_secondBox->value() == m_secondBox->minimum());

    if (!secondAtMinimum)
    {
        m_firstBox->setRange(m_min, m_secondBox->value());
        validValue = true;
    }

    if (!firstAtMinimum)
    {
        m_secondBox->setRange(m_firstBox->value() - 0.1, m_max);

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

    setValidValueState(validValue);
}

void SearchFieldRangeDouble::reset()
{
    m_firstBox->setRange(m_min, m_max);
    m_secondBox->setRange(m_min, m_max);
    m_firstBox->reset();
    m_secondBox->reset();
}

void SearchFieldRangeDouble::setValueWidgetsVisible(bool visible)
{
    m_firstBox->setVisible(visible);
    m_secondBox->setVisible(visible);
    m_betweenLabel->setVisible(visible);
}

QList<QRect> SearchFieldRangeDouble::valueWidgetRects() const
{
    QList<QRect> rects;
    rects << m_firstBox->geometry();
    rects << m_secondBox->geometry();

    return rects;
}

} // namespace Digikam
