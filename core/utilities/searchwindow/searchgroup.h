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

#pragma once

// Qt includes

#include <QWidget>
#include <QList>

// Local includes

#include "dexpanderbox.h"
#include "searchview.h"
#include "coredbsearchxml.h"

class QVBoxLayout;

namespace Digikam
{

class SearchGroupLabel;
class SearchFieldGroup;
class SearchFieldGroupLabel;

class SearchGroup : public AbstractSearchGroupContainer
{
    Q_OBJECT

public:

    enum Type
    {
        FirstGroup,
        ChainGroup
    };

public:

    explicit SearchGroup(SearchView* const parent);

    void setup(Type type = FirstGroup);

    void read(SearchXmlCachingReader& reader);
    void write(SearchXmlWriter& writer);
    void reset();

    Type groupType()                    const;

    QList<QRect> startupAnimationArea() const;

Q_SIGNALS:

    void removeRequested();

protected:

    SearchGroup* createSearchGroup()          override;
    void addGroupToLayout(SearchGroup* group) override;

protected:

    SearchView*                   m_view            = nullptr;

    QList<SearchFieldGroup*>      m_fieldGroups;
    QList<SearchFieldGroupLabel*> m_fieldLabels;

    QVBoxLayout*                  m_layout          = nullptr;
    SearchGroupLabel*             m_label           = nullptr;
    QVBoxLayout*                  m_subgroupLayout  = nullptr;

    Type                          m_groupType       = FirstGroup;
};

// -------------------------------------------------------------------------

class SearchGroupLabel : public QWidget
{
    Q_OBJECT

public:

    SearchGroupLabel(SearchViewThemedPartsCache* const cache,
                     SearchGroup::Type type,
                     QWidget* const parent = nullptr);
    ~SearchGroupLabel()             override;

    void setGroupOperator(SearchXml::Operator op);
    void setDefaultFieldOperator(SearchXml::Operator op);

    SearchXml::Operator groupOperator()        const;
    SearchXml::Operator defaultFieldOperator() const;

Q_SIGNALS:

    void removeClicked();

protected Q_SLOTS:

    void toggleShowOptions();
    void toggleGroupOperator();
    void boxesToggled();

protected:

    void setExtended(bool extended);
    void adjustOperatorOptions();
    void updateGroupLabel();

    void paintEvent(QPaintEvent*)   override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
