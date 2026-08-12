/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-20
 * Description : template list view.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTreeWidget>

// Local includes

#include "template.h"

namespace Digikam
{

class TemplateListItem : public QTreeWidgetItem
{

public:

    explicit TemplateListItem(QTreeWidget* const parent, const Template& t);
    ~TemplateListItem()          override;

    void     setTemplate(const Template& t);
    Template getTemplate() const;

private:

    Template m_template;

private:

    Q_DISABLE_COPY(TemplateListItem)
};

// -------------------------------------------------------------------

class TemplateList : public QTreeWidget
{
    Q_OBJECT

public:

    explicit TemplateList(QWidget* const parent = nullptr);
    ~TemplateList() override;

    TemplateListItem* find(const QString& title);

    void readSettings();
    void applySettings();
};

} // namespace Digikam
