/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-12
 * Description : Widget for assignment and confirmation of names for faces
 *
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "assignnamewidget.h"

// Qt includes

#include <QGridLayout>
#include <QKeyEvent>
#include <QToolButton>
#include <QApplication>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dexpanderbox.h"
#include "digikam_debug.h"
#include "addtagscombobox.h"
#include "addtagslineedit.h"
#include "album.h"
#include "albummanager.h"
#include "albumtreeview.h"
#include "facetagsiface.h"
#include "facetags.h"
#include "dimg.h"
#include "iteminfo.h"
#include "thememanager.h"
#include "applicationsettings.h"

namespace Digikam
{

class Q_DECL_HIDDEN AssignNameWidget::Private
{
public:

    explicit Private(AssignNameWidget* const qq);

    bool         isValid()                                        const;

    void         updateModes();
    void         updateContents();

    /**
     * Reject button shows different Tooltips and icons for
     * Unconfirmed and Unknown faces, however both
     * are of type UnconfirmedEditMode.
     * This method is responsible for the Tooltip updates.
     */
    void         updateRejectButton();

    void         updateIgnoreButton();

private:

    void         clearWidgets();
    void         checkWidgets();
    void         updateLayout();
    void         updateVisualStyle();

    QToolButton* createToolButton(const QIcon& icon,
                                  const QString& text,
                                  const QString& tip = QString()) const;

    QWidget* addTagsWidget()                                      const;

    template <class T> void setupAddTagsWidget(T* const widget);
    template <class T> void setAddTagsWidgetContents(T* const widget);

    void layoutAddTagsWidget(bool exceedBounds, int minimumContentsLength);
    void setSizePolicies(QSizePolicy::Policy h, QSizePolicy::Policy v);
    void setToolButtonStyles(Qt::ToolButtonStyle style);
    QString styleSheetFontDescriptor(const QFont& font)           const;

public:

    ItemInfo                   info;
    QVariant                   faceIdentifier;
    AlbumPointer<TAlbum>       currentTag;

    Mode                       mode             = InvalidMode;
    LayoutMode                 layoutMode       = InvalidLayout;
    VisualStyle                visualStyle      = InvalidVisualStyle;
    TagEntryWidgetMode         widgetMode       = InvalidTagEntryWidgetMode;

    AddTagsComboBox*           comboBox         = nullptr;
    AddTagsLineEdit*           lineEdit         = nullptr;
    QToolButton*               confirmButton    = nullptr;
    QToolButton*               rejectButton     = nullptr;
    QToolButton*               ignoreButton     = nullptr;
    DClickLabel*               clickLabel       = nullptr;

    bool                       modelsGiven      = false;
    TagModel*                  tagModel         = nullptr;
    CheckableAlbumFilterModel* tagFilterModel   = nullptr;
    TagPropertiesFilterModel*  tagFilteredModel = nullptr;
    AlbumPointer<TAlbum>       parentTag;

    QGridLayout*               layout           = nullptr;

    AssignNameWidget* const    q                = nullptr;
};

} // namespace Digikam
