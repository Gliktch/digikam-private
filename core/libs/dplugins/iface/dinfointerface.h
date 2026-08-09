/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-06
 * Description : abstract interface to image information.
 *               This class do not depend of digiKam database library
 *               to permit to reuse plugins with Showfoto.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMap>
#include <QAtomicInt>
#include <QSharedPointer>
#include <QString>
#include <QObject>
#include <QVariant>
#include <QUrl>
#include <QSize>
#include <QList>
#include <QDateTime>
#include <QDate>
#include <QAbstractItemModel>
#include <QFileDevice>

// Local includes

#include "digikam_export.h"
#include "digikam_config.h"
#include "captionvalues.h"
#include "metaengine.h"

#ifdef HAVE_GEOLOCATION
#   include "gpsitemcontainer.h"
#endif

namespace Digikam
{

enum class DItemAccessPurpose
{
    Export          = 1,
    Print           = 2,
    View            = 3,
    BatchProcess    = 4,
    ExternalOpen    = 5,
    DragOrClipboard = 6,
    MetadataWrite   = 7
};

enum class DItemAccessMutation
{
    ReadOnly         = 1,
    MayCreateOutputs = 2,
    MayModifyInputs  = 3
};

enum class DItemAccessSource
{
    PublicRepresentation = 1,
    InternalOriginal     = 2,
    WritableCheckout     = 3,
    PublicOriginal       = 4
};

enum class DItemAccessConsumerScope
{
    SameProcess     = 1,
    DetachedProcess = 2
};

enum class DItemAssociatedRole
{
    CompanionMedia    = 2,
    XmpSidecar        = 3,
    ConfiguredSidecar = 4
};

class DIGIKAM_EXPORT DItemAccessCancellationToken
{
public:

    DItemAccessCancellationToken() = default;

    void cancel();
    bool isCanceled() const;

private:

    QAtomicInt m_canceled = 0;

private:

    Q_DISABLE_COPY(DItemAccessCancellationToken)
};

class DIGIKAM_EXPORT DItemAccessRequest
{
public:

    bool isValid() const;

public:

    QString                 consumerIdentity;
    QList<QUrl>             logicalUrls;
    DItemAccessPurpose      purpose = static_cast<DItemAccessPurpose>(0);
    DItemAccessMutation     mutation = static_cast<DItemAccessMutation>(0);
    DItemAccessSource       requestedSource =
        static_cast<DItemAccessSource>(0);
    DItemAccessConsumerScope consumerScope =
        static_cast<DItemAccessConsumerScope>(0);
    bool                    allowPlaceholderFallback = false;
    bool                    allowPartialSelection = false;
};

class DIGIKAM_EXPORT DItemAccessFileFacts
{
public:

    bool isValid() const;

public:

    QDateTime               modificationDate;
    QFileDevice::Permissions permissions;
    bool                    available = false;
};

class DIGIKAM_EXPORT DItemAccessEntry
{
public:

    bool isValid() const;

public:

    QUrl logicalUrl;
    /// Concrete source URL only when deferred is false. A deferred entry is an
    /// authorization plan; acquireSource() is the sole way to obtain its I/O
    /// URL and the returned source handle owns that URL's lifetime.
    QUrl physicalUrl;
    bool placeholder = false;
    /// physicalUrl is empty until acquireSource() owns a bounded source for
    /// this item.
    bool deferred = false;
    /// True for sources such as /proc/self/fd links which are meaningful only
    /// inside the process that prepared them.
    bool sameProcessOnly = false;
    /// Original/snapshot filesystem facts to apply after writing an exported
    /// copy. They deliberately exclude ownership, ACLs and POSIX special bits.
    DItemAccessFileFacts fileFacts;
};

class DIGIKAM_EXPORT DItemAssociatedAccessEntry
{
public:

    bool isValid() const;

public:

    QUrl logicalUrl;
    QUrl physicalUrl;
    int  role = 0;
    int  ordinal = -1;
    bool sameProcessOnly = false;
    DItemAccessFileFacts fileFacts;
};

class DIGIKAM_EXPORT DItemAccessSourceHandle
{
public:

    virtual ~DItemAccessSourceHandle();

    bool isValid() const;
    DItemAccessEntry entry() const;
    QList<DItemAssociatedAccessEntry> associatedEntries() const;
    virtual bool validateAccess() const;

protected:

    explicit DItemAccessSourceHandle(
        const DItemAccessEntry& entry,
        const QList<DItemAssociatedAccessEntry>& associatedEntries = {});

private:

    friend class DItemAccessHandle;

    DItemAccessEntry m_entry;
    QList<DItemAssociatedAccessEntry> m_associatedEntries;
};

/**
 * Owned item-source preparation. Callers must retain this object for the full
 * lifetime of every worker, dialog, process handoff or server using its
 * physical URLs. Destroying it releases host-specific access leases.
 */
class DIGIKAM_EXPORT DItemAccessHandle
{
public:

    virtual ~DItemAccessHandle();

    bool isValid() const;
    bool isCanceled() const;
    QList<DItemAccessEntry> entries() const;
    QList<QUrl> physicalUrls() const;
    QList<QUrl> excludedLogicalUrls() const;
    QUrl logicalUrlFor(const QUrl& physicalUrl) const;

    /** Revalidates host-specific access immediately before source I/O. */
    virtual bool validateAccess(const QUrl& physicalUrl) const;
    virtual QSharedPointer<DItemAccessSourceHandle> acquireSource(
        const QUrl& logicalUrl,
        const QSharedPointer<DItemAccessCancellationToken>& cancellation = {}) const;

    static QSharedPointer<DItemAccessHandle> passThrough(
        const DItemAccessRequest& request);
    static QSharedPointer<DItemAccessHandle> canceled(
        const DItemAccessRequest& request);

protected:

    DItemAccessHandle(const QList<DItemAccessEntry>& entries,
                      const QList<QUrl>& excludedLogicalUrls,
                      bool canceled);

private:

    QList<DItemAccessEntry> m_entries;
    QList<QUrl>             m_excludedLogicalUrls;
    bool                    m_canceled = false;
};

class DIGIKAM_EXPORT DInfoInterface : public QObject
{
    Q_OBJECT

public:

    typedef QMap<QString, QVariant> DInfoMap;       ///< Map of properties name and value.
    typedef QList<int>              DAlbumIDs;      ///< List of Album ids.

public:

    explicit DInfoInterface(QObject* const parent);
    ~DInfoInterface() override = default;

public:

    /// Slot to call when date time stamp from item is changed.
    Q_SLOT virtual void slotDateTimeForUrl(const QUrl& url,
                                           const QDateTime& dt,
                                           bool updModDate);

    /// Slot to call when something in metadata from item is changed.
    Q_SLOT virtual void slotMetadataChangedForUrl(const QUrl& url);

    Q_SIGNAL void signalAlbumItemsRecursiveCompleted(const QList<QUrl>& imageList);

    Q_SIGNAL void signalShortcutPressed(const QString& shortcut, int val);

    /// Signal emitted when color label names are updated (digiKam only).
    Q_SIGNAL void signalColorLabelNamesUpdated(const QMap<int, QString>& labels);

public:

    ///@{
    /// Low level items and albums methods

    virtual QList<QUrl> currentSelectedItems()                                      const;
    virtual QList<QUrl> currentAlbumItems()                                         const;
    virtual QUrl        currentActiveItem()                                         const;
    virtual void        parseAlbumItemsRecursive();

    virtual QList<QUrl> albumItems(int)                                             const;
    virtual QList<QUrl> albumsItems(const DAlbumIDs&)                               const;
    virtual QList<QUrl> allAlbumItems()                                             const;

    /**
     * Prepares explicit logical items for an operation with an owned lifetime.
     * The base implementation is an unprotected pass-through. Database-aware
     * hosts may authenticate, exclude, substitute or lease different sources.
     */
    virtual QSharedPointer<DItemAccessHandle> prepareItemAccess(
        const DItemAccessRequest& request) const;

    virtual DInfoMap albumInfo(int)                                                 const;
    virtual void     setAlbumInfo(int, const DInfoMap&)                             const;

    virtual DInfoMap itemInfo(const QUrl&)                                          const;
    virtual void     setItemInfo(const QUrl&, const DInfoMap&);

    Q_SIGNAL void signalLastItemUrl(const QUrl&);
    ///@}

public:

    ///@{
    /// Albums chooser view methods (to use items from albums before to process).

    virtual QWidget*  albumChooser(QWidget* const parent)                           const;
    virtual DAlbumIDs albumChooserItems()                                           const;
    virtual bool      supportAlbums()                                               const;

    Q_SIGNAL void signalAlbumChooserSelectionChanged();
    ///@}

public:

    ///@{
    /// Album selector view methods (to upload items from an external place).

    virtual QWidget* uploadWidget(QWidget* const parent)                            const;
    virtual QUrl     uploadUrl()                                                    const;

    Q_SIGNAL void signalUploadUrlChanged();

    /// Url to upload new items without to use album selector.
    virtual QUrl     defaultUploadUrl()                                             const;

    Q_SIGNAL void signalImportedImage(const QUrl&);
    ///@}

public:

    /// Return an instance of tag filter model if host application support this feature, else null pointer.
    virtual QAbstractItemModel* tagFilterModel();

#ifdef HAVE_GEOLOCATION

    virtual QList<GPSItemContainer*> currentGPSItems()                              const;

#endif

public:

    /// Pass extra shortcut actions to widget and return prefixes of shortcuts
    virtual QMap<QString, QString> passShortcutActionsToWidget(QWidget* const)      const;

public:

    /// Manipulate with item
    virtual void deleteImage(const QUrl& url);

public:

    enum SetupPage
    {
        ExifToolPage = 0,
        ImageQualityPage
    };

    /// Open configuration dialog page.
    virtual void openSetupPage(SetupPage page);

    Q_SIGNAL void signalSetupChanged();

public:

    bool forceAlbumSelection = false;
};

// -------------------------------------------------------------------------------------------------------------

/**
 * DItemInfo is a class to get item information from host application (Showfoto or digiKam)
 * The interface is re-implemented in host and depend how item information must be retrieved
 * (from a database or by file metadata).
 * The easy way to use this container is given below:
 *
 *  // READ INFO FROM HOST ---------------------------------------------
 *
 *  QUrl                     itemUrl;                                   // The item url that you want to retrieve information.
 *  DInfoInterface*          hostIface;                                 // The host application interface instance.
 *
 *  DInfoInterface::DInfoMap info = hostIface->itemInfo(itemUrl);       // First stage is to get the information map from host application.
 *  DItemInfo item(info);                                               // Second stage, is to create the DIntenInfo instance for this item by url.
 *  QString   title       = item.name();                                // Now you can retrieve the title,
 *  QString   description = item.comment();                             // The comment,
 *  QDateTime time        = item.dateTime();                            // The time stamp, etc.
 *
 *  // WRITE INFO TO HOST ----------------------------------------------
 *
 *  QUrl                     itemUrl;                                   // The item url that you want to retrieve information.
 *  DInfoInterface*          hostIface;                                 // The host application interface instance.
 *
 *  DItemInfo item;                                                     // Create the DIntenInfo instance for this item with an empty internal info map.
 *  item.setRating(3);                                                  // Store rating to internal info map.
 *  item.setColorLabel(1);                                              // Store color label to internal info map.
 *  hostIface->setItemInfo(url, item.infoMap());                        // Update item information to host using internal info map.
 */

class DIGIKAM_EXPORT DItemInfo
{

public:

    DItemInfo()  = default;
    explicit DItemInfo(const DInfoInterface::DInfoMap&);
    ~DItemInfo() = default;

    DInfoInterface::DInfoMap infoMap() const;

public:

    QString            name()                                                       const;
    QString            title()                                                      const;
    QString            comment()                                                    const;
    QSize              dimensions()                                                 const;
    QDateTime          dateTime()                                                   const;
    QStringList        tagsPath()                                                   const;
    QStringList        keywords()                                                   const;

    CaptionsMap        titles()                                                     const;
    void               setTitles(const CaptionsMap&);
    CaptionsMap        captions()                                                   const;
    void               setCaptions(const CaptionsMap&);

    MetaEngine::AltLangMap  copyrights()                                            const;
    void                    setCopyrights(const MetaEngine::AltLangMap& map);
    MetaEngine::AltLangMap  copyrightNotices()                                      const;
    void                    setCopyrightNotices(const MetaEngine::AltLangMap& map);

    int                albumId()                                                    const;
    int                orientation()                                                const;
    void               setOrientation(int);
    int                rating()                                                     const;
    void               setRating(int);
    int                colorLabel()                                                 const;
    void               setColorLabel(int);
    int                pickLabel()                                                  const;
    void               setPickLabel(int);

    double             latitude()                                                   const;
    double             longitude()                                                  const;
    double             altitude()                                                   const;
    qlonglong          fileSize()                                                   const;
    QStringList        creators()                                                   const;
    QString            credit()                                                     const;
    QString            rights()                                                     const;
    QString            source()                                                     const;
    QString            lens()                                                       const;
    QString            make()                                                       const;
    QString            model()                                                      const;
    QString            exposureTime()                                               const;
    QString            sensitivity()                                                const;
    QString            aperture()                                                   const;
    QString            focalLength()                                                const;
    QString            focalLength35mm()                                            const;
    QString            videoCodec()                                                 const;

    bool hasGeolocationInfo()                                                       const;

private:

    QVariant parseInfoMap(const QString& key)                                       const;

private:

    DInfoInterface::DInfoMap m_info;
};

// -----------------------------------------------------------------

class DIGIKAM_EXPORT DAlbumInfo
{

public:

    explicit DAlbumInfo(const DInfoInterface::DInfoMap&);
    ~DAlbumInfo() = default;

public:

    QString title()                                                                 const;
    QString caption()                                                               const;
    QDate   date()                                                                  const;
    QString path()                                                                  const;
    QString albumPath()                                                             const;

private:

    DInfoInterface::DInfoMap m_info;
};

} // namespace Digikam
