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

#include <QString>

// Local includes

#include "GeoDocument.h"

namespace Marble
{

class GeoSceneIcon;

/**
 * @short The section item in a legend of a GeoScene document.
 */
class DIGIKAM_EXPORT GeoSceneItem : public GeoNode
{
public:

    explicit GeoSceneItem(const QString& name);
    ~GeoSceneItem() override;

    const char* nodeType() const override;

    QString name() const;

    QString text() const;
    void setText(const QString& text);

    bool checkable() const;
    void setCheckable(bool checkable);

    QString connectTo() const;
    void setConnectTo(const QString& text);

    int  spacing() const;
    void setSpacing(int spacing);

    const GeoSceneIcon* icon() const;
    GeoSceneIcon* icon();

private:

    Q_DISABLE_COPY(GeoSceneItem)

private:

    GeoSceneIcon*   m_icon = nullptr;

    QString         m_name;
    QString         m_text;
    QString         m_connectTo;

    bool            m_checkable;
    int             m_spacing;
};

} // namespace Marble
