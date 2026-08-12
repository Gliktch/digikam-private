/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-24
 * Description : manager for filters (registering, creating etc)
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QStringList>
#include <QList>
#include <QString>

// Local includes

#include "dimgfiltergenerator.h"
#include "digikam_export.h"

namespace Digikam
{

class FilterAction;

class DIGIKAM_EXPORT DImgFilterManager : public DImgFilterGenerator
{
public:

    DImgFilterManager();
    ~DImgFilterManager()                                            override;

    static DImgFilterManager* instance();

    /**
     * @return A list of the supported filter identifiers
     */
    QStringList supportedFilters()                                  override;

    /**
     * @return A list of supported versions of the given filter
     */
    QList<int> supportedVersions(const QString& filterIdentifier)   override;

    /**
     * @return The (untranslated) displayable name for the given identifier.
     * This is only possible for supported filters. If you have a FilterAction,
     * it may already contain a displayable name.
     */
    QString displayableName(const QString& filterIdentifier)        override;

    /**
     * @return The translated displayable name
     */
    QString i18nDisplayableName(const QString& filterIdentifier);
    QString i18nDisplayableName(const FilterAction& action);

    /**
     * @return An icon for the given filter.
     * If no icon is known, returns a null string.
     */
    QString filterIcon(const QString& filterIdentifier);
    QString filterIcon(const FilterAction& action);

    /**
     * @return True if the given filter, or, more specifically,
     * the given filter in the given version is supported.
     */
    bool isSupported(const QString& filterIdentifier)               override;
    bool isSupported(const QString& filterIdentifier, int version)  override;

    /**
     * @return True if the given filter is to be considered
     * as a step converting a RAW image to a normal image.
     */
    bool isRawConversion(const QString& filterIdentifier);

    /**
     * @brief Create a filter from an installed manager.
     *
     * @return 0 if no filter could be created. This is true
     * if identifier/version is not supported, or the filter is builtin.
     *
     * @note You probably want to use FilterActionFilter.
     */
    DImgThreadedFilter* createFilter(const QString& filterIdentifier,
                                     int version)                   override;

    /**
     * @brief Registers all filter provided by this generator.
     */
    void addGenerator(DImgFilterGenerator* const generator);
    void removeGenerator(DImgFilterGenerator* const generator);

private:

    /// @note disabled
    DImgFilterManager(const DImgFilterManager&)            = delete;
    DImgFilterManager& operator=(const DImgFilterManager&) = delete;

private:

    friend class DImgFilterManagerCreator;

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
