/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-02-05
 * Description : undo cache manager for image editor.
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005      by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dimg.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT UndoCache
{

public:

    explicit UndoCache();
    ~UndoCache();

    /**
     * Delete all cache files
     */
    void clear();

    /**
     * Delete all cache files starting from the given level upwards
     */
    void clearFrom(int level);

    /**
     * Write the image data into a cache file
     */
    bool putData(int level, const DImg& img) const;

    /**
     * Get the image data from a cache file
     */
    DImg getData(int level)                  const;

private:

    /// @note disabled
    UndoCache(const UndoCache&)            = delete;
    UndoCache& operator=(const UndoCache&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
