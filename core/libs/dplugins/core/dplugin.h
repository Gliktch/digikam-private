/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : abstract class to define digiKam plugin
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMap>
#include <QList>
#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QtPlugin>
#include <QObject>
#include <QIcon>
#include <QPointer>

// Local includes

#include "dpluginauthor.h"
#include "digikam_export.h"

/**
 * The plugin interface identifier definitions shared between DPlugins and Loader to
 * check the binary compatibility at run-time.
 */
#define DIGIKAM_DPLUGIN_GENERIC_IID   "org.kde.digikam.DPluginGeneric/1.1.0"
#define DIGIKAM_DPLUGIN_EDITOR_IID    "org.kde.digikam.DPluginEditor/1.1.0"
#define DIGIKAM_DPLUGIN_BQM_IID       "org.kde.digikam.DPluginBqm/1.1.0"
#define DIGIKAM_DPLUGIN_RAWIMPORT_IID "org.kde.digikam.DPluginRawImport/1.1.0"
#define DIGIKAM_DPLUGIN_DIMG_IID      "org.kde.digikam.DPluginDImg/1.1.0"

namespace Digikam
{

/**
 * A digiKam external plugin abstract class.
 */
class DIGIKAM_EXPORT DPlugin : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor with optional parent object
     */
    explicit DPlugin(QObject* const parent = nullptr);

    /**
     * Destructor
     */
    ~DPlugin() override;

public:

    /**
     * Return a list of authors as strings registered in this plugin.
     */
    QStringList pluginAuthors() const;

    /**
     * Return the internal version used to check the binary compatibility at run-time.
     * This is typically the same version of digiKam core used at compilation time.
     */
    QString version() const;

    /**
     * Return the should loaded property.
     * If it's true, the plugin must be loaded in application GUI at startup by plugin loader.
     */
    bool shouldLoaded() const;

    /**
     * Accessor to adjust the should loaded plugin property.
     * This property is adjusted by plugin loader at start-up.
     */
    void setShouldLoaded(bool b);

    /**
     * @brief Returns the file name of the library for this plugin.
     *        This string is filled at run-time by plugin loader.
     */
    QString libraryFileName() const;

    /**
     * @brief Sets the file name of the library for this plugin.
     *        This string is filled at run-time by plugin loader.
     */
    void setLibraryFileName(const QString&);

public:

    /**
     * Plugin method to clean up internal created objects.
     * This method is called by plugin loader.
     */
    virtual void cleanUp() {};

    /**
     * Return the online handbook section corresponding to this plugin.
     * It's used in plugin dialog Help button.
     * By default, no section is defined, and root page of the documentation is loaded by Help Button in this case.
     */
    virtual QString handbookSection() const;

    /**
     * Return the online handbook chapter from an handbook section corresponding to this plugin.
     * It's used in plugin dialog Help button.
     * By default, no chapter is defined, and root page of the section is loaded by Help Button in this case.
     * Note: a chapter is always included in a section. See handbookSection() for details.
     */
    virtual QString handbookChapter() const;

    /**
     * Return the online handbook reference from an handbook chapter corresponding to this plugin.
     * It's used in plugin dialog Help button.
     * By default, no reference is defined, and root page of the chapter is loaded by Help Button in this case.
     * Note: a reference is always included in a chapter. See handbookChapter() for details.
     */
    virtual QString handbookReference() const;

    /**
     * Return true if plugin can be configured in setup dialog about the visibility property.
     * Default implementation return true.
     */
    virtual bool hasVisibilityProperty() const;

    /**
     * Holds whether the plugin can be seen in parent view.
     */
    virtual void setVisible(bool b) = 0;

    /**
     * Return the amount of tools registered to all parents.
     */
    virtual int count() const = 0;

    /**
     * Return a list of categories as strings registered in this plugin.
     */
    virtual QStringList categories() const = 0;

    /**
     * Plugin factory method to create all internal object instances for a given parent.
     */
    virtual void setup(QObject* const parent) = 0;

    /**
     * @brief Returns the user-visible name of the plugin.
     *
     * The user-visible name should be context free, i.e. the name should
     * provide enough information as to what the plugin is about in the context
     * of digiKam.
     */
    virtual QString name() const = 0;

    /**
     * @brief Returns the unique internal identification property of the plugin.
     *        Must be formatted as "org.kde.digikam.plugin._PLUGIN_TYPE_._NAME_OF_PLUGIN_".
     *        Examples: "org.kde.digikam.plugin.generic.Calendar"
     *                  "org.kde.digikam.plugin.editor.AdjustCurvesTool"
     *                  "org.kde.digikam.plugin.bqm.NoiseReduction"
     */
    virtual QString iid() const = 0;

    /**
     * @brief Returns the unique top level internal identification property of the plugin interface.
     *        Must be formatted as "org.kde.digikam._NAME_OF_INTERFACE_/_VERSION_".
     *        Examples: "org.kde.digikam.DPluginGeneric/1.1.0"
     *                  "org.kde.digikam.DPluginEditor/1.1.0"
     *                  "org.kde.digikam.DPluginBqm/1.1.0"
     */
    virtual QString ifaceIid() const = 0;

    /**
     * @brief Returns a short description about the plugin.
     */
    virtual QString description() const = 0;

    /**
     * @brief Returns an icon for the plugin.
     *        Default implementation return the system plugin icon.
     */
    virtual QIcon icon() const;

    /**
     * @brief Returns authors list for the plugin.
     */
    virtual QList<DPluginAuthor> authors() const = 0;

    /**
     * @brief Returns a long description about the plugin.
     */
    virtual QString details() const = 0;

    /**
     * @brief Returns a map of extra data to show in plugin about dialog.
     */
    virtual QMap<QString, QStringList> extraAboutData() const { return QMap<QString, QStringList>(); };

    /**
     * @brief Returns a list of extra data row titles to show in tab of plugin about dialog.
     */
    virtual QStringList extraAboutDataRowTitles() const { return QStringList(); };

    /**
     * @brief Returns the tab title of data returned by extraAboutData().
     */
    virtual QString extraAboutDataTitle() const { return QString(); };

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

Q_DECLARE_TYPEINFO(Digikam::DPluginAuthor, Q_MOVABLE_TYPE);
