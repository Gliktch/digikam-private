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

#include "LonLatParser_p.h"

// qt includes

#include <QLocale>
#include <QRegularExpression>
#include <QSet>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "GeoDataCoordinates.h"
#include "digikam_debug.h"

namespace Marble
{

LonLatParser::LonLatParser()
    : m_lon(0.0)
    , m_lat(0.0)
    , m_north(QStringLiteral("n"))
    , m_east(QStringLiteral("e"))
    , m_south(QStringLiteral("s"))
    , m_west(QStringLiteral("w"))
    , m_decimalPointExp(createDecimalPointExp())
{
}

void LonLatParser::initAll()
{
    // already all initialized?
    if (! m_dirCapExp.isEmpty())
    {
        return;
    }

    const QLatin1String placeholder = QLatin1String("*");
    const QString separator = QStringLiteral("|");

    //: See https://community.kde.org/Marble/GeoDataCoordinatesTranslation#Direction_terms
    getLocaleList(m_northLocale, i18nc("North direction terms", "*"),
                  placeholder, separator);
    //: See https://community.kde.org/Marble/GeoDataCoordinatesTranslation#Direction_terms
    getLocaleList(m_eastLocale, i18nc("East direction terms", "*"),
                  placeholder, separator);
    //: See https://community.kde.org/Marble/GeoDataCoordinatesTranslation#Direction_terms
    getLocaleList(m_southLocale, i18nc("South direction terms", "*"),
                  placeholder, separator);
    //: See https://community.kde.org/Marble/GeoDataCoordinatesTranslation#Direction_terms
    getLocaleList(m_westLocale, i18nc("West direction terms", "*"),
                  placeholder, separator);

    // use a set to remove duplicates
    QSet<QString> dirs = QSet<QString>()
                         << m_north << m_east << m_south << m_west;
    dirs += QSet<QString>(m_northLocale.begin(), m_northLocale.end());
    dirs += QSet<QString>(m_eastLocale.begin(), m_eastLocale.end());
    dirs += QSet<QString>(m_southLocale.begin(), m_southLocale.end());
    dirs += QSet<QString>(m_westLocale.begin(), m_westLocale.end());

    QString fullNamesExp;
    QString simpleLetters;

    for (const QString& dir : dirs)
    {
        // collect simple letters
        if ((dir.length() == 1) && (QLatin1Char('a') <= dir.at(0)) && (dir.at(0) <= QLatin1Char('z')))
        {
            simpleLetters += dir;
            continue;
        }

        // okay to add '|' also for last, separates from firstLetters
        fullNamesExp += QRegularExpression::escape(dir) + QLatin1Char('|');
    }

    // Sets "(north|east|south|west|[nesw])" in en, as translated names match untranslated ones
    m_dirCapExp =
        QLatin1Char('(') + fullNamesExp + QLatin1Char('[') + simpleLetters + QLatin1String("])");

    // expressions for symbols of degree, minutes and seconds
    //: See https://community.kde.org/Marble/GeoDataCoordinatesTranslation#Coordinate_symbols
    getLocaleList(m_degreeLocale, i18nc("Degree symbol terms", "*"),
                  placeholder, separator);
    //: See https://community.kde.org/Marble/GeoDataCoordinatesTranslation#Coordinate_symbols
    getLocaleList(m_minutesLocale, i18nc("Minutes symbol terms", "*"),
                  placeholder, separator);
    //: See https://community.kde.org/Marble/GeoDataCoordinatesTranslation#Coordinate_symbols
    getLocaleList(m_secondsLocale, i18nc("Seconds symbol terms", "*"),
                  placeholder, separator);

    // Used unicode chars:
    // u00B0: ° DEGREE SIGN
    // u00BA: º MASCULINE ORDINAL INDICATOR (found used as degree sign)
    // u2032: ′ PRIME (minutes)
    // u00B4: ´ ACUTE ACCENT (found as minutes sign)
    // u02CA: ˊ MODIFIER LETTER ACUTE ACCENT
    // u2019: ’ RIGHT SINGLE QUOTATION MARK
    // u2033: ″ DOUBLE PRIME (seconds)
    // u201D: ” RIGHT DOUBLE QUOTATION MARK

    m_degreeExp = QStringLiteral(u"\u00B0|\u00BA");

    for (const QString& symbol : m_degreeLocale)
    {
        m_degreeExp += QLatin1Char('|') + QRegularExpression::escape(symbol);
    }

    m_minutesExp = QStringLiteral(u"'|\u2032|\u00B4|\u20C2|\u2019");

    for (const QString& symbol : m_minutesLocale)
    {
        m_minutesExp += QLatin1Char('|') + QRegularExpression::escape(symbol);
    }

    m_secondsExp = QStringLiteral(u"\"|\u2033|\u201D|''|\u2032\u2032|\u00B4\u00B4|\u20C2\u20C2|\u2019\u2019");

    for (const QString& symbol : m_secondsLocale)
    {
        m_secondsExp += QLatin1Char('|') + QRegularExpression::escape(symbol);
    }
}

bool LonLatParser::parse(const QString& string)
{
    const QString input = string.toLower().trimmed();

    // #1: Just two numbers, no directions, e.g. 74.2245 -32.2434 (assumes lat lon)
    {
        const QString numberCapExp = QStringLiteral("\\A(?:") +
                                     QStringLiteral("([-+]?\\d{1,3}%1?\\d*(?:[eE][+-]?\\d+)?)(?:,|;|\\s)\\s*").arg(m_decimalPointExp) +
                                     QStringLiteral("([-+]?\\d{1,3}%1?\\d*(?:[eE][+-]?\\d+)?)").arg(m_decimalPointExp) +
                                     QStringLiteral(")\\z");

        const QRegularExpression regex(numberCapExp);
        QRegularExpressionMatch match = regex.match(input);

        if (match.hasMatch())
        {
            m_lon = parseDouble(match.captured(2));
            m_lat = parseDouble(match.captured(1));

            return true;
        }
    }

    initAll();

    if (tryMatchFromD(input, PostfixDir))
    {
        return true;
    }

    if (tryMatchFromD(input, PrefixDir))
    {
        return true;
    }

    if (tryMatchFromDms(input, PostfixDir))
    {
        return true;
    }

    if (tryMatchFromDms(input, PrefixDir))
    {
        return true;
    }

    if (tryMatchFromDm(input, PostfixDir))
    {
        return true;
    }

    if (tryMatchFromDm(input, PrefixDir))
    {
        return true;
    }

    return false;
}

// #3: Sexagesimal
bool LonLatParser::tryMatchFromDms(const QString& input, DirPosition dirPosition)
{
    // direction as postfix
    const QString postfixCapExp = QStringLiteral("\\A(?:") +
                                  QStringLiteral("([-+]?)(\\d{1,3})(?:%3|\\s)\\s*(\\d{1,2})(?:%4|\\s)\\s*") +
                                  QStringLiteral("(\\d{1,2}%1?\\d*)(?:%5)?\\s*%2[,;]?\\s*") +
                                  QStringLiteral("([-+]?)(\\d{1,3})(?:%3|\\s)\\s*(\\d{1,2})(?:%4|\\s)\\s*") +
                                  QStringLiteral("(\\d{1,2}%1?\\d*)(?:%5)?\\s*%2") +
                                  QStringLiteral(")\\z");

    // direction as prefix
    const QString prefixCapExp = QStringLiteral("\\A(?:") +
                                 QStringLiteral("%2\\s*([-+]?)(\\d{1,3})(?:%3|\\s)\\s*(\\d{1,2})(?:%4|\\s)\\s*") +
                                 QStringLiteral("(\\d{1,2}%1?\\d*)(?:%5)?\\s*(?:,|;|\\s)\\s*") +
                                 QStringLiteral("%2\\s*([-+]?)(\\d{1,3})(?:%3|\\s)\\s*(\\d{1,2})(?:%4|\\s)\\s*") +
                                 QStringLiteral("(\\d{1,2}%1?\\d*)(?:%5)?") +
                                 QStringLiteral(")\\z");

    const QString& expTemplate = (dirPosition == PostfixDir) ? postfixCapExp
                                 : prefixCapExp;

    const QString numberCapExp = expTemplate.arg(m_decimalPointExp, m_dirCapExp,
                                                 m_degreeExp, m_minutesExp, m_secondsExp);

    const QRegularExpression regex(numberCapExp);
    QRegularExpressionMatch match = regex.match(input);

    if (!match.hasMatch())
    {
        return false;
    }

    bool isDir1LonDir;
    bool isLonDirPosHemisphere;
    bool isLatDirPosHemisphere;
    const QString dir1 = match.captured(dirPosition == PostfixDir ? 5 : 1);
    const QString dir2 = match.captured(dirPosition == PostfixDir ? 10 : 6);

    if (!isCorrectDirections(dir1, dir2, isDir1LonDir,
                             isLonDirPosHemisphere, isLatDirPosHemisphere))
    {
        return false;
    }

    const int valueStartIndex1 = (dirPosition == PostfixDir ? 1 : 2);
    const int valueStartIndex2 = (dirPosition == PostfixDir ? 6 : 7);
    m_lon = degreeValueFromDMS(match, isDir1LonDir ? valueStartIndex1 : valueStartIndex2,
                               isLonDirPosHemisphere);
    m_lat = degreeValueFromDMS(match, isDir1LonDir ? valueStartIndex2 : valueStartIndex1,
                               isLatDirPosHemisphere);

    return true;
}

// #4: Sexagesimal with minute precision
bool LonLatParser::tryMatchFromDm(const QString& input, DirPosition dirPosition)
{
    // direction as postfix
    const QString postfixCapExp = QStringLiteral("\\A(?:") +
                                  QStringLiteral("([-+]?)(\\d{1,3})(?:%3|\\s)\\s*(\\d{1,2}%1?\\d*)(?:%4)?\\s*%2[,;]?\\s*") +
                                  QStringLiteral("([-+]?)(\\d{1,3})(?:%3|\\s)\\s*(\\d{1,2}%1?\\d*)(?:%4)?\\s*%2") +
                                  QStringLiteral(")\\z");

    // direction as prefix
    const QString prefixCapExp = QStringLiteral("\\A(?:") +
                                 QStringLiteral("%2\\s*([-+]?)(\\d{1,3})(?:%3|\\s)\\s*(\\d{1,2}%1?\\d*)(?:%4)?\\s*(?:,|;|\\s)\\s*") +
                                 QStringLiteral("%2\\s*([-+]?)(\\d{1,3})(?:%3|\\s)\\s*(\\d{1,2}%1?\\d*)(?:%4)?") +
                                 QStringLiteral(")\\z");

    const QString& expTemplate = (dirPosition == PostfixDir) ? postfixCapExp
                                 : prefixCapExp;

    const QString numberCapExp = expTemplate.arg(m_decimalPointExp, m_dirCapExp,
                                                 m_degreeExp, m_minutesExp);
    const QRegularExpression regex(numberCapExp);
    QRegularExpressionMatch match = regex.match(input);

    if (!match.hasMatch())
    {
        return false;
    }

    bool isDir1LonDir;
    bool isLonDirPosHemisphere;
    bool isLatDirPosHemisphere;
    const QString dir1 = match.captured(dirPosition == PostfixDir ? 4 : 1);
    const QString dir2 = match.captured(dirPosition == PostfixDir ? 8 : 5);

    if (!isCorrectDirections(dir1, dir2, isDir1LonDir,
                             isLonDirPosHemisphere, isLatDirPosHemisphere))
    {
        return false;
    }

    const int valueStartIndex1 = (dirPosition == PostfixDir ? 1 : 2);
    const int valueStartIndex2 = (dirPosition == PostfixDir ? 5 : 6);
    m_lon = degreeValueFromDM(match, isDir1LonDir ? valueStartIndex1 : valueStartIndex2,
                              isLonDirPosHemisphere);
    m_lat = degreeValueFromDM(match, isDir1LonDir ? valueStartIndex2 : valueStartIndex1,
                              isLatDirPosHemisphere);

    return true;
}

// #2: Two numbers with directions
bool LonLatParser::tryMatchFromD(const QString& input, DirPosition dirPosition)
{
    // direction as postfix, e.g. 74.2245 N 32.2434 W
    const QString postfixCapExp = QStringLiteral("\\A(?:") +
                                  QStringLiteral("([-+]?\\d{1,3}%1?\\d*)(?:%3)?(?:\\s*)%2(?:,|;|\\s)\\s*") +
                                  QStringLiteral("([-+]?\\d{1,3}%1?\\d*)(?:%3)?(?:\\s*)%2") +
                                  QStringLiteral(")\\z");

    // direction as prefix, e.g. N 74.2245 W 32.2434
    const QString prefixCapExp = QStringLiteral("\\A(?:") +
                                 QStringLiteral("%2\\s*([-+]?\\d{1,3}%1?\\d*)(?:%3)?\\s*(?:,|;|\\s)\\s*") +
                                 QStringLiteral("%2\\s*([-+]?\\d{1,3}%1?\\d*)(?:%3)?") +
                                 QStringLiteral(")\\z");

    const QString& expTemplate = (dirPosition == PostfixDir) ? postfixCapExp
                                 : prefixCapExp;

    const QString numberCapExp = expTemplate.arg(m_decimalPointExp, m_dirCapExp, m_degreeExp);
    const QRegularExpression regex(numberCapExp);
    // qCWarning(DIGIKAM_GEOENGINE_LOG) << regex.isValid() << regex.errorString() << regex.pattern();
    QRegularExpressionMatch match = regex.match(input);

    if (!match.hasMatch())
    {
        //         qCWarning(DIGIKAM_GEOENGINE_LOG) << "LonLatParser::tryMatchFromD -> no match";
        return false;
    }

    //     qCWarning(DIGIKAM_GEOENGINE_LOG) << "LonLatParser::tryMatchFromD -> match" << match;

    bool isDir1LonDir;
    bool isLonDirPosHemisphere;
    bool isLatDirPosHemisphere;
    const QString dir1 = match.captured(dirPosition == PostfixDir ? 2 : 1);
    const QString dir2 = match.captured(dirPosition == PostfixDir ? 4 : 3);

    if (!isCorrectDirections(dir1, dir2, isDir1LonDir,
                             isLonDirPosHemisphere, isLatDirPosHemisphere))
    {
        return false;
    }

    const int valueStartIndex1 = (dirPosition == PostfixDir ? 1 : 2);
    const int valueStartIndex2 = (dirPosition == PostfixDir ? 3 : 4);
    m_lon = degreeValueFromD(match, isDir1LonDir ? valueStartIndex1 : valueStartIndex2,
                             isLonDirPosHemisphere);
    m_lat = degreeValueFromD(match, isDir1LonDir ? valueStartIndex2 : valueStartIndex1,
                             isLatDirPosHemisphere);

    return true;
}

double LonLatParser::parseDouble(const QString& input)
{
    // Decide by decimalpoint if system locale or C locale should be tried.
    // Otherwise if first trying with a system locale when the string is in C locale,
    // the "." might be misinterpreted as thousands group separator and thus a wrong
    // value yielded
    QLocale locale = QLocale::system();
    return input.contains(locale.decimalPoint()) ? locale.toDouble(input) : input.toDouble();
}

QString LonLatParser::createDecimalPointExp()
{
    const QString decimalPoint = QLocale::system().decimalPoint();

    return (decimalPoint == QLatin1String(".")) ? QStringLiteral("\\.") :
           QLatin1String("[.") + decimalPoint + QLatin1Char(']');
}

void LonLatParser::getLocaleList(QStringList& localeList, const QString& localeListString,
                                 const QLatin1String& placeholder, const QString& separator)
{
    const QString lowerLocaleListString = localeListString.toLower();

    if (lowerLocaleListString != placeholder)
    {
        localeList = lowerLocaleListString.split(separator, Qt::SkipEmptyParts);
    }
}

bool LonLatParser::isDirection(const QString& input, const QStringList& directions)
{
    return (directions.contains(input));
}

bool LonLatParser::isDirection(const QString& input, const QString& direction)
{
    return (input == direction);
}

bool LonLatParser::isOneOfDirections(const QString& input,
                                     const QString& firstDirection,
                                     const QString& secondDirection,
                                     bool& isFirstDirection)
{
    isFirstDirection = isDirection(input, firstDirection);
    return isFirstDirection || isDirection(input, secondDirection);
}

bool LonLatParser::isOneOfDirections(const QString& input,
                                     const QStringList& firstDirections,
                                     const QStringList& secondDirections,
                                     bool& isFirstDirection)
{
    isFirstDirection = isDirection(input, firstDirections);
    return isFirstDirection || isDirection(input, secondDirections);
}


bool LonLatParser::isLocaleLonDirection(const QString& input,
                                        bool& isDirPosHemisphere) const
{
    return isOneOfDirections(input, m_eastLocale, m_westLocale, isDirPosHemisphere);
}

bool LonLatParser::isLocaleLatDirection(const QString& input,
                                        bool& isDirPosHemisphere) const
{
    return isOneOfDirections(input, m_northLocale, m_southLocale, isDirPosHemisphere);
}

bool LonLatParser::isLonDirection(const QString& input,
                                  bool& isDirPosHemisphere) const
{
    return isOneOfDirections(input, m_east, m_west, isDirPosHemisphere);
}

bool LonLatParser::isLatDirection(const QString& input,
                                  bool& isDirPosHemisphere) const
{
    return isOneOfDirections(input, m_north, m_south, isDirPosHemisphere);
}


qreal LonLatParser::degreeValueFromDMS(const QRegularExpressionMatch& regexMatch, int c, bool isPosHemisphere)
{
    const bool isNegativeValue = (regexMatch.captured(c++) == QLatin1String("-"));
    const uint degree = regexMatch.captured(c++).toUInt();
    const uint minutes = regexMatch.captured(c++).toUInt();
    const qreal seconds = parseDouble(regexMatch.captured(c));

    qreal result = degree + (minutes * MIN2HOUR) + (seconds * SEC2HOUR);

    if (isNegativeValue)
    {
        result *= -1;
    }

    if (! isPosHemisphere)
    {
        result *= -1;
    }

    return result;
}

qreal LonLatParser::degreeValueFromDM(const QRegularExpressionMatch& regexMatch, int c, bool isPosHemisphere)
{
    const bool isNegativeValue = (regexMatch.captured(c++) == QLatin1String("-"));
    const uint degree = regexMatch.captured(c++).toUInt();
    const qreal minutes = parseDouble(regexMatch.captured(c));

    qreal result = degree + (minutes * MIN2HOUR);

    if (isNegativeValue)
    {
        result *= -1;
    }

    if (! isPosHemisphere)
    {
        result *= -1;
    }

    return result;
}

qreal LonLatParser::degreeValueFromD(const QRegularExpressionMatch& regexMatch, int c, bool isPosHemisphere)
{
    qreal result = parseDouble(regexMatch.captured(c));

    if (! isPosHemisphere)
    {
        result *= -1;
    }

    return result;
}

bool LonLatParser::isCorrectDirections(const QString& dir1, const QString& dir2,
                                       bool& isDir1LonDir,
                                       bool& isLonDirPosHemisphere,
                                       bool& isLatDirPosHemisphere) const
{
    // first try localized names
    isDir1LonDir = isLocaleLonDirection(dir1, isLonDirPosHemisphere);
    const bool resultLocale = isDir1LonDir ?
                              isLocaleLatDirection(dir2, isLatDirPosHemisphere) :
                              (isLocaleLatDirection(dir1, isLatDirPosHemisphere) &&
                               isLocaleLonDirection(dir2, isLonDirPosHemisphere));

    if (resultLocale)
    {
        return resultLocale;
    }

    // fallback to try english names as lingua franca
    isDir1LonDir = isLonDirection(dir1, isLonDirPosHemisphere);
    return isDir1LonDir ?
           isLatDirection(dir2, isLatDirPosHemisphere) :
           (isLatDirection(dir1, isLatDirPosHemisphere) &&
            isLonDirection(dir2, isLonDirPosHemisphere));
}

} // namespace Marble
