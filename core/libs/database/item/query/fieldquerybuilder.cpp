/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-22
 * Description : Building complex database SQL queries from search descriptions
 *
 * SPDX-FileCopyrightText: 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2026      by Srirupa Datta <srirupa dot sps at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "fieldquerybuilder.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QFile>
#include <QDir>
#include <QMap>
#include <QRectF>
#include <QUrl>
#include <QLocale>
#include <QUrlQuery>

// Local includes

#include "itemquerybuilder.h"
#include "digikam_debug.h"
#include "coredbaccess.h"
#include "coredb.h"
#include "geodetictools.h"

namespace Digikam
{

FieldQueryBuilder::FieldQueryBuilder(QString& sql,
                                     SearchXmlCachingReader& reader,
                                     QList<QVariant>* boundValues,
                                     ItemQueryPostHooks* const hooks,
                                     SearchXml::Relation relation)
    : sql        (sql),
      reader     (reader),
      boundValues(boundValues),
      hooks      (hooks),
      relation   (relation)
{
}

QString FieldQueryBuilder::prepareForLike(const QString& str) const
{
    if ((relation == SearchXml::Like) || (relation == SearchXml::NotLike))
    {
        return QLatin1Char('%') + str + QLatin1Char('%');
    }
    else
    {
        return str;
    }
}

void FieldQueryBuilder::addIntField(const QString& name)
{
    if ((relation == SearchXml::Interval) || (relation == SearchXml::IntervalOpen))
    {
        QList<int> values = reader.valueToIntList();

        if (values.size() != 2)
        {
            qCWarning(DIGIKAM_DATABASE_LOG) << "Relation Interval requires a list of two values";
            return;
        }

        sql += QLatin1String(" (") + name + QLatin1Char(' ');
        ItemQueryBuilder::addSqlRelation(sql,
                                         relation == SearchXml::Interval ? SearchXml::GreaterThanOrEqual
                                                                         : SearchXml::GreaterThan);
        sql += QLatin1String(" ? AND ") + name + QLatin1Char(' ');
        ItemQueryBuilder::addSqlRelation(sql,
                                         relation == SearchXml::Interval ? SearchXml::LessThanOrEqual
                                                                         : SearchXml::LessThan);
        sql += QLatin1String(" ?) ");

        *boundValues << values.first() << values.last();
    }
    else
    {
        sql += QLatin1String(" (") + name + QLatin1Char(' ');
        ItemQueryBuilder::addSqlRelation(sql, relation);
        sql += QLatin1String(" ?) ");
        *boundValues << reader.valueToInt();
    }
}

void FieldQueryBuilder::addLongField(const QString& name)
{
    if ((relation == SearchXml::Interval) || (relation == SearchXml::IntervalOpen))
    {
        QList<qlonglong> values = reader.valueToLongLongList();

        if (values.size() != 2)
        {
            qCWarning(DIGIKAM_DATABASE_LOG) << "Relation Interval requires a list of two values";
            return;
        }

        sql += QLatin1String(" (") + name + QLatin1Char(' ');
        ItemQueryBuilder::addSqlRelation(sql,
                                         relation == SearchXml::Interval ? SearchXml::GreaterThanOrEqual
                                                                         : SearchXml::GreaterThan);
        sql += QLatin1String(" ? AND ") + name + QLatin1Char(' ');
        ItemQueryBuilder::addSqlRelation(sql,
                                         relation == SearchXml::Interval ? SearchXml::LessThanOrEqual
                                                                         : SearchXml::LessThan);
        sql += QLatin1String(" ?) ");

        *boundValues << values.first() << values.last();
    }
    else
    {
        sql += QLatin1String(" (") + name + QLatin1Char(' ');
        ItemQueryBuilder::addSqlRelation(sql, relation);
        sql += QLatin1String(" ?) ");
        *boundValues << reader.valueToLongLong();
    }
}

void FieldQueryBuilder::addDoubleField(const QString& name)
{
    if ((relation == SearchXml::Interval) || (relation == SearchXml::IntervalOpen))
    {
        QList<double> values = reader.valueToDoubleList();

        if (values.size() != 2)
        {
            qCWarning(DIGIKAM_DATABASE_LOG) << "Relation Interval requires a list of two values";

            return;
        }

        sql += QLatin1String(" (") + name + QLatin1Char(' ');
        ItemQueryBuilder::addSqlRelation(sql,
                                         relation == SearchXml::Interval ? SearchXml::GreaterThanOrEqual
                                                                         : SearchXml::GreaterThan);
        sql += QLatin1String(" ? AND ") + name + QLatin1Char(' ');
        ItemQueryBuilder::addSqlRelation(sql,
                                         relation == SearchXml::Interval ? SearchXml::LessThanOrEqual
                                                                         : SearchXml::LessThan);
        sql += QLatin1String(" ?) ");

        *boundValues << values.first() << values.last();
    }
    else
    {
        sql += QLatin1String(" (") + name + QLatin1Char(' ');
        ItemQueryBuilder::addSqlRelation(sql, relation);
        sql += QLatin1String(" ?) ");
        *boundValues << reader.valueToDouble();
    }
}

void FieldQueryBuilder::addStringField(const QString& name)
{
    sql += QLatin1String(" (") + name + QLatin1Char(' ');
    ItemQueryBuilder::addSqlRelation(sql, relation);

    if (
        CoreDbAccess::parameters().isSQLite() &&
        (
         (relation == SearchXml::Like)        ||
         (relation == SearchXml::NotLike)
        )
       )
    {
        sql += QLatin1String(" ? ESCAPE '\\') ");
    }
    else
    {
        sql += QLatin1String(" ?) ");
    }

    *boundValues << prepareForLike(reader.value());
}

void FieldQueryBuilder::addDateField(const QString& name)
{
    if (relation == SearchXml::Equal)
    {
        // special case: split in < and >

        QDateTime date = QDateTime::fromString(reader.value(), Qt::ISODate);

        if (!date.isValid())
        {
            qCWarning(DIGIKAM_DATABASE_LOG) << "Date" << reader.value() << "is invalid";

            return;
        }

        if (date.time() == QTime(0, 0, 0, 0))
        {
            // day precision

            QDate startDate, endDate;
            startDate = date.date().addDays(-1);
            endDate   = date.date().addDays(1);
            *boundValues << startDate.toString(Qt::ISODate)
                         << endDate.toString(Qt::ISODate);
        }
        else
        {
            // sub-day precision

            QDateTime startDate, endDate;
            int diff;

            if      (date.time().hour() == 0)
            {
                diff = 3600;
            }
            else if (date.time().minute() == 0)
            {
                diff = 60;
            }
            else
            {
                diff = 1;
            }

            // we spare microseconds for the future

            startDate = date.addSecs(-diff);
            endDate   = date.addSecs(diff);
            *boundValues << startDate.toString(Qt::ISODate)
                         << endDate.toString(Qt::ISODate);
        }

        sql += QLatin1String(" (") + name + QLatin1Char(' ');
        ItemQueryBuilder::addSqlRelation(sql, SearchXml::GreaterThan);
        sql += QLatin1String(" ? AND ") + name + QLatin1Char(' ');
        ItemQueryBuilder::addSqlRelation(sql, SearchXml::LessThan);
        sql += QLatin1String(" ?) ");
    }
    else if (relation == SearchXml::Interval || relation == SearchXml::IntervalOpen)
    {
        QList<QString> values = reader.valueToStringList();

        if (values.size() != 2)
        {
            qCWarning(DIGIKAM_DATABASE_LOG) << "Relation Interval requires a list of two values";

            return;
        }

        sql += QLatin1String(" (") + name + QLatin1Char(' ');
        ItemQueryBuilder::addSqlRelation(sql,
                                         relation == SearchXml::Interval ? SearchXml::GreaterThanOrEqual
                                                                         : SearchXml::GreaterThan);
        sql += QLatin1String(" ? AND ") + name + QLatin1Char(' ');
        ItemQueryBuilder::addSqlRelation(sql,
                                         relation == SearchXml::Interval ? SearchXml::LessThanOrEqual
                                                                         : SearchXml::LessThan);
        sql += QLatin1String(" ?) ");

        *boundValues << values.first() << values.last();
    }
    else
    {
        sql += QLatin1String(" (") + name + QLatin1Char(' ');
        ItemQueryBuilder::addSqlRelation(sql, relation);
        sql += QLatin1String(" ?) ");
        *boundValues << reader.value();
    }
}

void FieldQueryBuilder::addRecentDateField(const QString& name, int seconds)
{
    if (seconds <= 0)
        return;

    const QDateTime endDate   = QDateTime::currentDateTime();
    const QDateTime startDate = endDate.addSecs(-seconds);

    sql += QLatin1String(" (") + name + QLatin1Char(' ');
    ItemQueryBuilder::addSqlRelation(sql, SearchXml::GreaterThanOrEqual);
    sql += QLatin1String(" ? AND ") + name + QLatin1Char(' ');
    ItemQueryBuilder::addSqlRelation(sql, SearchXml::LessThanOrEqual);
    sql += QLatin1String(" ?) ");

    *boundValues << startDate.toString(Qt::ISODate)
                 << endDate.toString(Qt::ISODate);
}

void FieldQueryBuilder::addChoiceIntField(const QString& name)
{
    if (relation == SearchXml::OneOf)
    {
        QList<int> values  = reader.valueToIntList();
        bool searchForNull = values.removeAll(-1);
        sql               += QLatin1String(" (") + name + QLatin1String(" IN (");
        CoreDB::addBoundValuePlaceholders(sql, values.size());

        if (searchForNull)
        {
            sql += QLatin1String(") OR ") + name + QLatin1String(" IS NULL");
        }
        else
        {
            sql += QLatin1String(") ");
        }

        for (int v : std::as_const(values))
        {
            *boundValues << v;
        }

        sql += QLatin1String(" ) ");
    }
    else
    {
        addIntField(name);
    }
}

void FieldQueryBuilder::addLongListField(const QString& name)
{
    if (relation == SearchXml::OneOf)
    {
        QList<qlonglong> values = reader.valueToLongLongList();
        sql += QLatin1String(" (") + name + QLatin1String(" IN (");
        CoreDB::addBoundValuePlaceholders(sql, values.size());
        sql += QLatin1String(") ");

        for (const qlonglong& v : std::as_const(values))
        {
            *boundValues << v;
        }

        sql += QLatin1String(" ) ");
    }
    else
    {
        addLongField(name);
    }
}

void FieldQueryBuilder::addIntBitmaskField(const QString& name)
{
    if (relation == SearchXml::OneOf)
    {
        QList<int> values  = reader.valueToIntList();
        bool searchForNull = values.removeAll(-1);
        sql               += QLatin1String("( ");
        bool first         = true;

        for (int i = 0 ; i < values.size() ; ++i)
        {
            if (!first)
            {
                sql += QLatin1String("OR ");
            }

            first = false;
            sql  += name + QLatin1String(" & ? ");
        }

        if (searchForNull)
        {
            sql += QLatin1String("OR ") + name + QLatin1String(" IS NULL ");
        }

        for (int v : std::as_const(values))
        {
            *boundValues << v;
        }

        sql += QLatin1String(" ) ");
    }
    else
    {
        if (relation == SearchXml::Equal)
        {
            sql += QLatin1String(" (") + name + QLatin1String(" & ") + QLatin1String(" ?) ");
        }
        else
        {
            sql += QLatin1String(" (NOT ") + name + QLatin1String(" & ") + QLatin1String(" ?) ");
        }

        *boundValues << reader.valueToDouble();
    }
}

void FieldQueryBuilder::addChoiceStringField(const QString& name)
{
    if (relation == SearchXml::OneOf)
    {
        QStringList values = reader.valueToStringList();

        if (values.isEmpty())
        {
            qCDebug(DIGIKAM_DATABASE_LOG) << "List for OneOf is empty";
            return;
        }

        QStringList simpleValues, wildcards;

        for (const QString& value : std::as_const(values))
        {
            if (value.contains(QLatin1Char('*')))
            {
                wildcards << value;
            }
            else
            {
                simpleValues << value;
            }
        }

        bool firstCondition =  true;
        sql                += QLatin1String(" (");

        if (!simpleValues.isEmpty())
        {
            firstCondition =  false;
            sql           += name + QLatin1String(" IN (");
            CoreDB::addBoundValuePlaceholders(sql, simpleValues.size());

            for (const QString& value : std::as_const(simpleValues))
            {
                *boundValues << value;
            }

            sql += QLatin1String(" ) ");
        }

        if (!wildcards.isEmpty())
        {
            for (QString wildcard : std::as_const(wildcards))
            {
                ItemQueryBuilder::addSqlOperator(sql, SearchXml::Or, firstCondition);
                firstCondition = false;
                wildcard.replace(QLatin1Char('*'), QLatin1Char('%'));
                sql           += QLatin1Char(' ') + name + QLatin1Char(' ');
                ItemQueryBuilder::addSqlRelation(sql, SearchXml::Like);
                sql           += QLatin1String(" ? ");
                *boundValues << wildcard;
            }
        }

        sql += QLatin1String(") ");
    }
    else
    {
        QString value = reader.value();

        if (relation == SearchXml::Like && value.contains(QLatin1Char('*')))
        {
            // Handle special case: * denotes the place if the wildcard,
            // Don't automatically prepend and append %.

            sql             += QLatin1String(" (") + name + QLatin1Char(' ');
            ItemQueryBuilder::addSqlRelation(sql, SearchXml::Like);
            sql             += QLatin1String(" ?) ");
            QString wildcard = reader.value();
            wildcard.replace(QLatin1Char('*'), QLatin1Char('%'));
            *boundValues << wildcard;
        }
        else
        {
            addStringField(name);
        }
    }
}

void FieldQueryBuilder::addPosition()
{
    if (relation == SearchXml::Near)
    {
        // First read attributes

        QStringView type           = reader.attributes().value(QLatin1String("type"));
        QStringView distanceString = reader.attributes().value(QLatin1String("distance"));

        // Distance in meters

        double distance           = 100;

        if (!distanceString.isEmpty())
        {
            distance = distanceString.toString().toDouble();
        }

        // Search type, "radius" or "rectangle"

        bool radiusSearch         = true;

        if (type == QLatin1String("radius"))
        {
            radiusSearch = true;
        }
        else if (type == QLatin1String("rectangle"))
        {
            radiusSearch = false;
        }

        // Get a list of doubles:
        // Longitude and Latitude in (decimal) degrees

        QList<double> list        = reader.valueToDoubleList();

        if (list.size() != 2)
        {
            qCWarning(DIGIKAM_DATABASE_LOG) << "Relation 'Near' requires a list of two values";
            return;
        }

        double lon = list.at(0);
        double lat = list.at(1);

        sql       += QLatin1String(" ( ");

        // Part 1: Rectangle search.
        // Get the coordinates of the (spherical) rectangle enclosing
        // the (spherical) circle given by our coordinates and the distance.
        // For this one-time computation we use the advanced code
        // which assumes the earth is a ellipsoid.

        // From the point (lon,lat) we go East, North, West, South,
        // and get the coordinates in degrees of a rectangle
        // of width and height 2*distance enclosing (lon,lat)

        QRectF rect;
        GeodeticCalculator calc;
        calc.setStartingGeographicPoint(lon, lat);

        // go west

        calc.setDirection(-90, distance);
        rect.setLeft(calc.destinationGeographicPoint().x());

        // go north (from first starting point!)

        calc.setDirection(0, distance);
        rect.setTop(calc.destinationGeographicPoint().y());

        // go east

        calc.setDirection(90, distance);
        rect.setRight(calc.destinationGeographicPoint().x());

        // go south

        calc.setDirection(180, distance);
        rect.setBottom(calc.destinationGeographicPoint().y());

        addRectanglePositionSearch(rect.x(), rect.y(), rect.right(), rect.bottom());

        if (radiusSearch)
        {
            // Part 2: Use the Haversine formula to filter out from
            // the matching pictures those that lie inside the
            // actual (spherical) circle.
            // This code only assumes that the earth is a sphere.
            // But this needs to be computed n times, so it's expensive.
            // We refrain from putting this into SQL, but use a post hook.

            /*
            Reference: www.usenet-replayer.com/faq/comp.infosystems.gis.html
            Pseudo code of the formula:
                Position 1 (lon1, lat1), position 2 (lon2, lat2), in Radians
                d: distance; R: radius of earth. Same unit (assume: meters)
            dlon = lon2 - lon1;
            dlat = lat2 - lat1;
            a = (sin(dlat/2))^2 + cos(lat1) * cos(lat2) * (sin(dlon/2))^2;
            c = 2 * arcsin(min(1,sqrt(a)));
            d = R * c;
            // We precompute c.
            */

            class Q_DECL_HIDDEN HaversinePostHook : public ItemQueryPostHook
            {
            public:

                HaversinePostHook(double _lat1Deg, double _lon1Deg,
                                  double _radiusOfCurvature, double _distance)
                {
                    lat1              = Coordinates::toRadians(_lat1Deg);
                    lon1              = Coordinates::toRadians(_lon1Deg);
                    distanceInRadians = _distance / _radiusOfCurvature;
                    cosLat1           = cos(lat1);
                }

                bool checkPosition(double lat2Deg, double lon2Deg) override
                {
                    double lat2 = Coordinates::toRadians(lat2Deg);
                    double lon2 = Coordinates::toRadians(lon2Deg);
                    double dlon = lon2 - lon1;
                    double dlat = lat2 - lat1;
                    double a    = pow(sin(dlat/2), 2) + cosLat1 * cos(lat2) * pow(sin(dlon/2),2);
                    double c    = 2 * asin(qMin(1.0, sqrt(a)));

                    return (c < distanceInRadians);
                }

                double lat1                 = 0.0;
                double lon1                 = 0.0;
                double distanceInRadians    = 0.0;
                double cosLat1              = 0.0;
            };

            // get radius (of the ellipsoid) in dependence of the latitude.

            double R = calc.ellipsoid().radiusOfCurvature(lat);
            hooks->addHook(new HaversinePostHook(lat, lon, R, distance));
        }

        sql += QLatin1String(" ) ");
    }
    else if (relation == SearchXml::Inside)
    {
        // First read attributes

        QStringView type = reader.attributes().value(QLatin1String("type"));

        // Search type, currently only "rectangle"

        if (type != QLatin1String("rectangle"))
        {
            qCWarning(DIGIKAM_DATABASE_LOG) << "Relation 'Inside' supports no other type than 'rectangle'";
            return;
        }

        // Get a list of doubles:
        // Longitude and Latitude in (decimal) degrees

        QList<double> list = reader.valueToDoubleList();

        if (list.size() != 4)
        {
            qCWarning(DIGIKAM_DATABASE_LOG) << "Relation 'Inside' requires a list of four values";
            return;
        }

        // the list contains (lon1,lat1), (lon2,lat2) in this order,
        // like (x,y), (right,bottom) of a rectangle,
        // or like (West,North), (East,South),
        // where the searched region contains any lon,lat
        // where lon1 < lon < lon2 and lat1 < lat < lat2.

        double lon1, lat1, lon2, lat2;
        lon1 = list.at(0);
        lat1 = list.at(1);
        lon2 = list.at(2);
        lat2 = list.at(3);

        sql += QLatin1String(" ( ");
        addRectanglePositionSearch(lon1, lat1, lon2, lat2);
        sql += QLatin1String(" ) ");
    }
}

void FieldQueryBuilder::addRectanglePositionSearch(double lon1, double lat1, double lon2, double lat2) const
{
    // lon1 is always West of lon2. If the rectangle crosses 180 longitude, we have to treat a special case.

    if (lon1 <= lon2)
    {
        sql += QString::fromUtf8(" ImagePositions.LongitudeNumber > ? AND ImagePositions.LatitudeNumber < ? "
               " AND ImagePositions.LongitudeNumber < ? AND ImagePositions.LatitudeNumber > ? ");
        *boundValues << lon1 << lat1 << lon2 << lat2;
    }
    else
    {
        // this effectively means splitting the rectangle is two parts, one East, one West
        // to the 180 line. But no need to check for less/greater than -180/180.

        sql += QString::fromUtf8(" (ImagePositions.LongitudeNumber > ? OR ImagePositions.LongitudeNumber < ?) "
               " AND ImagePositions.LatitudeNumber < ? AND ImagePositions.LatitudeNumber > ? ");
        *boundValues << lon1 << lon2 << lat1 << lat2;
    }
}

} // namespace Digikam
