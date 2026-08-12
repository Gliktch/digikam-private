/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-08-27
 * Description : Showfoto folder view bookmark edit dialog
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotofolderviewbookmarkdlg.h"

// Qt includes

#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPointer>
#include <QApplication>
#include <QStyle>
#include <QDir>
#include <QStandardPaths>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

#ifdef HAVE_KICONTHEMES
#   include <kicondialog.h>
#endif

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "dfileselector.h"
#include "dlayoutbox.h"
#include "dtextedit.h"
#include "dxmlguiwindow.h"
#include "showfotofolderviewbookmarklist.h"

using namespace Digikam;

namespace ShowFoto
{

class Q_DECL_HIDDEN ShowfotoFolderViewBookmarkDlg::Private
{
public:

    Private() = default;

public:

    bool                            create          = false;

    QLabel*                         topLabel        = nullptr;

    QString                         icon            = QLatin1String("folder");

    QPushButton*                    iconButton      = nullptr;
    QPushButton*                    resetIconButton = nullptr;

    QDialogButtonBox*               buttons         = nullptr;

    DTextEdit*                      titleEdit       = nullptr;
    DFileSelector*                  pathEdit        = nullptr;
    ShowfotoFolderViewBookmarkList* list            = nullptr;
};

ShowfotoFolderViewBookmarkDlg::ShowfotoFolderViewBookmarkDlg(ShowfotoFolderViewBookmarkList* const parent,
                                                             bool create)
    : QDialog(parent),
      d      (new Private)
{
    setModal(true);

    d->create  = create;
    d->list    = parent;
    d->buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    if (d->create)
    {
        setWindowTitle(i18nc("@title:window", "New Bookmark"));
    }
    else
    {
        setWindowTitle(i18nc("@title:window", "Edit Bookmark"));
    }

    QWidget* const page      = new QWidget(this);

    // --------------------------------------------------------

    QGridLayout* const grid  = new QGridLayout(page);

    d->topLabel              = new QLabel(page);
    d->topLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->topLabel->setWordWrap(false);

    // --------------------------------------------------------

    QLabel* const titleLabel = new QLabel(page);
    titleLabel->setText(i18nc("@label: bookmark properties", "&Title:"));

    d->titleEdit             = new DTextEdit(page);
    d->titleEdit->setLinesVisible(1);
    d->titleEdit->setPlaceholderText(i18nc("@info", "Enter bookmark title here..."));
    d->titleEdit->setToolTip(i18nc("@info", "The bookmark title which must be unique and not empty"));
    titleLabel->setBuddy(d->titleEdit);

    // --------------------------------------------------------

    QLabel* const iconTextLabel = new QLabel(page);
    iconTextLabel->setText(i18nc("@label", "&Icon:"));

    d->iconButton               = new QPushButton(page);
    d->iconButton->setFixedSize(40, 40);
    d->iconButton->setIcon(QIcon::fromTheme(d->icon));
    iconTextLabel->setBuddy(d->iconButton);

    d->resetIconButton          = new QPushButton(QIcon::fromTheme(QLatin1String("view-refresh")),
                                                  i18nc("@action:button", "Reset"), page);

#ifndef HAVE_KICONTHEMES

    iconTextLabel->hide();
    d->iconButton->hide();
    d->resetIconButton->hide();

#endif

    // --------------------------------------------------------

    QLabel* const pathLabel = new QLabel(page);
    pathLabel->setText(i18nc("@label: bookmark properties", "&Path:"));

    d->pathEdit             = new DFileSelector(page);
    d->pathEdit->setFileDlgMode(QFileDialog::Directory);
    d->pathEdit->setFileDlgOptions(QFileDialog::ShowDirsOnly);
    d->pathEdit->lineEdit()->setPlaceholderText(i18nc("@info", "Enter bookmark path here..."));
    pathLabel->setBuddy(d->pathEdit);

    // --------------------------------------------------------

    grid->addWidget(titleLabel,         0, 0, 1, 1);
    grid->addWidget(d->titleEdit,       0, 1, 1, 3);
    grid->addWidget(iconTextLabel,      1, 0, 1, 1);
    grid->addWidget(d->iconButton,      1, 1, 1, 1);
    grid->addWidget(d->resetIconButton, 1, 2, 1, 1);
    grid->addWidget(pathLabel,          2, 0, 1, 1);
    grid->addWidget(d->pathEdit,        2, 1, 1, 3);
    grid->setRowStretch(3, 10);
    grid->setColumnStretch(3, 10);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    // --------------------------------------------------------

    connect(d->titleEdit, SIGNAL(textChanged()),
            this, SLOT(slotModified()));

    connect(d->pathEdit->lineEdit(), SIGNAL(textChanged(QString)),
            this, SLOT(slotModified()));

    connect(d->pathEdit, SIGNAL(signalUrlSelected(QUrl)),
            this, SLOT(slotModified()));

    connect(d->iconButton, SIGNAL(clicked()),
            this, SLOT(slotIconChanged()));

    connect(d->resetIconButton, SIGNAL(clicked()),
            this, SLOT(slotIconResetClicked()));

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(slotAccept()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));

    // --------------------------------------------------------

    d->titleEdit->setFocus();
    adjustSize();
}

ShowfotoFolderViewBookmarkDlg::~ShowfotoFolderViewBookmarkDlg()
{
    delete d;
}

bool ShowfotoFolderViewBookmarkDlg::canAccept() const
{
    bool b = true;

    if (d->create)
    {
        b = !d->list->bookmarkExists(title());
    }

    return (
            !title().isEmpty() &&
            !path().isEmpty()  &&
            b
           );
}

void ShowfotoFolderViewBookmarkDlg::slotAccept()
{
    if (canAccept())
    {
        accept();
    }
}

void ShowfotoFolderViewBookmarkDlg::slotModified()
{
    d->buttons->button(QDialogButtonBox::Ok)->setEnabled(canAccept());
}

QString ShowfotoFolderViewBookmarkDlg::title() const
{
    return d->titleEdit->text();
}

QString ShowfotoFolderViewBookmarkDlg::icon() const
{
    return d->icon;
}

QString ShowfotoFolderViewBookmarkDlg::path() const
{
    return d->pathEdit->fileDlgPath();
}

void ShowfotoFolderViewBookmarkDlg::setTitle(const QString& title)
{
    d->titleEdit->setText(title);
}

void ShowfotoFolderViewBookmarkDlg::setIcon(const QString& icon)
{
    d->icon = icon;
}

void ShowfotoFolderViewBookmarkDlg::setPath(const QString& path)
{
    d->pathEdit->setFileDlgPath(path);
}

void ShowfotoFolderViewBookmarkDlg::slotIconResetClicked()
{
    d->icon = QLatin1String("folder");
    d->iconButton->setIcon(QIcon::fromTheme(d->icon));
}

void ShowfotoFolderViewBookmarkDlg::slotIconChanged()
{

#ifdef HAVE_KICONTHEMES

    QPointer<KIconDialog> dlg = new KIconDialog(this);
    dlg->setup(KIconLoader::NoGroup, KIconLoader::Application, false, 20, false, false, false);
    QString ico               = dlg->openDialog();
    delete dlg;

    if (ico.isEmpty() || (ico == d->icon))
    {
        return;
    }

    d->icon                   = ico;
    d->iconButton->setIcon(QIcon::fromTheme(d->icon));

#endif

}

bool ShowfotoFolderViewBookmarkDlg::bookmarkDialog(ShowfotoFolderViewBookmarkList* const parent,
                                                   QString& title,
                                                   QString& icon,
                                                   QString& path,
                                                   bool create)
{
    QPointer<ShowfotoFolderViewBookmarkDlg> dlg = new ShowfotoFolderViewBookmarkDlg(parent, create);
    dlg->setTitle(title);
    dlg->setIcon(icon);
    dlg->setPath(path);

    bool valRet = dlg->exec();

    if (valRet == QDialog::Accepted)
    {
        title = dlg->title();
        icon  = dlg->icon();
        path  = dlg->path();

        if (!path.endsWith(QDir::separator()))
        {
            path.append(QDir::separator());
        }
    }

    delete dlg;

    return valRet;
}

void ShowfotoFolderViewBookmarkDlg::slotHelp()
{
    Digikam::openOnlineDocumentation(QLatin1String("showfoto_editor"),
                                     QLatin1String("showfoto_leftsidebar"),
                                     QLatin1String("showfoto-folderstab"));
}

} // namespace ShowFoto

#include "moc_showfotofolderviewbookmarkdlg.cpp"
