/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a dialog to play video.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "vidplayerdlg.h"

// Qt includes

#include <QGridLayout>
#include <QApplication>
#include <QStyle>

// Local includes

#include "digikam_globals.h"
#include "mediaplayerview.h"

namespace Digikam
{

class Q_DECL_HIDDEN VidPlayerDlg::Private
{
public:

    Private() = default;

public:

    MediaPlayerView* player = nullptr;
};

VidPlayerDlg::VidPlayerDlg(const QString& file, QWidget* const parent)
    : QDialog(parent),
      d      (new Private)
{
    setModal(false);
    setWindowTitle(file);

    d->player               = new MediaPlayerView(this);
    d->player->setCurrentItem(QUrl::fromLocalFile(file));

    // ----------------------

    QGridLayout* const grid = new QGridLayout(this);
    grid->setSpacing(layoutSpacing());

    grid->addWidget(d->player, 0, 0, 1, 1);
    grid->setColumnStretch(0, 10);
    setLayout(grid);

    // ----------------------

    connect(d->player, SIGNAL(signalEscapePreview()),
            this, SLOT(accept()));
}

VidPlayerDlg::~VidPlayerDlg()
{
    delete d;
}

} // namespace Digikam

#include "moc_vidplayerdlg.cpp"
