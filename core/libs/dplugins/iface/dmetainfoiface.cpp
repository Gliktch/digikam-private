/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-06
 * Description : interface to item information for shared tools
 *               based on DMetadata. This interface is used in all cases
 *               where no database is available (aka Showfoto).
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dmetainfoiface.h"

// Qt includes

#include <QFileInfo>
#include <QStandardPaths>
#include <QScopedPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dmetadata.h"
#include "photoinfocontainer.h"
#include "videoinfocontainer.h"
#include "template.h"
#include "dfileselector.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN DMetaInfoIface::Private
{
public:

    Private() = default;

public:

    DFileSelector* dirSelector = nullptr;
    QList<QUrl>    urls;
    QUrl           caUrl;
};

DMetaInfoIface::DMetaInfoIface(QObject* const parent,
                               const QList<QUrl>& lst,
                               const QUrl& currentActive)
    : DInfoInterface(parent),
      d             (new Private)
{
    d->urls  = lst;
    d->caUrl = currentActive;
}

DMetaInfoIface::~DMetaInfoIface()
{
    delete d;
}

void DMetaInfoIface::slotDateTimeForUrl(const QUrl& url, const QDateTime& /*dt*/, bool /*updModDate*/)
{
    Q_EMIT signalItemChanged(url);
}

void DMetaInfoIface::slotMetadataChangedForUrl(const QUrl& url)
{
    Q_EMIT signalItemChanged(url);
}

void DMetaInfoIface::parseAlbumItemsRecursive()
{
    Q_EMIT signalAlbumItemsRecursiveCompleted(d->urls);
}

QUrl DMetaInfoIface::currentActiveItem() const
{
    return d->caUrl;
}

QList<QUrl> DMetaInfoIface::currentAlbumItems() const
{
    return d->urls;
}

QList<QUrl> DMetaInfoIface::currentSelectedItems() const
{
    // No multiple items selection is available in this interface.

    return currentAlbumItems();
}

QList<QUrl> DMetaInfoIface::allAlbumItems() const
{
    // No album management is available in this interface.

    return currentAlbumItems();
}

DMetaInfoIface::DInfoMap DMetaInfoIface::itemInfo(const QUrl& url) const
{
    DInfoMap map;

    if (d->urls.contains(url))
    {
        QScopedPointer<DMetadata> meta(new DMetadata(url.toLocalFile()));
        QString   def = QLatin1String("x-default");
        QFileInfo info(url.toLocalFile());

        map.insert(QLatin1String("name"),               info.fileName());
        map.insert(QLatin1String("title"),              meta->getItemTitles().value(def).caption);
        map.insert(QLatin1String("comment"),            meta->getItemComments().value(def).caption);
        map.insert(QLatin1String("orientation"),        (int)meta->getItemOrientation());
        map.insert(QLatin1String("datetime"),           meta->getItemDateTime());
        map.insert(QLatin1String("rating"),             meta->getItemRating());
        map.insert(QLatin1String("colorlabel"),         meta->getItemColorLabel());
        map.insert(QLatin1String("picklabel"),          meta->getItemPickLabel());
        map.insert(QLatin1String("filesize"),           (qlonglong)info.size());
        map.insert(QLatin1String("dimensions"),         meta->getItemDimensions());

        // Get digiKam Tags Path list of picture from metadata.
        // Ex.: "City/Paris/Monuments/Notre Dame"

        QStringList tagsPath;
        meta->getItemTagsPath(tagsPath);
        map.insert(QLatin1String("tagspath"),           tagsPath);

        // Get digiKam Tags name (keywords) list of picture from metadata.
        // Ex.: "Notre Dame"

        QStringList keywords = meta->getMetadataField(MetadataInfo::Keywords).toStringList();
        map.insert(QLatin1String("keywords"),           keywords);

        // Get GPS location of picture from metadata.

        double lat = 0.0;
        double lng = 0.0;
        double alt = 0.0;

        if (meta->getGPSInfo(lat, lng, alt))
        {
            map.insert(QLatin1String("latitude"),       lat);
            map.insert(QLatin1String("longitude"),      lng);
            map.insert(QLatin1String("altitude"),       alt);
        }

        // Get Copyright information of picture from metadata.

        Template temp;
        meta->getCopyrightInformation(temp);

        map.insert(QLatin1String("creators"),           temp.authors());
        map.insert(QLatin1String("credit"),             temp.credit());
        map.insert(QLatin1String("rights"),             temp.copyright().value(def));
        map.insert(QLatin1String("source"),             temp.source());

        PhotoInfoContainer photoInfo = meta->getPhotographInformation();
        map.insert(QLatin1String("lens"),               photoInfo.lens);
        map.insert(QLatin1String("make"),               photoInfo.make);
        map.insert(QLatin1String("model"),              photoInfo.model);
        map.insert(QLatin1String("exposuretime"),       photoInfo.exposureTime);
        map.insert(QLatin1String("sensitivity"),        photoInfo.sensitivity);
        map.insert(QLatin1String("aperture"),           photoInfo.aperture);
        map.insert(QLatin1String("focallength"),        photoInfo.focalLength);
        map.insert(QLatin1String("focalLength35mm"),    photoInfo.focalLength35mm);

        // Get Video information from metadata

        VideoInfoContainer videoInfo = meta->getVideoInformation();
        map.insert(QLatin1String("videocodec"),         videoInfo.videoCodec);

        // TODO: add more video metadata as needed

        // Get complex text containers.

        CaptionsMap titles   = meta->getItemTitles();
        map.insert(QLatin1String("titles"),             QVariant::fromValue(titles));
        CaptionsMap captions = meta->getItemComments();
        map.insert(QLatin1String("captions"),           QVariant::fromValue(captions));

        Template tpl                     = meta->getMetadataTemplate();
        DMetadata::AltLangMap copyrights = tpl.copyright();
        map.insert(QLatin1String("copyrights"),         QVariant::fromValue(copyrights));
        DMetadata::AltLangMap notices    = tpl.rightUsageTerms();
        map.insert(QLatin1String("copyrightnotices"),   QVariant::fromValue(notices));
    }

    return map;
}

void DMetaInfoIface::setItemInfo(const QUrl& url, const DInfoMap& map)
{
    QScopedPointer<DMetadata> meta(new DMetadata(url.toLocalFile()));
    QStringList keys = map.keys();

    if (map.contains(QLatin1String("orientation")))
    {
        meta->setItemOrientation((DMetadata::ImageOrientation)map[QLatin1String("orientation")].toInt());
        keys.removeAll(QLatin1String("orientation"));
    }

    if (map.contains(QLatin1String("rating")))
    {
        meta->setItemRating(map[QLatin1String("rating")].toInt());
        keys.removeAll(QLatin1String("rating"));
    }

    if (map.contains(QLatin1String("colorlabel")))
    {
        meta->setItemColorLabel(map[QLatin1String("colorlabel")].toInt());
        keys.removeAll(QLatin1String("colorlabel"));
    }

    if (map.contains(QLatin1String("picklabel")))
    {
        meta->setItemPickLabel(map[QLatin1String("picklabel")].toInt());
        keys.removeAll(QLatin1String("picklabel"));
    }

    if (map.contains(QLatin1String("titles")))
    {
        meta->setItemTitles(qvariant_cast<CaptionsMap>(map[QLatin1String("titles")]));
        keys.removeAll(QLatin1String("titles"));
    }

    if (map.contains(QLatin1String("captions")))
    {
        meta->setItemComments(qvariant_cast<CaptionsMap>(map[QLatin1String("captions")]));
        keys.removeAll(QLatin1String("captions"));
    }

    if (map.contains(QLatin1String("copyrights")))
    {
        Template tpl = meta->getMetadataTemplate();
        tpl.setCopyright(qvariant_cast<DMetadata::AltLangMap>(map[QLatin1String("copyrights")]));
        meta->setMetadataTemplate(tpl);
        keys.removeAll(QLatin1String("copyrights"));
    }

    if (map.contains(QLatin1String("copyrightnotices")))
    {
        Template tpl = meta->getMetadataTemplate();
        tpl.setRightUsageTerms(qvariant_cast<DMetadata::AltLangMap>(map[QLatin1String("copyrightnotices")]));
        meta->setMetadataTemplate(tpl);
        keys.removeAll(QLatin1String("copyrightnotices"));
    }

    if (!keys.isEmpty())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Keys not yet supported in DMetaInfoIface::setItemInfo():" << keys;
    }

    Q_EMIT signalItemChanged(url);
}

bool DMetaInfoIface::supportAlbums() const
{
    return false;
}

QWidget* DMetaInfoIface::uploadWidget(QWidget* const parent) const
{
    if (!d->dirSelector)
    {
        d->dirSelector = new DFileSelector(parent);
        d->dirSelector->setFileDlgMode(QFileDialog::Directory);
        d->dirSelector->setFileDlgOptions(QFileDialog::ShowDirsOnly);
        d->dirSelector->setFileDlgTitle(i18nc("@title:window", "Destination Folder"));
        d->dirSelector->lineEdit()->setPlaceholderText(i18n("Output Destination Path"));

        connect(d->dirSelector, SIGNAL(signalUrlSelected(QUrl)),
                this, SIGNAL(signalUploadUrlChanged()));
    }

    QFileInfo info(!d->urls.isEmpty() ? d->urls[0].toLocalFile() : QString());
    d->dirSelector->setFileDlgPath(info.absolutePath());

    return d->dirSelector;
}

QUrl DMetaInfoIface::uploadUrl() const
{
    return QUrl::fromLocalFile(d->dirSelector->fileDlgPath());
}

QUrl DMetaInfoIface::defaultUploadUrl() const
{
    QUrl place       = QUrl::fromLocalFile(QDir::homePath());
    QStringList pics = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);

    if (!pics.isEmpty())
    {
        place = QUrl::fromLocalFile(pics.first());
    }

    QList<QUrl> lst = currentAlbumItems();

    if (!lst.isEmpty())
    {
        QUrl trg = lst.first().adjusted(QUrl::RemoveFilename);

        if (!trg.isEmpty())
        {
            place = trg;
        }
    }

    return place;
}

void DMetaInfoIface::deleteImage(const QUrl& url)
{

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))

    QFile::moveToTrash(url.toLocalFile());

#else

    QFile::remove(url.toLocalFile());

#endif

    Q_EMIT signalRemoveImageFromAlbum(url);
}

#ifdef HAVE_GEOLOCATION

QList<GPSItemContainer*> DMetaInfoIface::currentGPSItems() const
{
    QList<GPSItemContainer*> items;
    const auto sel = currentSelectedItems();

    for (const QUrl& url : sel)
    {
        items << new GPSItemContainer(url);
    }

    return items;
}

#endif

} // namespace Digikam

#include "moc_dmetainfoiface.cpp"
