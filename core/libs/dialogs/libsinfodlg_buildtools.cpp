/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-07-11
 * Description : shared libraries list dialog common to digiKam and Showfoto
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "libsinfodlg_p.h"

// Local includes

#include "digikam_gitversion.h"
#include "digikam_builddate.h"

namespace Digikam
{

void LibsInfoDlg::populateBuildTools()
{
    m_buildtools       = new QTreeWidgetItem(listView(), QStringList() << i18nc("@title", "Build Environment"));
    listView()->addTopLevelItem(m_buildtools);

    new QTreeWidgetItem(m_buildtools, QStringList() <<
                        i18nc(Private::CONTEXT, "Host")                      << QLatin1String(CMAKE_HOST_PRETTY_NAME));

    new QTreeWidgetItem(m_buildtools, QStringList() <<
                        i18nc(Private::CONTEXT, "Cmake")                     << QLatin1String(CMAKE_VERSION_STRING));

    new QTreeWidgetItem(m_buildtools, QStringList() <<
                        i18nc(Private::CONTEXT, "Compiler")                  << QString::fromLatin1("%1 - %2")
                                                                                .arg(QLatin1String(CMAKE_CXX_COMPILER_ID))
                                                                                .arg(QLatin1String(CMAKE_CXX_COMPILER_VERSION)));
#ifdef CCACHE_VERSION

    new QTreeWidgetItem(m_buildtools, QStringList() <<
                        i18nc(Private::CONTEXT, "Ccache")                    << QLatin1String(CCACHE_VERSION));

#else

    new QTreeWidgetItem(m_buildtools, QStringList() <<
                        i18nc(Private::CONTEXT, "Ccache support")            << d->supportedNo());

#endif

    new QTreeWidgetItem(m_buildtools, QStringList() <<
                        i18nc(Private::CONTEXT, "Build date")                << QLocale().toString(digiKamBuildDate(), QLocale::ShortFormat));

    new QTreeWidgetItem(m_buildtools, QStringList() <<
                        i18nc(Private::CONTEXT, "Build target")              << QLatin1String(digikam_build_type));

    new QTreeWidgetItem(m_buildtools, QStringList() <<
                        i18nc(Private::CONTEXT, "Build host architecture")   << QLatin1String(CMAKE_HOST_SYSTEM_PROCESSOR));

    new QTreeWidgetItem(m_buildtools, QStringList() <<
                        i18nc(Private::CONTEXT, "Build target architecture") << QLatin1String(CMAKE_SYSTEM_PROCESSOR));

    QString gitRev     = QLatin1String(GITVERSION);
    QString gitBra     = QLatin1String(GITBRANCH);

    if (
        !gitRev.isEmpty()                           &&
        !gitBra.isEmpty()                           &&
        !gitRev.startsWith(QLatin1String("unknown")) &&
        !gitRev.startsWith(QLatin1String("export")) &&
        !gitBra.startsWith(QLatin1String("unknown"))
       )
    {
        const int maxStringLength         = 10;
        QString gitVer                    = gitRev.left(maxStringLength / 2 - 2) +
                                            QLatin1String("...")                 +
                                            gitRev.right(maxStringLength / 2 - 1);

        QLabel* const gitRevLbl           = new QLabel(QString::fromLatin1("<a href='https://invent.kde.org/graphics/digikam/commit/%1'>%2</a>")
                                                       .arg(gitRev).arg(gitVer),
                                                       listView());
        gitRevLbl->setOpenExternalLinks(true);

        QTreeWidgetItem* const gitRevItem = new QTreeWidgetItem(m_buildtools);
        gitRevItem->setText(0, i18nc(Private::CONTEXT, "Git revision"));
        listView()->setItemWidget(gitRevItem, 1, gitRevLbl);

        new QTreeWidgetItem(m_buildtools, QStringList()    <<
                            i18nc(Private::CONTEXT, "Git branch")   << gitBra);
    }
    else
    {
        new QTreeWidgetItem(m_buildtools, QStringList()    <<
                            i18nc(Private::CONTEXT, "Git revision") << i18nc("@info: git revision", "Unknown"));

        new QTreeWidgetItem(m_buildtools, QStringList()    <<
                            i18nc(Private::CONTEXT, "Git branch")   << i18nc("@info: git branch", "Unknown"));
    }
}

} // namespace Digikam
