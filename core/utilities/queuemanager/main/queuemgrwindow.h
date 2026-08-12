/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-21
 * Description : Batch Queue Manager GUI
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMap>
#include <QString>
#include <QCloseEvent>
#include <QUrl>

// Local includes

#include "iteminfo.h"
#include "setup.h"
#include "dhistoryview.h"
#include "dxmlguiwindow.h"

class QAction;

namespace Digikam
{

class ActionData;
class BatchToolsFactory;
class AssignedBatchTools;
class QueueListViewItem;
class QueuePool;

class QueueMgrWindow : public DXmlGuiWindow
{
    Q_OBJECT

public:

    ~QueueMgrWindow()                                     override;

    static QueueMgrWindow* queueManagerWindow();
    static bool            queueManagerWindowCreated();

    void addNewQueue();
    void loadItemInfos(const ItemInfoList& list, int queueId);
    void loadItemInfosToCurrentQueue(const ItemInfoList& list);
    void loadItemInfosToNewQueue(const ItemInfoList& list);
    void refreshView();
    void applySettings();

    /**
     * Return a map of all queues available from pool (index and title).
     */
    QMap<int, QString> queuesMap()                  const;

    bool isBusy()                                   const;
    int  currentQueueId()                           const;

    bool queryClose()                                     override;

    DInfoInterface* infoIface(DPluginAction* const)       override;

    QueuePool* queuePool()                          const;

Q_SIGNALS:

    void signalWindowHasMoved();
    void signalBqmIsBusy(bool);

protected:

    void moveEvent(QMoveEvent* e)                         override;

public Q_SLOTS:

    void slotRun();
    void slotRunAll();
    void slotStop();
    void slotAssignQueueSettings(const QString&);
    void slotUpdateQueueSettings(const QString&);

private:

    void customizedFullScreenMode(bool set)               override;
    void closeEvent(QCloseEvent* e)                       override;
    void setupActions();
    void setupConnections();
    void setupUserArea();
    void setupStatusBar();
    void readSettings();
    void writeSettings();
    void refreshStatusBar();
    void populateToolsList();
    void setup(Setup::Page page);
    void addHistoryMessage(QueueListViewItem* const cItem,
                           const QString& msg,
                           DHistoryView::EntryType type);

    bool checkTargetAlbum(int queueId);
    void busy(bool busy);
    void processOneQueue();
    void processingAborted();

    QueueMgrWindow();

private Q_SLOTS:

    void slotSetup()                                      override;
    void slotComponentsInfo()                             override;
    void slotDBStat()                                     override;
    void slotOnlineVersionCheck()                         override;
    void slotAction(const Digikam::ActionData&);
    void slotHistoryEntryClicked(int, qlonglong);
    void slotAssignedToolsChanged(const AssignedBatchTools&);
    void slotQueueContentsChanged();
    void slotItemSelectionChanged();
    void slotQueueProcessed();
    void slotSaveWorkflow();

private:

    /// @note disabled
    QueueMgrWindow(QWidget*) = delete;

private:

    class Private;
    Private* const d = nullptr;

    static QueueMgrWindow* m_instance;
};

} // namespace Digikam
