/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-24
 * Description : class for creating a particular filter
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QString>
#include <QStringList>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DImgThreadedFilter;

class DIGIKAM_EXPORT DImgFilterGenerator
{
public:

    DImgFilterGenerator()          = default;
    virtual ~DImgFilterGenerator() = default;

    /// @return A list with identifiers of supported filters
    virtual QStringList supportedFilters()                                  = 0;

    /// @return A list with the supported versions for the given identifier
    virtual QList<int>  supportedVersions(const QString& filterIdentifier)  = 0;

    /// @return A QString with filter name for displaying in views
    virtual QString displayableName(const QString& filterIdentifier)        = 0;

    /// @brief Convenience methods
    virtual bool isSupported(const QString& filterIdentifier);
    virtual bool isSupported(const QString& filterIdentifier, int version);

    /// @brief Create the filter for the given combination of identifier and version
    virtual DImgThreadedFilter* createFilter(const QString& filterIdentifier,
                                             int version)                   = 0;
private:

    Q_DISABLE_COPY(DImgFilterGenerator)
};

// -----------------------------------------------------------------------------------

template <class T>
class BasicDImgFilterGenerator : public DImgFilterGenerator
{
public:

    /**
     * @brief A sample implementation for one DImgThreadedFilter class.
     * The class must provide two static methods, FilterIdentifier() and SupportedVersions().
     */

    BasicDImgFilterGenerator() = default;

    QStringList supportedFilters() override
    {
        return QList<QString>() << T::FilterIdentifier();
    }

    QList<int> supportedVersions(const QString& filterIdentifier) override
    {
        if (filterIdentifier == T::FilterIdentifier())
        {
            return T::SupportedVersions();
        }

        return QList<int>();
    }

    DImgThreadedFilter* createFilter(const QString& filterIdentifier, int version) override
    {
        if ((filterIdentifier == T::FilterIdentifier()) &&
            T::SupportedVersions().contains(version))
        {
            T* const t = new T;
            t->setFilterVersion(version);

            return t;
        }

        return nullptr;
    }

    QString displayableName(const QString& filterIdentifier) override
    {
        if (filterIdentifier == T::FilterIdentifier())
        {
            return T::DisplayableName();
        }

        return QString();
    }
};

} // namespace Digikam
