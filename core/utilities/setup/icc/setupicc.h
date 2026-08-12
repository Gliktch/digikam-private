/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-11-24
 * Description : Color management setup tab.
 *
 * SPDX-FileCopyrightText: 2005-2007 by F.J. Cruz <fj dot cruz at supercable dot es>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>
#include <QMap>
#include <QDir>
#include <QUrl>

// Local includes

#include "digikam_export.h"

class QDialogButtonBox;

namespace Digikam
{

class IccProfile;

class DIGIKAM_EXPORT SetupICC : public QScrollArea
{
    Q_OBJECT

public:

    enum ICCTab
    {
        Behavior = 0,
        Profiles,
        Advanced
    };

public:

    explicit SetupICC(QDialogButtonBox* const dlgBtnBox, QWidget* const parent = nullptr);
    ~SetupICC() override;

    void applySettings();

    void setActiveTab(ICCTab tab);
    ICCTab activeTab() const;

    static bool iccRepositoryIsValid();

private:

    void readSettings(bool restore = false);
    void fillCombos(bool report);
    void setWidgetsEnabled(bool enabled);
    void profileInfo(const IccProfile&);

private Q_SLOTS:

    void slotToggledEnabled();
    void slotUrlChanged();
    void slotUrlTextChanged();
    void slotClickedIn();
    void slotClickedWork();
    void slotClickedMonitor();
    void slotClickedProof();
    void slotShowDefaultSearchPaths();
    void slotMissingToggled(bool);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
