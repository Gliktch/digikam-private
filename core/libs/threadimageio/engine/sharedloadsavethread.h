/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-01-16
 * Description : image file IO threaded interface.
 *
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "managedloadsavethread.h"

namespace Digikam
{

class DIGIKAM_EXPORT SharedLoadSaveThread : public ManagedLoadSaveThread
{
    Q_OBJECT

public:

    explicit SharedLoadSaveThread(QObject* const parent = nullptr);
    ~SharedLoadSaveThread() override = default;

    void loadShared(const LoadingDescription& description,
                    AccessMode mode,
                    LoadingPolicy policy = LoadingPolicyAppend);

private:

    /// @note disabled
    SharedLoadSaveThread(const SharedLoadSaveThread&)            = delete;
    SharedLoadSaveThread& operator=(const SharedLoadSaveThread&) = delete;
};

} // namespace Digikam
