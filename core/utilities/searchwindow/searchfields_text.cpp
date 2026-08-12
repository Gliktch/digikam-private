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

SearchFieldText::SearchFieldText(QObject* const parent)
    : SearchField(parent)
{
}

void SearchFieldText::setupValueWidgets(QGridLayout* layout, int row, int column)
{
    m_edit = new QLineEdit;
    layout->addWidget(m_edit, row, column, 1, 3);

    connect(m_edit, SIGNAL(textChanged(QString)),
            this, SLOT(valueChanged(QString)));
}

void SearchFieldText::read(SearchXmlCachingReader& reader)
{
    QString value = reader.value();
    m_edit->setText(value);
}

void SearchFieldText::write(SearchXmlWriter& writer)
{
    QString value = m_edit->text();

    if (!value.isEmpty())
    {
        writer.writeField(m_name, SearchXml::Like);
        writer.writeValue(value);
        writer.finishField();
    }
}

void SearchFieldText::reset()
{
    m_edit->setText(QString());
}

void SearchFieldText::setValueWidgetsVisible(bool visible)
{
    m_edit->setVisible(visible);
}

QList<QRect> SearchFieldText::valueWidgetRects() const
{
    QList<QRect> rects;
    rects << m_edit->geometry();

    return rects;
}

void SearchFieldText::valueChanged(const QString& text)
{
    setValidValueState(!text.isEmpty());
}

} // namespace Digikam
