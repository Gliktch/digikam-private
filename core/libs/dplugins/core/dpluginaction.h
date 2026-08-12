/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : action container for external plugin
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QAction>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DPluginAction : public QAction
{
    Q_OBJECT

public:

    /// Plugin action types to resume where they can be used.
    enum ActionType
    {
        InvalidType   = -1,     ///< An invalid action category.

        Generic       = 0,      ///< Generic action available everywhere (AlbumView, Editor, LightTable, and Survey).
        Editor                  ///< Specific action for Image Editor and Showfoto.
    };

    /// Plugin action categories.
    enum ActionCategory
    {
        InvalidCat    = -1,

        GenericExport = 0,      ///< Generic export action.
        GenericImport,          ///< Generic import action.
        GenericTool,            ///< Generic processing action.
        GenericMetadata,        ///< Generic Metadata adjustment action.
        GenericView,            ///< Generic View action (as Slideshow).

        EditorFile,             ///< Image Editor file action.
        EditorColors,           ///< Image Editor color correction action.
        EditorEnhance,          ///< Image Editor enhance action.
        EditorTransform,        ///< Image Editor transform action.
        EditorDecorate,         ///< Image Editor decorate action.
        EditorFilters           ///< Image Editor special effects action.
    };

    /// Plugin action types via QAction data container.
    enum PluginActionData
    {
        NoData        = 0,
        AlbumData
    };

public:

    explicit DPluginAction(QObject* const parent = nullptr);
    ~DPluginAction() override = default;

    /**
     * Manage the internal action category.
     */
    void setActionCategory(ActionCategory cat);
    ActionCategory actionCategory()  const;
    QString actionCategoryToString() const;

    /**
     * Return the action type depending of category.
     */
    ActionType actionType()          const;

    /**
     * Return the plugin id string hosting this action.
     */
    QString pluginId()               const;

    /**
     * Return the XML section to merge in KXMLGUIClient host XML definition.
     */
    QString xmlSection()             const;

    /**
     * Return details as string about action properties.
     * For debug purpose only.
     */
    QString toString()               const;

    static bool pluginActionLessThan(DPluginAction* const a, DPluginAction* const b);
};

} // namespace Digikam
