/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDialog>

// Local includes

#include "digikam_export.h"

class QString;

namespace Marble
{

class TileCreator;
class TileCreatorDialogPrivate;

class DIGIKAM_EXPORT TileCreatorDialog : public QDialog
{
    Q_OBJECT

public:

    explicit TileCreatorDialog(TileCreator* creator, QWidget* parent = nullptr);
    ~TileCreatorDialog() override;

public Q_SLOTS:

    void setProgress(int progress);
    void setSummary(const QString& name, const QString& description);

private Q_SLOTS:

    void cancelTileCreation();

private:

    Q_DISABLE_COPY(TileCreatorDialog)

private:

    TileCreatorDialogPrivate* const d = nullptr;
};

} // namespace Marble
