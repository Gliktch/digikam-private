/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-24
 * Description : Batch Tools Factory.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "batchtool.h"
#include "batchtoolutils.h"

namespace Digikam
{

class BqmInfoIface;

class BatchToolsFactory : public QObject
{
    Q_OBJECT

public:

    BatchToolsFactory();
    ~BatchToolsFactory() override;

    static BatchToolsFactory* instance();

    void           registerTool(BatchTool* const tool);
    BatchTool*     findTool(const QString& name, BatchTool::BatchToolGroup group) const;
    BatchToolsList toolsList()                                                    const;
    BqmInfoIface*  infoIface()                                                    const;

private:

    /// @note disabled
    explicit BatchToolsFactory(QObject*) = delete;

private:

    class Private;
    Private* const d = nullptr;

    friend class BatchToolsFactoryCreator;
};

} // namespace Digikam
