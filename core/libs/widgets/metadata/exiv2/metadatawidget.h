/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-22
 * Description : a generic widget to display metadata
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QString>
#include <QUrl>

// Local includes

#include "dmetadata.h"
#include "digikam_export.h"

namespace Digikam
{

class MetadataListView;

class DIGIKAM_EXPORT MetadataWidget : public QWidget
{
    Q_OBJECT

public:

    enum TagFilters
    {
        NONE = 0,
        PHOTO,
        CUSTOM
    };

public:

    explicit MetadataWidget(QWidget* const parent, const QString& name = QString());
    ~MetadataWidget() override;

    int     getMode()           const;
    void    setMode(int mode);

    QStringList getTagsFilter() const;
    void        setTagsFilter(const QStringList& list);

    QString getCurrentItemKey() const;
    void    setCurrentItemByKey(const QString& itemKey);

    void    setUserAreaWidget(QWidget* const w);

public:

    virtual QString getTagTitle(const QString& key);
    virtual QString getTagDescription(const QString& key);

    virtual bool loadFromData(const QString& fileName,
                              const DMetadata& data = DMetadata());

    virtual bool loadFromURL(const QUrl& url) = 0;

Q_SIGNALS:

    void signalSetupMetadataFilters();

protected Q_SLOTS:

    virtual void slotSaveMetadataToFile() = 0;

protected:

    void   enabledToolButtons(bool);
    void   setFileName(const QString& fileName);
    MetadataListView* view() const;

    bool   setMetadata(const DMetadata& data = DMetadata());
    DMetadata* getMetadata() const;

    void   setMetadataMap(const DMetadata::MetaDataMap& data = DMetadata::MetaDataMap());
    const  DMetadata::MetaDataMap& getMetadataMap();

    void   setIfdList(const DMetadata::MetaDataMap& ifds, const QStringList& tagsFilter = QStringList());
    void   setIfdList(const DMetadata::MetaDataMap& ifds, const QStringList& keysFilter,
                      const QStringList& tagsFilter);

    QUrl   saveMetadataToFile(const QString& caption, const QString& fileFilter);
    bool   storeMetadataToFile(const QUrl& url, const QByteArray& metaData);

    /**
     * Call this method in children class constructors to init signal/slots connections.
     */
    void setup();

    QString metadataToText() const;

protected:

    virtual void buildView();
    virtual void setMetadataEmpty();

    virtual bool    decodeMetadata()         = 0;
    virtual QString getMetadataTitle() const = 0;

private Q_SLOTS:

    void slotCopy2Clipboard();
    void slotPrintMetadata();
    void slotFilterChanged(QAction*);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
