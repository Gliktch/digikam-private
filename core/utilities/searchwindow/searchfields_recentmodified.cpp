/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2026-02-07
 * Description : User interface for searches
 *
 * SPDX-FileCopyrightText: 2026 by Srirupa Datta <srirupa dot sps at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "searchfields_p.h"

// Qt includes

#include <QComboBox>
#include <QGridLayout>
#include <QSizePolicy>
#include <QSpinBox>

// Local includes

#include "searchutilities.h"

namespace Digikam
{

SearchFieldRecentModified::SearchFieldRecentModified(QObject* const parent)
    : SearchField(parent)
{
}

void SearchFieldRecentModified::setupValueWidgets(QGridLayout* layout, int row, int column)
{
    m_amountBox = new QSpinBox;                 // cppcheck-suppress publicAllocationError
    m_amountBox->setObjectName(QLatin1String("SearchFieldRecentModified_Amount"));
    m_amountBox->setRange(0, 999999);
    m_amountBox->setValue(0);

    m_unitCombo = new QComboBox;
    m_unitCombo->setObjectName(QLatin1String("SearchFieldRecentModified_UnitCombo"));
    m_unitCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    m_unitCombo->addItem(i18n("seconds"), 1);
    m_unitCombo->addItem(i18n("minutes"), 60);
    m_unitCombo->addItem(i18n("hours"),   3600);
    m_unitCombo->addItem(i18n("days"),    86400);
    m_unitCombo->setCurrentIndex(-1);

    connect(m_amountBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SearchFieldRecentModified::unitChanged);

    connect(m_unitCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SearchFieldRecentModified::unitChanged);

    layout->addWidget(m_amountBox, row, column,     1, 1);
    layout->addWidget(m_unitCombo, row, column + 1, 1, 2);

    setValidValueState(false);

    if (m_pendingSeconds > 0)
    {
        applySeconds(m_pendingSeconds);
        m_pendingSeconds = -1;
    }
}

void SearchFieldRecentModified::applySeconds(int seconds)
{
    if (!m_amountBox || !m_unitCombo)
    {
        return;
    }

    if (seconds <= 0)
    {
        reset();

        return;
    }

    struct Unit { int mult; };
    const Unit units[] =
    {
        { 86400 },
        { 3600  },
        { 60    },
        { 1     }
    };

    for (const Unit& u : units)
    {
        if ((seconds % u.mult) == 0)
        {
            m_amountBox->setValue(seconds / u.mult);

            for (int i = 0 ; i < m_unitCombo->count() ; ++i)
            {
                if (m_unitCombo->itemData(i).toInt() == u.mult)
                {
                    m_unitCombo->setCurrentIndex(i);
                    break;
                }
            }

            setValidValueState(deltaSeconds() > 0);

            return;
        }
    }

    m_amountBox->setValue(seconds);
    m_unitCombo->setCurrentIndex(0);

    setValidValueState(deltaSeconds() > 0);
}

void SearchFieldRecentModified::unitChanged()
{
    setValidValueState(deltaSeconds() > 0);
}

qint64 SearchFieldRecentModified::deltaSeconds() const
{
    if (!m_amountBox || !m_unitCombo)
    {
        return 0;
    }

    if (m_unitCombo->currentIndex() < 0)
    {
        return 0;
    }

    const int amount = m_amountBox->value();

    if (amount <= 0)
    {
        return 0;
    }

    const qint64 mult = m_unitCombo->currentData().toLongLong();

    return ((mult > 0) ? qint64(amount) * mult : 0);
}

void SearchFieldRecentModified::read(SearchXmlCachingReader& reader)
{
    const int seconds = reader.valueToInt();

    if (!m_amountBox || !m_unitCombo)
    {
        m_pendingSeconds = seconds;

        return;
    }

    applySeconds(seconds);
}

void SearchFieldRecentModified::write(SearchXmlWriter& writer)
{
    const qint64 seconds = deltaSeconds();

    if (seconds <= 0)
    {
        return;
    }

    writer.writeField(m_name, SearchXml::Equal);
    writer.writeValue(int(seconds));
    writer.finishField();
}

void SearchFieldRecentModified::reset()
{
    if (m_amountBox)
    {
        m_amountBox->setValue(0);
    }

    if (m_unitCombo)
    {
        m_unitCombo->setCurrentIndex(-1);
    }

    setValidValueState(false);
}

void SearchFieldRecentModified::setValueWidgetsVisible(bool visible)
{
    if (m_amountBox)
    {
        m_amountBox->setVisible(visible);
    }

    if (m_unitCombo)
    {
        m_unitCombo->setVisible(visible);
    }
}

QList<QRect> SearchFieldRecentModified::valueWidgetRects() const
{
    QList<QRect> rects;

    if (m_amountBox)
    {
        rects << m_amountBox->geometry();
    }

    if (m_unitCombo)
    {
        rects << m_unitCombo->geometry();
    }

    return rects;
}

} // namespace Digikam
