/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-08-27
 * Description : Showfoto folder view undo command.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotofolderviewundo.h"

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"

namespace ShowFoto
{

class Q_DECL_HIDDEN ShowfotoFolderViewUndo::Private
{

public:

    Private() = default;

    ShowfotoFolderViewSideBar* view     = nullptr;
    QString                    oldPath;
    QString                    newPath;
};

ShowfotoFolderViewUndo::ShowfotoFolderViewUndo(ShowfotoFolderViewSideBar* const view,
                                               const QString& newPath)
    : QUndoCommand(),
      d           (new Private)
{
    d->view    = view;
    d->oldPath = d->view->currentFolder();
    d->newPath = newPath;
}

ShowfotoFolderViewUndo::~ShowfotoFolderViewUndo()
{
    delete d;
}

QString ShowfotoFolderViewUndo::undoPath() const
{
    return d->oldPath;
}

void ShowfotoFolderViewUndo::undo()
{
    d->view->setCurrentPathWithoutUndo(d->oldPath);
}

void ShowfotoFolderViewUndo::redo()
{
    d->view->setCurrentPathWithoutUndo(d->newPath);
}

} // namespace ShowFoto
