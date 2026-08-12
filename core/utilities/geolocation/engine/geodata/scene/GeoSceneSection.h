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
#include <QVector>

// Local includes

#include "GeoDocument.h"

namespace Marble
{

class GeoSceneItem;

/**
 * @short Section of a GeoScene document.
 */
class DIGIKAM_EXPORT GeoSceneSection : public GeoNode
{
public:

    explicit GeoSceneSection(const QString& name);
    ~GeoSceneSection() override;

    const char* nodeType() const override;

    /**
     * @brief  Add an item to the legend section
     * @param  item  the new item
     */
    void addItem(GeoSceneItem*);
    GeoSceneItem* item(const QString&);
    QVector<GeoSceneItem*> items() const;

    QString name() const;

    QString heading() const;
    void setHeading(const QString& heading);

    bool checkable() const;
    void setCheckable(bool checkable);

    QString connectTo() const;
    void setConnectTo(const QString& text);

    int  spacing() const;
    void setSpacing(int spacing);

    QString radio() const;
    void setRadio(const QString& radio);

private:

    Q_DISABLE_COPY(GeoSceneSection)

private:

    /// The vector holding all the items in the legend section.
    /// (We want to preserve the order and don't care
    /// much about speed here), so we don't use a hash
    QVector<GeoSceneItem*>  m_items;

    QString                 m_name;
    QString                 m_heading;
    QString                 m_connectTo;
    QString                 m_radio;

    bool                    m_checkable;
    int                     m_spacing;
};

} // namespace Marble
