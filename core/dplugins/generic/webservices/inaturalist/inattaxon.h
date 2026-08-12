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

#pragma once

// Qt includes

#include <QString>
#include <QUrl>
#include <QList>

namespace DigikamGenericINatPlugin
{

/**
 * A taxon is obtained via the iNaturalist API and it is read-only; thus there
 * are no member functions to modify it and only const-references to its members
 * are returned.
 *
 * Taxa have a unique numeric id and a name - the scientific or Latin name.
 * Taxa also have a textual rank (e.g. 'species') and a numeric rank-level (10
 * for species).
 * The common name may not be present and its value depends on the locale: it
 * can be Chien, Gedomesticeerde Hond, Домашняя Собака, Domestic Dog, Haushund,
 * or Perro Doméstico for the iNaturalist taxon with id 47144 and scientific
 * name 'Canis familiaris'. Even in the same country and language common names
 * differ from place to place. E.g. 'Umbellularia californica' is the scientific
 * name of a tree in the laurel family that occurs in California and Oregon. Its
 * common name is 'California Bay Laurel' in California and 'Oregon Myrtle' in
 * Oregon.
 * Member 'ancestors' is a list of parent taxa ordered from the highest
 * rank 'kingdom' downwards. The url fetches a small, square-shape icon and is
 * unavailable for some taxa. The matched term is only non-empty for taxa
 * returned from auto-completion.
 */
class Taxon
{
public:

    Taxon(int id,
          int parentId,
          const QString& name,
          const QString& rank,
          double rankLevel,
          const QString& commonName,
          const QString& matchedTerm,
          const QUrl& squareUrl,
          const QList<Taxon>& ancestors);
    Taxon();
    Taxon(const Taxon&);
    ~Taxon();

    bool                isValid()     const;
    int                 id()          const;
    int                 parentId()    const;
    const QString&      name()        const;
    QString             htmlName()    const;
    const QString&      rank()        const;
    double              rankLevel()   const;
    const QString&      commonName()  const;
    const QList<Taxon>& ancestors()   const;
    const QUrl&         squareUrl()   const;
    const QString&      matchedTerm() const;

    Taxon& operator= (const Taxon&);
    bool   operator!=(const Taxon&)   const;
    bool   operator==(const Taxon&)   const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericINatPlugin
