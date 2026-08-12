/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-12-31
 * Description : digiKam plugin about dialog
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dpluginaboutdlg.h"

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
#include <QDateTime>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "itempropertiestab.h"

namespace Digikam
{

DPluginAboutDlg::DPluginAboutDlg(DPlugin* const tool, QWidget* const parent)
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
    logo->setPixmap(tool->icon().pixmap(QSize(48, 48)));

    // --------------------------------------------------------

    QLabel* const header            = new QLabel(page);
    header->setWordWrap(true);
    header->setText(QString::fromUtf8("<font size=\"5\">%1</font><br>"
                                      "<b>%2 %3</b>"
                                      "<p>%4</p>")
                    .arg(tool->name())
                    .arg(i18nc("@label", "Version"))
                    .arg(tool->version())
                    .arg(tool->description()));

    QTabWidget* const tab           = new QTabWidget(page);

    // --------------------------------------------------------

    QTextBrowser* const details     = new QTextBrowser(tab);
    details->setOpenExternalLinks(true);
    details->setOpenLinks(false);
    details->setFocusPolicy(Qt::NoFocus);
    details->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

    QString txtDetails = tool->details();
    txtDetails.replace(QLatin1String("\n"), QLatin1String("<br>"));
    details->setText(txtDetails);

    tab->addTab(details, i18nc("@title", "Details"));

    // --------------------------------------------------------

    QTextBrowser* const authors = new QTextBrowser(tab);
    authors->setOpenExternalLinks(false);
    authors->setOpenLinks(false);
    authors->setFocusPolicy(Qt::NoFocus);

    QString alist;
    const auto ats = tool->authors();

    for (const DPluginAuthor& auth : ats)
    {
        alist += QString::fromUtf8("<b>%1</b><ul>"
                                   "<li><i>%2</i></li>"
                                   "<li>%3</li>"
                                   "<li>%4</li></ul><br>")
                 .arg(auth.name)
                 .arg(auth.email)
                 .arg(auth.years)
                 .arg(auth.roles);
    }

    authors->setText(alist);

    tab->addTab(authors, i18nc("@title", "Authors"));

    // --------------------------------------------------------

    QTreeWidget* const props        = new QTreeWidget(tab);
    props->setSortingEnabled(false);
    props->setRootIsDecorated(false);
    props->setSelectionMode(QAbstractItemView::SingleSelection);
    props->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    props->setAllColumnsShowFocus(true);
    props->setColumnCount(2);
    props->setHeaderLabels(QStringList() << i18nc("@title: DPlugin properties", "Name")
                                         << i18nc("@title: DPlugin properties", "Value"));

    new QTreeWidgetItem(props, QStringList() << i18nc("@item", "Interface ID") << tool->ifaceIid());
    new QTreeWidgetItem(props, QStringList() << i18nc("@item", "Tool ID")      << tool->iid());
    new QTreeWidgetItem(props, QStringList() << i18nc("@item", "Library")      << tool->libraryFileName());

    QFileInfo fi(tool->libraryFileName());

    new QTreeWidgetItem(props, QStringList() << i18nc("@item", "File Size")    << ItemPropertiesTab::humanReadableBytesCount(fi.size()));
    new QTreeWidgetItem(props, QStringList() << i18nc("@item", "File Date")    << asUserDateTime(fi.lastModified()));
    new QTreeWidgetItem(props, QStringList() << i18nc("@item", "File Owner")   << fi.owner());
    new QTreeWidgetItem(props, QStringList() << i18nc("@item", "File Group")   << fi.group());

    tab->addTab(props, i18nc("@title", "Properties"));

    // --------------------------------------------------------

    QMap<QString, QStringList> list = tool->extraAboutData();

    if (!list.isEmpty())
    {
        QTreeWidget* const extra = new QTreeWidget(tab);
        extra->setSortingEnabled(false);
        extra->setRootIsDecorated(false);
        extra->setSelectionMode(QAbstractItemView::SingleSelection);
        extra->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        extra->setAllColumnsShowFocus(true);
        QStringList titles = tool->extraAboutDataRowTitles();
        extra->setColumnCount(titles.count());
        extra->setHeaderLabels(titles);

        for (QMap<QString, QStringList>::const_iterator it = list.constBegin() ; it != list.constEnd() ; ++it)
        {
            new QTreeWidgetItem(extra, QStringList() << it.key() << it.value());
        }

        tab->addTab(extra, tool->extraAboutDataTitle());
    }

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

void DPluginAboutDlg::slotOnlineHandbook()
{
    openOnlineDocumentation(m_tool->handbookSection(),
                            m_tool->handbookChapter(),
                            m_tool->handbookReference());
}

} // namespace Digikam

#include "moc_dpluginaboutdlg.cpp"
