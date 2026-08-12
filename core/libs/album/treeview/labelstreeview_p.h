/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-05-17
 * Description : Album Labels Tree View - Private containers and classes.
 *
 * SPDX-FileCopyrightText: 2014-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "labelstreeview.h"

// QT includes

#include <QApplication>
#include <QPainter>
#include <QValidator>
#include <QStyledItemDelegate>
#include <QUrl>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "coredbsearchxml.h"
#include "searchtabheader.h"
#include "thememanager.h"
#include "albummanager.h"
#include "albumtreeview.h"
#include "itemlister.h"
#include "coredbaccess.h"
#include "coredb.h"
#include "colorlabelfilter.h"
#include "colorlabelwidget.h"
#include "picklabelfilter.h"
#include "tagscache.h"
#include "applicationsettings.h"
#include "dnotificationwrapper.h"
#include "digikamapp.h"
#include "tagsactionmngr.h"
#include "ratingwidget.h"
#include "dbjobsmanager.h"

namespace Digikam
{

class Q_DECL_HIDDEN ColorLabelValidator : public QValidator
{
    Q_OBJECT

public:

    explicit ColorLabelValidator(int maxLength, QObject* const parent = nullptr);

    QValidator::State validate(QString& input, int& /*pos*/)                                             const override;

private:

    int m_maxLength = 32;
};

// ---

class Q_DECL_HIDDEN ColorLabelDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:

    explicit ColorLabelDelegate(int maxLength, QObject* const parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void setEditorData(QWidget* editor, const QModelIndex& index)                                        const override;

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index)              const override;

private:

    int m_maxLength = 32;
};

// ---

class Q_DECL_HIDDEN LabelsTreeView::Private
{
public:

    explicit Private(QTreeWidget* const w);

public:

    QFont                      regularFont;
    QSize                      iconSize;

    QTreeWidgetItem*           ratings                      = nullptr;
    QTreeWidgetItem*           picks                        = nullptr;
    QTreeWidgetItem*           colors                       = nullptr;

    QTreeWidgetItemIterator    itemIterator;

    bool                       isCheckableTreeView          = false;
    bool                       isLoadingState               = false;
    int                        iconSizeFromSetting          = 0;

    QHash<Labels, QList<int> > selectedLabels;

public:

    const QString              configRatingSelectionEntry   = QLatin1String("RatingSelection");
    const QString              configPickSelectionEntry     = QLatin1String("PickSelection");
    const QString              configColorSelectionEntry    = QLatin1String("ColorSelection");
    const QString              configExpansionEntry         = QLatin1String("Expansion");
};

} // namespace Digikam
