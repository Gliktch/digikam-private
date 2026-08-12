/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-12
 * Description : OSM Nominatim backend for Reverse Geocoding
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2010      by Gabriel Voicu <ping dot gabi at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QNetworkReply>
#include <QList>
#include <QUrl>
#include <QMap>

// Local includes

#include "backend-rg.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT BackendOsmRG : public RGBackend
{
    Q_OBJECT

public:

    explicit BackendOsmRG(QObject* const parent);
    ~BackendOsmRG()                                                         override;

    QMap<QString, QString> makeQMapFromXML(const QString& xmlData);

    void callRGBackend(const QList<RGInfo>& rgList,const QString& language) override;
    QString getErrorMessage()                                               override;
    QString backendName()                                                   override;
    void cancelRequests()                                                   override;

private Q_SLOTS:

    void nextPhoto();
    void slotFinished(QNetworkReply* reply);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
