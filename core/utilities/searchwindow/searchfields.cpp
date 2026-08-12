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

SearchField::SearchField(QObject* const parent)
    : QObject(parent)
{
    m_label                = new QLabel;
    m_detailLabel          = new QLabel;
    m_clearButton          = new AnimatedClearButton;
}

void SearchField::setup(QGridLayout* const layout, int line)
{
    if (line == -1)
    {
        line = layout->rowCount();
    }

    // 10px indent

    layout->setColumnMinimumWidth(0, 10);

    // set stretch for the value widget columns

    layout->setColumnStretch(3, 1);
    layout->setColumnStretch(5, 1);

    // push value widgets to the left

    layout->setColumnStretch(6, 1);

    setupLabels(layout, line);

    // value widgets can use columns 3,4,5.
    // In the case of "from ... to ..." fields, column 3 and 5 can contain spin boxes etc.,
    // and 4 can contain a label in between.
    // In other cases, a widget or sublayout spanning the three columns is recommended.

    setupValueWidgets(layout, line, 3);

    // setup the clear button that appears dynamically

    if (qApp->isLeftToRight())
    {
        m_clearButton->setPixmap(QIcon::fromTheme(QLatin1String("edit-clear-locationbar-rtl"))
                                 .pixmap(QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize)));
    }
    else
    {
        m_clearButton->setPixmap(QIcon::fromTheme(QLatin1String("edit-clear-locationbar-ltr"))
                                 .pixmap(QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize)));
    }

    // Important: Don't cause re-layouting when button gets hidden/shown!

    m_clearButton->stayVisibleWhenAnimatedOut(true);
    m_clearButton->setToolTip(i18n("Reset contents"));

    connect(m_clearButton, SIGNAL(clicked()),
            this, SLOT(clearButtonClicked()));

    connect(m_clearButton, SIGNAL(visibleChanged(bool)),
            m_clearButton, SLOT(slotPixmapEnabled(bool)));

    layout->addWidget(m_clearButton, line, 7);
}

void SearchField::setupLabels(QGridLayout* layout, int line)
{
    m_label->setObjectName(QLatin1String("SearchField_MainLabel"));
    m_detailLabel->setObjectName(QLatin1String("SearchField_DetailLabel"));
    layout->addWidget(m_label, line, 1);
    layout->addWidget(m_detailLabel, line, 2);
}

void SearchField::setFieldName(const QString& fieldName)
{
    m_name = fieldName;
}

void SearchField::setText(const QString& label, const QString& detailLabel)
{
    m_label->setText(label);
    m_detailLabel->setText(detailLabel);
}

bool SearchField::supportsField(const QString& fieldName)
{
    return (m_name == fieldName);
}

void SearchField::setVisible(bool visible)
{
    m_label->setVisible(visible && m_categoryLabelVisible);
    m_detailLabel->setVisible(visible);
    m_clearButton->setShallBeShown(visible);
    setValueWidgetsVisible(visible);
}

bool SearchField::isVisible()
{
    // the detail label is considered representative for all widgets

    return m_detailLabel->isVisible();
}

void SearchField::setCategoryLabelVisible(bool visible)
{
    if (m_categoryLabelVisible == visible)
    {
        return;
    }

    m_categoryLabelVisible = visible;

    // update status: compare setVisible() and isVisible()

    m_label->setVisible(m_detailLabel->isVisible() && m_categoryLabelVisible);
}

void SearchField::setCategoryLabelVisibleFromPreviousField(SearchField* previousField)
{
    if (previousField->m_label->text() == m_label->text())
    {
        setCategoryLabelVisible(false);
    }
    else
    {
        setCategoryLabelVisible(true);
    }

    if (
        (previousField->m_name == QLatin1String("filesize")) &&
        (this->m_name          == QLatin1String("bytesize"))
       )
    {
        connect(previousField, &SearchField::signalVisibilityChanged,
                this, &SearchField::clearButtonClicked);

        connect(this, &SearchField::signalVisibilityChanged,
                previousField, &SearchField::clearButtonClicked);
    }
}

QList<QRect> SearchField::widgetRects(WidgetRectType type) const
{
    QList<QRect> rects;

    if (type == LabelAndValueWidgetRects)
    {
        rects << m_label->geometry();
        rects << m_detailLabel->geometry();
    }

    rects += valueWidgetRects();

    return rects;
}

void SearchField::clearButtonClicked()
{
    reset();
}

void SearchField::setValidValueState(bool valueIsValid)
{
    if (valueIsValid != m_valueIsValid)
    {
        m_valueIsValid = valueIsValid;

        // NOTE: setVisible visibility is independent from animateVisible visibility!

        m_clearButton->animateVisible(m_valueIsValid);

        Q_EMIT signalVisibilityChanged();
    }
}

} // namespace Digikam

#include "moc_searchfields.cpp"
