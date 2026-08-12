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

// Local includes

#include "GeoDataContainer.h"
#include "digikam_export.h"

class QDateTime;

namespace Marble
{

class GeoDataAbstractView;
class GeoDataUpdate;
class GeoDataNetworkLinkControlPrivate;

class DIGIKAM_EXPORT GeoDataNetworkLinkControl : public GeoDataContainer
{
public:

    GeoDataNetworkLinkControl();

    GeoDataNetworkLinkControl(const GeoDataNetworkLinkControl& other);

    ~GeoDataNetworkLinkControl() override;

    GeoDataNetworkLinkControl& operator=(const GeoDataNetworkLinkControl& other);

    bool operator==(const GeoDataNetworkLinkControl& other) const;
    bool operator!=(const GeoDataNetworkLinkControl& other) const;

    GeoDataFeature* clone() const override;

    /** Provides type information for downcasting a GeoNode */
    const char* nodeType() const override;

    qreal minRefreshPeriod() const;
    void setMinRefreshPeriod(qreal minRefreshPeriod);

    qreal maxSessionLength() const;
    void setMaxSessionLength(qreal maxSessionLength);

    QString cookie() const;
    void setCookie(const QString& cookie);

    QString message() const;
    void setMessage(const QString& message);

    QString linkName() const;
    void setLinkName(const QString& linkName);

    QString linkDescription() const;
    void setLinkDescription(const QString& linkDescription);

    QString linkSnippet() const;
    void setLinkSnippet(const QString& linkSnippet);
    int maxLines() const;
    void setMaxLines(int maxLines);

    QDateTime expires() const;
    void setExpires(const QDateTime& expires);

    GeoDataUpdate& update();
    const GeoDataUpdate& update() const;
    void setUpdate(const GeoDataUpdate& update);

    GeoDataAbstractView* networkAbstractView() const;
    /**
     * Sets the abstract view and takes control of this pointer.
     */
    void setNetworkAbstractView(GeoDataAbstractView* abstractView);

private:

    Q_DECLARE_PRIVATE(GeoDataNetworkLinkControl)
};

} // namespace Marble
