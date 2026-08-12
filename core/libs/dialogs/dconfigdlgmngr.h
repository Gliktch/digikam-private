/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-11-03
 * Description : Automatic retrieving, saving and resetting skeleton based settings in a dialog.
 *
 * SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2003      by Benjamin C Meyer <ben plus kdelibs at meyerhome dot net>
 * SPDX-FileCopyrightText: 2003      by Waldo Bastian <bastian at kde dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QHash>
#include <QWidget>
#include <QByteArray>

// Local includes

#include "digikam_export.h"

class KConfigSkeleton;
class KConfigSkeletonItem;

namespace Digikam
{

/**
 * The DConfigDlgMngr class provides a means of automatically
 * retrieving, saving and resetting basic settings.
 * It also can emit signals when settings have been changed
 * (settings were saved) or modified (the user changes a checkbox
 * from on to off).
 *
 * The object names of the widgets to be managed have to correspond to the names of the
 * configuration entries in the KConfigSkeleton object plus an additional
 * "kcfg_" prefix. For example a widget with the object name "kcfg_MyOption"
 * would be associated to the configuration entry "MyOption".
 *
 * The widget classes of Qt are supported out of the box.
 *
 * Custom widget classes are supported if they have a Q_PROPERTY defined for the
 * property representing the value edited by the widget. By default the property
 * is used for which "USER true" is set. For using another property, see below.
 */
class DIGIKAM_EXPORT DConfigDlgMngr : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor.
     * @param parent Dialog widget to manage
     * @param conf Object that contains settings
     */
    DConfigDlgMngr(QWidget* const parent, KConfigSkeleton* const conf);

    /**
     * Destructor.
     */
    ~DConfigDlgMngr()                                                      override;

    /**
     * Add additional widgets to manage
     * @param widget Additional widget to manage, including all its children
     */
    void addWidget(QWidget* const widget);

    /**
     * Returns whether the current state of the known widgets are
     * different from the state in the config object.
     */
    bool hasChanged()                                                const;

    /**
     * Returns whether the current state of the known widgets are
     * the same as the default state in the config object.
     */
    bool isDefault()                                                 const;

public:

    /**
     * Retrieve the map between widgets class names and the
     * USER properties used for the configuration values.
     */
    static QHash<QString, QByteArray>* propertyMap();

    /**
     * Retrieve the map between widgets class names and signals that are listened
     * to detect changes in the configuration values.
     */
    static QHash<QString, QByteArray>* changedMap();

Q_SIGNALS:

    /**
     * One or more of the settings have been saved (such as when the user
     * clicks on the Apply button).  This is only emitted by updateSettings()
     * whenever one or more setting were changed and consequently saved.
     */
    void settingsChanged();

    /**
     * One or more of the settings have been changed.
     * @param widget - The widget group (pass in via addWidget()) that
     * contains the one or more modified setting.
     * @see settingsChanged()
     */
    void widgetSettingsChanged(QWidget* widget);

    /**
     * If retrieveSettings() was told to track changes then if
     * any known setting was changed this signal will be emitted.  Note
     * that a settings can be modified several times and might go back to the
     * original saved state. hasChanged() will tell you if anything has
     * actually changed from the saved values.
     */
    void widgetModified();

public Q_SLOTS:

    /**
     * Traverse the specified widgets, saving the settings of all known
     * widgets in the settings object.
     *
     * Example use: User clicks Ok or Apply button in a configure dialog.
     */
    void updateSettings();

    /**
     * Traverse the specified widgets, sets the state of all known
     * widgets according to the state in the settings object.
     *
     * Example use: Initialisation of dialog.
     * Example use: User clicks Reset button in a configure dialog.
     */
    void updateWidgets();

    /**
     * Traverse the specified widgets, sets the state of all known
     * widgets according to the default state in the settings object.
     *
     * Example use: User clicks Defaults button in a configure dialog.
     */
    void updateWidgetsDefault();

protected:

    /**
     * @param trackChanges - If any changes by the widgets should be tracked
     * set true.  This causes the emitting the modified() signal when
     * something changes.
     */
    void init(bool trackChanges);

    /**
     * Recursive function that finds all known children.
     * Goes through the children of widget and if any are known and not being
     * ignored, stores them in currentGroup.  Also checks if the widget
     * should be disabled because it is set immutable.
     * @param widget - Parent of the children to look at.
     * @param trackChanges - If true then tracks any changes to the children of
     * widget that are known.
     * @return bool - If a widget was set to something other than its default.
     */
    bool parseChildren(const QWidget* widget, bool trackChanges);

    /**
     * Finds the USER property name using Qt's MetaProperty system, and caches
     * it in the property map (the cache could be retrieved by propertyMap() ).
     */
    QByteArray getUserProperty(const QWidget* widget)                const;

    /**
     * Find the property to use for a widget by querying the "kcfg_property"
     * property of the widget. Like a widget can use a property other than the
     * USER property.
     */
    QByteArray getCustomProperty(const QWidget* widget)              const;

    /**
     * Finds the changed signal of the USER property using Qt's MetaProperty system.
     */
    QByteArray getUserPropertyChangedSignal(const QWidget* widget)   const;

    /**
     * Find the changed signal of the property to use for a widget by querying
     * the "kcfg_propertyNotify" property of the widget. Like a widget can use a
     * property change signal other than the one for USER property, if there even is one.
     */
    QByteArray getCustomPropertyChangedSignal(const QWidget* widget) const;

    /**
     * Set a property
     */
    void setProperty(QWidget* w, const QVariant& v);

    /**
     * Retrieve a property
     */
    QVariant property(QWidget* w)                                    const;

    /**
     * Setup secondary widget properties
     */
    void setupWidget(QWidget* widget, KConfigSkeletonItem* item);

    /**
     * Initializes the property maps
     */
    static void initMaps();

private:

    class Private;
    Private* const d = nullptr;

    Q_DISABLE_COPY(DConfigDlgMngr)
};

} // namespace Digikam
