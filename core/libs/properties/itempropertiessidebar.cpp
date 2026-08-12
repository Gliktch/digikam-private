/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : item properties side bar (without support of digiKam database).
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itempropertiessidebar.h"

// Qt includes

#include <QSplitter>
#include <QFileInfo>
#include <QApplication>
#include <QIcon>
#include <QLocale>
#include <QMimeDatabase>
#include <QMimeType>
#include <QClipboard>
#include <QMimeData>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "drawdecoder.h"
#include "dimg.h"
#include "dmetadata.h"
#include "dfileoperations.h"
#include "itempropertiestab.h"
#include "itempropertiesmetadatatab.h"
#include "itempropertiescolorstab.h"
#include "itempropertiesversionstab.h"

#ifdef HAVE_GEOLOCATION
#   include "itempropertiesgpstab.h"
#endif // HAVE_GEOLOCATION

namespace Digikam
{

ItemPropertiesSideBar::ItemPropertiesSideBar(QWidget* const parent,
                                             SidebarSplitter* const splitter,
                                             Qt::Edge side,
                                             bool mimimizedDefault)
    : Sidebar(parent, splitter, side, mimimizedDefault)
{
    m_propertiesStackedView  = new QStackedWidget(parent);
    m_propertiesTab          = new ItemPropertiesTab(parent);
    m_metadataTab            = new ItemPropertiesMetadataTab(parent);
    m_colorTab               = new ItemPropertiesColorsTab(parent);

    m_propertiesStackedView->addWidget(m_propertiesTab);

    // NOTE: Special case with Showfoto which will only be able to load image, not video.

    if (QApplication::applicationName() != QLatin1String("digikam"))
    {
        m_propertiesTab->setVideoInfoDisable(true);
    }

    m_propertiesTab->widget(ItemPropertiesTab::FileProperties)->setButtonIcon(QIcon::fromTheme(QLatin1String("edit-copy")));
    m_propertiesTab->widget(ItemPropertiesTab::FileProperties)->setButtonVisible(true);
    m_propertiesTab->widget(ItemPropertiesTab::FileProperties)->setButtonToolTip(i18n("Copy the all properties in clipboard"));

    m_propertiesTab->widget(ItemPropertiesTab::ImageProperties)->setButtonIcon(QIcon::fromTheme(QLatin1String("folder-open")));
    m_propertiesTab->widget(ItemPropertiesTab::ImageProperties)->setButtonVisible(true);
    m_propertiesTab->widget(ItemPropertiesTab::ImageProperties)->setButtonToolTip(i18n("Open item in the file manager"));

    appendTab(m_propertiesStackedView, QIcon::fromTheme(QLatin1String("configure")),        i18nc("@title: item properties", "Properties"));
    appendTab(m_metadataTab,           QIcon::fromTheme(QLatin1String("format-text-code")), i18nc("@title: item properties", "Metadata")); // krazy:exclude=iconnames
    appendTab(m_colorTab,              QIcon::fromTheme(QLatin1String("fill-color")),       i18nc("@title: item properties", "Colors"));

#ifdef HAVE_GEOLOCATION

    m_gpsTab = new ItemPropertiesGPSTab(parent);
    appendTab(m_gpsTab,                QIcon::fromTheme(QLatin1String("globe")),            i18nc("@title: item properties", "Map"));

#endif // HAVE_GEOLOCATION

    connect(m_propertiesTab, &ItemPropertiesTab::signalItemButtonPressed,
            this, &ItemPropertiesSideBar::slotFileButtonPressed);

    connect(m_metadataTab, SIGNAL(signalSetupMetadataFilters(int)),
            this, SIGNAL(signalSetupMetadataFilters(int)));

    connect(m_metadataTab, SIGNAL(signalSetupExifTool()),
            this, SIGNAL(signalSetupExifTool()));

    // --- NOTE: use dynamic binding as slotChangedTab() is a virtual method which can be re-implemented in derived classes.

    connect(this, &ItemPropertiesSideBar::signalChangedTab,
            this, &ItemPropertiesSideBar::slotChangedTab);
}

void ItemPropertiesSideBar::itemChanged(const QUrl& url, const QRect& rect, DImg* const img)
{
    if (!url.isValid())
    {
        return;
    }

    m_currentUrl         = url;
    m_currentRect        = rect;
    m_image              = img;
    m_dirtyPropertiesTab = false;
    m_dirtyMetadataTab   = false;
    m_dirtyColorTab      = false;
    m_dirtyGpsTab        = false;
    m_dirtyHistoryTab    = false;

    slotChangedTab(getActiveTab());
}

void ItemPropertiesSideBar::slotNoCurrentItem()
{
    m_currentUrl = QUrl();

    m_propertiesTab->setCurrentUrl();
    m_metadataTab->setCurrentUrl();
    m_colorTab->setData();

#ifdef HAVE_GEOLOCATION

    m_gpsTab->setCurrentUrl();

#endif // HAVE_GEOLOCATION

    m_dirtyPropertiesTab = false;
    m_dirtyMetadataTab   = false;
    m_dirtyColorTab      = false;
    m_dirtyGpsTab        = false;
    m_dirtyHistoryTab    = false;
}

void ItemPropertiesSideBar::slotImageSelectionChanged(const QRect& rect)
{
    m_currentRect = rect;

    if (m_dirtyColorTab)
    {
        m_colorTab->setSelection(rect);
    }
    else
    {
        slotChangedTab(m_colorTab);
    }
}

void ItemPropertiesSideBar::slotChangedTab(QWidget* tab)
{
    if (!m_currentUrl.isValid())
    {

#ifdef HAVE_GEOLOCATION

        m_gpsTab->setActive(tab == m_gpsTab);

#endif // HAVE_GEOLOCATION

        return;
    }

    setCursor(Qt::WaitCursor);

    if      ((tab == m_propertiesStackedView) && !m_dirtyPropertiesTab)
    {
        m_propertiesTab->setCurrentUrl(m_currentUrl);
        setImagePropertiesInformation(m_currentUrl);
        m_dirtyPropertiesTab = true;
    }
    else if ((tab == m_metadataTab) && !m_dirtyMetadataTab)
    {
        m_metadataTab->setCurrentUrl(m_currentUrl);
        m_dirtyMetadataTab = true;
    }
    else if ((tab == m_colorTab) && !m_dirtyColorTab)
    {
        m_colorTab->setData(m_currentUrl, m_currentRect, m_image);
        m_dirtyColorTab = true;
    }

#ifdef HAVE_GEOLOCATION

    else if ((tab == m_gpsTab) && !m_dirtyGpsTab)
    {
        m_gpsTab->setCurrentUrl(m_currentUrl);
        m_dirtyGpsTab = true;
    }

    m_gpsTab->setActive(tab == m_gpsTab);

#endif // HAVE_GEOLOCATION

    unsetCursor();
}

void ItemPropertiesSideBar::setImagePropertiesInformation(const QUrl& url)
{
    if (!url.isValid())
    {
        return;
    }

    QString str;
    QString unavailable(QString::fromUtf8("<i>%1</i>").arg(i18nc("@info: item properties", "unavailable")));
    QFileInfo fileInfo(url.toLocalFile());
    QScopedPointer<DMetadata> metaData(new DMetadata(url.toLocalFile()));

    // -- File system information -----------------------------------------

    str = asUserDateTime(fileInfo.lastModified());
    m_propertiesTab->setFileModifiedDate(str);

    str = QString::fromUtf8("%1 (%2)").arg(ItemPropertiesTab::humanReadableBytesCount(fileInfo.size()))
                                      .arg(QLocale().toString(fileInfo.size()));
    m_propertiesTab->setFileSize(str);
    m_propertiesTab->setFileOwner(QString::fromUtf8("%1 - %2").arg(fileInfo.owner()).arg(fileInfo.group()));
    m_propertiesTab->setFilePermissions(ItemPropertiesTab::permissionsString(fileInfo));

    // -- Image Properties --------------------------------------------------

    QSize   dims;
    QString bitDepth, colorMode;
    QString rawFilesExt = DRawDecoder::rawFiles();
    QString ext         = fileInfo.suffix().toUpper();

    if (!ext.isEmpty() && rawFilesExt.toUpper().contains(ext))
    {
        m_propertiesTab->setImageMime(i18nc("@info: item properties", "RAW Image"));
        bitDepth    = QLatin1String("48");
        dims        = metaData->getItemDimensions();
        colorMode   = i18nc("@info: item properties", "Uncalibrated");
    }
    else
    {
        m_propertiesTab->setImageMime(QMimeDatabase().mimeTypeForFile(fileInfo).comment());

        dims      = metaData->getPixelSize();

        DImg img;
        img.loadItemInfo(url.toLocalFile(), false, false, false, false);
        bitDepth.setNum(img.originalBitDepth());
        colorMode = DImg::colorModelToString(img.originalColorModel());
    }

    QString mpixels = QLocale().toString(dims.width()*dims.height()/1000000.0, 'f', 1);
    str = (!dims.isValid()) ? i18nc("@info: item properties", "Unknown")
                            : i18nc("@info: item properties", "%1x%2 (%3Mpx)",
            dims.width(), dims.height(), mpixels);
    m_propertiesTab->setItemDimensions(str);

    if (!dims.isValid())
    {
        str = i18nc("@info: item properties", "Unknown");
    }
    else
    {
        m_propertiesTab->aspectRatioToString(dims.width(), dims.height(), str);
    }

    m_propertiesTab->setImageRatio(str);
    m_propertiesTab->setImageColorMode(colorMode.isEmpty() ? unavailable : colorMode);
    m_propertiesTab->setImageBitDepth(bitDepth.isEmpty()   ? unavailable : i18nc("@info: item properties", "%1 bpp", bitDepth));
    m_propertiesTab->setHasSidecar(DMetadata::hasSidecar(url.toLocalFile()) ? i18nc("@info: item properties", "Yes")
                                                                            : i18nc("@info: item properties", "No"));
    double alt, lat, lng;
    m_propertiesTab->setHasGPSInfo(metaData->getGPSInfo(alt, lat, lng) ? i18nc("@info: item properties", "Yes")
                                                                       : i18nc("@info: item properties", "No"));

    // -- Photograph information ------------------------------------------

    PhotoInfoContainer photoInfo = metaData->getPhotographInformation();

    m_propertiesTab->setPhotoInfoDisable(photoInfo.isEmpty());
    ItemPropertiesTab::shortenedMakeInfo(photoInfo.make);
    ItemPropertiesTab::shortenedModelInfo(photoInfo.model);
    m_propertiesTab->setPhotoMake(photoInfo.make.isEmpty()   ? unavailable : photoInfo.make);
    m_propertiesTab->setPhotoModel(photoInfo.model.isEmpty() ? unavailable : photoInfo.model);

    if (photoInfo.dateTime.isValid())
    {
        str = asUserDateTime(photoInfo.dateTime);
        m_propertiesTab->setPhotoDateTime(str);
    }
    else
    {
        m_propertiesTab->setPhotoDateTime(unavailable);
    }

    m_propertiesTab->setPhotoLens(photoInfo.lens.isEmpty()         ? unavailable : photoInfo.lens);
    m_propertiesTab->setPhotoAperture(photoInfo.aperture.isEmpty() ? unavailable : photoInfo.aperture);

    if (photoInfo.focalLength35mm.isEmpty())
    {
        m_propertiesTab->setPhotoFocalLength(photoInfo.focalLength.isEmpty() ? unavailable : photoInfo.focalLength);
    }
    else
    {
        str = i18nc("@info: item properties", "%1 (%2)", photoInfo.focalLength, photoInfo.focalLength35mm);
        m_propertiesTab->setPhotoFocalLength(str);
    }

    m_propertiesTab->setPhotoExposureTime(photoInfo.exposureTime.isEmpty() ? unavailable : photoInfo.exposureTime);
    m_propertiesTab->setPhotoSensitivity(photoInfo.sensitivity.isEmpty()   ? unavailable : i18nc("@info: item properties", "%1 ISO", photoInfo.sensitivity));

    if      (photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
    {
        m_propertiesTab->setPhotoExposureMode(unavailable);
    }
    else if (!photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
    {
        m_propertiesTab->setPhotoExposureMode(photoInfo.exposureMode);
    }
    else if (photoInfo.exposureMode.isEmpty() && !photoInfo.exposureProgram.isEmpty())
    {
        m_propertiesTab->setPhotoExposureMode(photoInfo.exposureProgram);
    }
    else
    {
        str = QString::fromUtf8("%1 / %2").arg(photoInfo.exposureMode).arg(photoInfo.exposureProgram);
        m_propertiesTab->setPhotoExposureMode(str);
    }

    m_propertiesTab->setPhotoFlash(photoInfo.flash.isEmpty()               ? unavailable : photoInfo.flash);
    m_propertiesTab->setPhotoWhiteBalance(photoInfo.whiteBalance.isEmpty() ? unavailable : photoInfo.whiteBalance);

    // -- Audio/Video information ------------------------------------------

    VideoInfoContainer videoInfo = metaData->getVideoInformation();

    m_propertiesTab->setVideoInfoDisable(videoInfo.isEmpty());

    m_propertiesTab->setVideoAspectRatio(videoInfo.aspectRatio.isEmpty()           ? unavailable : videoInfo.aspectRatio);
    m_propertiesTab->setVideoDuration(videoInfo.duration.isEmpty()                 ? unavailable : videoInfo.duration);
    m_propertiesTab->setVideoFrameRate(videoInfo.frameRate.isEmpty()               ? unavailable : videoInfo.frameRate);
    m_propertiesTab->setVideoVideoCodec(videoInfo.videoCodec.isEmpty()             ? unavailable : videoInfo.videoCodec);
    m_propertiesTab->setVideoAudioBitRate(videoInfo.audioBitRate.isEmpty()         ? unavailable : videoInfo.audioBitRate);
    m_propertiesTab->setVideoAudioChannelType(videoInfo.audioChannelType.isEmpty() ? unavailable : videoInfo.audioChannelType);
    m_propertiesTab->setVideoAudioCodec(videoInfo.audioCodec.isEmpty()             ? unavailable : videoInfo.audioCodec);

    // -- Title, caption, ratings, tag information ---------------------

    CaptionsMap titles = metaData->getItemTitles();
    QString title;

    if      (titles.contains(QLatin1String("x-default")))
    {
        title = titles.value(QLatin1String("x-default")).caption;
    }
    else if (!titles.isEmpty())
    {
        title = titles.begin().value().caption;
    }

    m_propertiesTab->setTitle(title);

    CaptionsMap captions = metaData->getItemComments();
    QString caption;

    if      (captions.contains(QLatin1String("x-default")))
    {
        caption = captions.value(QLatin1String("x-default")).caption;
    }
    else if (!captions.isEmpty())
    {
        caption = captions.begin().value().caption;
    }

    m_propertiesTab->setCaption(caption);

    m_propertiesTab->setRating(metaData->getItemRating());

    QStringList tagPaths;
    metaData->getItemTagsPath(tagPaths);
    m_propertiesTab->setTags(tagPaths);

    // Not supported
    m_propertiesTab->setVersionedInfo(QString());
    m_propertiesTab->setGroupedInfo(QString());

    m_propertiesTab->showOrHideCachedProperties();
}

void ItemPropertiesSideBar::doLoadState()
{
    Sidebar::doLoadState();

    /// @todo m_propertiesTab should load its settings from our group

    m_propertiesTab->setObjectName(QLatin1String("Image Properties SideBar Expander"));

    KConfigGroup group = getConfigGroup();

    m_propertiesTab->readSettings(group);

#ifdef HAVE_GEOLOCATION

    const KConfigGroup groupGPSTab      = KConfigGroup(&group, entryName(QLatin1String("GPS Properties Tab")));
    m_gpsTab->readSettings(groupGPSTab);

#endif // HAVE_GEOLOCATION

    const KConfigGroup groupColorTab    = KConfigGroup(&group, entryName(QLatin1String("Color Properties Tab")));
    m_colorTab->readSettings(groupColorTab);

    const KConfigGroup groupMetadataTab = KConfigGroup(&group, entryName(QLatin1String("Metadata Properties Tab")));
    m_metadataTab->readSettings(groupMetadataTab);
}

void ItemPropertiesSideBar::doSaveState()
{
    Sidebar::doSaveState();

    KConfigGroup group = getConfigGroup();

    m_propertiesTab->writeSettings(group);

#ifdef HAVE_GEOLOCATION

    KConfigGroup groupGPSTab      = KConfigGroup(&group, entryName(QLatin1String("GPS Properties Tab")));
    m_gpsTab->writeSettings(groupGPSTab);

#endif // HAVE_GEOLOCATION

    KConfigGroup groupColorTab    = KConfigGroup(&group, entryName(QLatin1String("Color Properties Tab")));
    m_colorTab->writeSettings(groupColorTab);

    KConfigGroup groupMetadataTab = KConfigGroup(&group, entryName(QLatin1String("Metadata Properties Tab")));
    m_metadataTab->writeSettings(groupMetadataTab);
}

void ItemPropertiesSideBar::slotLoadMetadataFilters()
{
    m_metadataTab->loadFilters();
}

void ItemPropertiesSideBar::slotFileButtonPressed(int index)
{
    switch (index)
    {
        case ItemPropertiesTab::FileProperties:
        {
            QMimeData* const mimeData = new QMimeData();
            mimeData->setText(m_propertiesTab->propertiesToText());
            QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);

            break;
        }

        case ItemPropertiesTab::ImageProperties:
        {
            DFileOperations::openInFileManager(QList<QUrl>() << m_currentUrl);

            break;
        }

        default:
        {
            break;
        }
    }
}

} // namespace Digikam

#include "moc_itempropertiessidebar.cpp"
