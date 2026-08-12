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

#include <QMutexLocker>
#include <QtAlgorithms>

// Local includes

#include "templatemanager.h"
#include "facetagseditor.h"
#include "tagscache.h"
#include "coredbaccess.h"
#include "itemcomments.h"
#include "iteminfo.h"
#include "coredbwatch.h"
#include "metadatahub.h"

namespace Digikam
{

class Q_DECL_HIDDEN DisjointMetadata::Private : public DisjointMetadataDataFields
{
public:

    Private();

    // use the automatic copy constructor

    explicit Private(const DisjointMetadataDataFields& other);
    explicit Private(const Private& other);

public:

    void makeConnections(DisjointMetadata* const q);

    template <class T>
    void loadSingleValue(const T& data,
                         T& storage,
                         DisjointMetadataDataFields::Status& status)
    {
        switch (status)
        {
            case DisjointMetadataDataFields::MetadataInvalid:
            {
                storage = data;
                status  = DisjointMetadataDataFields::MetadataAvailable;

                break;
            }

            case DisjointMetadataDataFields::MetadataAvailable:
            {
                // we have two values. If they are equal, status is unchanged

                if (data == storage)
                {
                    break;
                }

                // they are not equal. We need to enter the disjoint state.

                status = DisjointMetadataDataFields::MetadataDisjoint;

                break;
            }

            case DisjointMetadataDataFields::MetadataDisjoint:
            {
                break;
            }
        }
    };

public:

    QMutex mutex;
};

} // namespace Digikam
