/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : manager to load external plugins at run-time
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dpluginloader_p.h"

// Qt includes

#include <QStringList>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "dplugingeneric.h"
#include "dplugineditor.h"
#include "dplugindimg.h"
#include "dpluginrawimport.h"

namespace Digikam
{

class Q_DECL_HIDDEN DPluginLoaderCreator
{
public:

    DPluginLoader object;
};

Q_GLOBAL_STATIC(DPluginLoaderCreator, creator)

// -----------------------------------------------------

DPluginLoader::DPluginLoader()
    : QObject(),
      d      (new Private)
{
}

DPluginLoader::~DPluginLoader()
{
    delete d;
}

DPluginLoader* DPluginLoader::instance()
{
    return &creator->object;
}

void DPluginLoader::init()
{
    d->loadPlugins();
}

void DPluginLoader::cleanUp()
{
    for (DPlugin* const p : std::as_const(d->allPlugins))
    {
        p->cleanUp();
    }

    d->allPlugins.clear();
}

QString DPluginLoader::configGroupName() const
{
    return QLatin1String("EnabledDPlugins");
}

QList<DPlugin*> DPluginLoader::allPlugins() const
{
    return d->allPlugins;
}

QList<DPluginAction*> DPluginLoader::pluginsActions(DPluginAction::ActionType type, QObject* const parent) const
{
    QList<DPluginAction*> list;

    for (DPlugin* const p : std::as_const(d->allPlugins))
    {
        const DPluginGeneric* const gene = dynamic_cast<DPluginGeneric*>(p);

        if (gene)
        {
            const auto acs = gene->actions(parent);

            for (DPluginAction* const ac : acs)      // cppcheck-suppress constVariablePointer
            {
                if (ac && (ac->actionType() == type))
                {
                    list << ac;
                }
            }
        }
    }

    if (list.isEmpty())
    {
        for (DPlugin* const p : std::as_const(d->allPlugins))
        {
            const DPluginEditor* const edit = dynamic_cast<DPluginEditor*>(p);

            if (edit)
            {
                const auto acs = edit->actions(parent);

                for (DPluginAction* const ac : acs)      // cppcheck-suppress constVariablePointer
                {
                    if (ac && (ac->actionType() == type))
                    {
                        list << ac;
                    }
                }
            }
        }
    }

    std::sort(list.begin(), list.end(), DPluginAction::pluginActionLessThan);

    return list;
}

QList<DPluginAction*> DPluginLoader::pluginsActions(DPluginAction::ActionCategory cat, QObject* const parent) const
{
    QList<DPluginAction*> list;

    for (DPlugin* const p : std::as_const(d->allPlugins))
    {
        const DPluginGeneric* const gene = dynamic_cast<DPluginGeneric*>(p);

        if (gene)
        {
            const auto acs = gene->actions(parent);

            for (DPluginAction* const ac : acs)      // cppcheck-suppress constVariablePointer
            {
                if (ac && (ac->actionCategory() == cat))
                {
                    list << ac;
                }
            }
        }
    }

    if (list.isEmpty())
    {
        for (DPlugin* const p : std::as_const(d->allPlugins))
        {
            const DPluginEditor* const edit = dynamic_cast<DPluginEditor*>(p);

            if (edit)
            {
                const auto acs = edit->actions(parent);

                for (DPluginAction* const ac : acs)      // cppcheck-suppress constVariablePointer
                {
                    if (ac && (ac->actionCategory() == cat))
                    {
                        list << ac;
                    }
                }
            }
        }
    }

    std::sort(list.begin(), list.end(), DPluginAction::pluginActionLessThan);

    return list;
}

QList<DPluginAction*> DPluginLoader::pluginActions(const QString& pluginIID, QObject* const parent) const
{
    QList<DPluginAction*> list;

    for (DPlugin* const p : std::as_const(d->allPlugins))
    {
        const DPluginGeneric* const gene = dynamic_cast<DPluginGeneric*>(p);

        if (gene)
        {
            if (p->iid() == pluginIID)
            {
                const auto acs = gene->actions(parent);

                for (DPluginAction* const ac : acs)      // cppcheck-suppress constVariablePointer
                {
                    list << ac;
                }

                break;
            }
        }
    }

    if (list.isEmpty())
    {
        for (DPlugin* const p : std::as_const(d->allPlugins))
        {
            const DPluginEditor* const edit = dynamic_cast<DPluginEditor*>(p);

            if (edit)
            {
                if (p->iid() == pluginIID)
                {
                    const auto acs = edit->actions(parent);

                    for (DPluginAction* const ac : acs)      // cppcheck-suppress constVariablePointer
                    {
                        list << ac;
                    }

                    break;
                }
            }
        }
    }

    std::sort(list.begin(), list.end(), DPluginAction::pluginActionLessThan);

    return list;
}

DPluginAction* DPluginLoader::pluginAction(const QString& actionName, QObject* const parent) const
{
    for (DPlugin* const p : std::as_const(d->allPlugins))
    {
        const DPluginGeneric* const gene = dynamic_cast<DPluginGeneric*>(p);

        if (gene)
        {
            const auto acs = gene->actions(parent);

            for (DPluginAction* const ac : acs)
            {
                if (ac && (ac->objectName() == actionName))
                {    // cppcheck-suppress useStlAlgorithm
                    return ac;
                }
            }
        }

        const DPluginEditor* const edit = dynamic_cast<DPluginEditor*>(p);

        if (edit)
        {
            const auto acs = edit->actions(parent);

            for (DPluginAction* const ac : acs)
            {
                if (ac && (ac->objectName() == actionName))
                {   // cppcheck-suppress useStlAlgorithm
                    return ac;
                }
            }
        }
    }

    qCCritical(DIGIKAM_GENERAL_LOG) << "DPluginAction named" << actionName
                                    << "not found in" << parent->objectName()
                                    << "(" << parent << ")";

    return nullptr;
}

QString DPluginLoader::pluginXmlSections(DPluginAction::ActionCategory cat, QObject* const parent) const
{
    QString xml;
    const auto acs = pluginsActions(cat, parent);

    for (const DPluginAction* const ac : acs)
    {
        xml.append(ac->xmlSection());
    }

    return xml;
}

void DPluginLoader::appendPluginToBlackList(const QString& filename)
{
    d->blacklist << filename;
}

void DPluginLoader::appendPluginToWhiteList(const QString& filename)
{
    d->whitelist << filename;
}

void DPluginLoader::registerGenericPlugins(QObject* const parent)
{
    for (DPlugin* const p : std::as_const(d->allPlugins))
    {
        DPluginGeneric* const gene = dynamic_cast<DPluginGeneric*>(p);

        if (gene)
        {
            gene->setup(parent);
            gene->setVisible(p->shouldLoaded());
/*
            qCDebug(DIGIKAM_GENERAL_LOG) << "Generic plugin named" << gene->name()
                                         << "registered to" << parent;
*/
        }
    }
}

void DPluginLoader::registerEditorPlugins(QObject* const parent)
{
    for (DPlugin* const p : std::as_const(d->allPlugins))
    {
        DPluginEditor* const edit = dynamic_cast<DPluginEditor*>(p);

        if (edit)
        {
            edit->setup(parent);
            edit->setVisible(p->shouldLoaded());
/*
            qCDebug(DIGIKAM_GENERAL_LOG) << "Editor plugin named" << edit->name()
                                         << "registered to" << parent;
*/
        }
    }
}

void DPluginLoader::registerRawImportPlugins(QObject* const parent)
{
    for (DPlugin* const p : std::as_const(d->allPlugins))
    {
        DPluginRawImport* const raw = dynamic_cast<DPluginRawImport*>(p);

        if (raw)
        {
            raw->setup(parent);
/*
            qCDebug(DIGIKAM_GENERAL_LOG) << "Raw Import plugin named" << raw->name()
                                         << "registered to" << parent;
*/
        }
    }
}

DImgLoaderSettings* DPluginLoader::exportWidget(const QString& format) const
{
    for (DPlugin* const p : std::as_const(d->allPlugins))
    {
        const DPluginDImg* const dimg = dynamic_cast<DPluginDImg*>(p);

        if (dimg)
        {
            DImgLoaderSettings* const widget = dimg->exportWidget(format);

            if (widget)
            {
                return widget;
            }
        }
    }

    return nullptr;
}

bool DPluginLoader::canImport(const QString& format) const
{
    for (DPlugin* const p : std::as_const(d->allPlugins))
    {
        const DPluginDImg* const dimg = dynamic_cast<DPluginDImg*>(p);

        if (dimg && dimg->canRead(QFileInfo(QString::fromLatin1("foo.%1").arg(format)), true))
        {
            return true;
        }
    }

    return false;
}

bool DPluginLoader::canExport(const QString& format) const
{
    for (DPlugin* const p : std::as_const(d->allPlugins))
    {
        const DPluginDImg* const dimg = dynamic_cast<DPluginDImg*>(p);

        if (dimg && dimg->canWrite(format))
        {
            return true;
        }
    }

    return false;
}

} // namespace Digikam

#include "moc_dpluginloader.cpp"
