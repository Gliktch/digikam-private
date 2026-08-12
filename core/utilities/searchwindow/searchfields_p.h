/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-01-20
 * Description : User interface for searches
 *
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "searchfields.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QApplication>
#include <QGridLayout>
#include <QTimeEdit>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QStyle>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "coredb.h"
#include "squeezedcombobox.h"
#include "albummanager.h"
#include "albummodel.h"
#include "dexpanderbox.h"
#include "dlayoutbox.h"
#include "albumselectcombobox.h"
#include "choicesearchutilities.h"
#include "dimg.h"
#include "dmetadata.h"
#include "ddateedit.h"
#include "tagtreeview.h"
#include "ratingsearchutilities.h"
#include "searchfieldgroup.h"
#include "searchwindow.h"
#include "tagscache.h"
#include "colorlabelfilter.h"
#include "picklabelfilter.h"
#include "applicationsettings.h"
#include "itempropertiestab.h"
