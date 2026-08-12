/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-07-11
 * Description : general info list dialog
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "infodlg.h"

// Qt includes

#include <QStringList>
#include <QString>
#include <QLabel>
#include <QGridLayout>
#include <QHeaderView>
#include <QMimeData>
#include <QClipboard>
#include <QApplication>
#include <QStyle>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "daboutdata.h"

namespace Digikam
{

class Q_DECL_HIDDEN InfoDlg::Private
{
public:

    Private() = default;

public:

    QTreeWidget*      listView  = nullptr;
    QWidget*          page      = nullptr;
    QDialogButtonBox* buttons   = nullptr;
    QTabWidget*       tabView   = nullptr;
};

InfoDlg::InfoDlg(QWidget* const parent)
    : QDialog(parent),
      d      (new Private)
{
    setModal(false);
    setWindowTitle(i18nc("@title:window", "Shared Libraries and Components Information"));

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Help  |
                                      QDialogButtonBox::Apply |
                                      QDialogButtonBox::Ok, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    d->buttons->button(QDialogButtonBox::Apply)->setText(i18n("Copy to Clipboard"));

    d->page                 = new QWidget(this);
    QGridLayout* const grid = new QGridLayout(d->page);

    // --------------------------------------------------------

    QLabel* const logo      = new QLabel(d->page);

    if (QApplication::applicationName() == QLatin1String("digikam"))
    {
        logo->setPixmap(QIcon::fromTheme(QLatin1String("digikam")).pixmap(QSize(48,48)));
    }
    else
    {
        logo->setPixmap(QIcon::fromTheme(QLatin1String("showfoto")).pixmap(QSize(48,48)));
    }

    // --------------------------------------------------------

    QLabel* const header    = new QLabel(d->page);
    header->setWordWrap(true);
    header->setText(i18n("<font size=\"5\">%1</font><br><b>Version %2</b>"
                         "<p>%3</p><p><i>%4</i></p>",
                         QApplication::applicationName(),
                         QApplication::applicationVersion(),
                         DAboutData::digiKamSlogan(),
                         DAboutData::digiKamFamily()));

    // --------------------------------------------------------

    d->tabView              = new QTabWidget(d->page);

    d->listView             = new QTreeWidget(d->page);
    d->listView->setSortingEnabled(false);
    d->listView->setRootIsDecorated(false);
    d->listView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->listView->setAllColumnsShowFocus(true);
    d->listView->setColumnCount(2);
    d->listView->header()->setSectionResizeMode(QHeaderView::Stretch);
    d->listView->header()->setVisible(false);

    d->tabView->addTab(d->listView, i18n("Items List"));
    d->tabView->setTabBarAutoHide(true);

    // --------------------------------------------------------

    grid->addWidget(logo,        0, 0, 1, 1);
    grid->addWidget(header,      0, 1, 1, 1);

    // row 1 can be expanded by custom widgets in the subclassed dialog

    grid->addWidget(d->tabView,  2, 0, 1, -1);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(2, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(layoutSpacing());

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(d->page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    // --------------------------------------------------------

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));

    // --- NOTE: use dynamic binding as slotCopy2ClipBoard() is a virtual slot which can be re-implemented in derived classes.

    connect(d->buttons->button(QDialogButtonBox::Apply), &QPushButton::clicked,
            this, &InfoDlg::slotCopy2ClipBoard);

    resize(400, 500);
}

InfoDlg::~InfoDlg()
{
    delete d;
}

QTabWidget* InfoDlg::tabView() const
{
    return d->tabView;
}

QTreeWidget* InfoDlg::listView() const
{
    return d->listView;
}

QDialogButtonBox* InfoDlg::buttonBox() const
{
    return d->buttons;
}

QWidget* InfoDlg::mainWidget() const
{
    return d->page;
}

void InfoDlg::setInfoMap(const QMap<QString, QString>& list)
{
    for (QMap<QString, QString>::const_iterator it = list.constBegin() ;
         it != list.constEnd() ; ++it)
    {
        new QTreeWidgetItem(d->listView, QStringList() << it.key() << it.value());
    }
}

void InfoDlg::slotCopy2ClipBoard()
{
    QString textInfo;

    textInfo.append(QApplication::applicationName());
    textInfo.append(QLatin1String(" version "));
    textInfo.append(QApplication::applicationVersion());
    textInfo.append(QLatin1Char('\n'));

    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        textInfo.append((*it)->text(0));
        textInfo.append(QLatin1String(": "));
        textInfo.append((*it)->text(1));
        textInfo.append(QLatin1Char('\n'));
        ++it;
    }

    QMimeData* const mimeData = new QMimeData();
    mimeData->setText(textInfo);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

void InfoDlg::slotHelp()
{
    openOnlineDocumentation();
}

} // namespace Digikam

#include "moc_infodlg.cpp"
