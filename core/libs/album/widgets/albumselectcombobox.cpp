/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-05-09
 * Description : A combo box for selecting albums
 *
 * SPDX-FileCopyrightText: 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albumselectcombobox.h"

// Qt includes

#include <QSortFilterProxyModel>
#include <QTreeView>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "albummodel.h"
#include "albumfiltermodel.h"
#include "albumtreeview.h"
#include "tagtreeview.h"
#include "contextmenuhelper.h"

namespace Digikam
{

class Q_DECL_HIDDEN AlbumSelectComboBox::Private
{
public:

    explicit Private(AlbumSelectComboBox* const qq)
        : q(qq)
    {
    }

    void updateCheckable();
    void updateCloseOnActivate();

public:

    AbstractCheckableAlbumModel* model                  = nullptr;
    AlbumFilterModel*            filterModel            = nullptr;
    QString                      noSelectionText;
    bool                         recursive              = false;
    bool                         isCheckable            = true;
    bool                         closeOnActivate        = false;
    bool                         allSelectedText        = false;
    bool                         showCheckStateSummary  = true;

    AlbumSelectComboBox* const   q;
};

AlbumSelectComboBox::AlbumSelectComboBox(QWidget* const parent)
    : TreeViewLineEditComboBox(parent),
      d                       (new Private(this))
{
    d->noSelectionText = i18n("No Album Selected");

    // Workaround for QLineEdit text when QComboBox loses focus

    /// @note use dynamic binding as updateText() is a virtual slot which can be re-implemented in derived classes.

    connect(this, &AlbumSelectComboBox::editTextChanged,
            this, &AlbumSelectComboBox::updateText);
}

AlbumSelectComboBox::~AlbumSelectComboBox()
{
    delete d;
}

void AlbumSelectComboBox::setDefaultAlbumModel()
{
    d->noSelectionText = i18n("No Album Selected");
    setAlbumModels(new AlbumModel(AlbumModel::IgnoreRootAlbum, this));
    view()->expandToDepth(0);
}

void AlbumSelectComboBox::setDefaultTagModel()
{
    d->noSelectionText = i18n("No Tag Selected");
    setAlbumModels(new TagModel(AlbumModel::IgnoreRootAlbum, this));
}

void AlbumSelectComboBox::setAlbumModels(AbstractCheckableAlbumModel* _model, AlbumFilterModel* _filterModel)
{
    d->model = _model;
    d->model->setRecursive(d->recursive);

    if (_filterModel)
    {
        d->filterModel = _filterModel;
    }
    else
    {
        d->filterModel = new AlbumFilterModel(this);
/*
        d->filterModel->setDynamicSortFilter(true);
*/
        d->filterModel->setSourceAlbumModel(d->model);
    }

    d->updateCheckable();

    QComboBox::setModel(d->filterModel);
    installView();

    d->updateCloseOnActivate();
    updateText();
}

void AlbumSelectComboBox::installView(QAbstractItemView* v)
{
    if (view())
    {
        return;
    }

    TreeViewLineEditComboBox::installView(v);

    // cppcheck-suppress nullPointerRedundantCheck
    view()->sortByColumn(0, Qt::AscendingOrder);

    // cppcheck-suppress nullPointerRedundantCheck
    view()->setSortingEnabled(true);

    // cppcheck-suppress nullPointerRedundantCheck
    view()->collapseAll();
}

void AlbumSelectComboBox::setCheckable(bool checkable)
{
    if (checkable == d->isCheckable)
    {
        return;
    }

    d->isCheckable = checkable;
    d->updateCheckable();
}

bool AlbumSelectComboBox::isCheckable() const
{
    return d->isCheckable;
}

void AlbumSelectComboBox::Private::updateCheckable()
{
    if (!model)
    {
        return;
    }

    model->setCheckable(isCheckable);

    if (isCheckable)
    {
        connect(model, SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
                q, SLOT(updateText()));
    }
    else
    {
        disconnect(model, SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
                   q, SLOT(updateText()));
    }
}

void AlbumSelectComboBox::setCloseOnActivate(bool close)
{
    if (d->closeOnActivate == close)
    {
        return;
    }

    d->closeOnActivate = close;
    d->updateCloseOnActivate();
}

void AlbumSelectComboBox::Private::updateCloseOnActivate()
{
    if (!q->view())
    {
        return;
    }

    if (closeOnActivate)
    {
        connect(q->view(), SIGNAL(activated(QModelIndex)),
                q, SLOT(hidePopup()));
    }
    else
    {
        disconnect(q->view(), SIGNAL(activated(QModelIndex)),
                   q, SLOT(hidePopup()));
    }
}

void AlbumSelectComboBox::setRecursive(bool recursive)
{
    d->recursive = recursive; // used if a new model will be set

    if (d->model)
    {
        d->model->setRecursive(recursive);
    }
}

void AlbumSelectComboBox::setNoSelectionText(const QString& text)
{
    d->noSelectionText = text;
    updateText();
}

void AlbumSelectComboBox::setAllSelectedText(bool all)
{
    d->allSelectedText = all;
    updateText();
}

void AlbumSelectComboBox::setShowCheckStateSummary(bool show)
{
    d->showCheckStateSummary = show;
    updateText();
}

AbstractCheckableAlbumModel* AlbumSelectComboBox::model() const
{
    return d->model;
}

QSortFilterProxyModel* AlbumSelectComboBox::filterModel() const
{
    return d->filterModel;
}

void AlbumSelectComboBox::hidePopup()
{
    // just make this a slot

    TreeViewLineEditComboBox::hidePopup();
    updateText();
}

void AlbumSelectComboBox::updateText()
{
    if (!d->isCheckable || !d->showCheckStateSummary)
    {
        return;
    }

    QList<Album*> checkedAlbums;

    if (!d->allSelectedText)
    {
        checkedAlbums = d->model->checkedAlbums();
    }
    else
    {
        if (d->model->albumType() == Album::TAG)
        {
            checkedAlbums = AlbumManager::instance()->allTAlbums();
        }
        else
        {
            checkedAlbums = AlbumManager::instance()->allPAlbums();
        }
    }

    QList<Album*> partiallyCheckedAlbums = d->model->partiallyCheckedAlbums();
    QString newIncludeText;
    QString newExcludeText;

    if (!checkedAlbums.isEmpty())
    {
        if (checkedAlbums.count() == 1)
        {
            newIncludeText = checkedAlbums.first()->title();
        }
        else
        {
            if (d->model->albumType() == Album::TAG)
            {
                newIncludeText = i18np("1 Tag selected", "%1 Tags selected",
                                       checkedAlbums.count());
            }
            else
            {
                newIncludeText = i18np("1 Album selected", "%1 Albums selected",
                                       checkedAlbums.count());
            }
        }
    }

    if (!partiallyCheckedAlbums.isEmpty() && !d->allSelectedText)
    {
        if (d->model->albumType() == Album::TAG)
        {
            newExcludeText = i18np("1 Tag excluded", "%1 Tags excluded",
                                   partiallyCheckedAlbums.count());
        }
        else
        {
            newExcludeText = i18np("1 Album excluded", "%1 Albums excluded",
                                   partiallyCheckedAlbums.count());
        }
    }

    blockSignals(true);

    if      (newIncludeText.isEmpty() && newExcludeText.isEmpty())
    {
        setLineEditText(d->noSelectionText);
    }
    else if (newIncludeText.isEmpty() || newExcludeText.isEmpty())
    {
        setLineEditText(newIncludeText + newExcludeText);
    }
    else
    {
        setLineEditText(newIncludeText + QLatin1String(", ") + newExcludeText);
    }

    blockSignals(false);
}

// ---------------------------------------------------------------------------------------------------

AbstractAlbumTreeViewSelectComboBox::AbstractAlbumTreeViewSelectComboBox(QWidget* const parent)
    : AlbumSelectComboBox(parent)
{
}

void AbstractAlbumTreeViewSelectComboBox::installView(QAbstractItemView* view)
{
    if (!view)
    {
        view = m_treeView;
    }

    AlbumSelectComboBox::installView(view);
}

void AbstractAlbumTreeViewSelectComboBox::sendViewportEventToView(QEvent* e)
{
    // needed for StayPoppedUpComboBox

    m_treeView->viewportEvent(e);
}

void AbstractAlbumTreeViewSelectComboBox::setTreeView(AbstractAlbumTreeView* const treeView)
{
    // this is independent from the installView mechanism, just to override
    // the tree view created below without the need to subclass

    if (!m_treeView)
    {
        m_treeView = treeView;
    }
}

// -------------------------------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN CheckUncheckContextMenuElement : public QObject,
                                                     public AbstractAlbumTreeView::ContextMenuElement
{
    Q_OBJECT

public:

    explicit CheckUncheckContextMenuElement(QObject* const parent)
        : QObject(parent)
    {
    }

    void addActions(AbstractAlbumTreeView* view, ContextMenuHelper& cmh, Album* album) override
    {
        AbstractCheckableAlbumModel* const checkable = qobject_cast<AbstractCheckableAlbumModel*>(view->albumModel());

        if (checkable)
        {
            cmh.setAlbumModel(checkable);
            cmh.addAlbumCheckUncheckActions(album);
        }
    }
};

void AbstractAlbumTreeViewSelectComboBox::addCheckUncheckContextMenuActions()
{
    if (m_treeView)
    {
        m_treeView->setEnableContextMenu(true);
        m_treeView->addContextMenuElement(new CheckUncheckContextMenuElement(this));
    }
}

// ---------------------------------------------------------------------------------

AlbumTreeViewSelectComboBox::AlbumTreeViewSelectComboBox(QWidget* const parent)
    : AbstractAlbumTreeViewSelectComboBox(parent)
{
}

AlbumTreeView* AlbumTreeViewSelectComboBox::treeView() const
{
    return static_cast<AlbumTreeView*>(m_treeView);
}

void AlbumTreeViewSelectComboBox::setDefaultModel()
{
    setAlbumModels(nullptr, nullptr);
}

void AlbumTreeViewSelectComboBox::setAlbumModels(AlbumModel* model, CheckableAlbumFilterModel* filterModel)
{
    if (!m_treeView)
    {
        AlbumTreeView::Flags flags;
        m_treeView = new AlbumTreeView(this, flags);
    }

    if (!model)
    {
        model = new AlbumModel(AlbumModel::IgnoreRootAlbum, this);
    }

    if (!filterModel)
    {
        filterModel = new CheckableAlbumFilterModel(this);
    }

    treeView()->setPAlbumModel(model);
    treeView()->setCheckableAlbumFilterModel(filterModel);

    AlbumSelectComboBox::setAlbumModels(treeView()->checkableAlbumModel(), treeView()->albumFilterModel());

    treeView()->setRootIsDecorated(true);
    treeView()->expandToDepth(0);
}

// ---------------------------------------------------------------------------------------------------

TagTreeViewSelectComboBox::TagTreeViewSelectComboBox(QWidget* const parent)
    : AbstractAlbumTreeViewSelectComboBox(parent)
{
}

TagTreeView* TagTreeViewSelectComboBox::treeView() const
{
    return static_cast<TagTreeView*>(m_treeView);
}

void TagTreeViewSelectComboBox::setDefaultModel()
{
    setAlbumModels(nullptr, nullptr);
}

void TagTreeViewSelectComboBox::setAlbumModels(TagModel* model,
                                               TagPropertiesFilterModel* filteredModel,
                                               CheckableAlbumFilterModel* filterModel)
{
    if (!m_treeView)
    {
        TagTreeView::Flags flags;
        m_treeView = new TagTreeView(this, flags);
    }

    if (!model)
    {
        model = new TagModel(AlbumModel::IgnoreRootAlbum, this);
    }

    if (!filteredModel)
    {
        filteredModel = new TagPropertiesFilterModel(this);
    }

    if (!filterModel)
    {
        filterModel = new CheckableAlbumFilterModel(this);
    }

    treeView()->setTAlbumModel(model);
    treeView()->setTAlbumFilterModel(filteredModel, filterModel);

    AlbumSelectComboBox::setAlbumModels(treeView()->checkableAlbumModel(), treeView()->albumFilterModel());
}

} // namespace Digikam

#include "albumselectcombobox.moc"

#include "moc_albumselectcombobox.cpp"
