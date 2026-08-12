/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-07-13
 * Description : Modified context menu helper for import tool
 *
 * SPDX-FileCopyrightText: 2012      by Islam Wazery <wazery at ubuntu dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "importcontextmenu.h"

// Qt includes

#include <QDir>
#include <QIcon>
#include <QAction>

// KDE includes

// Pragma directives to reduce warnings from KDE header files.
#if defined(Q_CC_GNU)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wmismatched-tags"
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <klocalizedstring.h>
#include <kactioncollection.h>

#ifdef HAVE_KIO
#   include <kopenwithdialog.h>
#endif

// Restore warnings
#if defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Local includes

#include "digikam_debug.h"
#include "importui.h"
#include "picklabelwidget.h"
#include "colorlabelwidget.h"
#include "ratingwidget.h"
#include "tagmodificationhelper.h"
#include "tagspopupmenu.h"
#include "fileactionmngr.h"
#include "dservicemenu.h"

#ifdef Q_OS_WIN
#   include <windows.h>
#   include <shellapi.h>
#endif

namespace Digikam
{

class Q_DECL_HIDDEN ImportContextMenuHelper::Private
{
public:

    explicit Private(ImportContextMenuHelper* const qq)
        : q(qq)
    {
    }

    QList<qlonglong>             selectedIds;
    QList<QUrl>                  selectedItems;

    QMap<int, QAction*>          queueActions;
    QMap<QString, KService::Ptr> servicesMap;
    QMap<QString, DServiceInfo>  newServicesMap;

    ImportFilterModel*           importFilterModel   = nullptr;

    QMenu*                       parent              = nullptr;
    QMenu*                       ABCmenu             = nullptr;

    KActionCollection*           stdActionCollection = nullptr;

    ImportContextMenuHelper*     q                   = nullptr;

#ifdef Q_OS_WIN

    QString                      lpVerbWin;
    QString                      lpFileWin;

#endif

public:

    QAction* copyFromMainCollection(const QString& name) const
    {
        QAction* const mainAction = stdActionCollection->action(name);

        if (!mainAction)
        {
            return nullptr;
        }

        QAction* const action = new QAction(mainAction->icon(), mainAction->text(), q);
        action->setToolTip(mainAction->toolTip());

        return action;
    }
};

ImportContextMenuHelper::ImportContextMenuHelper(QMenu* const parent, KActionCollection* const actionCollection)
    : QObject(parent),
      d      (new Private(this))
{
    d->parent = parent;

    if (!actionCollection)
    {
        d->stdActionCollection = ImportUI::instance()->actionCollection();
    }
    else
    {
        d->stdActionCollection = actionCollection;
    }
}

ImportContextMenuHelper::~ImportContextMenuHelper()
{
    delete d;
}

void ImportContextMenuHelper::addAction(const QString& name, bool addDisabled)
{
    QAction* const action = d->stdActionCollection->action(name);
    addAction(action, addDisabled);
}

void ImportContextMenuHelper::addAction(QAction* action, bool addDisabled)
{
    if (!action)
    {
        return;
    }

    if (action->isEnabled() || addDisabled)
    {
        d->parent->addAction(action);
    }
}

void ImportContextMenuHelper::addSubMenu(QMenu* subMenu)
{
    d->parent->addMenu(subMenu);
}

void ImportContextMenuHelper::addSeparator()
{
    d->parent->addSeparator();
}

void ImportContextMenuHelper::addAction(QAction* action, QObject* recv, const char* slot,
                                        bool addDisabled)
{
    if (!action)
    {
        return;
    }

    connect(action, SIGNAL(triggered()),
            recv, slot);

    addAction(action, addDisabled);
}

void ImportContextMenuHelper::addServicesMenu(const QList<QUrl>& selectedItems)
{
    setSelectedItems(selectedItems);

    if (!ImportUI::instance()->cameraUseUMSDriver())
    {
        return;
    }

#ifdef Q_OS_MAC

    QList<QUrl> appUrls = DServiceMenu::MacApplicationsForFiles(selectedItems);

    if (!appUrls.isEmpty())
    {
        QMenu* const servicesMenu    = new QMenu(d->parent);
        qDeleteAll(servicesMenu->actions());

        QAction* const serviceAction = servicesMenu->menuAction();
        serviceAction->setText(i18nc("@action: context menu", "Open With"));

        for (const QUrl& aurl : std::as_const(appUrls))
        {
            QAction* const action = servicesMenu->addAction(DServiceMenu::MacApplicationBundleName(aurl));
            action->setIcon(DServiceMenu::MacApplicationBundleIcon(aurl));
            action->setData(aurl);
        }

        addAction(serviceAction);

        connect(servicesMenu, SIGNAL(triggered(QAction*)),
                this, SLOT(slotOpenWith(QAction*)));
    }

#else // LINUX and WINDOWS

#   ifdef HAVE_KIO

    KService::List offers = DServiceMenu::servicesForOpenWith(selectedItems);

    if (!offers.isEmpty())
    {
        QMenu* const servicesMenu    = new QMenu(d->parent);
        qDeleteAll(servicesMenu->actions());

        QAction* const serviceAction = servicesMenu->menuAction();
        serviceAction->setText(i18nc("@action: context menu", "Open With"));

        for (const KService::Ptr& service : std::as_const(offers))
        {
            QString name          = service->name().replace(QLatin1Char('&'), QLatin1String("&&"));
            QAction* const action = servicesMenu->addAction(name);
            action->setIcon(QIcon::fromTheme(service->icon()));
            action->setData(service->name());
            d->servicesMap[name]  = service;
        }

        servicesMenu->addSeparator();
        servicesMenu->addAction(i18nc("@action: open item with other application", "Other..."));

        addAction(serviceAction);

        connect(servicesMenu, SIGNAL(triggered(QAction*)),
                this, SLOT(slotOpenWith(QAction*)));
    }
    else
    {
        QAction* const serviceAction = new QAction(i18nc("@action: context menu", "Open With..."), this);
        addAction(serviceAction);

        connect(serviceAction, SIGNAL(triggered()),
                this, SLOT(slotOpenWith()));
    }

#   else

    QList<DServiceInfo> offers = DServiceMenu::servicesForOpen(selectedItems);

    if (!offers.isEmpty())
    {
        QMenu* const servicesMenu    = new QMenu(d->parent);
        qDeleteAll(servicesMenu->actions());

        QAction* const serviceAction = servicesMenu->menuAction();
        serviceAction->setText(i18nc("@action: context menu", "Open With"));

        for (const DServiceInfo& sinfo : std::as_const(offers))
        {
            QAction* const action = servicesMenu->addAction(sinfo.name);
            action->setIcon(DServiceMenu::getIconFromService(sinfo));
            action->setData(sinfo.name);
            d->newServicesMap[sinfo.name] = sinfo;
        }

#   ifdef Q_OS_WIN

        if (selectedItems.size() == 1)
        {
            servicesMenu->addSeparator();
            servicesMenu->addAction(i18nc("@action: open item with other application", "Other..."));
        }

#   endif // Q_OS_WIN

        addAction(serviceAction);

        connect(servicesMenu, SIGNAL(triggered(QAction*)),
                this, SLOT(slotOpenWith(QAction*)));
    }

#   ifdef Q_OS_WIN

    else
    {
        if (selectedItems.size() == 1)
        {
            QAction* const serviceAction = new QAction(i18nc("@action: context menu", "Open With..."), this);
            addAction(serviceAction);

            connect(serviceAction, SIGNAL(triggered()),
                    this, SLOT(slotOpenWith()));
        }
    }

#   endif // Q_OS_WIN

#   endif // HAVE_KIO

#endif // Q_OS_MAC

}

void ImportContextMenuHelper::slotOpenWith()
{
    // call the slot with an "empty" action

    slotOpenWith(nullptr);
}

void ImportContextMenuHelper::slotOpenWith(QAction* action)
{
#ifdef Q_OS_MAC

    QList<QUrl> list = d->selectedItems;
    QUrl aurl        = action ? action->data().toUrl() : QUrl();

    if (!aurl.isEmpty())
    {
        DServiceMenu::MacOpenFilesWithApplication(list, aurl);
    }

#else // LINUX and WINDOWS

    QList<QUrl> list = d->selectedItems;
    QString name     = action ? action->data().toString() : QString();

#   ifdef HAVE_KIO

    KService::Ptr service;

    if (name.isEmpty())
    {
        QPointer<KOpenWithDialog> dlg = new KOpenWithDialog(list);

        if (dlg->exec() != KOpenWithDialog::Accepted)
        {
            delete dlg;
            return;
        }

        service = dlg->service();

        if (!service)
        {
            // User entered a custom command

            if (!dlg->text().isEmpty())
            {
                DServiceMenu::runFiles(dlg->text(), list);
            }

            delete dlg;
            return;
        }

        delete dlg;
    }
    else
    {
        service = d->servicesMap[name];
    }

    DServiceMenu::runFiles(service, list);

#   else

    if (name.isEmpty())
    {

#   ifdef Q_OS_WIN

        // See Bug #380065 for details.

        if (list.size() == 1)
        {
            d->lpVerbWin          = QLatin1String("openas");
            d->lpFileWin          = QDir::toNativeSeparators(list.first().toLocalFile());

            SHELLEXECUTEINFOW sei = {};
            sei.cbSize            = sizeof(sei);
            sei.fMask             = SEE_MASK_INVOKEIDLIST | SEE_MASK_NOASYNC;
            sei.nShow             = SW_SHOWNORMAL;
            sei.lpVerb            = (LPCWSTR)d->lpVerbWin.utf16();
            sei.lpFile            = (LPCWSTR)d->lpFileWin.utf16();

            ShellExecuteEx(&sei);

            qCDebug(DIGIKAM_GENERAL_LOG) << "ShellExecuteEx::openas return code:" << GetLastError();
        }

#   endif // Q_OS_WIN

        return;
    }

    DServiceMenu::runFiles(d->newServicesMap[name], list);

#   endif // HAVE_KIO

#endif // Q_OS_MAC

}

void ImportContextMenuHelper::addRotateMenu(itemIds& /*ids*/)
{
/*
    setSelectedIds(ids);

    QMenu* const imageRotateMenu = new QMenu(i18n("Rotate"), d->parent);
    imageRotateMenu->setIcon(QIcon::fromTheme(QLatin1String("object-rotate-right")));

    QAction* const left = new QAction(this);
    left->setObjectName(QLatin1String("rotate_ccw"));
    left->setText(i18nc("rotate image left", "Left"));

    connect(left, SIGNAL(triggered(bool)),
            this, SLOT(slotRotate()));

    imageRotateMenu->addAction(left);

    QAction* const right = new QAction(this);
    right->setObjectName(QLatin1String("rotate_cw");
    right->setText(i18nc("rotate image right", "Right")));

    connect(right, SIGNAL(triggered(bool)),
            this, SLOT(slotRotate()));

    imageRotateMenu->addAction(right);

    d->parent->addMenu(imageRotateMenu);
*/
}

void ImportContextMenuHelper::slotRotate()
{
/*
    TODO: Implement rotate in import tool.

    if (sender()->objectName() == "rotate_ccw")
    {
        FileActionMngr::instance()->transform(CamItemInfoList(d->selectedIds), MetaEngineRotation::Rotate270);
    }
    else
    {
        FileActionMngr::instance()->transform(CamItemInfoList(d->selectedIds), MetaEngineRotation::Rotate90);
    }
*/
}

void ImportContextMenuHelper::addAssignTagsMenu(itemIds& /*ids*/)
{
/*
    setSelectedIds(ids);

    QMenu* const assignTagsPopup = new TagsPopupMenu(ids, TagsPopupMenu::RECENTLYASSIGNED, d->parent);
    assignTagsPopup->menuAction()->setText(i18n("Assign Tag"));
    assignTagsPopup->menuAction()->setIcon(QIcon::fromTheme(QLatin1String("tag")));
    d->parent->addMenu(assignTagsPopup);

    connect(assignTagsPopup, SIGNAL(signalTagActivated(int)),
            this, SIGNAL(signalAssignTag(int)));

    connect(assignTagsPopup, SIGNAL(signalPopupTagsView()),
            this, SIGNAL(signalPopupTagsView()));
*/
}

void ImportContextMenuHelper::addRemoveTagsMenu(itemIds& /*ids*/)
{
/*
    setSelectedIds(ids);

    QMenu* const removeTagsPopup = new TagsPopupMenu(ids, TagsPopupMenu::REMOVE, d->parent);
    removeTagsPopup->menuAction()->setText(i18n("Remove Tag"));
    removeTagsPopup->menuAction()->setIcon(QIcon::fromTheme(QLatin1String("tag")));
    d->parent->addMenu(removeTagsPopup);

    connect(removeTagsPopup, SIGNAL(signalTagActivated(int)),
            this, SIGNAL(signalRemoveTag(int)));
*/
}

void ImportContextMenuHelper::addLabelsAction()
{
    QMenu* const menuLabels           = new QMenu(i18nc("@title:menu", "Assign Labels"), d->parent);
    PickLabelMenuAction* const pmenu  = new PickLabelMenuAction(d->parent);
    ColorLabelMenuAction* const cmenu = new ColorLabelMenuAction(d->parent);
    RatingMenuAction* const rmenu     = new RatingMenuAction(d->parent);
    menuLabels->addAction(pmenu->menuAction());
    menuLabels->addAction(cmenu->menuAction());
    menuLabels->addAction(rmenu->menuAction());
    addSubMenu(menuLabels);

    connect(pmenu, SIGNAL(signalPickLabelChanged(int)),
            this, SIGNAL(signalAssignPickLabel(int)));

    connect(cmenu, SIGNAL(signalColorLabelChanged(int)),
            this, SIGNAL(signalAssignColorLabel(int)));

    connect(rmenu, SIGNAL(signalRatingChanged(int)),
            this, SIGNAL(signalAssignRating(int)));
}

void ImportContextMenuHelper::slotABCMenuTriggered(QAction* action)
{
    QString name = action->iconText();

    Q_EMIT signalAddNewTagFromABCMenu(name);
}

void ImportContextMenuHelper::setImportFilterModel(ImportFilterModel* model)
{
    d->importFilterModel = model;
}

QAction* ImportContextMenuHelper::exec(const QPoint& pos, QAction* at)
{
    QAction* const choice = d->parent->exec(pos, at);

    if (choice)
    {
        // check if a BQM action has been triggered

        for (QMap<int, QAction*>::const_iterator it = d->queueActions.constBegin() ;
             it != d->queueActions.constEnd() ; ++it)
        {
            if (choice == it.value())
            {
/*
                Q_EMIT signalAddToExistingQueue(it.key());
*/
                return choice;
            }
        }
    }

    return choice;
}

void ImportContextMenuHelper::setSelectedIds(itemIds& ids)
{
    if (d->selectedIds.isEmpty())
    {
        d->selectedIds = ids;
    }
}

void ImportContextMenuHelper::setSelectedItems(const QList<QUrl>& urls)
{
    if (d->selectedItems.isEmpty())
    {
        d->selectedItems = urls;
    }
}

} // namespace Digikam

#include "moc_importcontextmenu.cpp"
