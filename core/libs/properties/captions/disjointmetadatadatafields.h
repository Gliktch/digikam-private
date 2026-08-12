/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-08-17
 * Description : Helper class for Image Description Editor Tab
 *
 * SPDX-FileCopyrightText: 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "disjointmetadata.h"

// Qt includes

#include <QMap>
#include <QList>
#include <QDateTime>
#include <QStringList>

// Local includes

#include "captionvalues.h"
#include "template.h"

namespace Digikam
{

/**
 * This class was split from DisjointMetadata::Private to allow to use the automatic C++ copy constructor
 * (DisjointMetadata::Private contains a QMutex and is thus non-copyable)
 */
class DisjointMetadataDataFields
{

public:

    /**
     * The status enum describes the result of joining several metadata sets.
     * If only one set has been added, the status is always MetadataAvailable.
     * If no set has been added, the status is always MetadataInvalid
     */
    enum Status
    {
        MetadataInvalid,   ///< Not yet filled with any value
        MetadataAvailable, ///< Only one data set has been added, or a common value is available
        MetadataDisjoint   ///< No common value is available. For rating and dates, the interval is available.
    };

public:

    DisjointMetadataDataFields() = default;

    bool              dateTimeChanged   = false;
    bool              titlesChanged     = false;
    bool              commentsChanged   = false;
    bool              pickLabelChanged  = false;
    bool              colorLabelChanged = false;
    bool              ratingChanged     = false;
    bool              templateChanged   = false;
    bool              tagsChanged       = false;
    bool              withoutTags       = false;

    int               pickLabel         = -1;
    int               highestPickLabel  = -1;
    int               colorLabel        = -1;
    int               highestColorLabel = -1;
    int               rating            = -1;
    int               highestRating     = -1;
    int               count             = 0;

    QDateTime         dateTime;
    QDateTime         lastDateTime;

    CaptionsMap       titles;
    CaptionsMap       comments;

    Template          metadataTemplate;

    QMap<int, Status> tags;

    QStringList       tagList;

    Status            dateTimeStatus    = MetadataInvalid;
    Status            titlesStatus      = MetadataInvalid;
    Status            commentsStatus    = MetadataInvalid;
    Status            pickLabelStatus   = MetadataInvalid;
    Status            colorLabelStatus  = MetadataInvalid;
    Status            ratingStatus      = MetadataInvalid;
    Status            templateStatus    = MetadataInvalid;

    QList<int>        tagIds;
    bool              invalid           = false;
};

} // namespace Digikam
