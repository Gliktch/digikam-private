/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2026-06-13
 * Description : Filename matching helper for timelapse grouping
 *
 * SPDX-FileCopyrightText: 2026 by François Martin <kde at fmartin dot ch>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "timelapsefilenamematch.h"

// C++ includes

#include <limits>

// Qt includes

#include <QDate>
#include <QTime>

namespace Digikam
{

namespace
{

class PrefixDateToken
{
public:

    QDate   date;
    QString textBeforeDate;
    QString textAfterDate;
    bool    isValid = false;
};

PrefixDateToken prefixDateToken(const QString& prefix)
{
    qsizetype tokenEnd = prefix.size();

    while ((tokenEnd > 0) && !prefix.at(tokenEnd - 1).isDigit())
    {
        if (prefix.at(tokenEnd - 1).isLetterOrNumber())
        {
            return {};
        }

        --tokenEnd;
    }

    if (tokenEnd < 8)
    {
        return {};
    }

    const qsizetype tokenStart = tokenEnd - 8;

    if ((tokenStart > 0) && prefix.at(tokenStart - 1).isDigit())
    {
        return {};
    }

    for (qsizetype index = tokenStart ; index < tokenEnd ; ++index)
    {
        if (!prefix.at(index).isDigit())
        {
            return {};
        }
    }

    const int   year  = prefix.mid(tokenStart,     4).toInt();
    const int   month = prefix.mid(tokenStart + 4, 2).toInt();
    const int   day   = prefix.mid(tokenStart + 6, 2).toInt();
    const QDate date(year, month, day);

    if (!date.isValid())
    {
        return {};
    }

    return { date, prefix.left(tokenStart), prefix.mid(tokenEnd), true };
}

QTime timeFromFilenameValue(qulonglong value, qsizetype valueLength)
{
    if (valueLength == 6)
    {
        const int hour   = static_cast<int>(value / 10000);
        const int minute = static_cast<int>((value / 100) % 100);
        const int second = static_cast<int>(value % 100);

        return QTime(hour, minute, second);
    }

    if (valueLength == 9)
    {
        const int hour        = static_cast<int>(value / 10000000);
        const int minute      = static_cast<int>((value / 100000) % 100);
        const int second      = static_cast<int>((value / 1000) % 100);
        const int millisecond = static_cast<int>(value % 1000);

        return QTime(hour, minute, second, millisecond);
    }

    return QTime();
}

bool timestampPreceeds(const TimelapseFilenameMatch& first, const TimelapseFilenameMatch& second)
{
    if (first.valueLength != second.valueLength)
    {
        return false;
    }

    const PrefixDateToken firstDateToken  = prefixDateToken(first.prefix);
    const PrefixDateToken secondDateToken = prefixDateToken(second.prefix);

    if (!firstDateToken.isValid || !secondDateToken.isValid)
    {
        return false;
    }

    if (firstDateToken.textBeforeDate != secondDateToken.textBeforeDate)
    {
        return false;
    }

    if (firstDateToken.textAfterDate != secondDateToken.textAfterDate)
    {
        return false;
    }

    const QTime firstTime  = timeFromFilenameValue(first.value, first.valueLength);
    const QTime secondTime = timeFromFilenameValue(second.value, second.valueLength);

    if (!firstTime.isValid() || !secondTime.isValid())
    {
        return false;
    }

    if (firstDateToken.date != secondDateToken.date)
    {
        return (firstDateToken.date < secondDateToken.date);
    }

    return (firstTime < secondTime);
}

QString suffixWithoutCoverMarker(const QString& suffix)
{
    if (suffix.startsWith(QLatin1String("_COVER")))
    {
        return suffix.mid(6);
    }

    return suffix;
}

bool isLargestFixedWidthCounterValue(const TimelapseFilenameMatch& match)
{
    if (match.valueLength <= 0)
    {
        return false;
    }

    qulonglong largestValue = 0;

    for (qsizetype index = 0 ; index < match.valueLength ; ++index)
    {
        if (largestValue > ((std::numeric_limits<qulonglong>::max() - 9) / 10))
        {
            return false;
        }

        largestValue = (largestValue * 10) + 9;
    }

    return (match.value == largestValue);
}

bool counterPreceeds(const TimelapseFilenameMatch& first, const TimelapseFilenameMatch& second)
{
    if (first.prefix != second.prefix)
    {
        return false;
    }

    if ((first.value < std::numeric_limits<qulonglong>::max()) &&
        ((first.value + 1) == second.value))
    {
        return true;
    }

    if ((first.valueLength != second.valueLength) || (second.value != 0))
    {
        return false;
    }

    return isLargestFixedWidthCounterValue(first);
}

bool burstCoverPreceeds(const TimelapseFilenameMatch& first, const TimelapseFilenameMatch& second)
{
    if (!counterPreceeds(first, second))
    {
        return false;
    }

    if (!first.prefix.contains(QLatin1String("BURST")))
    {
        return false;
    }

    if (first.suffix == second.suffix)
    {
        return false;
    }

    return (suffixWithoutCoverMarker(first.suffix) == second.suffix);
}

bool matchesSameFilenameSequence(const TimelapseFilenameMatch& first, const TimelapseFilenameMatch& second)
{
    if (!first.containsValue || !second.containsValue)
    {
        return false;
    }

    if (first.prefix != second.prefix)
    {
        return false;
    }

    if (first.suffix == second.suffix)
    {
        return true;
    }

    if (!first.prefix.contains(QLatin1String("BURST")))
    {
        return false;
    }

    return (suffixWithoutCoverMarker(first.suffix) == suffixWithoutCoverMarker(second.suffix));
}

qsizetype sequenceStartIndex(const QList<TimelapseFilenameMatch>& nameSortedMatches,
                             qsizetype startIndex,
                             qsizetype endIndex)
{
    if ((endIndex - startIndex) < 2)
    {
        return startIndex;
    }

    if (!nameSortedMatches.at(endIndex - 1).directlyPreceeds(nameSortedMatches.at(startIndex)))
    {
        return startIndex;
    }

    qsizetype result = startIndex;

    for (qsizetype index = startIndex + 1 ; index < endIndex ; ++index)
    {
        if (!nameSortedMatches.at(index - 1).directlyPreceeds(nameSortedMatches.at(index)))
        {
            result = index;
        }
    }

    return result;
}

} // namespace

TimelapseFilenameMatch::TimelapseFilenameMatch(const QString& filename)
{
    if (filename.isEmpty())
    {
        return;
    }

    const qsizetype suffixStart = filename.lastIndexOf(QLatin1Char('.'));
    const qsizetype searchEnd   = (suffixStart > 0) ? suffixStart : filename.size();

    qsizetype lastDigit = searchEnd - 1;

    while ((lastDigit >= 0) && !filename.at(lastDigit).isDigit())
    {
        --lastDigit;
    }

    if (lastDigit < 0)
    {
        prefix = filename;

        return;
    }

    qsizetype firstDigit = lastDigit;

    while ((firstDigit > 0) && filename.at(firstDigit - 1).isDigit())
    {
        --firstDigit;
    }

    prefix = filename.left(firstDigit);
    valueLength = lastDigit - firstDigit + 1;
    value  = filename.mid(firstDigit,
                          valueLength).toULongLong(&containsValue);

    suffix = filename.mid(lastDigit + 1);
}

bool TimelapseFilenameMatch::directlyPreceeds(TimelapseFilenameMatch const& other) const
{
    if (!containsValue || !other.containsValue)
    {
        return false;
    }

    if (suffix != other.suffix)
    {
        return burstCoverPreceeds(*this, other);
    }

    if (counterPreceeds(*this, other))
    {
        return true;
    }

    return timestampPreceeds(*this, other);
}

QList<qsizetype> timelapseFilenameSequenceOrder(const QList<TimelapseFilenameMatch>& nameSortedMatches)
{
    QList<qsizetype> orderedIndexes;
    orderedIndexes.reserve(nameSortedMatches.size());

    for (qsizetype runStart = 0 ; runStart < nameSortedMatches.size() ; )
    {
        qsizetype runEnd = runStart + 1;

        while ((runEnd < nameSortedMatches.size()) &&
               matchesSameFilenameSequence(nameSortedMatches.at(runStart), nameSortedMatches.at(runEnd)))
        {
            ++runEnd;
        }

        const qsizetype runSequenceStart = sequenceStartIndex(nameSortedMatches, runStart, runEnd);

        for (qsizetype index = runSequenceStart ; index < runEnd ; ++index)
        {
            orderedIndexes.append(index);
        }

        for (qsizetype index = runStart ; index < runSequenceStart ; ++index)
        {
            orderedIndexes.append(index);
        }

        runStart = runEnd;
    }

    return orderedIndexes;
}

} // namespace Digikam
