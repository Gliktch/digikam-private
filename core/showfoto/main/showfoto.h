/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : stand alone digiKam image editor GUI
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2006 by Tom Albers <tomalbers at kde dot nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>

// Local includes

#include "editorwindow.h"
#include "showfotoiteminfo.h"

namespace ShowFoto
{

class Showfoto : public Digikam::EditorWindow
{
    Q_OBJECT

public:

    explicit Showfoto(const QList<QUrl>& urlList, QWidget* const parent = nullptr);
    ~Showfoto()                                             override;

public:

    DInfoInterface* infoIface(DPluginAction* const ac)      override;

    virtual void show();

private:

    bool queryClose()                                       override;

    Digikam::ThumbBarDock* thumbBar()                 const override;
    Digikam::Sidebar*      rightSideBar()             const override;

private Q_SLOTS:

    void slotForward()                                      override;
    void slotBackward()                                     override;
    void slotLast()                                         override;
    void slotFirst()                                        override;
    void slotShowfotoItemInfoActivated(const ShowfotoItemInfo& info);
    void slotClearThumbBar();
    void slotRemoveItemInfos(const QList<ShowfotoItemInfo>& infos);

    void slotChanged()                                      override;
    void slotUpdateItemInfo()                               override;

    void slotOnlineVersionCheck()                           override;

Q_SIGNALS:

    void signalInfoList(const ShowfotoItemInfoList&);

// -- Internal IO file methods implemented in showfoto_iofiles.cpp ----------------------------------------

private:

    bool save()                                             override;
    bool saveAs()                                           override;
    void moveFile()                                         override;
    void finishSaving(bool success)                         override;
    QUrl saveDestinationUrl()                               override;
    bool saveNewVersion()                                   override;
    bool saveCurrentVersion()                               override;
    bool saveNewVersionAs()                                 override;
    bool saveNewVersionInFormat(const QString&)             override;

    void saveIsComplete()                                   override;
    void saveAsIsComplete()                                 override;
    void saveVersionIsComplete()                            override;

private Q_SLOTS:

    void slotRemoveImageFromAlbum(const QUrl& url);
    void slotDeleteCurrentItem()                            override;

    void slotPrepareToLoad()                                override;
    void slotLoadingStarted(const QString& filename)        override;
    void slotLoadingFinished(const QString& filename,
                             bool success)                  override;
    void slotSavingStarted(const QString& filename)         override;

    void slotRevert()                                       override;

Q_SIGNALS:

    void signalLoadCurrentItem(const QList<QUrl>& urlList);

// -- Internal open file methods implemented in showfoto_open.cpp ----------------------------------------

private:

    void openFolder(const QUrl& url);
    void openUrls(const QList<QUrl>& urls);

private Q_SLOTS:

    void slotFileWithDefaultApplication()                   override;
    void slotOpenWith(QAction* action = nullptr)            override;
    void slotOpenFile();
    void slotOpenFolder();
    void slotOpenFolderFromPath(const QString& path);
    void slotOpenUrl(const ShowfotoItemInfo& info);
    void slotAddedDropedItems(QDropEvent*)                  override;
    void slotDroppedUrls(const QList<QUrl>& droppedUrls, bool dropped, const QUrl& current);
    void slotOpenFilesfromPath(const QStringList& files, const QString& current);
    void slotAppendFilesfromPath(const QStringList& files, const QString& current);

Q_SIGNALS:

    void signalOpenFolder(const QUrl&);
    void signalOpenFile(const QList<QUrl>& urls);

// -- Internal setup methods implemented in showfoto_config.cpp ----------------------------------------

public Q_SLOTS:

    void slotSetup()                                        override;
    void slotSetupICC()                                     override;

private Q_SLOTS:

    void slotThemeChanged();

private:

    bool setup(bool iccSetupPage = false);
    void applySettings();

    /**
     * Determine sort ordering for the entries from the Showfoto settings.
     */
    void applySortSettings();

    void readSettings()                                     override;
    void saveSettings()                                     override;

private Q_SLOTS:

    void slotSetupMetadataFilters(int);
    void slotSetupExifTool();

// -- Internal setup methods implemented in showfoto_setup.cpp ----------------------------------------

private:

    void setupActions()                                     override;
    void setupConnections()                                 override;
    void setupUserArea()                                    override;

    void toggleActions(bool val)                            override;
    void toggleNavigation(int index);
    void addServicesMenu()                                  override;

private Q_SLOTS:

    void slotContextMenu()                                  override;

// -- Import tools methods implemented in showfoto_import.cpp -------------------------------------

private Q_SLOTS:

    void slotImportedImagefromScanner(const QUrl& url);

// -- Internal private container --------------------------------------------------------------------

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace ShowFoto
