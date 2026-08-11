/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDialog>

// Local includes

#include "digikam_gui_export.h"

namespace Digikam
{

enum class PrivacyProfileImportOfferResult
{
    NotShown,
    Dismissed,
    PublicationPrepared
};

class DIGIKAM_GUI_EXPORT PrivacyProfileImportDialog : public QDialog
{
    Q_OBJECT

public:

    explicit PrivacyProfileImportDialog(bool startupOffer,
                                        QWidget* parent = nullptr);
    ~PrivacyProfileImportDialog() override;

    static PrivacyProfileImportOfferResult offerAtStartup(QWidget* parent = nullptr);
    static bool restorePreviousProfile(QWidget* parent = nullptr);

    bool publicationPrepared() const;

protected:

    void reject() override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
