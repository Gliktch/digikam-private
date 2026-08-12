/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-01-29
 * Description : Database cleaner.
 *
 * SPDX-FileCopyrightText: 2017-2018 by Mario Frank <mario dot frank at uni minus potsdam dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dbshrinkdialog.h"

// Qt includes

#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QThread>
#include <QVBoxLayout>
#include <QTimer>
#include <QListWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikamapp.h"
#include "maintenancethread.h"

namespace Digikam
{

class Q_DECL_HIDDEN DbShrinkDialog::Private
{
public:

    Private() = default;

public:

    int             active          = -1;
    DWorkingPixmap* progressPix     = nullptr;
    QTimer*         progressTimer   = nullptr;
    int             progressIndex   = 1;
    QListWidget*    statusList      = nullptr;
};

DbShrinkDialog::DbShrinkDialog(QWidget* const parent)
    : QDialog(parent),
      d      (new Private)
{
    setAttribute(Qt::WA_DeleteOnClose, false);

    d->progressPix                  = new DWorkingPixmap(this);
    d->progressTimer                = new QTimer(parent);
    d->statusList                   = new QListWidget(this);
    QVBoxLayout* const statusLayout = new QVBoxLayout(this);

    QLabel* const infos             = new QLabel(i18n("<p>Database shrinking in progress.</p>"
                                          "<p>Currently, your databases are being shrunk. "
                                          "This will take some time - depending on "
                                          "your databases size.</p>"
                                          "<p>We have to freeze digiKam in order to "
                                          "prevent database corruption. This info box "
                                          "will vanish when the shrinking process is "
                                          "finished.</p>"
                                          "Current Status:"),
                                          this);
    infos->setWordWrap(true);
    statusLayout->addWidget(infos);

    d->statusList->addItem(i18n("Core DB"));
    d->statusList->addItem(i18n("Thumbnails DB"));
    d->statusList->addItem(i18n("Face Recognition DB"));
    d->statusList->addItem(i18n("Similarity DB"));

    for (int i = 0 ; i < 4 ; ++i)
    {
        d->statusList->item(i)->setIcon(QIcon::fromTheme(QLatin1String("system-run")));
    }
/*
    d->statusList->setMinimumSize(0, 0);
    d->statusList->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    d->statusList->adjustSize();
*/
    d->statusList->setMaximumHeight(4 * d->statusList->sizeHintForRow(0) +
                                    2 * d->statusList->frameWidth());
    statusLayout->addWidget(d->statusList);

    connect(d->progressTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTimerDone()));
}

DbShrinkDialog::~DbShrinkDialog()
{
    d->progressTimer->stop();
    delete d;
}

void DbShrinkDialog::setActive(const int pos)
{
    d->active = pos;

    if (d->progressTimer->isActive())
    {
        if (d->active < 0)
        {
            d->progressTimer->stop();
        }
    }
    else
    {
        if (d->active >= 0)
        {
            d->statusList->item(d->active)->setIcon(d->progressPix->frameAt(0));
            d->progressTimer->start(300);
            d->progressIndex = 1;
        }
    }
}

void DbShrinkDialog::setIcon(const int pos, const QIcon& icon)
{
    if (d->active == pos)
    {
        d->active = -1;
    }

    d->statusList->item(pos)->setIcon(icon);
}

int DbShrinkDialog::exec()
{
    d->active = 0;
    d->progressTimer->start(300);

    return QDialog::exec();
}

void DbShrinkDialog::slotProgressTimerDone()
{
    if (d->active < 0)
    {
        return;
    }

    if (d->progressIndex == d->progressPix->frameCount())
    {
        d->progressIndex = 0;
    }

    d->statusList->item(d->active)->setIcon(d->progressPix->frameAt(d->progressIndex));
    ++d->progressIndex;
}

} // namespace Digikam

#include "moc_dbshrinkdialog.cpp"
