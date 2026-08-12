/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QUrl>

// Local includes

#include "dwizardpage.h"

namespace Digikam
{

class CollectionPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit CollectionPage(QWizard* const dlg);
    ~CollectionPage() override;

    bool checkSettings();
    void saveSettings();

    QString firstAlbumPath() const;

private:

    bool checkRootAlbum(QString& rootAlbumFolder);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
