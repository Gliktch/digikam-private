/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-01-20
 * Description : User interface for searches
 *
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QSet>
#include <QWidget>

// Local includes

#include "dexpanderbox.h"
#include "coredbsearchxml.h"

class QGridLayout;
class QLabel;
class QVBoxLayout;

namespace Digikam
{

class SearchFieldGroupLabel;
class SearchGroup;
class SearchField;
class VisibilityController;

class SearchFieldGroup : public QWidget
{
    Q_OBJECT

public:

    explicit SearchFieldGroup(SearchGroup* const parent);

    void addField(SearchField* const field);
    void setLabel(SearchFieldGroupLabel* const label);

    SearchField* fieldForName(const QString& fieldName) const;
    void write(SearchXmlWriter& writer);

    void reset();

    void markField(SearchField* const field);
    void clearMarkedFields();
    QList<QRect> areaOfMarkedFields()                   const;

public Q_SLOTS:

    void setFieldsVisible(bool visible);

protected Q_SLOTS:

    void slotLabelClicked();

protected:

    QList<SearchField*>    m_fields;
    QGridLayout*           m_layout     = nullptr;
    SearchFieldGroupLabel* m_label      = nullptr;
    VisibilityController*  m_controller = nullptr;
    QSet<SearchField*>     m_markedFields;

private:

    /// @note disabled
    SearchFieldGroup(QWidget*) = delete;
};

// -----------------------------------------------------------------------

class SearchFieldGroupLabel : public QWidget
{
    Q_OBJECT

public:

    explicit SearchFieldGroupLabel(QWidget* const parent);

    void setTitle(const QString& title);

public Q_SLOTS:

    void displayExpanded();
    void displayFolded();

Q_SIGNALS:

    void clicked();

protected:

    QString      m_title;
    DClickLabel* m_titleLabel   = nullptr;
    QLabel*      m_expandLabel  = nullptr;
};

} // namespace Digikam
