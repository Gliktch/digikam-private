/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-06-10
 * Description : Progress observer for duplicate scanning
 *
 * SPDX-FileCopyrightText: 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QImage>

// Local includes

#include "haariface.h"
#include "dbjob.h"
#include "iteminfo.h"
#include "digikam_export.h"

namespace Digikam
{

class SearchesJob;

class DIGIKAM_DATABASE_EXPORT DuplicatesProgressObserver : public HaarProgressObserver
{

public:

    explicit DuplicatesProgressObserver(SearchesJob* const thread);
    ~DuplicatesProgressObserver()                                           override;

    void imageProcessed(const ItemInfo& inf, const QImage& img, int dup)    override;
    bool isCanceled()                                                       override;

private:

    SearchesJob* m_job = nullptr;

private:

    Q_DISABLE_COPY(DuplicatesProgressObserver)
};

} // namespace Digikam
