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

#include "batchtoolsfactory.h"

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "dpluginloader.h"
#include "dpluginbqm.h"
#include "bqminfoiface.h"

namespace Digikam
{

class Q_DECL_HIDDEN BatchToolsFactory::Private
{

public:

    Private() = default;

    BqmInfoIface*  iface     = nullptr;

    BatchToolsList toolsList;
};

// --------------------------------------------------------------------------------

class Q_DECL_HIDDEN BatchToolsFactoryCreator
{
public:

    BatchToolsFactory object;
};

Q_GLOBAL_STATIC(BatchToolsFactoryCreator, creator)

// --------------------------------------------------------------------------------

BatchToolsFactory::BatchToolsFactory()
    : QObject(),
      d      (new Private)
{
    d->iface                 = new BqmInfoIface(this);
    DPluginLoader* const dpl = DPluginLoader::instance();
    bool hasJXLSupport       = dpl->canExport(QLatin1String("JXL"));
    bool hasWEBPSupport      = dpl->canExport(QLatin1String("WEBP"));
    bool hasAVIFSupport      = dpl->canExport(QLatin1String("AVIF"));
    const auto plugins       = dpl->allPlugins();

    for (DPlugin* const p : plugins)
    {
        DPluginBqm* const bqm = dynamic_cast<DPluginBqm*>(p);

        if (bqm)
        {
            // NOTE: Do not load JXL convert BQM plugin if DImg do not have JXL support.

            if (
                (bqm->iid() == QLatin1String("org.kde.digikam.plugin.bqm.ConvertToJxl")) &&
                !hasJXLSupport
               )
            {
                continue;
            }

            if (
                (bqm->iid() == QLatin1String("org.kde.digikam.plugin.bqm.ConvertToWebp")) &&
                !hasWEBPSupport
               )
            {
                continue;
            }

            if (
                (bqm->iid() == QLatin1String("org.kde.digikam.plugin.bqm.ConvertToAvif")) &&
                !hasAVIFSupport
               )
            {
                continue;
            }

            bqm->setup(this);
/*
            qCDebug(DIGIKAM_GENERAL_LOG) << "BQM plugin named" << bqm->name()
                                         << "registered to" << this;
*/

            const auto ts = bqm->tools(this);

            for (BatchTool* const t : ts)
            {
                registerTool(t);
            }
        }
    }
}

BatchToolsFactory::~BatchToolsFactory()
{
    delete d;
}

BatchToolsFactory* BatchToolsFactory::instance()
{
    return &creator->object;
}

BatchToolsList BatchToolsFactory::toolsList() const
{
    return d->toolsList;
}

BqmInfoIface* BatchToolsFactory::infoIface() const
{
    return d->iface;
}

void BatchToolsFactory::registerTool(BatchTool* const tool)
{
    if (!tool)
    {
        return;
    }

    d->toolsList.append(tool);
}

BatchTool* BatchToolsFactory::findTool(const QString& name, BatchTool::BatchToolGroup group) const
{
    for (BatchTool* const tool : std::as_const(d->toolsList))
    {
        if ((tool->objectName() == name) && (tool->toolGroup() == group))
        {   // cppcheck-suppress useStlAlgorithm
            return tool;
        }
    }

    return nullptr;
}

} // namespace Digikam

#include "moc_batchtoolsfactory.cpp"
