/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-06-05
 * Description : DB Jobs Info
 *
 * SPDX-FileCopyrightText: 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dbjobinfo.h"

namespace Digikam
{

void DBJobInfo::setFoldersJob()
{
    m_folders = true;
}

bool DBJobInfo::isFoldersJob() const
{
    return m_folders;
}

void DBJobInfo::setListAvailableImagesOnly()
{
    m_listAvailableImagesOnly = true;
}

bool DBJobInfo::isListAvailableImagesOnly() const
{
    return m_listAvailableImagesOnly;
}

void DBJobInfo::setRecursive()
{
    m_recursive = true;
}

bool DBJobInfo::isRecursive() const
{
    return m_recursive;
}

// ---------------------------------------------

AlbumsDBJobInfo::AlbumsDBJobInfo()
    : DBJobInfo()
{
}

void AlbumsDBJobInfo::setAlbumRootId(int id)
{
    m_albumRootId = id;
}

int AlbumsDBJobInfo::albumRootId()
{
    return m_albumRootId;
}

void AlbumsDBJobInfo::setAlbum(const QString& album)
{
    m_album = album;
}

QString AlbumsDBJobInfo::album()
{
    return m_album;
}

// ---------------------------------------------

TagsDBJobInfo::TagsDBJobInfo()
    : DBJobInfo()
{
}

void TagsDBJobInfo::setFaceFoldersJob()
{
    m_faceFolders = true;
}

bool TagsDBJobInfo::isFaceFoldersJob() const
{
    return m_faceFolders;
}

void TagsDBJobInfo::setSpecialTag(const QString& tag)
{
    m_specialTag = tag;
}

QString TagsDBJobInfo::specialTag() const
{
    return m_specialTag;
}

void TagsDBJobInfo::setTagsIds(const QList<int>& tagsIds)
{
    m_tagsIds = tagsIds;
}

QList<int> TagsDBJobInfo::tagsIds() const
{
    return m_tagsIds;
}

// ---------------------------------------------

GPSDBJobInfo::GPSDBJobInfo()
    : DBJobInfo()
{
}

void GPSDBJobInfo::setDirectQuery()
{
    m_directQuery = true;
}

bool GPSDBJobInfo::isDirectQuery() const
{
    return m_directQuery;
}

void GPSDBJobInfo::setLat1(qreal lat)
{
    m_lat1 = lat;
}

qreal GPSDBJobInfo::lat1() const
{
    return m_lat1;
}

void GPSDBJobInfo::setLng1(qreal lng)
{
    m_lng1 = lng;
}

qreal GPSDBJobInfo::lng1() const
{
    return m_lng1;
}

void GPSDBJobInfo::setLat2(qreal lat)
{
    m_lat2 = lat;
}

qreal GPSDBJobInfo::lat2() const
{
    return m_lat2;
}

void GPSDBJobInfo::setLng2(qreal lng)
{
    m_lng2 = lng;
}

qreal GPSDBJobInfo::lng2() const
{
    return m_lng2;
}

// ---------------------------------------------

SearchesDBJobInfo::SearchesDBJobInfo(QList<int>&& searchIds)
    : DBJobInfo  (),
      m_searchIds(std::move(searchIds))
{
}

SearchesDBJobInfo::SearchesDBJobInfo(QSet<qlonglong>&& imageIds,
                                     bool isAlbumUpdate,
                                     HaarIface::RefImageSelMethod referenceSelectionMethod,
                                     QSet<qlonglong>&& refImageIds)
    : DBJobInfo                 (),
      m_duplicates              (true),
      m_albumUpdate             (isAlbumUpdate),
      m_imageIds                (std::move(imageIds)),
      m_refImageIds             (std::move(refImageIds)),
      m_refImageSelectionMethod (referenceSelectionMethod)
{
}

bool SearchesDBJobInfo::isDuplicatesJob() const
{
    return m_duplicates;
}

bool SearchesDBJobInfo::isAlbumUpdate() const
{
    return m_albumUpdate;
}

void SearchesDBJobInfo::setSearchResultRestriction(int type)
{
    m_searchResultRestriction = type;
}

int SearchesDBJobInfo::searchResultRestriction() const
{
    return m_searchResultRestriction;
}

const QList<int>& SearchesDBJobInfo::searchIds() const
{
    return m_searchIds;
}

const QSet<qlonglong>& SearchesDBJobInfo::imageIds() const
{
    return m_imageIds;
}

const QSet<qlonglong>& SearchesDBJobInfo::refImageIds() const
{
    return m_refImageIds;
}

HaarIface::RefImageSelMethod SearchesDBJobInfo::refImageSelectionMethod() const
{
    return m_refImageSelectionMethod;
}

void SearchesDBJobInfo::setMinThreshold(double t)
{
    m_minThreshold = qBound(0.4, t, 1.0);
}

double SearchesDBJobInfo::minThreshold() const
{
    return m_minThreshold;
}

void SearchesDBJobInfo::setMaxThreshold(double t)
{
    m_maxThreshold = qBound(0.4, t, 1.0);
}

double SearchesDBJobInfo::maxThreshold() const
{
    return m_maxThreshold;
}

// ---------------------------------------------

DatesDBJobInfo::DatesDBJobInfo()
    : DBJobInfo()
{
}

void DatesDBJobInfo::setStartDate(const QDate& date)
{
    m_startDate = date;
}

QDate DatesDBJobInfo::startDate() const
{
    return m_startDate;
}

void DatesDBJobInfo::setEndDate(const QDate& date)
{
    m_endDate = date;
}

QDate DatesDBJobInfo::endDate() const
{
    return m_endDate;
}

} // namespace Digikam
