/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-19
 * Description : A tab to display general item information
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QColor>
#include <QUrl>
#include <QFileInfo>

// Local includes

#include "template.h"
#include "dexpanderbox.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ItemPropertiesTab : public DExpanderBox
{
    Q_OBJECT

public:

    enum Section
    {
        FileProperties = 0,
        ImageProperties,
        PhotoProperties,
        VideoProperties,
        digiKamProperties,
        TagsProperties,
        LocationProperties,
        RightProperties
    };

public:

    explicit ItemPropertiesTab(QWidget* const parent);
    ~ItemPropertiesTab() override;

    void showOrHideCachedProperties();

    ///@{
    /// Setter methods (itempropertiestab_setters.cpp)

    void setCurrentUrl(const QUrl& url = QUrl());

    void setPhotoInfoDisable(const bool b);
    void setVideoInfoDisable(const bool b);

    void setFileName(const QString& str);
    void setFileFolder(const QString& str);
    void setFileModifiedDate(const QString& str);
    void setFileSize(const QString& str);
    void setFileOwner(const QString& str);
    void setFilePermissions(const QString& str);

    void setItemDimensions(const QString& str);
    void setImageRatio(const QString& str);
    void setImageMime(const QString& str);
    void setImageBitDepth(const QString& str);
    void setImageColorMode(const QString& str);
    void setHasSidecar(const QString& str);
    void setHasGPSInfo(const QString& str);
    void setVersionedInfo(const QString& str);
    void setGroupedInfo(const QString& str);
    void setGroupedWidget(QWidget* const wdg);

    void setPhotoMake(const QString& str);
    void setPhotoModel(const QString& str);
    void setPhotoDateTime(const QString& str);
    void setPhotoLens(const QString& str);
    void setPhotoAperture(const QString& str);
    void setPhotoFocalLength(const QString& str);
    void setPhotoExposureTime(const QString& str);
    void setPhotoSensitivity(const QString& str);
    void setPhotoExposureMode(const QString& str);
    void setPhotoFlash(const QString& str);
    void setPhotoWhiteBalance(const QString& str);

    void setVideoAspectRatio(const QString& str);
    void setVideoAudioBitRate(const QString& str);
    void setVideoAudioChannelType(const QString& str);
    void setVideoAudioCodec(const QString& str);
    void setVideoDuration(const QString& str);
    void setVideoFrameRate(const QString& str);
    void setVideoVideoCodec(const QString& str);

    void setTitle(const QString& str);
    void setCaption(const QString& str);
    void setPickLabel(int pickId);
    void setColorLabel(int colorId);
    void setRating(int rating);
    void setTags(const QStringList& tagPaths,
                 const QStringList& tagNames = QStringList(),
                 const QStringList& peopleTagPaths = QStringList(),
                 const QStringList& peopleTagNames = QStringList());
    void setTemplate(const Template& t);

    QString propertiesToText() const;

    ///@}

    ///@{
    /// Helper methods (itempropertiestab_helpers.cpp)

    /**
     * Shortens the tag paths by sorting and then cutting identical paths from the second
     * and following paths (only the first item gives the full path).
     * If you want to retain information about which tag path is sorted where,
     * you can optionally give a QVariant list. This list shall contain an identifier
     * for the tag path at the same index and will be resorted as the returned list.
     */
    static QStringList shortenedTagPaths(const QStringList& tagPaths, QList<QVariant>* identifiers = nullptr);

    /**
     * This methods shortens make an model camera info to prevent bloating GUI
     * See bug #265231 for details.
     */
    static void shortenedMakeInfo(QString& make);
    static void shortenedModelInfo(QString& model);

    /**
     * Write a string with aspect ratio information formatted
     */
    static bool aspectRatioToString(int width, int height, QString& arString);

    /**
     * Return file permissions string.
     */
    static QString permissionsString(const QFileInfo& fi);

    /**
     * Return an human readable string of file size in 'bytes'.
     * If 'si' is true, a decade of bytes is interpreted on base of 1000 byte, else 1024.
     */
    static QString humanReadableBytesCount(qint64 bytes, bool si = false);

private:

    static double doubleToHumanReadableFraction(double val, long* num, long* den, long maxden = 2);

    ///@}

    void appendProperty(QString& data, const QString& key, const QString& value) const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
