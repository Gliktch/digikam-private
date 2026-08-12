/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-24
 * Description : Qt Model for Albums - filter model
 *
 * SPDX-FileCopyrightText: 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009      by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "albumfiltermodel.h"

// Qt includes

#include <QSortFilterProxyModel>
#include <QHeaderView>

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "albummodel.h"
#include "applicationsettings.h"
#include "itemsortcollator.h"
#include "facetags.h"
