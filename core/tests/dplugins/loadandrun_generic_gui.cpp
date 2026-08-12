/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : stand alone test application for plugin
 *               loader and generic tools.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt Includes

#include <QApplication>
#include <QCommandLineParser>
#include <QUrl>
#include <QIcon>
#include <QLibraryInfo>

// Local includes

#include "metaengine.h"
#include "dmetainfoiface.h"
#include "dpluginloader.h"
#include "dplugingeneric.h"
#include "digikam_debug.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    qCDebug(DIGIKAM_TESTS_LOG) << QLibraryInfo::path(QLibraryInfo::LibrariesPath);
    qCDebug(DIGIKAM_TESTS_LOG) << QLibraryInfo::path(QLibraryInfo::LibraryExecutablesPath);
    qCDebug(DIGIKAM_TESTS_LOG) << QLibraryInfo::path(QLibraryInfo::PluginsPath);

#else

    qCDebug(DIGIKAM_TESTS_LOG) << QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    qCDebug(DIGIKAM_TESTS_LOG) << QLibraryInfo::location(QLibraryInfo::LibraryExecutablesPath);
    qCDebug(DIGIKAM_TESTS_LOG) << QLibraryInfo::location(QLibraryInfo::PluginsPath);

#endif

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.setApplicationDescription(QLatin1String("Test application to run digiKam generic plugins as stand alone\n"
                                                   "Example: ./loadandrun_generic -l \"org.kde.digikam.plugin.generic.TimeAdjust\" -a \"timeadjust_edit\" /mnt/photo/*.jpg"));

    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("list"), QLatin1String("List all available plugins")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("l"),    QLatin1String("Unique name ID of the plugin to use"), QLatin1String("Plugin IID")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("a"),    QLatin1String("Plugin internal action name to run"),  QLatin1String("Internal Action Name")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("w"),    QLatin1String("Wait until plugin non-modal dialog is closed")));
    parser.addPositionalArgument(QLatin1String("files"), QLatin1String("File(s) to open"), QLatin1String("+[file(s)]"));
    parser.process(app);

    QList<QUrl> urlList;
    const QStringList args = parser.positionalArguments();

    for (auto& arg : args)
    {
        urlList.append(QUrl::fromLocalFile(arg));
    }

    DMetaInfoIface iface(qApp, urlList, QUrl());
    DPluginLoader* const dpl = DPluginLoader::instance();
    dpl->init();
    dpl->registerGenericPlugins(&iface);

    bool found = false;

    if      (parser.isSet(QString::fromLatin1("list")))
    {
        const auto dpls = dpl->allPlugins();

        for (DPlugin* const p : dpls)
        {
            DPluginGeneric* const gene = dynamic_cast<DPluginGeneric*>(p);

            if (gene)
            {
                qCDebug(DIGIKAM_TESTS_LOG) << "--------------------------------------------";
                qCDebug(DIGIKAM_TESTS_LOG) << "IID    :" << p->iid();
                qCDebug(DIGIKAM_TESTS_LOG) << "Name   :" << p->name();
                qCDebug(DIGIKAM_TESTS_LOG) << "Version:" << p->version();
                qCDebug(DIGIKAM_TESTS_LOG) << "Desc   :" << p->description();

                QString authors;
                const auto auths = p->authors();

                for (const DPluginAuthor& au : auths)
                {
                    authors.append(au.toString());
                    authors.append(QLatin1String(" ; "));
                }

                qCDebug(DIGIKAM_TESTS_LOG) << "Authors:" << authors;

                QString actions;
                const auto acs = gene->actions(&iface);

                for (const DPluginAction* const ac : acs)
                {
                    actions.append(ac->toString());
                    actions.append(QLatin1String(" ; "));
                }

                qCDebug(DIGIKAM_TESTS_LOG) << "Actions:" << actions;
            }
        }

        return 0;
    }
    else if (parser.isSet(QString::fromLatin1("l")))
    {
        const QString name = parser.value(QString::fromLatin1("l"));
        QString action;

        if (parser.isSet(QString::fromLatin1("a")))
        {
            action = parser.value(QString::fromLatin1("a"));
        }
        else
        {
            qCDebug(DIGIKAM_TESTS_LOG) << "Plugin action name to run is missing...";
            qCDebug(DIGIKAM_TESTS_LOG) << "Use --help option for details.";
            return -1;
        }

        MetaEngine::initializeExiv2();
        const auto dpls = dpl->allPlugins();

        for (DPlugin* const p : dpls)
        {
            if (p->iid() == name)
            {
                DPluginGeneric* const gene = dynamic_cast<DPluginGeneric*>(p);

                if (gene)
                {
                    found                   = true;
                    DPluginAction* const ac = gene->findActionByName(action, &iface);

                    if (ac)
                    {
                        ac->trigger();

                        if (parser.isSet(QString::fromLatin1("w")))
                        {
                            app.exec();
                        }
                    }
                    else
                    {
                        qCDebug(DIGIKAM_TESTS_LOG) << action << "action not found in plugin!";

                        QString actions;
                        const auto gacs = gene->actions(&iface);

                        for (const DPluginAction* const gac : gacs)
                        {
                            actions.append(gac->toString());
                            actions.append(QLatin1String(" ; "));
                        }

                        qCDebug(DIGIKAM_TESTS_LOG) << "Available Actions:" << actions;
                    }

                    break;
                }
            }
        }
    }
    else
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "Command line option not recognized...";
        qCDebug(DIGIKAM_TESTS_LOG) << "Use --help option for details.";
        return -1;
    }

    if (!found)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "Plugin not found!";
        return -1;
    }

    return 0;
}
