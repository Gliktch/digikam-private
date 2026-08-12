/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-05-16
 * Description : a tool to export GPS data to KML file.
 *
 * SPDX-FileCopyrightText: 2006-2007 by Stephane Pontier <shadow dot walker at free dot fr>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QColor>
#include <QList>
#include <QUrl>
#include <QDir>
#include <QDomDocument>
#include <QPointer>
#include <QImage>

// Local includes

#include "kmlgpsdataparser.h"
#include "dmetadata.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericGeolocationEditPlugin
{

class KmlExport : public QObject
{
    Q_OBJECT

public:

    explicit KmlExport(DInfoInterface* const iface);
    ~KmlExport()                                                              override;

    void setUrls(const QList<QUrl>& urls);

    /*! generate the kml element for pictures with thumbnails
     *  @param QUrl the URL of the picture
     *  @param kmlAlbum the album used
     */
    void generateImagesthumb(const QUrl&, QDomElement& kmlAlbum);

    /*! Produce a web-friendly file name
     *  otherwise, while google earth works fine, maps.google.com may not find pictures and thumbnail
     *  thank htmlexport
     *  @param the filename
     *  @return the webifyed filename
     */
    QString webifyFileName(const QString& fileName)                     const;

    /*! Generate a square thumbnail from @fullImage of @size x @size pixels
     *  @param fullImage the original image
     *  @param size the size of the thumbnail
     *  @return the thumbnail
     */
    QImage generateSquareThumbnail(const QImage& fullImage, int size)   const;

    /*! Generate a square thumbnail from @fullImage of @size x @size pixels
     *  with a white border
     *  @param fullImage the original image
     *  @param size the size of the thumbnail
     *  @return the thumbnail
     */
    QImage generateBorderedThumbnail(const QImage& fullImage, int size) const;

    void   addTrack(QDomElement& kmlAlbum);
    void   generate();

Q_SIGNALS:

    void signalProgressChanged(const int currentProgress);

private:

    void getConfig();
    void logInfo(const QString& msg);
    void logError(const QString& msg);
    void logWarning(const QString& msg);
    bool copyDir(const QString& srcFilePath, const QString& dstFilePath);

    /*!
     *  \fn KmlExport::addKmlElement(QDomElement target, QString tag)
     *  Add a new element
     *  @param target the parent element to which add the element
     *  @param tag the new element name
     *  @return the New element
     */
    QDomElement addKmlElement(QDomElement& target, const QString& tag)  const;

    /*!
     *  \fn KmlExport::addKmlTextElement(QDomElement target, QString tag, QString text)
     *  Add a new element with a text
     *  @param target the parent element to which add the element
     *  @param tag the new element name
     *  @param text the text content of the new element
     *  @return the New element
     */
    QDomElement addKmlTextElement(QDomElement& target,
                                  const QString& tag,
                                  const QString& text)                  const;

    /*!
     *  \fn KmlExport::addKmlHtmlElement(QDomElement target, QString tag, QString text)
     *  Add a new element with html content (html entities are escaped and text is wrapped in a CDATA section)
     *  @param target the parent element to which add the element
     *  @param tag the new element name
     *  @param text the HTML content of the new element
     *  @return the New element
     */
    QDomElement addKmlHtmlElement(QDomElement& target,
                                  const QString& tag,
                                  const QString& text)                  const;

private:

    bool                        m_localTarget           = true;
    bool                        m_optimize_googlemap    = false;
    bool                        m_GPXtracks             = false;

    int                         m_iconSize              = 33;
    int                         m_googlemapSize         = 32;
    int                         m_size                  = 320;
    int                         m_altitudeMode          = 0;
    int                         m_TimeZone              = 12;
    int                         m_LineWidth             = 4;
    int                         m_GPXOpacity            = 64;
    int                         m_GPXAltitudeMode       = 0;

    /// directory used in kmldocument structure
    QString                     m_imageDirBasename;
    QString                     m_GPXFile;
    QString                     m_UrlDestDir;

    /**
     * Temporary directory where everything will be created.
     * m_imageDir is nested in m_tempDestDir.
     */
    QDir                        m_tempDestDir;
    QDir                        m_imageDir;

    /// Directory selected by user
    QString                     m_baseDestDir;

    QString                     m_imgdir;
    QString                     m_KMLFileName;

    QColor                      m_GPXColor;

    QList<QUrl>                 m_urls;
    DInfoInterface*             m_iface                 = nullptr;
    DMetadata*                  m_meta                  = nullptr;

    /// the root document, used to create all QDomElements
    QDomDocument*               m_kmlDocument           = nullptr;

    /// the GPS parsed data
    KMLGeoDataParser            m_gpxParser;

    /// To store errors and warnings while processing.
    QStringList                 m_logData;
};

} // namespace DigikamGenericGeolocationEditPlugin
