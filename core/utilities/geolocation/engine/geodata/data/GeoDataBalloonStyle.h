// SPDX-License-Identifier: LGPL-2.1-or-later
//
// SPDX-FileCopyrightText: 2012 Mohammed Nafees <nafees.technocool@gmail.com>
//

#pragma once

// Qt includes

#include <QColor>

// Local includes

#include "MarbleGlobal.h"
#include "GeoDataColorStyle.h"
#include "digikam_export.h"

namespace Marble
{

class GeoDataBalloonStylePrivate;

class DIGIKAM_EXPORT GeoDataBalloonStyle : public GeoDataColorStyle
{
public:

    enum DisplayMode
    {
        Default,
        Hide
    };

public:

    GeoDataBalloonStyle();

    GeoDataBalloonStyle(const GeoDataBalloonStyle& other);

    GeoDataBalloonStyle& operator=(const GeoDataBalloonStyle& other);
    bool operator==(const GeoDataBalloonStyle& other) const;
    bool operator!=(const GeoDataBalloonStyle& other) const;

    ~GeoDataBalloonStyle() override;

    /** Provides type information for downcasting a GeoNode */
    const char* nodeType() const override;


    QColor backgroundColor() const;
    void setBackgroundColor(const QColor& color);

    QColor textColor() const;
    void setTextColor(const QColor& color);

    QString text() const;
    void setText(const QString& text);

    DisplayMode displayMode() const;
    void setDisplayMode(DisplayMode mode);

    void pack(QDataStream& stream) const override;

    void unpack(QDataStream& stream) override;

private:

    GeoDataBalloonStylePrivate* const d = nullptr;
};

} // namespace Marble
