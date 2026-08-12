/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : manager to load external plugins at run-time
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QList>

// Local includes

#include "digikam_export.h"
#include "dinfointerface.h"
#include "dplugin.h"
#include "dpluginaction.h"
#include "dimgloadersettings.h"

namespace Digikam
{

/**
 * @short The class that handles digiKam's external plugins.
 *
 * Ownership policy for plugins:
 *
 * The DPluginLoader creates new objects and transfer ownership.
 * In order to create the objects, the DPluginLoader internally has a list of the tools
 * which are owned by the DPluginLoader and destroyed by it.
 */
class DIGIKAM_EXPORT DPluginLoader : public QObject
{
    Q_OBJECT

 public:

    /**
     * @brief instance: returns the singleton of plugin loader
     * @return DPluginLoader global instance
     */
    static DPluginLoader* instance();

    /**
     * Return the config group name used to store the list of plugins to load at startup.
     */
    QString configGroupName() const;

    /**
     * Register all Generic plugin actions to parent object.
     */
    void registerGenericPlugins(QObject* const parent);

    /**
     * Register all Editor plugin actions to parent object.
     */
    void registerEditorPlugins(QObject* const parent);

    /**
     * Register all Raw Import plugin to parent object.
     */
    void registerRawImportPlugins(QObject* const parent);

    /**
     * Init plugin loader. Call this method to parse and load relevant plugins installed on your system.
     */
    void init();

     /**
     * Unload all loaded plugins. Call this method before the main instance is closed.
     */
    void cleanUp();

    /**
     * @brief Returns all available plugins.
     */
    QList<DPlugin*> allPlugins() const;

    /**
     * @brief Returns a list of plugin actions set as type for a given parent.
     *        If no plugin have found in this category, this returns an empty list.
     */
    QList<DPluginAction*> pluginsActions(DPluginAction::ActionType type, QObject* const parent) const;

    /**
     * @brief Returns a list of plugin actions set as category for a given parent.
     *        If no plugin have found in this category, this returns an empty list.
     */
    QList<DPluginAction*> pluginsActions(DPluginAction::ActionCategory cat, QObject* const parent) const;

    /**
     * @brief Returns the plugin actions corresponding to a plugin internal ID string for a given parent.
     *        If not found, this returns an empty list.
     */
    QList<DPluginAction*> pluginActions(const QString& pluginIID, QObject* const parent) const;

    /**
     * @brief Returns the plugin action corresponding to a action name for a given parent.
     *        If not found, this returns a null pointer.
     */
    DPluginAction* pluginAction(const QString& actionName, QObject* const parent) const;

    /**
     * @brief Returns all xml sections as string of plugin actions set with a kind of category for a given parent.
     */
    QString pluginXmlSections(DPluginAction::ActionCategory cat, QObject* const parent) const;

    /**
     * @brief appendPluginToBlackList Prevent that a plugin is loaded from the given filename
     * @param filename The name of the file excluding file extension to blacklist. E.g.
     * to ignore "HtmlGalleryPlugin.so" on Linux and "HtmlGalleryPlugin.dll" on Windows, pass "HtmlGalleryPlugin"
     */
    void appendPluginToBlackList(const QString& filename);

    /**
     * @brief appendPluginToWhiteList Add a plugin to the whitelist of tools. If the whitelist is not
     * empty, only whitelisted tools are loaded. If a tool is both whitelisted and blacklisted,
     * it will not be loaded.
     * @param filename The name of the file excluding file extension to whitelist. E.g.
     * to not ignore "HtmlGalleryPlugin.so" on Linux and "HtmlGalleryPlugin.dll" on Windows, pass "HtmlGalleryPlugin"
     */
    void appendPluginToWhiteList(const QString& filename);

    /**
     * Return true if format is supported by a DPluginDImg to import image.
     */
    bool canImport(const QString& format) const;

    /**
     * Return true if format is supported by a DPluginDImg to export image.
     */
    bool canExport(const QString& format) const;

    /**
     * Return a new widget instance from a DPluginDImg to show settings while exporting image to specified format.
     * Return nullptr if format is not supported or if no settings widget is available for this format.
     */
    DImgLoaderSettings* exportWidget(const QString& format) const;

private:

    /// @note disabled constructor and destructor
    DPluginLoader();
    explicit DPluginLoader(QObject*);
    ~DPluginLoader() override;

    Q_DISABLE_COPY(DPluginLoader)

    class Private;
    Private* const d = nullptr;

    friend class DPluginLoaderCreator;
};

} // namespace Digikam
