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

SearchFieldColorDepth::SearchFieldColorDepth(QObject* const parent)
    : SearchFieldComboBox(parent)
{
}

void SearchFieldColorDepth::setupValueWidgets(QGridLayout* layout, int row, int column)
{
    SearchFieldComboBox::setupValueWidgets(layout, row, column);
    m_comboBox->addItem(i18n("any color depth"));
    m_comboBox->addItem(i18n("8 bits per channel"), 8);
    m_comboBox->addItem(i18n("16 bits per channel"), 16);

    m_comboBox->setCurrentIndex(0);
}

void SearchFieldColorDepth::read(SearchXmlCachingReader& reader)
{
    SearchXml::Relation relation = reader.fieldRelation();

    if (relation == SearchXml::Equal)
    {
        int bits = reader.valueToInt();

        if      (bits == 8)
        {
            m_comboBox->setCurrentIndex(1);
        }
        else if (bits == 16)
        {
            m_comboBox->setCurrentIndex(2);
        }
    }
}

} // namespace Digikam
