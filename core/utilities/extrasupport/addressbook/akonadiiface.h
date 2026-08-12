/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-15
 * Description : Plasma Address Book contacts interface
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QMenu>
#include <QAction>

// Local includes

#include "digikam_export.h"
#include "digikam_config.h"

class KJob;

namespace Digikam
{

class DIGIKAM_EXPORT AkonadiIface : public QObject
{
    Q_OBJECT

public:

    explicit AkonadiIface(QMenu* const parent);
    virtual ~AkonadiIface();

Q_SIGNALS:

    void signalContactTriggered(const QString&);

private Q_SLOTS:

    void slotABCSearchResult(KJob*);
    void slotABCMenuTriggered(QAction*);

private:

    QMenu* m_parent  = nullptr;
    QMenu* m_ABCmenu = nullptr;
};

} // namespace Digikam
