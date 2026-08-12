/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-09-27
 * Description : Showfoto stack view item
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotostackviewitem.h"

// Qt includes

#include <QIcon>
#include <QPainter>
#include <QLocale>
#include <QDateTime>
#include <QMimeDatabase>
#include <QFileInfo>

// KDE includes

#include <klocalizedstring.h>

// Local include

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "showfotostackviewlist.h"
#include "showfotoitemsortsettings.h"
#include "itempropertiestab.h"
#include "drawdecoder.h"

using namespace Digikam;

namespace ShowFoto
{

ShowfotoStackViewItem::ShowfotoStackViewItem(ShowfotoStackViewList* const parent)
    : QTreeWidgetItem(parent)
{
    setDisabled(false);
    setSelected(false);
}

void ShowfotoStackViewItem::setInfo(const ShowfotoItemInfo& info)
{
    m_info                 = info;
    setText(ShowfotoStackViewList::FileName, m_info.name);

    QDateTime dt           = (m_info.ctime.isValid() ? m_info.ctime : m_info.dtime);
    QString str            = asUserDateTime(dt);
    setText(ShowfotoStackViewList::FileDate, str);
    setData(ShowfotoStackViewList::FileDate, Qt::UserRole, dt);

    QFileInfo fileInfo(m_info.name);
    QString rawFilesExt    = DRawDecoder::rawFiles();
    QString ext            = fileInfo.suffix().toUpper();

    if (!ext.isEmpty() && rawFilesExt.toUpper().contains(ext))
    {
        setText(ShowfotoStackViewList::FileType, i18nc("@info: item properties", "RAW Image"));
    }
    else
    {
        setText(ShowfotoStackViewList::FileType, QMimeDatabase().mimeTypeForFile(fileInfo).comment());
    }

    str                    = ItemPropertiesTab::humanReadableBytesCount(m_info.size);
    setText(ShowfotoStackViewList::FileSize, str);
    setData(ShowfotoStackViewList::FileSize, Qt::UserRole, info.size);
}

ShowfotoItemInfo ShowfotoStackViewItem::info() const
{
    return m_info;
}

void ShowfotoStackViewItem::setThumbnail(const QPixmap& thumb)
{
    QPixmap pix = thumb.scaled(treeWidget()->iconSize(), Qt::KeepAspectRatio,
                                                         Qt::FastTransformation);

    QPixmap icon(treeWidget()->iconSize());
    icon.fill(Qt::transparent);
    QPainter p(&icon);
    p.drawPixmap((icon.width()  - pix.width() ) / 2,
                 (icon.height() - pix.height()) / 2,
                 pix);

    setIcon(0, icon);
}

bool ShowfotoStackViewItem::operator<(const QTreeWidgetItem& other) const
{
    ShowfotoStackViewList* const parent = dynamic_cast<ShowfotoStackViewList*>(treeWidget());

    if (!parent)
    {
        return false;
    }

    int result                     = 0;
    int column                     = parent->sortColumn();
    Qt::SortOrder currentSortOrder = (Qt::SortOrder)parent->sortOrder();

    switch (column)
    {
        case ShowfotoStackViewList::FileSize:
        {
            result = (ShowfotoItemSortSettings::compareByOrder(data(ShowfotoStackViewList::FileSize, Qt::UserRole).toInt(),
                                                               other.data(ShowfotoStackViewList::FileSize, Qt::UserRole).toInt(),
                                                               currentSortOrder));
            break;
        }

        case ShowfotoStackViewList::FileType:
        {
            result = (ShowfotoItemSortSettings::naturalCompare(text(ShowfotoStackViewList::FileType),
                                                               other.text(ShowfotoStackViewList::FileType),
                                                               currentSortOrder,
                                                               Qt::CaseSensitive));
            break;
        }

        case ShowfotoStackViewList::FileDate:
        {
            result = (ShowfotoItemSortSettings::compareByOrder(data(ShowfotoStackViewList::FileDate, Qt::UserRole).toDateTime(),
                                                               other.data(ShowfotoStackViewList::FileDate, Qt::UserRole).toDateTime(),
                                                               currentSortOrder));
            break;
        }

        default:    // ShowfotoStackViewList::FileName
        {
            result = (ShowfotoItemSortSettings::naturalCompare(text(ShowfotoStackViewList::FileName),
                                                               other.text(ShowfotoStackViewList::FileName),
                                                               currentSortOrder,
                                                               Qt::CaseSensitive));
            break;
        }
    }

    return (result < 0);
}

} // namespace ShowFoto
