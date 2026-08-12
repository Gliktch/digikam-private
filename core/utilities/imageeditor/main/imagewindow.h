/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-02-12
 * Description : digiKam image editor GUI
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QCloseEvent>
#include <QString>
#include <QUrl>

// Local includes

#include "editorwindow.h"
#include "iteminfo.h"

class QDragMoveEvent;
class QDropEvent;

namespace Digikam
{

class CollectionImageChangeset;

class ImageWindow : public EditorWindow
{
    Q_OBJECT

public:

    ~ImageWindow()                                                      override;

    static ImageWindow* imageWindow();
    static bool         imageWindowCreated();

    bool queryClose()                                                   override;
    void toggleTag(int tagID);

public:

    DInfoInterface* infoIface(DPluginAction* const ac)                  override;

    VersionManager* versionManager()                              const override;

public Q_SLOTS:

    void loadItemInfos(const ItemInfoList& imageInfoList,
                       const ItemInfo& imageInfoCurrent,
                       const QString& caption);
    void openImage(const ItemInfo& info);

    void slotAssignPickLabel(int pickId);
    void slotAssignColorLabel(int colorId);
    void slotAssignRating(int rating);

Q_SIGNALS:

    void signalURLChanged(const QUrl& url);
    void signalSavingDialogProgress(float value);

protected:

    void showEvent(QShowEvent* e)                                       override;
    void dropEvent(QDropEvent* e)                                       override;
    void closeEvent(QCloseEvent* e)                                     override;
    void dragMoveEvent(QDragMoveEvent* e)                               override;

private:

    /// @note disabled
    ImageWindow();
    ImageWindow(QWidget*);

    void loadIndex(const QModelIndex& index);

    bool save()                                                         override;
    bool saveAs()                                                       override;
    bool saveNewVersion()                                               override;
    bool saveCurrentVersion()                                           override;
    bool saveNewVersionAs()                                             override;
    bool saveNewVersionInFormat(const QString& format)                  override;

    QUrl saveDestinationUrl()                                           override;
    bool hasOriginalToRestore()                                         override;
    DImageHistory resolvedImageHistory(const DImageHistory& history)    override;

    void prepareImageToSave();
    void saveFaceTagsToImage(const ItemInfo& info);

    void saveIsComplete()                                               override;
    void saveAsIsComplete()                                             override;
    void saveVersionIsComplete()                                        override;

    void setViewToURL(const QUrl& url);
    void deleteCurrentItem(bool ask, bool permanently);
    void removeCurrent();

    void assignPickLabel(const ItemInfo& info, int pickId);
    void assignColorLabel(const ItemInfo& info, int colorId);
    void assignRating(const ItemInfo& info, int rating);
    void toggleTag(const ItemInfo& info, int tagID);

    ThumbBarDock* thumbBar()                                      const override;
    Sidebar*      rightSideBar()                                  const override;

Q_SIGNALS: // private signals

    void loadCurrentLater();

private Q_SLOTS:

    void slotLoadItemInfosStage2();
    void slotThumbBarModelReady();

    void slotForward()                                                  override;
    void slotBackward()                                                 override;
    void slotFirst()                                                    override;
    void slotLast()                                                     override;
    void slotFileWithDefaultApplication()                               override;

    void slotToMainWindow();

    void slotThumbBarImageSelected(const ItemInfo&);
    void slotLoadCurrent();
    void slotDeleteCurrentItem() override;
    void slotDeleteCurrentItemPermanently();
    void slotDeleteCurrentItemPermanentlyDirectly();
    void slotTrashCurrentItemDirectly();

    void slotChanged()                                                  override;
    void slotUpdateItemInfo()                                           override;
    void slotFileOriginChanged(const QString&)                          override;

    void slotRevert()                                                   override;
    void slotOpenOriginal()                                             override;

    void slotAssignTag(int tagID);
    void slotRemoveTag(int tagID);

    void slotRatingChanged(const QUrl&, int);
    void slotColorLabelChanged(const QUrl&, int);
    void slotPickLabelChanged(const QUrl&, int);
    void slotToggleTag(const QUrl&, int);

    void slotFileMetadataChanged(const QUrl&);
/*
    void slotCollectionImageChange(const CollectionImageChangeset&);
    void slotRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
*/
    void slotDroppedOnThumbbar(const QList<ItemInfo>& infos);

    void slotComponentsInfo()                                           override;
    void slotDBStat()                                                   override;
    void slotOnlineVersionCheck()                                       override;

    void slotAddedDropedItems(QDropEvent*)                              override;
    void slotOpenWith(QAction* action = nullptr)                        override;

    void slotRightSideBarActivateTitles();
    void slotRightSideBarActivateComments();
    void slotRightSideBarActivateAssignedTags();

// -- Internal setup methods implemented in imagewindow_config.cpp ----------------------------------------

public Q_SLOTS:

    void slotSetup()                                                    override;
    void slotSetupICC()                                                 override;

    void slotSetupChanged();

// -- Internal setup methods implemented in imagewindow_setup.cpp ----------------------------------------

private:

    void setupActions()                                                 override;
    void setupConnections()                                             override;
    void setupUserArea()                                                override;

    void addServicesMenu()                                              override;

private Q_SLOTS:

    void slotContextMenu()                                              override;

// -- Import tools methods implemented in imagewindow_import.cpp -------------------------------------

private Q_SLOTS:

    void slotImportedImagefromScanner(const QUrl& url);

// -- Internal private container --------------------------------------------------------------------

private:

    static ImageWindow* m_instance;

    class Private;
    Private* const      d = nullptr;
};

} // namespace Digikam
