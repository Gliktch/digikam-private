/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : setup Image Editor page
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>

namespace Digikam
{

class SetupEditor : public QScrollArea
{
    Q_OBJECT

public:

    enum EditorTab
    {
        EditorWindow = 0,
        Versioning,
        SaveSettings,
        RAWBehavior,
        RAWDefaultSettings
    };

public:

    explicit SetupEditor(QWidget* const parent = nullptr);
    ~SetupEditor() override;

    void setActiveTab(EditorTab tab);
    EditorTab activeTab() const;

    void applySettings();

private:

    void readSettings();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
