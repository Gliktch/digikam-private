/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-03-20
 * Description : a tool to export images to iNaturalist web service
 *
 * SPDX-FileCopyrightText: 2021      by Joerg Lohse <joergmlpts at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "inatwidget.h"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QApplication>
#include <QStyle>
#include <QDialog>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "wscomboboxdelegate.h"
#include "wscomboboxintermediate.h"
#include "ditemslist.h"
#include "inatsuggest.h"
#include "dtextedit.h"

using namespace Digikam;

namespace DigikamGenericINatPlugin
{

extern const DItemsListView::ColumnType ItemDate;
extern const DItemsListView::ColumnType ItemLocation;

class Q_DECL_HIDDEN INatWidget::Private
{
public:

    Private() = default;

    QString                 serviceName;
    QPushButton*            removeAccount               = nullptr;
    QLabel*                 accountIcon                 = nullptr;
    QLabel*                 identificationImage         = nullptr;
    QLabel*                 identificationLabel         = nullptr;
    QLabel*                 closestKnownObservation     = nullptr;
    DPlainTextEdit*         observationDescription      = nullptr;
    TaxonEdit*              identificationEdit          = nullptr;
    SuggestTaxonCompletion* taxonPopup                  = nullptr;
    QSpinBox*               photoMaxTimeDiffSpB         = nullptr;
    QSpinBox*               photoMaxDistanceSpB         = nullptr;
    QSpinBox*               closestObservationMaxSpB    = nullptr;
    QComboBox*              placesComboBox              = nullptr;
    QPushButton*            moreOptionsButton           = nullptr;
    QWidget*                moreOptionsWidget           = nullptr;
    DItemsList*             imglst                      = nullptr;
};

} // namespace DigikamGenericINatPlugin
