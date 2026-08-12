/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-08-20
 * Description : Localization of Strings Config widget.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QTreeWidget>

// Local includes

#include "digikam_export.h"
#include "searchtextbar.h"

namespace Digikam
{

class DIGIKAM_EXPORT LanguagesList : public QTreeWidget
{
    Q_OBJECT

public:

    explicit LanguagesList(QWidget* const parent);

public Q_SLOTS:

    void slotSearchTextChanged(const SearchTextSettings& settings);

Q_SIGNALS:

    void signalSearchResult(int);
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT LocalizeConfig : public QWidget
{
    Q_OBJECT

public:

    explicit LocalizeConfig(QWidget* const parent = nullptr);
    ~LocalizeConfig() override;

    void applySettings();
    void readSettings();

private Q_SLOTS:

    void slotTranslatorChanged();

private:

    void populateAltLanguages();
    void populateTranslatorLanguages();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
