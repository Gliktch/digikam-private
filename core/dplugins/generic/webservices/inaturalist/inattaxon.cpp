/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-03-20
 * Description : a tool to export images to iNaturalist web service
 *
 * SPDX-FileCopyrightText: 2021      by Joerg Lohse <joergmlpts at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "inattaxon.h"

// Local includes

#include "inatutils.h"

namespace DigikamGenericINatPlugin
{

class Q_DECL_HIDDEN Taxon::Private
{
public:

    Private() = default;

    int          id         = -1;
    int          parentId   = -1;
    QString      name;
    QString      rank;
    double       rankLevel  = -1.0;
    QString      commonName;
    QString      matchedTerm;
    QUrl         squareUrl;
    QList<Taxon> ancestors;
};

Taxon::Taxon(int id,
             int parentId,
             const QString& name,
             const QString& rank,
             double rankLevel,
             const QString& commonName,
             const QString& matchedTerm,
             const QUrl& squareUrl,
             const QList<Taxon>& ancestors)
    : d(new Private)
{
    d->id          = id;
    d->parentId    = parentId;
    d->name        = name;
    d->rank        = rank;
    d->rankLevel   = rankLevel;
    d->commonName  = commonName;
    d->matchedTerm = matchedTerm;
    d->squareUrl   = squareUrl;
    d->ancestors   = ancestors;
}

Taxon::Taxon()
    : d(new Private)
{
}

Taxon::Taxon(const Taxon& other)
    : d(new Private)
{
    *d = *other.d;
}

Taxon::~Taxon()
{
    delete d;
}

Taxon& Taxon::operator=(const Taxon& other)
{
    *d = *other.d;

    return *this;
}

bool Taxon::operator!=(const Taxon& other) const
{
    return (d->id != other.d->id);
}

bool Taxon::operator==(const Taxon& other) const
{
    return (d->id == other.d->id);
}

int Taxon::id() const
{
    return d->id;
}

int Taxon::parentId() const
{
    return d->parentId;
}

const QString& Taxon::name() const
{
    return d->name;
}

/**
 * return italicized scientific name
 */
QString Taxon::htmlName() const
{
    static const QString species    = QLatin1String("species");
    static const QString subspecies = QLatin1String("subspecies");
    static const QString variety    = QLatin1String("variety");
    static const QString hybrid     = QLatin1String("hybrid");
    static const QString italicOn   = QLatin1String("<i>");
    static const QString italicOff  = QLatin1String("</i>");
    static const QChar   blank      = QLatin1Char(' ');

    QString result;

    if ((rank() != species) && (rank() != subspecies) && (rank() != variety))
    {
        result    = localizedTaxonomicRank(rank()) + blank;
        result[0] = result[0].toTitleCase();
    }

    QStringList split = name().split(blank);

    if      (
             (split.count() == 3) &&
             ((rank() == subspecies) || (rank() == variety) || (rank() == hybrid))
            )
    {
        QString txt = (rank() == subspecies) ? QLatin1String(" ssp. ")
                                             : (rank() == variety) ? QLatin1String(" var. ")
                                                                   : QLatin1String(" x ");
        result += italicOn + split[0] + blank + split[1] + italicOff + txt +
                  italicOn + split[2] + italicOff;
    }
    else if (split.count() == 4)
    {
        result += italicOn + split[0] + blank + split[1] + italicOff + blank +
                  split[2] + blank + italicOn + split[3] + italicOff;
    }
    else
    {
        result += italicOn + name() + italicOff;
    }

    return result;
}

const QString& Taxon::rank() const
{
    return d->rank;
}

double Taxon::rankLevel() const
{
    return d->rankLevel;
}

const QString& Taxon::matchedTerm() const
{
    return d->matchedTerm;
}

const QUrl& Taxon::squareUrl() const
{
    return d->squareUrl;
}

const QString& Taxon::commonName() const
{
    return d->commonName;
}

const QList<Taxon>& Taxon::ancestors() const
{
    return d->ancestors;
}

bool Taxon::isValid() const
{
    return (d->id != -1);
}

} // namespace DigikamGenericINatPlugin
