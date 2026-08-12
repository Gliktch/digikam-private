/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-02-23
 * Description : a widget to filter album contents by text query
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dlayoutbox.h"
#include "searchtextbar.h"
#include "itemfiltersettings.h"

class QAction;

namespace Digikam
{

class SearchTextBar;

class TextFilter : public DHBox
{
    Q_OBJECT

public:

    explicit TextFilter(QWidget* const parent);
    ~TextFilter() override;

    SearchTextBar* searchTextBar() const;

    void reset();

    SearchTextFilterSettings::TextFilterFields searchTextFields();
    void setsearchTextFields(SearchTextFilterSettings::TextFilterFields fields);

Q_SIGNALS:

    void signalSearchTextFilterSettings(const SearchTextFilterSettings&);

private Q_SLOTS:

    void slotSearchTextFieldsChanged();
    void slotSearchFieldsChanged(QAction*);

private:

    void checkMenuActions(bool checked);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
