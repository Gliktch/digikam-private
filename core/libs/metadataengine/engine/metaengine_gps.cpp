/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               GPS manipulation methods
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2012 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metaengine_p.h"

// C ANSI includes

#include <math.h>

// C++ includes

#include <climits>
#include <cmath>

// Local includes

#include "digikam_debug.h"
#include "geodetictools.h"

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace Digikam
{

bool MetaEngine::getGPSInfo(double& altitude, double& latitude, double& longitude) const
{
    // Some GPS device do not set Altitude. So a valid GPS position can be with a zero value.
    // No need to check return value.

    getGPSAltitude(&altitude);

    if (!getGPSLatitudeNumber(&latitude))
    {
         return false;
    }

    if (!getGPSLongitudeNumber(&longitude))
    {
         return false;
    }

    return true;
}

bool MetaEngine::getGPSLatitudeNumber(double* const latitude) const
{
    *latitude = 0.0;

    // Try XMP first. Reason: XMP in sidecar may be more up-to-date than EXIF in original image.

    if (convertFromGPSCoordinateString(getXmpTagString("Xmp.exif.GPSLatitude"), latitude))
    {
        return true;
    }

    // Now try to get the reference from Exif.

    const QByteArray latRef = getExifTagData("Exif.GPSInfo.GPSLatitudeRef");

    if (!latRef.isEmpty() && d->decodeGPSCoordinate("Exif.GPSInfo.GPSLatitude", latitude))
    {
        if ((latRef[0] == 'S') && (*latitude > 0.0))
        {
            *latitude *= -1.0;
        }

        if ((*latitude < -90.0) || (*latitude > 90.0))
        {
            return false;
        }

        return true;
    }

    return false;
}

bool MetaEngine::getGPSLongitudeNumber(double* const longitude) const
{
    *longitude = 0.0;

    // Try XMP first. Reason: XMP in sidecar may be more up-to-date than EXIF in original image.

    if (convertFromGPSCoordinateString(getXmpTagString("Xmp.exif.GPSLongitude"), longitude))
    {
        return true;
    }

    // Now try to get the reference from Exif.

    const QByteArray lngRef = getExifTagData("Exif.GPSInfo.GPSLongitudeRef");

    if (!lngRef.isEmpty() && d->decodeGPSCoordinate("Exif.GPSInfo.GPSLongitude", longitude))
    {
        if ((lngRef[0] == 'W') && (*longitude > 0.0))
        {
            *longitude *= -1.0;
        }

        if ((*longitude < -180.0) || (*longitude > 180.0))
        {
            return false;
        }

        return true;
    }

    return false;
}

bool MetaEngine::getGPSAltitude(double* const altitude) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        double num = 0.0;
        double den = 1.0;
        *altitude  = 0.0;

        // Try XMP first. Reason: XMP in sidecar may be more up-to-date than EXIF in original image.

        QString altRefXmp = getXmpTagString("Xmp.exif.GPSAltitudeRef");

        if (altRefXmp.isEmpty())
        {
            altRefXmp = QLatin1String("0");
        }

        const QString altXmp = getXmpTagString("Xmp.exif.GPSAltitude");

        if (!altXmp.isEmpty())
        {
            num = altXmp.section(QLatin1Char('/'), 0, 0).toDouble();
            den = altXmp.section(QLatin1Char('/'), 1, 1).toDouble();

            if (den == 0.0)
            {
                return false;
            }

            *altitude = num / den;

            if (altRefXmp == QLatin1String("1"))
            {
                *altitude *= -1.0;
            }

            return true;
        }

        // Get the reference from Exif (above/below sea level)

        QByteArray altRef = getExifTagData("Exif.GPSInfo.GPSAltitudeRef");

        if (altRef.isEmpty())
        {
            altRef = QByteArray("0");
        }

        // Altitude decoding from Exif.

        Exiv2::ExifKey exifKey3("Exif.GPSInfo.GPSAltitude");
        Exiv2::ExifData exifData(d->exifMetadata());
        Exiv2::ExifData::const_iterator it = exifData.findKey(exifKey3);

        if (it != exifData.end() && (*it).count())
        {
            num = (double)((*it).toRational(0).first);
            den = (double)((*it).toRational(0).second);

            if (den == 0.0)
            {
                return false;
            }

            *altitude = num / den;

            if (altRef[0] == '1')
            {
                *altitude *= -1.0;
            }
        }
        else
        {
            return false;
        }

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot get GPS tag with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QString MetaEngine::getGPSLatitudeString() const
{
    double latitude = 0.0;

    if (!getGPSLatitudeNumber(&latitude))
    {
        return QString();
    }

    return convertToGPSCoordinateString(true, latitude);
}

QString MetaEngine::getGPSLongitudeString() const
{
    double longitude = 0.0;

    if (!getGPSLongitudeNumber(&longitude))
    {
        return QString();
    }

    return convertToGPSCoordinateString(false, longitude);
}

bool MetaEngine::initializeGPSInfo()
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        // TODO: what happens if these already exist?

        // Do all the easy constant ones first.
        // GPSVersionID tag: standard says is should be four bytes: 02 00 00 00
        // (and, must be present).

        Exiv2::Value::AutoPtr value = Exiv2::Value::create(Exiv2::unsignedByte);
        value->read("2 0 0 0");
        d->exifMetadata().add(Exiv2::ExifKey("Exif.GPSInfo.GPSVersionID"), value.get());

        // Datum: the datum of the measured data. If not given, we insert WGS-84.

        d->exifMetadata()["Exif.GPSInfo.GPSMapDatum"] = "WGS-84";

#ifdef _XMP_SUPPORT_

        setXmpTagString("Xmp.exif.GPSVersionID", QLatin1String("2.0.0.0"));
        setXmpTagString("Xmp.exif.GPSMapDatum",  QLatin1String("WGS-84"));

#endif // _XMP_SUPPORT_

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot initialize GPS data with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::setGPSInfo(const double altitude, const double latitude, const double longitude)
{
    return setGPSInfo(&altitude, latitude, longitude);
}

bool MetaEngine::setGPSInfo(const double* const altitude, const double latitude, const double longitude)
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        // In first, we need to clean up all existing GPS info
        // if the coordinate deviation is greater than 100 m.

        double lat = 0.0;
        double lon = 0.0;

        if (getGPSLatitudeNumber(&lat) && getGPSLongitudeNumber(&lon))
        {
            GeodeticCalculator calc;
            calc.setStartingGeographicPoint(lon, lat);
            calc.setDestinationGeographicPoint(longitude, latitude);

            if (calc.orthodromicDistance() > 100.0)
            {
                removeGPSInfo();
            }
        }
        else
        {
            removeGPSInfo();
        }

        // now re-initialize the GPS info:

        if (!initializeGPSInfo())
        {
            return false;
        }

        char scratchBuf[100] = { 0 };
        long int nom   = 0;
        long int denom = 1;
        long int deg   = 0;
        long int min   = 0;

        // Now start adding data.

        // ALTITUDE.

        if (altitude)
        {
            // Altitude reference: byte "00" meaning "above sea level", "01" meaning "behind sea level".

            Exiv2::Value::AutoPtr value = Exiv2::Value::create(Exiv2::unsignedByte);

            if ((*altitude) >= 0) value->read("0");
            else                  value->read("1");

            d->exifMetadata().add(Exiv2::ExifKey("Exif.GPSInfo.GPSAltitudeRef"), value.get());

            // And the actual altitude, as absolute value..

            convertToRational(fabs(*altitude), &nom, &denom, 4);
            snprintf(scratchBuf, 100, "%ld/%ld", nom, denom);
            d->exifMetadata()["Exif.GPSInfo.GPSAltitude"] = scratchBuf;

#ifdef _XMP_SUPPORT_

            setXmpTagString("Xmp.exif.GPSAltitudeRef", ((*altitude) >= 0) ? QLatin1String("0") : QLatin1String("1"));
            setXmpTagString("Xmp.exif.GPSAltitude",    QLatin1String(scratchBuf));

#endif // _XMP_SUPPORT_

        }

        // LATITUDE
        // Latitude reference:
        // latitude < 0 : "S"
        // latitude > 0 : "N"

        d->exifMetadata()["Exif.GPSInfo.GPSLatitudeRef"] = (latitude < 0 ) ? "S" : "N";

        // Now the actual latitude itself.
        // This is done as three rationals.
        // I choose to do it as:
        //   dd/1 - degrees.
        //   mmmm/100 - minutes
        //   0/1 - seconds
        // Exif standard says you can do it with minutes
        // as mm/1 and then seconds as ss/1, but its
        // (slightly) more accurate to do it as
        //  mmmm/100 than to split it.
        // We also absolute the value (with fabs())
        // as the sign is encoded in LatRef.
        // Further note: original code did not translate between
        //   dd.dddddd to dd mm.mm - that's why we now multiply
        //   by 6000 - x60 to get minutes, x1000000 to get to mmmm/1000000.

        deg   = (int)floor(fabs(latitude)); // Slice off after decimal.
        min   = (int)floor((fabs(latitude) - floor(fabs(latitude))) * 60000000);
        snprintf(scratchBuf, 100, "%ld/1 %ld/1000000 0/1", deg, min);
        d->exifMetadata()["Exif.GPSInfo.GPSLatitude"] = scratchBuf;

#ifdef _XMP_SUPPORT_

        /**
         * NOTE: The XMP spec does not mention Xmp.exif.GPSLatitudeRef,
         * because the reference is included in Xmp.exif.GPSLatitude.
         * See bug #450982.
         */

        setXmpTagString("Xmp.exif.GPSLatitude",    convertToGPSCoordinateString(true, latitude));

#endif // _XMP_SUPPORT_

        // LONGITUDE
        // Longitude reference:
        // longitude < 0 : "W"
        // longitude > 0 : "E"

        d->exifMetadata()["Exif.GPSInfo.GPSLongitudeRef"] = (longitude < 0 ) ? "W" : "E";

        // Now the actual longitude itself.
        // This is done as three rationals.
        // I choose to do it as:
        //   dd/1 - degrees.
        //   mmmm/100 - minutes
        //   0/1 - seconds
        // Exif standard says you can do it with minutes
        // as mm/1 and then seconds as ss/1, but its
        // (slightly) more accurate to do it as
        //  mmmm/100 than to split it.
        // We also absolute the value (with fabs())
        // as the sign is encoded in LongRef.
        // Further note: original code did not translate between
        //   dd.dddddd to dd mm.mm - that's why we now multiply
        //   by 6000 - x60 to get minutes, x1000000 to get to mmmm/1000000.

        deg   = (int)floor(fabs(longitude)); // Slice off after decimal.
        min   = (int)floor((fabs(longitude) - floor(fabs(longitude))) * 60000000);
        snprintf(scratchBuf, 100, "%ld/1 %ld/1000000 0/1", deg, min);
        d->exifMetadata()["Exif.GPSInfo.GPSLongitude"] = scratchBuf;

#ifdef _XMP_SUPPORT_

        /**
         * NOTE: The XMP spec does not mention Xmp.exif.GPSLongitudeRef,
         * because the reference is included in Xmp.exif.GPSLongitude.
         * See bug #450982.
         */

        setXmpTagString("Xmp.exif.GPSLongitude",    convertToGPSCoordinateString(false, longitude));

#endif // _XMP_SUPPORT_

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Exif GPS tag with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::setGPSInfo(const double altitude, const QString& latitude, const QString& longitude)
{
    double longitudeValue = 0.0;
    double latitudeValue  = 0.0;

    if (!convertFromGPSCoordinateString(latitude, &latitudeValue))
    {
        return false;
    }

    if (!convertFromGPSCoordinateString(longitude, &longitudeValue))
    {
        return false;
    }

    return setGPSInfo(&altitude, latitudeValue, longitudeValue);
}

bool MetaEngine::removeGPSInfo()
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        QStringList gpsTagsKeys;

        for (Exiv2::ExifData::const_iterator it = d->exifMetadata().begin() ;
             it != d->exifMetadata().end() ; ++it)
        {
            QString key = QString::fromStdString(it->key());

            if (key.section(QLatin1Char('.'), 1, 1) == QLatin1String("GPSInfo"))
            {
                gpsTagsKeys.append(key);
            }
        }

        for (QStringList::const_iterator it2 = gpsTagsKeys.constBegin() ;
             it2 != gpsTagsKeys.constEnd() ; ++it2)
        {
            Exiv2::ExifKey gpsKey((*it2).toLatin1().constData());
            Exiv2::ExifData::iterator it3 = d->exifMetadata().findKey(gpsKey);

            if (it3 != d->exifMetadata().end())
            {
                d->exifMetadata().erase(it3);
            }
        }

#ifdef _XMP_SUPPORT_

        /**
         * NOTE: The XMP spec does not mention Xmp.exif.GPSLongitudeRef,
         * and Xmp.exif.GPSLatitudeRef. But because we write historicaly until 7.6.0 release
         * them in setGPSInfo(), we should also remove them here.
         * See bug #450982.
         */
        removeXmpTag("Xmp.exif.GPSLatitudeRef");
        removeXmpTag("Xmp.exif.GPSLongitudeRef");

        removeXmpTag("Xmp.exif.GPSVersionID");
        removeXmpTag("Xmp.exif.GPSLatitude");
        removeXmpTag("Xmp.exif.GPSLongitude");
        removeXmpTag("Xmp.exif.GPSAltitudeRef");
        removeXmpTag("Xmp.exif.GPSAltitude");
        removeXmpTag("Xmp.exif.GPSTimeStamp");
        removeXmpTag("Xmp.exif.GPSSatellites");
        removeXmpTag("Xmp.exif.GPSStatus");
        removeXmpTag("Xmp.exif.GPSMeasureMode");
        removeXmpTag("Xmp.exif.GPSDOP");
        removeXmpTag("Xmp.exif.GPSSpeedRef");
        removeXmpTag("Xmp.exif.GPSSpeed");
        removeXmpTag("Xmp.exif.GPSTrackRef");
        removeXmpTag("Xmp.exif.GPSTrack");
        removeXmpTag("Xmp.exif.GPSImgDirectionRef");
        removeXmpTag("Xmp.exif.GPSImgDirection");
        removeXmpTag("Xmp.exif.GPSMapDatum");
        removeXmpTag("Xmp.exif.GPSDestLatitude");
        removeXmpTag("Xmp.exif.GPSDestLongitude");
        removeXmpTag("Xmp.exif.GPSDestBearingRef");
        removeXmpTag("Xmp.exif.GPSDestBearing");
        removeXmpTag("Xmp.exif.GPSDestDistanceRef");
        removeXmpTag("Xmp.exif.GPSDestDistance");
        removeXmpTag("Xmp.exif.GPSProcessingMethod");
        removeXmpTag("Xmp.exif.GPSAreaInformation");
        removeXmpTag("Xmp.exif.GPSDifferential");

#endif // _XMP_SUPPORT_

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot remove Exif GPS tag with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

void MetaEngine::convertToRational(const double number, long int* const numerator,
                                   long int* const denominator, const int rounding)
{
    // This function converts the given decimal number
    // to a rational (fractional) number.
    //
    // Examples in comments use Number as 25.12345, Rounding as 4.

    // Split up the number.

    double whole      = trunc(number);
    double fractional = number - whole;

    // Calculate the "number" used for rounding.
    // This is 10^Digits - ie, 4 places gives us 10000.

    double rounder    = pow(10.0, rounding);

    // Round the fractional part, and leave the number
    // as greater than 1.
    // To do this we: (for example)
    //  0.12345 * 10000 = 1234.5
    //  floor(1234.5) = 1234 - now bigger than 1 - ready...

    fractional        = round(fractional * rounder);

    // Convert the whole thing to a fraction.
    // Fraction is:
    //     (25 * 10000) + 1234   251234
    //     ------------------- = ------ = 25.1234
    //           10000            10000

    double numTemp    = (whole * rounder) + fractional;
    double denTemp    = rounder;

    // Now we should reduce until we can reduce no more.

    // Try simple reduction...
    // if   Num
    //     ----- = integer out then....
    //      Den

    if (trunc(numTemp / denTemp) == (numTemp / denTemp))
    {
        // Divide both by Denominator.

        numTemp /= denTemp;

        // cppcheck-suppress duplicateExpression
        denTemp /= denTemp;
    }

    // And, if that fails, brute force it.

    while (1)
    {
        // Jump out if we cannot integer divide one.

        if ((numTemp / 2) != trunc(numTemp / 2)) break;
        if ((denTemp / 2) != trunc(denTemp / 2)) break;

        // Otherwise, divide away.

        numTemp /= 2;
        denTemp /= 2;
    }

    // Copy out the numbers.

    *numerator   = (int)numTemp;
    *denominator = (int)denTemp;
}

void MetaEngine::convertToRationalSmallDenominator(const double number, long int* const numerator, long int* const denominator)
{
    // This function converts the given decimal number
    // to a rational (fractional) number.
    //
    // This method, in contrast to the method above, will retrieve the smallest possible
    // denominator. It is tested to retrieve the correct value for 1/x, with 0 < x <= 1000000.
    // Note: This requires double precision, storing in float breaks some numbers (49, 59, 86,...)

    // Split up the number.

    double whole      = trunc(number);
    double fractional = number - whole;

    /*
     * Find best rational approximation to a double
     * by C.B. Falconer, 2006-09-07. Released to public domain.
     *
     * Newsgroups: comp.lang.c, comp.programming
     * From: CBFalconer <cbfalconer@yahoo.com>
     * Date: Thu, 07 Sep 2006 17:35:30 -0400
     * Subject: Rational approximations
     */

    int      lastnum = 500; // this is _not_ the largest possible denominator
    long int num, approx, bestnum = 0, bestdenom = 1;
    double   value, error, leasterr, criterion;

    value = fractional;

    if (value == 0.0)
    {
        *numerator   = (long int)whole;
        *denominator = 1;
        return;
    }

    criterion = 2 * value * DBL_EPSILON;

    for (leasterr = value, num = 1 ; num < lastnum ; ++num)
    {
        approx = (int)(num / value + 0.5);
        error  = fabs((double)num / approx - value);

        if (error < leasterr)
        {
            bestnum   = num;
            bestdenom = approx;
            leasterr  = error;

            if (leasterr <= criterion)
            {
                break;
            }
        }
    }

    // add whole number part

    if (bestdenom * whole > (double)INT_MAX)
    {
        // In some cases, we would generate an integer overflow.
        // Fall back to Gilles's code which is better suited for such numbers.

        convertToRational(number, numerator, denominator, 5);
    }
    else
    {
        bestnum     += bestdenom * (long int)whole;
        *numerator   = bestnum;
        *denominator = bestdenom;
    }
}

double MetaEngine::convertDegreeAngleToDouble(double degrees, double minutes, double seconds)
{
    if (degrees > 0.0)
    {
        return (degrees + (minutes / 60.0) + (seconds / 3600.0));
    }

    return (degrees - (minutes / 60.0) - (seconds / 3600.0));
}

QString MetaEngine::convertToGPSCoordinateString(const long int numeratorDegrees, const long int denominatorDegrees,
                                                 const long int numeratorMinutes, const long int denominatorMinutes,
                                                 const long int numeratorSeconds, const long int denominatorSeconds,
                                                 const char directionReference)
{
    /**
     * Precision:
     * A second at sea level measures 30m for our purposes, a minute 1800m.
     * (for more details, see https://en.wikipedia.org/wiki/Geographic_coordinate_system)
     * This means with a decimal precision of 8 for minutes we get +/-0,018mm.
     * (if I calculated correctly)
     */

    QString coordinate;
    long int denSecs = denominatorSeconds;

    // be relaxed with seconds of 0/0

    if (
        (denSecs          == 0) &&
        (numeratorSeconds == 0)
       )
    {
        denSecs = 1;
    }

    if      (
             (denominatorDegrees == 1) &&
             (denominatorMinutes == 1) &&
             (denSecs            == 1)
            )
    {
        // use form DDD,MM,SSk

        coordinate = QLatin1String("%1,%2,%3%4");
        coordinate = coordinate.arg(numeratorDegrees).arg(numeratorMinutes).arg(numeratorSeconds).arg(directionReference);
    }
    else if (
             (denominatorDegrees == 1)   &&
             (denominatorMinutes == 100) &&
             (denSecs            == 1)
            )
    {
        // use form DDD,MM.mmk

        coordinate             = QLatin1String("%1,%2%3");
        double minutes         = (double)numeratorMinutes / (double)denominatorMinutes;
        minutes               += (double)numeratorSeconds / 60.0;
        QString minutesString  = QString::number(minutes, 'f', 8);

        while (minutesString.endsWith(QLatin1String("0")) && !minutesString.endsWith(QLatin1String(".0")))
        {
            minutesString.chop(1);
        }

        coordinate = coordinate.arg(numeratorDegrees).arg(minutesString).arg(directionReference);
    }
    else if (
             (denominatorDegrees == 0) ||
             (denominatorMinutes == 0) ||
             (denSecs            == 0)
            )
    {
        // Invalid. 1/0 is everything but 0. As is 0/0.

        return QString();
    }
    else
    {
        // use form DDD,MM.mmk

        coordinate             = QLatin1String("%1,%2%3");
        double degrees         = (double)numeratorDegrees  / (double)denominatorDegrees;
        double wholeDegrees    = trunc(degrees);
        double minutes         = (double)numeratorMinutes  / (double)denominatorMinutes;
        minutes               += (degrees - wholeDegrees) * 60.0;
        minutes               += ((double)numeratorSeconds / (double)denSecs) / 60.0;
        QString minutesString  = QString::number(minutes, 'f', 8);

        while (minutesString.endsWith(QLatin1String("0")) && !minutesString.endsWith(QLatin1String(".0")))
        {
            minutesString.chop(1);
        }

        coordinate = coordinate.arg((int)wholeDegrees).arg(minutesString).arg(directionReference);
    }

    return coordinate;
}

QString MetaEngine::convertToGPSCoordinateString(const bool isLatitude, double coordinate)
{
    if ((coordinate < -360.0) || (coordinate > 360.0))
    {
        return QString();
    }

    QString coordinateString;

    char directionReference;

    if (isLatitude)
    {
        if (coordinate < 0)
        {
            directionReference = 'S';
        }
        else
        {
            directionReference = 'N';
        }
    }
    else
    {
        if (coordinate < 0)
        {
            directionReference = 'W';
        }
        else
        {
            directionReference = 'E';
        }
    }

    // remove sign

    coordinate       = fabs(coordinate);

    int degrees      = (int)floor(coordinate);

    // get fractional part

    coordinate       = coordinate - (double)(degrees);

    // To minutes

    double minutes   = coordinate * 60.0;

    // use form DDD,MM.mmk

    coordinateString = QLatin1String("%1,%2%3");
    coordinateString = coordinateString.arg(degrees);
    coordinateString = coordinateString.arg(minutes, 0, 'f', 8).arg(directionReference);

    return coordinateString;
}

bool MetaEngine::convertFromGPSCoordinateString(const QString& gpsString,
                                                long int* const numeratorDegrees, long int* const denominatorDegrees,
                                                long int* const numeratorMinutes, long int* const denominatorMinutes,
                                                long int* const numeratorSeconds, long int* const denominatorSeconds,
                                                char* const directionReference)
{
    if (gpsString.isEmpty())
    {
        return false;
    }

    *directionReference = gpsString.at(gpsString.length() - 1).toUpper().toLatin1();
    QString coordinate  = gpsString.left(gpsString.length() - 1);
    QStringList parts   = coordinate.split(QLatin1String(","));

    if      (parts.size() == 2)
    {
        // form DDD,MM.mmk

        *denominatorDegrees = 1;
        *denominatorMinutes = 1000000;
        *denominatorSeconds = 1;

        *numeratorDegrees   = parts[0].toLong();

        double minutes      = parts[1].toDouble();
        minutes            *= 1000000;

        *numeratorMinutes   = (long)round(minutes);
        *numeratorSeconds   = 0;

        return true;
    }
    else if (parts.size() == 3)
    {
        // use form DDD,MM,SSk

        *denominatorDegrees = 1;
        *denominatorMinutes = 1;
        *denominatorSeconds = 1000000;

        *numeratorDegrees   = parts[0].toLong();
        *numeratorMinutes   = parts[1].toLong();

        double seconds      = parts[2].toDouble();
        seconds            *= 1000000;

        *numeratorSeconds   = (long)round(seconds);

        return true;
    }
    else
    {
        return false;
    }
}

bool MetaEngine::convertFromGPSCoordinateString(const QString& gpsString, double* const degrees)
{
    if (gpsString.isEmpty())
    {
        return false;
    }

    char directionReference = gpsString.at(gpsString.length() - 1).toUpper().toLatin1();
    QString coordinate      = gpsString.left(gpsString.length() - 1);
    QStringList parts       = coordinate.split(QLatin1String(","));

    if      (parts.size() == 2)
    {
        // form DDD,MM.mmk

        *degrees  = parts[0].toLong();
        *degrees += parts[1].toDouble() / 60.0;

        if ((directionReference == 'W') || (directionReference == 'S'))
        {
            *degrees *= -1.0;
        }

        return true;
    }
    else if (parts.size() == 3)
    {
        // use form DDD,MM,SSk

        *degrees  = parts[0].toLong();
        *degrees += parts[1].toLong() / 60.0;
        *degrees += parts[2].toDouble() / 3600.0;

        if ((directionReference == 'W') || (directionReference == 'S'))
        {
            *degrees *= -1.0;
        }

        return true;
    }
    else
    {
        return false;
    }
}

bool MetaEngine::convertToUserPresentableNumbers(const QString& gpsString,
                                                 int* const degrees, int* const minutes,
                                                 double* const seconds, char* const directionReference)
{
    if (gpsString.isEmpty())
    {
        return false;
    }

    *directionReference = gpsString.at(gpsString.length() - 1).toUpper().toLatin1();
    QString coordinate  = gpsString.left(gpsString.length() - 1);
    QStringList parts   = coordinate.split(QLatin1String(","));

    if      (parts.size() == 2)
    {
        // form DDD,MM.mmk

        *degrees                 = parts[0].toInt();
        double fractionalMinutes = parts[1].toDouble();
        *minutes                 = (int)trunc(fractionalMinutes);
        *seconds                 = (fractionalMinutes - (double)(*minutes)) * 60.0;

        return true;
    }
    else if (parts.size() == 3)
    {
        // use form DDD,MM,SSk

        *degrees = parts[0].toInt();
        *minutes = parts[1].toInt();
        *seconds = parts[2].toDouble();

        return true;
    }
    else
    {
        return false;
    }
}

void MetaEngine::convertToUserPresentableNumbers(const bool isLatitude, double coordinate,
                                                 int* const degrees, int* const minutes,
                                                 double* const seconds, char* const directionReference)
{
    if (isLatitude)
    {
        if (coordinate < 0)
        {
            *directionReference = 'S';
        }
        else
        {
            *directionReference = 'N';
        }
    }
    else
    {
        if (coordinate < 0)
        {
            *directionReference = 'W';
        }
        else
        {
            *directionReference = 'E';
        }
    }

    // remove sign

    coordinate  = fabs(coordinate);
    *degrees    = (int)floor(coordinate);

    // get fractional part

    coordinate  = coordinate - (double)(*degrees);

    // To minutes

    coordinate *= 60.0;
    *minutes    = (int)floor(coordinate);

    // get fractional part

    coordinate  = coordinate - (double)(*minutes);

    // To seconds

    coordinate *= 60.0;
    *seconds    = coordinate;
}

} // namespace Digikam

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif
