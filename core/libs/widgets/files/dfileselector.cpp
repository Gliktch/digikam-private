/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * date        : 2014-09-12
 * Description : a file or folder selector widget
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dfileselector.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN DFileSelector::Private
{
public:

    Private() = default;

public:

    QLineEdit*            edit      = nullptr;
    QPushButton*          btn       = nullptr;

    QFileDialog::FileMode fdMode    = QFileDialog::ExistingFile;
    QString               fdFilter;
    QString               fdTitle;
    QFileDialog::Options  fdOptions = QFileDialog::Options();
};

DFileSelector::DFileSelector(QWidget* const parent)
    : DHBox(parent),
      d    (new Private)
{
    d->edit    = new QLineEdit(this);
    d->edit->setClearButtonEnabled(true);
    d->btn     = new QPushButton(i18n("Browse..."), this);
    setStretchFactor(d->edit, 10);
    setContentsMargins(0, 0, 0, 0);

    connect(d->btn, SIGNAL(clicked()),
            this, SLOT(slotBtnClicked()));
}

DFileSelector::~DFileSelector()
{
    delete d;
}

QLineEdit* DFileSelector::lineEdit() const
{
    return d->edit;
}

void DFileSelector::setFileDlgPath(const QString& path)
{
    d->edit->setText(QDir::toNativeSeparators(path));
}

QString DFileSelector::fileDlgPath() const
{
    return QDir::fromNativeSeparators(d->edit->text());
}

void DFileSelector::setFileDlgMode(QFileDialog::FileMode mode)
{
    d->fdMode = mode;
}

void DFileSelector::setFileDlgFilter(const QString& filter)
{
    d->fdFilter = filter;
}

void DFileSelector::setFileDlgTitle(const QString& title)
{
    d->fdTitle = title;
}

void DFileSelector::setFileDlgOptions(QFileDialog::Options opts)
{
    d->fdOptions = opts;
}

void DFileSelector::slotBtnClicked()
{
    if (d->fdMode == QFileDialog::ExistingFiles)
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << "Multiple selection is not supported";
        return;
    }

    // Never pass a parent to File Dialog, else duplicate dialogs will be shown

    QPointer<DFileDialog> fileDlg = new DFileDialog;

    fileDlg->setDirectory(QFileInfo(fileDlgPath()).filePath());

    // It is important to set up the mode first and then the options

    fileDlg->setFileMode(d->fdMode);
    fileDlg->setOptions(d->fdOptions);

    if (!d->fdFilter.isNull())
    {
        fileDlg->setNameFilter(d->fdFilter);
    }

    if (!d->fdTitle.isNull())
    {
        fileDlg->setWindowTitle(d->fdTitle);
    }

    Q_EMIT signalOpenFileDialog();

    fileDlg->exec();

    if (fileDlg->hasAcceptedUrls())
    {
        QStringList sel = fileDlg->selectedFiles();
        setFileDlgPath(sel.first());

        Q_EMIT signalUrlSelected(QUrl::fromLocalFile(sel.first()));
    }

    delete fileDlg;
}

} // namespace Digikam

#include "moc_dfileselector.cpp"
