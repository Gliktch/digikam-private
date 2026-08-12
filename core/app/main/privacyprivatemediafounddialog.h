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
#include <QSet>

// Local includes

#include "digikam_gui_export.h"
#include "privacyportablediscovery.h"

namespace Digikam
{

/**
 * Grouped password authentication/preflight for portable private-media
 * import. Rows are verified asynchronously; Continue Import publishes every
 * successfully unlocked category and leaves unresolved rows untouched.
 */
class DIGIKAM_GUI_EXPORT PrivacyPrivateMediaFoundDialog : public QDialog
{
    Q_OBJECT

public:

    explicit PrivacyPrivateMediaFoundDialog(
        const QString& scanRoot,
        const PrivacyPortableDiscoveryResult& discovery,
        QWidget* parent = nullptr);
    ~PrivacyPrivateMediaFoundDialog() override;

    /** Runs discovery on the selected folder and shows the dialog only when
     * candidates exist. Returns true when at least one category was
     * imported. */
    static bool offer(const QString& scanRoot, QWidget* parent = nullptr);

    /** Proxy paths with positive privacy evidence whose recovery identity is
     * not yet mapped to a P1 category. These must not be scanned ordinarily. */
    static QSet<QString> unresolvedProxyPaths(
        const PrivacyPortableDiscoveryResult& discovery,
        const PrivacyRepositorySnapshot& snapshot);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
