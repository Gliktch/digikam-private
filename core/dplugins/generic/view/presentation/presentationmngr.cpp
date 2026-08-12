/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-01-31
 * Description : a presentation tool.
 *
 * SPDX-FileCopyrightText: 2006-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at googlemail dot com>
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "presentationmngr.h"

// C++ includes

#include <cstdlib>

// Qt includes

#include <QTime>
#include <QPair>
#include <QStringList>
#include <QAction>
#include <QWindow>
#include <QApplication>
#include <QMessageBox>
#include <QRandomGenerator>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "presentationwidget.h"
#include "presentationcontainer.h"

#ifdef HAVE_OPENGL
#   include "presentationgl.h"
#   include "presentationkb.h"
#endif

namespace DigikamGenericPresentationPlugin
{

PresentationMngr::PresentationMngr(QObject* const parent, DInfoInterface* const iface)
    : QObject(parent)
{
      m_sharedData        = new PresentationContainer();
      m_sharedData->iface = iface;
}

PresentationMngr::~PresentationMngr()
{
    delete m_dialog;
    delete m_sharedData;
}

void PresentationMngr::setPlugin(DPlugin* const plugin)
{
    m_plugin = plugin;
}

void PresentationMngr::addFiles(const QList<QUrl>& urls)
{
    m_sharedData->urlList = urls;
}

void PresentationMngr::showConfigDialog()
{
    m_dialog = new PresentationDlg(QApplication::activeWindow(), m_sharedData);

    connect(m_dialog, SIGNAL(buttonStartClicked()),
            this, SLOT(slotSlideShow()));

    m_dialog->setPlugin(m_plugin);
    m_dialog->show();
}

void PresentationMngr::slotSlideShow()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp        = config->group(QLatin1String("Presentation Settings"));
    bool opengl             = grp.readEntry("OpenGL",  false);
    bool shuffle            = grp.readEntry("Shuffle", false);
    bool wantKB             = grp.readEntry("Effect Name (OpenGL)") == QLatin1String("Ken Burns");

    if (m_sharedData->urlList.isEmpty())
    {
        QMessageBox::information(QApplication::activeWindow(), QString(), i18n("There are no images to show."));
        return;
    }

    if (shuffle)
    {
        QList<QUrl>::iterator it = m_sharedData->urlList.begin();
        QList<QUrl>::iterator it1;

        for (uint i = 0 ; i < (uint) m_sharedData->urlList.size() ; ++i)
        {
            int inc = QRandomGenerator::global()->bounded(m_sharedData->urlList.count());

            it1  = m_sharedData->urlList.begin();
            it1 += inc;

            std::swap(*(it++), *(it1));
        }
    }

    if (!opengl)
    {
        PresentationWidget* const slide = new PresentationWidget(m_sharedData);
        slide->show();
    }
    else
    {

#ifdef HAVE_OPENGL

        bool supportsOpenGL = true;

        if (wantKB)
        {
            PresentationKB* const slide = new PresentationKB(m_sharedData);
            slide->show();

            if (!slide->checkOpenGL())
            {
                supportsOpenGL = false;
                slide->close();
            }

        }
        else
        {
            PresentationGL* const slide = new PresentationGL(m_sharedData);
            slide->show();

            if (!slide->checkOpenGL())
            {
                supportsOpenGL = false;
                slide->close();
            }
        }

        if (!supportsOpenGL)
        {
            QMessageBox::critical(QApplication::activeWindow(), QString(),
                                  i18n("OpenGL support is not available on your system."));
        }

#else

        Q_UNUSED(wantKB);

#endif

    }
}

} // namespace DigikamGenericPresentationPlugin

#include "moc_presentationmngr.cpp"
