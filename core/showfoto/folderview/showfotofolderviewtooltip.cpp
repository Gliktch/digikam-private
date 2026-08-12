/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-08-27
 * Description : Tool tip for Showfoto folder-view item.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotofolderviewtooltip.h"

// Qt includes

#include <QDateTime>
#include <QPainter>
#include <QScopedPointer>
#include <QPixmap>
#include <QFileInfo>
#include <QTextDocument>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "showfotosettings.h"
#include "showfotoiteminfo.h"
#include "showfotofolderviewlist.h"
#include "showfotofolderviewmodel.h"
#include "showfototooltipfiller.h"

namespace ShowFoto
{

class Q_DECL_HIDDEN ShowfotoFolderViewToolTip::Private
{
public:

    Private() = default;

    ShowfotoFolderViewList* view = nullptr;
    QModelIndex             index;
};

ShowfotoFolderViewToolTip::ShowfotoFolderViewToolTip(ShowfotoFolderViewList* const view)
    : DItemToolTip(),
      d           (new Private)
{
    d->view = view;
}

ShowfotoFolderViewToolTip::~ShowfotoFolderViewToolTip()
{
    delete d;
}

void ShowfotoFolderViewToolTip::setIndex(const QModelIndex& index)
{
    d->index = index;

    if (!d->index.isValid() || !ShowfotoSettings::instance()->getShowToolTip())
    {
        hide();
    }
    else
    {
        updateToolTip();
        reposition();

        if (isHidden() && !toolTipIsEmpty())
        {
            show();
        }
    }
}

QRect ShowfotoFolderViewToolTip::repositionRect()
{
    if (!d->index.isValid())
    {
        return QRect();
    }

    QRect rect = d->view->visualRect(d->index);
    rect.moveTopLeft(d->view->viewport()->mapToGlobal(rect.topLeft()));

    return rect;
}

QString ShowfotoFolderViewToolTip::tipContents()
{
    if (!d->index.isValid())
    {
        return QString();
    }

    ShowfotoFolderViewModel* const model = dynamic_cast<ShowfotoFolderViewModel*>(d->view->model());

    if (!model)
    {
        return QString();
    }

    QString path              = model->filePath(d->index);
    ShowfotoItemInfo iteminfo = ShowfotoItemInfo::itemInfoFromFile(QFileInfo(path));

    return ShowfotoToolTipFiller::ShowfotoItemInfoTipContents(iteminfo);
}

} // namespace ShowFoto

#include "moc_showfotofolderviewtooltip.cpp"
