/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-07-07
 * Description : a tool to export images to Flickr web service
 *
 * SPDX-FileCopyrightText: 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Luka Renko <lure at kubuntu dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "flickrwidget.h"

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
#include "flickrlist.h"
#include "dtextedit.h"

using namespace Digikam;

namespace DigikamGenericFlickrPlugin
{

class Q_DECL_HIDDEN FlickrWidget::Private
{
public:

    Private() = default;

    QString                 serviceName;
    QPushButton*            removeAccount               = nullptr;
    QPushButton*            extendedTagsButton          = nullptr;
    QPushButton*            extendedPublicationButton   = nullptr;

    QCheckBox*              exportHostTagsCheckBox      = nullptr;
    QCheckBox*              stripSpaceTagsCheckBox      = nullptr;
    QCheckBox*              addExtraTagsCheckBox        = nullptr;
    QCheckBox*              familyCheckBox              = nullptr;
    QCheckBox*              friendsCheckBox             = nullptr;
    QCheckBox*              publicCheckBox              = nullptr;

    QGroupBox*              extendedTagsBox             = nullptr;
    QGroupBox*              extendedPublicationBox      = nullptr;

    DTextEdit*              tagsLineEdit                = nullptr;

    WSComboBoxIntermediate* contentTypeComboBox         = nullptr;
    WSComboBoxIntermediate* safetyLevelComboBox         = nullptr;

    FlickrList*             imglst                      = nullptr;
};

} // namespace DigikamGenericFlickrPlugin
