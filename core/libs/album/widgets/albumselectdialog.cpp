/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-16
 * Description : a dialog to select a target album to download
 *               pictures from camera
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albumselectdialog.h"

// Qt includes

#include <QLabel>
#include <QGridLayout>
#include <QPixmap>
#include <QPointer>
#include <QStandardPaths>
#include <QApplication>
#include <QStyle>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_globals.h"
#include "album.h"
#include "albummanager.h"
#include "albumselectwidget.h"
#include "albumthumbnailloader.h"
#include "collectionmanager.h"
#include "dxmlguiwindow.h"

namespace Digikam
{

class Q_DECL_HIDDEN AlbumSelectDialog::Private
{
public:

    Private() = default;

public:

    QDialogButtonBox*  buttons      = nullptr;

    AlbumSelectWidget* albumSel     = nullptr;

    SearchTextBar*     searchBar    = nullptr;
};

AlbumSelectDialog::AlbumSelectDialog(QWidget* const parent, PAlbum* const albumToSelect, const QString& header)
    : QDialog(parent),
      d      (new Private)
{
    setWindowFlags((windowFlags() & ~Qt::Dialog) |
                   Qt::Window                    |
                   Qt::WindowCloseButtonHint     |
                   Qt::WindowMinMaxButtonsHint);

    setObjectName(QLatin1String("Select Album Dialog"));
    setWindowTitle(i18nc("@title:window", "Select Album"));

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Help |
                                      QDialogButtonBox::Ok   |
                                      QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    // -------------------------------------------------------------

    QWidget* const page     = new QWidget(this);
    QGridLayout* const grid = new QGridLayout(page);
    QLabel* const logo      = new QLabel(page);
    logo->setPixmap(QIcon::fromTheme(QLatin1String("digikam")).pixmap(QSize(48,48)));

    QLabel* const message   = new QLabel(page);
    message->setWordWrap(true);

    if (!header.isEmpty())
    {
        message->setText(header);
    }

    d->albumSel = new AlbumSelectWidget(page, albumToSelect, true);

    grid->addWidget(logo,        0, 0, 1, 1);
    grid->addWidget(message,     1, 0, 1, 1);
    grid->addWidget(d->albumSel, 0, 1, 3, 1);
    grid->setColumnStretch(0, 2);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(2, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(layoutSpacing());

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    // -------------------------------------------------------------

    connect(d->albumSel, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(d->albumSel, SIGNAL(completerActivated()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));

    // -------------------------------------------------------------

    slotSelectionChanged();
}

AlbumSelectDialog::~AlbumSelectDialog()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(objectName());

    DXmlGuiWindow::saveWindowSize(windowHandle(), group);

    delete d;
}

void AlbumSelectDialog::slotSelectionChanged()
{
    const PAlbum* const currentAlbum = d->albumSel->currentAlbum();

    bool enabled = !(!currentAlbum || currentAlbum->isRoot());
    d->buttons->button(QDialogButtonBox::Ok)->setEnabled(enabled);
}

PAlbum* AlbumSelectDialog::selectAlbum(QWidget* const parent, PAlbum* const albumToSelect, const QString& header)
{
    QPointer<AlbumSelectDialog> dlg = new AlbumSelectDialog(parent, albumToSelect, header);

    if (dialogExec(dlg) != QDialog::Accepted)
    {
        delete dlg;
        return nullptr;
    }

    PAlbum* const selectedAlbum = dlg->d->albumSel->currentAlbum();

    if (!selectedAlbum || (selectedAlbum->isRoot()))
    {
        delete dlg;
        return nullptr;
    }

    delete dlg;

    return selectedAlbum;
}

void AlbumSelectDialog::slotHelp()
{
    openOnlineDocumentation(QLatin1String("left_sidebar"),
                            QLatin1String("albums_view"));
}

void AlbumSelectDialog::showEvent(QShowEvent* e)
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(objectName());

    DXmlGuiWindow::setGoodDefaultWindowSize(windowHandle());
    DXmlGuiWindow::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size());

    QDialog::showEvent(e);
}

} // namespace Digikam

#include "moc_albumselectdialog.cpp"
