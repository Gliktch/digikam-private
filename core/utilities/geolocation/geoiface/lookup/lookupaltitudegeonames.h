/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-04-30
 * Description : Class for geonames.org based altitude lookup
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QNetworkReply>

// Local includes

#include "lookupaltitude.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT LookupAltitudeGeonames : public LookupAltitude
{
    Q_OBJECT

public:

    explicit LookupAltitudeGeonames(QObject* const parent);
    ~LookupAltitudeGeonames()                             override;

    QString backendName()                           const override;
    QString backendHumanName()                      const override;

    void addRequests(const Request::List& requests)       override;
    Request::List getRequests()                     const override;
    Request getRequest(const int index)             const override;

    void startLookup()                                    override;
    StatusAltitude getStatus()                      const override;
    QString errorMessage()                          const override;
    void cancel()                                         override;

private Q_SLOTS:

    void slotFinished(QNetworkReply* reply);

private:

    void startNextRequest();

private:

    class Private;
    const QScopedPointer<Private> d;
};

} // namespace Digikam
