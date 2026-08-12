/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-05
 * Description : factory of basic models used for views in digikam
 *
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010      by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "abstractalbummodel.h"
#include "albumfiltermodel.h"
#include "albummodel.h"
#include "itemversionsmodel.h"

namespace Digikam
{

/**
 * This class is simply a factory of all models that build the core of the
 * digikam application.
 *
 * @author jwienke
 */
class DModelFactory: public QObject
{
    Q_OBJECT

public:

    DModelFactory();
    ~DModelFactory() override;

    TagModel*          getTagModel()          const;
    TagModel*          getTagFaceModel()      const;
    TagModel*          getTagFilterModel()    const;

    AlbumModel*        getAlbumModel()        const;
    SearchModel*       getSearchModel()       const;
    DateAlbumModel*    getDateAlbumModel()    const;
    ItemVersionsModel* getItemVersionsModel() const;

private Q_SLOTS:

    void slotApplicationSettingsChanged();

private:

    /// @note disabled
    explicit DModelFactory(QObject*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
