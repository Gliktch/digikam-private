/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-03
 * Description : A dialog base class which can handle multiple pages.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2007      by Matthias Kretz <kretz at kde dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dconfigdlgmodels_p.h"

// Qt includes

#include <QPointer>
#include <QIcon>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

DConfigDlgModel::DConfigDlgModel(QObject* const parent)
    : QAbstractItemModel(parent),
      d_ptr             (nullptr)
{
}

DConfigDlgModel::DConfigDlgModel(DConfigDlgModelPrivate& dd, QObject* const parent)
    : QAbstractItemModel(parent),
      d_ptr             (&dd)
{
    d_ptr->q_ptr = this;
}

DConfigDlgModel::~DConfigDlgModel()
{
    delete d_ptr;
}

// ----------------------------------------------------------------------------------

class Q_DECL_HIDDEN DConfigDlgWdgItem::Private
{
public:

    Private() = default;

    ~Private()
    {
        delete widget;
        widget = nullptr;
    }

public:

    QString           name;
    QString           header;
    QIcon             icon;
    QPointer<QWidget> widget;
    bool              checkable = false;
    bool              checked   = false;
    bool              enabled   = true;
};

DConfigDlgWdgItem::DConfigDlgWdgItem(QWidget* const widget)
    : QObject(nullptr),
      d      (new Private)
{
    d->widget = widget;

    /**
     * Hide the widget, otherwise when the widget has this DConfigDlgView as
     * parent the widget is shown outside the QStackedWidget if the page
     * was not selected ( and reparented ) yet.
     */
    if (d->widget)
    {
        d->widget->hide();
    }
}

DConfigDlgWdgItem::DConfigDlgWdgItem(QWidget* const widget, const QString& name)
    : QObject(nullptr),
      d      (new Private)
{
    d->widget = widget;
    d->name   = name;

    /**
     * Hide the widget, otherwise when the widget has this DConfigDlgView as
     * parent the widget is shown outside the QStackedWidget if the page
     * was not selected ( and reparented ) yet.
     */
    if (d->widget)
    {
        d->widget->hide();
    }
}

DConfigDlgWdgItem::~DConfigDlgWdgItem()
{
    delete d;
}

void DConfigDlgWdgItem::setEnabled(bool enabled)
{
    d->enabled = enabled;

    if (d->widget)
    {
        d->widget->setEnabled(enabled);
    }

    Q_EMIT changed();
}

bool DConfigDlgWdgItem::isEnabled() const
{
    return d->enabled;
}

QWidget* DConfigDlgWdgItem::widget() const
{
    return d->widget;
}

void DConfigDlgWdgItem::setName(const QString& name)
{
    d->name = name;

    Q_EMIT changed();
}

QString DConfigDlgWdgItem::name() const
{
    return d->name;
}

void DConfigDlgWdgItem::setHeader(const QString& header)
{
    d->header = header;

    Q_EMIT changed();
}

QString DConfigDlgWdgItem::header() const
{
    return d->header;
}

void DConfigDlgWdgItem::setIcon(const QIcon& icon)
{
    d->icon = icon;

    Q_EMIT changed();
}

QIcon DConfigDlgWdgItem::icon() const
{
    return d->icon;
}

void DConfigDlgWdgItem::setCheckable(bool checkable)
{
    d->checkable = checkable;

    Q_EMIT changed();
}

bool DConfigDlgWdgItem::isCheckable() const
{
    return d->checkable;
}

void DConfigDlgWdgItem::setChecked(bool checked)
{
    d->checked = checked;

    Q_EMIT toggled(checked);
    Q_EMIT changed();
}

bool DConfigDlgWdgItem::isChecked() const
{
    return d->checked;
}

// ---------------------------------------------------------------------------------

PageItem::PageItem(DConfigDlgWdgItem* const pageWidgetItem, PageItem* const parent)
    : mPageWidgetItem(pageWidgetItem),
      mParentItem    (parent)
{
}

PageItem::~PageItem()
{
    delete mPageWidgetItem;
    mPageWidgetItem = nullptr;

    qDeleteAll(mChildItems);
}

// cppcheck-suppress constParameterPointer
void PageItem::appendChild(PageItem* const item)
{
    mChildItems.append(item);
}

void PageItem::insertChild(int row, PageItem* const item)
{
    mChildItems.insert(row, item);
}

void PageItem::removeChild(int row)
{
    mChildItems.removeAt(row);
}

PageItem* PageItem::child(int row)
{
    return mChildItems.value(row);
}

int PageItem::childCount() const
{
    return mChildItems.count();
}

int PageItem::columnCount() const
{
    return 1;
}

PageItem* PageItem::parent() const
{
    return mParentItem;
}

int PageItem::row() const
{
    if (mParentItem)
    {
        return mParentItem->mChildItems.indexOf(const_cast<PageItem*>(this));
    }

    return 0;
}

DConfigDlgWdgItem* PageItem::pageWidgetItem() const
{
    return mPageWidgetItem;
}

PageItem* PageItem::findChild(const DConfigDlgWdgItem* item)
{
    if (mPageWidgetItem == item)
    {
        return this;
    }

    for (int i = 0 ; i < mChildItems.count() ; ++i)
    {
        PageItem* const pageItem = mChildItems[i]->findChild(item);

        if (pageItem)
        {
            return pageItem;
        }
    }

    return nullptr;
}

void PageItem::dump(int indent)
{
    QString prefix;

    for (int i = 0 ; i < indent ; ++i)
    {
        prefix.append(QLatin1Char(' '));
    }

    const QString name = (mPageWidgetItem ? mPageWidgetItem->name() : QLatin1String("root"));
    qCDebug(DIGIKAM_GENERAL_LOG) << QString::asprintf("%s (%p)", qPrintable(QString(QString::fromLatin1("%1%2")).arg(prefix, name)),
                                                                 reinterpret_cast<void*>(this));

    for (int i = 0 ; i < mChildItems.count() ; ++i)
    {
        mChildItems[ i ]->dump(indent + 2);
    }
}

// ---------------------------------------------------------------------------------

DConfigDlgWdgModel::DConfigDlgWdgModel(QObject* const parent)
    : DConfigDlgModel(*new DConfigDlgWdgModelPrivate, parent)
{
}

int DConfigDlgWdgModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant DConfigDlgWdgModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    const PageItem* const itm = static_cast<PageItem *>(index.internalPointer());

    if      (role == Qt::DisplayRole)
    {
        return QVariant(itm->pageWidgetItem()->name());
    }
    else if (role == Qt::DecorationRole)
    {
        return QVariant(itm->pageWidgetItem()->icon());
    }
    else if (role == HeaderRole)
    {
        return QVariant(itm->pageWidgetItem()->header());
    }
    else if (role == WidgetRole)
    {
        return QVariant::fromValue(itm->pageWidgetItem()->widget());
    }
    else if (role == Qt::CheckStateRole)
    {
        if (itm->pageWidgetItem()->isCheckable())
        {
            return (itm->pageWidgetItem()->isChecked() ? Qt::Checked : Qt::Unchecked);
        }
        else
        {
            return QVariant();
        }
    }
    else
    {
        return QVariant();
    }
}

bool DConfigDlgWdgModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
    {
        return false;
    }

    if (role != Qt::CheckStateRole)
    {
        return false;
    }

    const PageItem* const itm = static_cast<PageItem*>(index.internalPointer());

    if (!itm)
    {
        return false;
    }

    if (!itm->pageWidgetItem()->isCheckable())
    {
        return false;
    }

    if (value.toInt() == Qt::Checked)
    {
        itm->pageWidgetItem()->setChecked(true);
    }
    else
    {
        itm->pageWidgetItem()->setChecked(false);
    }

    return true;
}

Qt::ItemFlags DConfigDlgWdgModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags flags = Qt::ItemIsSelectable;

    const PageItem* const itm = static_cast<PageItem*>(index.internalPointer());

    if (itm->pageWidgetItem()->isCheckable())
    {
        flags |= Qt::ItemIsUserCheckable;
    }

    if (itm->pageWidgetItem()->isEnabled())
    {
        flags |= Qt::ItemIsEnabled;
    }

    return flags;
}

QModelIndex DConfigDlgWdgModel::index(int row, int column, const QModelIndex& parent) const
{
    PageItem* parentItem = nullptr;

    if (parent.isValid())
    {
        parentItem = static_cast<PageItem*>(parent.internalPointer());
    }
    else
    {
        parentItem = d_func()->rootItem;
    }

    PageItem* const childItem = parentItem->child(row);

    if (childItem)
    {
        return createIndex(row, column, childItem);
    }
    else
    {
        return QModelIndex();
    }
}

QModelIndex DConfigDlgWdgModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    PageItem* const itm        = static_cast<PageItem*>(index.internalPointer());
    PageItem* const parentItem = itm->parent();

    if (parentItem == d_func()->rootItem)
    {
        return QModelIndex();
    }
    else
    {
        return createIndex(parentItem->row(), 0, parentItem);
    }
}

int DConfigDlgWdgModel::rowCount(const QModelIndex& parent) const
{
    const PageItem* parentItem = nullptr;

    if (!parent.isValid())
    {
        parentItem = d_func()->rootItem;
    }
    else
    {
        parentItem = static_cast<PageItem *>(parent.internalPointer());
    }

    return parentItem->childCount();
}

DConfigDlgWdgItem* DConfigDlgWdgModel::addPage(QWidget* widget, const QString& name)
{
    DConfigDlgWdgItem* const itm = new DConfigDlgWdgItem(widget, name);
    addPage(itm);

    return itm;
}

void DConfigDlgWdgModel::addPage(DConfigDlgWdgItem* item)
{
    Q_EMIT layoutAboutToBeChanged();

    Q_D(DConfigDlgWdgModel);

    connect(item, SIGNAL(changed()),
            this, SLOT(_k_itemChanged()));

    connect(item, SIGNAL(toggled(bool)),
            this, SLOT(_k_itemToggled(bool)));

    // The row to be inserted

    int row = d->rootItem->childCount();

    beginInsertRows(QModelIndex(), row, row);

    PageItem* const pageItem = new PageItem(item, d->rootItem);
    d->rootItem->appendChild(pageItem);

    endInsertRows();

    Q_EMIT layoutChanged();
}

DConfigDlgWdgItem* DConfigDlgWdgModel::insertPage(DConfigDlgWdgItem* before, QWidget* widget, const QString& name)
{
    DConfigDlgWdgItem* const itm = new DConfigDlgWdgItem(widget, name);

    insertPage(before, itm);

    return itm;
}

void DConfigDlgWdgModel::insertPage(DConfigDlgWdgItem* before, DConfigDlgWdgItem* item)
{
    PageItem* const beforePageItem = d_func()->rootItem->findChild(before);

    if (!beforePageItem)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Invalid DConfigDlgWdgItem passed!";
        return;
    }

    Q_EMIT layoutAboutToBeChanged();

    connect(item, SIGNAL(changed()),
            this, SLOT(_k_itemChanged()));

    connect(item, SIGNAL(toggled(bool)),
            this, SLOT(_k_itemToggled(bool)));

    PageItem* const prnt = beforePageItem->parent();

    // The row to be inserted

    int row                = beforePageItem->row();

    QModelIndex ind;

    if (prnt != d_func()->rootItem)
    {
        ind = createIndex(prnt->row(), 0, prnt);
    }

    beginInsertRows(ind, row, row);

    PageItem* const newPageItem = new PageItem(item, prnt);
    prnt->insertChild(row, newPageItem);

    endInsertRows();

    Q_EMIT layoutChanged();
}

DConfigDlgWdgItem* DConfigDlgWdgModel::addSubPage(DConfigDlgWdgItem* parent, QWidget* widget, const QString& name)
{
    DConfigDlgWdgItem* const itm = new DConfigDlgWdgItem(widget, name);

    addSubPage(parent, itm);

    return itm;
}

void DConfigDlgWdgModel::addSubPage(DConfigDlgWdgItem* parent, DConfigDlgWdgItem* item)
{
    PageItem* const parentPageItem = d_func()->rootItem->findChild(parent);

    if (!parentPageItem)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Invalid DConfigDlgWdgItem passed!";
        return;
    }

    Q_EMIT layoutAboutToBeChanged();

    connect(item, SIGNAL(changed()),
            this, SLOT(_k_itemChanged()));

    connect(item, SIGNAL(toggled(bool)),
            this, SLOT(_k_itemToggled(bool)));

    // The row to be inserted

    int row = parentPageItem->childCount();

    QModelIndex ind;

    if (parentPageItem != d_func()->rootItem)
    {
        ind = createIndex(parentPageItem->row(), 0, parentPageItem);
    }

    beginInsertRows(ind, row, row);

    PageItem* const newPageItem = new PageItem(item, parentPageItem);
    parentPageItem->appendChild(newPageItem);

    endInsertRows();

    Q_EMIT layoutChanged();
}

void DConfigDlgWdgModel::removePage(DConfigDlgWdgItem* item)
{
    if (!item)
    {
        return;
    }

    Q_D(DConfigDlgWdgModel);

    PageItem* const pageItem = d->rootItem->findChild(item);

    if (!pageItem)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Invalid DConfigDlgWdgItem passed!";
        return;
    }

    Q_EMIT layoutAboutToBeChanged();

    disconnect(item, SIGNAL(changed()),
               this, SLOT(_k_itemChanged()));

    disconnect(item, SIGNAL(toggled(bool)),
               this, SLOT(_k_itemToggled(bool)));

    PageItem* const parentPageItem = pageItem->parent();
    int row                        = parentPageItem->row();

    QModelIndex ind;

    if (parentPageItem != d->rootItem)
    {
        ind = createIndex(row, 0, parentPageItem);
    }

    beginRemoveRows(ind, pageItem->row(), pageItem->row());

    parentPageItem->removeChild(pageItem->row());
    delete pageItem;

    endRemoveRows();

    Q_EMIT layoutChanged();
}

DConfigDlgWdgItem* DConfigDlgWdgModel::item(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return nullptr;
    }

    const PageItem* const item = static_cast<PageItem*>(index.internalPointer());

    if (!item)
    {
        return nullptr;
    }

    return item->pageWidgetItem();
}

QModelIndex DConfigDlgWdgModel::index(const DConfigDlgWdgItem* item) const
{
    if (!item)
    {
        return QModelIndex();
    }

    const PageItem* pageItem = d_func()->rootItem->findChild(item);

    if (!pageItem)
    {
        return QModelIndex();
    }

    return createIndex(pageItem->row(), 0,

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                       pageItem

#else

                       // cppcheck-suppress cstyleCast
                       (void*)pageItem

#endif

    );
}

} // namespace Digikam

#include "moc_dconfigdlgmodels.cpp"
