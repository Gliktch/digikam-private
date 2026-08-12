/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-09-27
 * Description : Showfoto stack view favorites item
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotostackviewfavoriteitem.h"

// Qt includes

#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local include

#include "digikam_debug.h"
#include "showfotosettings.h"
#include "ditemtooltip.h"

using namespace Digikam;

namespace ShowFoto
{

class Q_DECL_HIDDEN ShowfotoStackViewFavoriteItem::Private
{
public:

    Private() = default;

public:

    int         favoriteType = ShowfotoStackViewFavoriteItem::FavoriteRoot;
    QString     hierarchy;
    QList<QUrl> urls;
    QString     desc;
    QDate       date;
    QUrl        current;
};

ShowfotoStackViewFavoriteItem::ShowfotoStackViewFavoriteItem(QTreeWidget* const parent)
    : QTreeWidgetItem(parent),
      d              (new Private)
{
    setDisabled(false);
    setSelected(false);
    setFavoriteType(FavoriteRoot);
}

ShowfotoStackViewFavoriteItem::ShowfotoStackViewFavoriteItem(QTreeWidgetItem* const parent, int favType)
    : QTreeWidgetItem(parent),
      d              (new Private)
{
    setDisabled(false);
    setSelected(false);
    setFavoriteType(favType);
}

ShowfotoStackViewFavoriteItem::~ShowfotoStackViewFavoriteItem()
{
    delete d;
}

void ShowfotoStackViewFavoriteItem::setName(const QString& name)
{
    setText(0, name);
    setHierarchy(hierarchyFromParent(name, dynamic_cast<ShowfotoStackViewFavoriteItem*>(parent())));
    updateToolTip();
}

QString ShowfotoStackViewFavoriteItem::hierarchyFromParent(const QString& name, ShowfotoStackViewFavoriteItem* const pitem)
{
    QString hrch = QLatin1String("/");

    if (pitem && !name.isEmpty())
    {
        hrch = pitem->hierarchy() + name + QLatin1String("/");
    }

    return hrch;
}

QString ShowfotoStackViewFavoriteItem::name() const
{
    return text(0);
}

void ShowfotoStackViewFavoriteItem::setHierarchy(const QString& hierarchy)
{
    d->hierarchy = hierarchy;
}

QString ShowfotoStackViewFavoriteItem::hierarchy() const
{
    return d->hierarchy;
}

void ShowfotoStackViewFavoriteItem::setFavoriteType(int favoriteType)
{
    d->favoriteType = favoriteType;

    switch (d->favoriteType)
    {
        case FavoriteRoot:
        {
            setName(i18nc("@title", "My Favorites"));
            setIcon(0, QIcon::fromTheme(QLatin1String("folder-root")));
            setDescription(QString());
            setDate(QDate());
            setUrls(QList<QUrl>());

            break;
        }

        case FavoriteFolder:
        {
            setIcon(0, QIcon::fromTheme(QLatin1String("folder")));
            setDescription(QString());
            setDate(QDate());
            setUrls(QList<QUrl>());

            break;
        }

        default:    // FavoriteItem
        {
            setIcon(0, QIcon::fromTheme(QLatin1String("folder-favorites")));

            break;
        }
    }
}

int ShowfotoStackViewFavoriteItem::favoriteType() const
{
    return d->favoriteType;
}

void ShowfotoStackViewFavoriteItem::setDescription(const QString& desc)
{
    d->desc = desc;

    updateToolTip();
}

QString ShowfotoStackViewFavoriteItem::description() const
{
    return d->desc;
}

void ShowfotoStackViewFavoriteItem::setDate(const QDate& date)
{
    d->date = date;

    updateToolTip();
}

QDate ShowfotoStackViewFavoriteItem::date() const
{
    return d->date;
}

void ShowfotoStackViewFavoriteItem::setUrls(const QList<QUrl>& urls)
{
    d->urls = urls;

    updateToolTip();
}

QList<QUrl> ShowfotoStackViewFavoriteItem::urls() const
{
    return d->urls;
}

void ShowfotoStackViewFavoriteItem::setCurrentUrl(const QUrl& url)
{
    d->current = url;
}

QUrl ShowfotoStackViewFavoriteItem::currentUrl() const
{
    if (!d->current.isValid() && !urls().isEmpty())
    {
        return urls().constFirst();
    }

    return d->current;
}

QStringList ShowfotoStackViewFavoriteItem::urlsToPaths() const
{
    QStringList files;

    const auto curls = urls();

    for (const QUrl& url : curls)
    {
        files << url.toLocalFile();
    }

    return files;
}

void ShowfotoStackViewFavoriteItem::updateToolTip()
{
    if (!ShowfotoSettings::instance()->getShowToolTip())
    {
        return;
    }

    switch (d->favoriteType)
    {
        case FavoriteRoot:
        {
            break;
        }

        case FavoriteFolder:
        {
            DToolTipStyleSheet cnt(ShowfotoSettings::instance()->getToolTipFont());
            QString tip  = cnt.tipHeader;

            tip += cnt.headBeg + i18nc("@title", "Favorite Folder Properties") + cnt.headEnd;

            tip += cnt.cellBeg + i18nc("@info: favorite folder title property", "Name:") + cnt.cellMid;
            tip += name() + cnt.cellEnd;

            tip += cnt.cellBeg + i18nc("@info: favorite folder hierarchy property", "Hierarchy:") + cnt.cellMid;
            tip += hierarchy() + cnt.cellEnd;

            tip += cnt.tipFooter;

            setToolTip(0, tip);

            break;
        }

        default:    // FavoriteItem
        {
            QString desc = description().isEmpty() ? QLatin1String("---") : description();
            DToolTipStyleSheet cnt(ShowfotoSettings::instance()->getToolTipFont());
            QString tip  = cnt.tipHeader;

            tip += cnt.headBeg + i18nc("@title", "Favorite Item Properties") + cnt.headEnd;

            tip += cnt.cellBeg + i18nc("@info: favorite item title property", "Name:") + cnt.cellMid;
            tip += name() + cnt.cellEnd;

            tip += cnt.cellBeg + i18nc("@info: favorite item date property", "Created:") + cnt.cellMid;
            tip += date().toString() + cnt.cellEnd;

            tip += cnt.cellBeg + i18nc("@info: favorite item hierarchy property", "Hierarchy:") + cnt.cellMid;
            tip += hierarchy() + cnt.cellEnd;

            tip += cnt.cellBeg + i18nc("@info; favorite item count elements property", "Items:") + cnt.cellMid;
            tip += QString::number(urls().count()) + cnt.cellEnd;

            tip += cnt.cellSpecBeg + i18nc("@info: favorite item caption property", "Description:") + cnt.cellSpecMid +
                   cnt.breakString(desc) + cnt.cellSpecEnd;

            tip += cnt.tipFooter;

            setToolTip(0, tip);

            break;
        }
    }
}

} // namespace ShowFoto
