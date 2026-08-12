/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-11-08
 * Description : face detection, recognition, and training controller
 *
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "facesengine.h"

// Qt includes

#include <QClipboard>
#include <QVBoxLayout>
#include <QTimer>
#include <QIcon>
#include <QPushButton>
#include <QApplication>
#include <QTextEdit>
#include <QHash>
#include <QPixmap>
#include <QException>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "digikam_debug.h"
#include "digikamapp.h"
#include "dnotificationwidget.h"
#include "coredb.h"
#include "album.h"
#include "albummanager.h"
#include "albumpointer.h"
#include "iteminfojob.h"
#include "facetags.h"
#include "mlpipelinepackagenotify.h"
#include "facepipelinedetectrecognize.h"
#include "facepipelinerecognize.h"
#include "facepipelineretrain.h"
#include "facepipelinereset.h"
#include "facebackgroundrecognition.h"

namespace Digikam
{

class Q_DECL_HIDDEN BenchmarkMessageDisplay : public QWidget
{
    Q_OBJECT

public:

    explicit BenchmarkMessageDisplay(const QString& richText);

private:

    /// @note disabled
    BenchmarkMessageDisplay(QWidget*);
};

// --------------------------------------------------------------------------

class Q_DECL_HIDDEN FacesEngine::Private
{
public:

    Private() = default;

public:

    FacesEngine::InputSource    source          = FacesEngine::Albums;
    bool                        benchmark       = false;

    AlbumPointerList<>          albumTodoList;
    ItemInfoList                infoTodoList;
    QList<qlonglong>            idsTodoList;

    ItemInfoJob                 albumListing;
    FacePipelineBase*           newPipeline     = nullptr;

    int totalFacesFound                         = 0;
};

} // namespace Digikam
