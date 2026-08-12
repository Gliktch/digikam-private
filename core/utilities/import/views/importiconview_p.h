/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-25-07
 * Description : Private Qt item view for images
 *
 * SPDX-FileCopyrightText:      2012 by Islam Wazery <wazery at ubuntu dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "importiconview.h"
#include "importdelegate.h"
#include "importsettings.h"
#include "importoverlays.h"

namespace Digikam
{

class ImportNormalDelegate;

class Q_DECL_HIDDEN ImportIconView::Private : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(ImportIconView)

public:

    explicit Private(ImportIconView* const qq);
    ~Private() override = default;

    void updateOverlays();

public:

    ItemViewUtilities*     utilities            = nullptr;
    ImportNormalDelegate*  normalDelegate       = nullptr;

    bool                   overlaysActive       = false;

    ImportRotateOverlay*   rotateLeftOverlay    = nullptr;
    ImportRotateOverlay*   rotateRightOverlay   = nullptr;

private:

    ImportIconView*        q_ptr                = nullptr;
};

} // namespace Digikam
