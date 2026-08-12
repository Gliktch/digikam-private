/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-12-31
 * Description : Geolocation plugin about dialog
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "geopluginaboutdlg.h"

// Qt includes

#include <QMap>
#include <QStringList>
#include <QString>
#include <QLabel>
#include <QTextBrowser>
#include <QGridLayout>
#include <QTabWidget>
#include <QApplication>
#include <QStyle>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTreeWidget>
#include <QListWidget>
#include <QHeaderView>
#include <QFileInfo>
#include <QLocale>
#include <QIcon>
#include <QDateTime>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "itempropertiestab.h"

namespace Digikam
{

GeoPluginAboutDlg::GeoPluginAboutDlg(PluginInterface* const tool, QWidget* const parent)
    : QDialog(parent),
      m_tool (tool)
{
    setWindowFlags((windowFlags() & ~Qt::Dialog) |
                   Qt::Window                    |
                   Qt::WindowCloseButtonHint     |
                   Qt::WindowMinMaxButtonsHint);

    setModal(true);
    setWindowTitle(i18nc("@title:window", "About %1 Plugin", tool->name()));

    QDialogButtonBox* const buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Help, this);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    QWidget* const page             = new QWidget(this);
    QGridLayout* const grid         = new QGridLayout(page);

    // --------------------------------------------------------

    QLabel* const logo              = new QLabel(page);
    logo->setPixmap(!tool->icon().isNull() ? m_tool->icon().pixmap(QSize(48, 48))
                                           : QIcon::fromTheme(QLatin1String("plugins")).pixmap(QSize(48, 48)));

    // --------------------------------------------------------

    QLabel* const header            = new QLabel(page);
    header->setWordWrap(true);
    header->setText(QString::fromUtf8("<font size=\"5\">%1</font><br>"
                                      "<b>%2 %3</b>"
                                      "<p>%4</p>")
                    .arg(m_tool->name())
                    .arg(i18nc("@label", "Version"))
                    .arg(m_tool->version())
                    .arg(m_tool->description()));

    QTabWidget* const tab           = new QTabWidget(page);

    // --------------------------------------------------------

    QTextBrowser* const authors = new QTextBrowser(tab);
    authors->setOpenExternalLinks(false);
    authors->setOpenLinks(false);
    authors->setFocusPolicy(Qt::NoFocus);

    QString alist    = i18n("<p>Copyright %1</p>", m_tool->copyrightYears());
    const auto auths = m_tool->pluginAuthors();

    for (const PluginAuthor& auth : auths)
    {
        alist += QString::fromUtf8("<b>%1</b><ul>"
                                   "<li><i>%2</i></li>"
                                   "<li>%3</li></ul><br>")
                 .arg(auth.name)
                 .arg(auth.email)
                 .arg(auth.task);
    }

    authors->setText(alist);

    tab->addTab(authors, i18nc("@title", "Authors"));

    // --------------------------------------------------------

    grid->addWidget(logo,   0, 0, 1,  1);
    grid->addWidget(header, 0, 1, 1,  1);
    grid->addWidget(tab,    2, 0, 1, -1);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(2, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(layoutSpacing());


    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(page);
    vbx->addWidget(buttons);
    setLayout(vbx);

    // --------------------------------------------------------

    connect(buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotOnlineHandbook()));

    resize(400, 500);
}

void GeoPluginAboutDlg::slotOnlineHandbook()
{
    openOnlineDocumentation(QLatin1String("setup_application"),
                            QLatin1String("geolocation_settings"));
}

} // namespace Digikam

#include "moc_geopluginaboutdlg.cpp"
