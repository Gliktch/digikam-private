/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : Setup view panel for dplugins.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>

namespace ShowFoto
{

class ShowfotoSetupPlugins : public QScrollArea
{
    Q_OBJECT

public:

    enum PluginTab
    {
        Generic = 0,
        Editor,
        Loaders
    };

public:

    explicit ShowfotoSetupPlugins(QWidget* const parent = nullptr);
    ~ShowfotoSetupPlugins()     override;

    void setActiveTab(PluginTab tab);
    PluginTab activeTab() const;

    void applySettings();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace ShowFoto
