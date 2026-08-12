/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-13
 * Description : progress manager
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2004      by Till Adam <adam at kde dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "progressmanager.h"

// Qt includes

#include <QApplication>
#include <QAtomicInt>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QEventLoop>
#include <QMessageBox>
#include <QIcon>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN ProgressItem::Private
{
public:

    typedef QMap<ProgressItem*, bool> ProgressItemMap;

public:

    Private() = default;

public:

    volatile bool   waitingForKids      = false;
    volatile bool   canceled            = false;
    bool            usesBusyIndicator   = false;
    bool            canBeCanceled       = false;
    bool            hasThumb            = false;
    bool            showAtStart         = false;   // Force to show progress item when it's add in progress manager

    QAtomicInt      progress            = 0;
    QAtomicInt      total               = 0;
    QAtomicInt      completed           = 0;

    QString         id;
    QString         label;
    QString         status;

    ProgressItem*   parent              = nullptr;
    ProgressItemMap children;
};

ProgressItem::ProgressItem(ProgressItem* const parent, const QString& id,
                           const QString& label, const QString& status,
                           bool canBeCanceled, bool hasThumb)
    : d(new Private)
{
    d->canBeCanceled = canBeCanceled;
    d->hasThumb      = hasThumb;
    d->id            = id;
    d->label         = label;
    d->status        = status;
    d->parent        = parent;
}

ProgressItem::~ProgressItem()
{
    delete d;
}

void ProgressItem::setShowAtStart(bool showAtStart)
{
    d->showAtStart = showAtStart;
}

bool ProgressItem::showAtStart() const
{
    return d->showAtStart;
}

void ProgressItem::setComplete()
{
    if (d->children.isEmpty())
    {
        if (!d->canceled)
        {
            setProgress(100);
        }

        Q_EMIT progressItemCompleted(this);
    }
    else
    {
        d->waitingForKids = true;
    }
}

void ProgressItem::addChild(ProgressItem* const kiddo)
{
    d->children.insert(kiddo, true);
}

void ProgressItem::removeChild(ProgressItem* const kiddo)
{
    d->children.remove(kiddo);

    // in case we were waiting for the last kid to go away, now is the time

    if ((d->children.count() == 0) && d->waitingForKids)
    {
        Q_EMIT progressItemCompleted(this);
    }
}

void ProgressItem::cancel()
{
    if (d->canceled || !d->canBeCanceled)
    {
        return;
    }

    d->canceled = true;

    // Cancel all children.

    QList<ProgressItem*> kids = d->children.keys();
    QList<ProgressItem*>::Iterator it(kids.begin());
    QList<ProgressItem*>::Iterator end(kids.end());

    for ( ; it != end ; ++it)
    {
        ProgressItem* const kid = *it;

        if (kid->canBeCanceled())
        {
            kid->cancel();
        }
    }

    setStatus(i18n("Aborting..."));

    Q_EMIT progressItemCanceled(this);
    Q_EMIT progressItemCanceledById(this->id());
}

void ProgressItem::setLabel(const QString& v)
{
    d->label = v;

    Q_EMIT progressItemLabel(this, d->label);
}

void ProgressItem::setStatus(const QString& v)
{
    d->status = v;

    Q_EMIT progressItemStatus(this, d->status);
}

void ProgressItem::setUsesBusyIndicator(bool useBusyIndicator)
{
    d->usesBusyIndicator = useBusyIndicator;

    Q_EMIT progressItemUsesBusyIndicator(this, useBusyIndicator);
}

void ProgressItem::setThumbnail(const QIcon& icon)
{
    if (!hasThumbnail())
    {
        return;
    }

    int iconSize = 48;

    if (icon.isNull())
    {
        Q_EMIT progressItemThumbnail(this, QIcon::fromTheme(QLatin1String("image-missing")).pixmap(iconSize));

        return;
    }

    QPixmap pix = icon.pixmap(iconSize);

    Q_EMIT progressItemThumbnail(this, pix);
}

void ProgressItem::reset()
{
    setProgress(0);
    setStatus(QString());
    d->completed = 0;
}

void ProgressItem::setProgress(unsigned int v)
{
    d->progress.fetchAndStoreOrdered(v);

    Q_EMIT progressItemProgress(this, v);
}

void ProgressItem::updateProgress()
{
    int total = d->total;
    setProgress(total? d->completed * 100 / total : 0);
}

bool ProgressItem::advance(unsigned int v)
{
    bool complete = incCompletedItems(v);
    updateProgress();

    return complete;
}

void ProgressItem::setTotalItems(unsigned int v)
{
    d->total.fetchAndStoreOrdered(v);
}

void ProgressItem::incTotalItems(unsigned int v)
{
    d->total.fetchAndAddOrdered(v);
}

unsigned int ProgressItem::totalItems() const
{
    return d->total;
}

bool ProgressItem::setCompletedItems(unsigned int v)
{
    d->completed.fetchAndStoreOrdered(v);

    return (v == (unsigned int)d->total);
}

unsigned int ProgressItem::completedItems() const
{
    return d->completed;
}

bool ProgressItem::incCompletedItems(unsigned int v)
{
    unsigned int previous = d->completed.fetchAndAddOrdered(v);

    return ((previous+v) == (unsigned int) d->total);
}

bool ProgressItem::totalCompleted() const
{
    return (d->completed == d->total);
}

bool ProgressItem::canceled() const
{
    return d->canceled;
}

const QString& ProgressItem::id() const
{
    return d->id;
}

ProgressItem* ProgressItem::parent() const
{
    return d->parent;
}

const QString& ProgressItem::label() const
{
    return d->label;
}

const QString& ProgressItem::status() const
{
    return d->status;
}

bool ProgressItem::canBeCanceled() const
{
    return d->canBeCanceled;
}

bool ProgressItem::usesBusyIndicator() const
{
    return d->usesBusyIndicator;
}

bool ProgressItem::hasThumbnail() const
{
    return d->hasThumb;
}

unsigned int ProgressItem::progress() const
{
    return d->progress;
}

// --------------------------------------------------------------------------

class Q_DECL_HIDDEN ProgressManagerCreator
{
public:

    ProgressManager object;
};

class Q_DECL_HIDDEN ProgressManager::Private
{
public:

    Private() = default;

    void addItem(ProgressItem* const t, ProgressItem* const parent);
    void removeItem(ProgressItem* const t);

public:

    QMutex                        mutex;
    QHash<QString, ProgressItem*> transactions;
    QAtomicInt                    uID           = 1000;

    QEventLoop*                   waitingLoop   = nullptr;
};

Q_GLOBAL_STATIC(ProgressManagerCreator, creator)

void ProgressManager::Private::addItem(ProgressItem* const t, ProgressItem* const parent)
{
    if (!t)
    {
        return;
    }

    QMutexLocker lock(&mutex);
    transactions.insert(t->id(), t);

    if (parent)
    {
        parent->addChild(t);
    }
}

void ProgressManager::Private::removeItem(ProgressItem* const t)
{
    if (!t)
    {
        return;
    }

    QMutexLocker lock(&mutex);
    transactions.remove(t->id());

    if (t->parent())
    {
        t->parent()->removeChild(t);
    }
}

// --------------------------------------------------------------------------

ProgressManager::ProgressManager()
    : QObject(),
      d      (new Private)
{
    if (thread() != QApplication::instance()->thread())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Attention: ProgressManager was created from a thread. Create it in the main thread!";
        moveToThread(QApplication::instance()->thread());
    }

    d->waitingLoop = new QEventLoop(this);

    connect(this, SIGNAL(completeTransactionDeferred(ProgressItem*)),
            this, SLOT(slotTransactionCompletedDeferred(ProgressItem*)));
}

ProgressManager::~ProgressManager()
{
    delete d;
}

ProgressManager* ProgressManager::instance()
{
    return (creator.isDestroyed() ? nullptr : &creator->object);
}

bool ProgressManager::isEmpty() const
{
    return d->transactions.isEmpty();
}

ProgressItem* ProgressManager::findItembyId(const QString& id) const
{
    if (id.isEmpty())
    {
        return nullptr;
    }

    QMutexLocker lock(&d->mutex);

    return d->transactions.value(id);
}

QString ProgressManager::getUniqueID()
{
    return QString::number(d->uID.fetchAndAddOrdered(1));
}

ProgressItem* ProgressManager::createProgressItemImpl(ProgressItem* const parent,
                                                      const QString& id,
                                                      const QString& label,
                                                      const QString& status,
                                                      bool  cancellable,
                                                      bool  hasThumb)
{
    ProgressItem* t = findItembyId(id);

    if (!t)
    {
        t = new ProgressItem(parent, id, label, status, cancellable, hasThumb);
        addProgressItemImpl(t, parent);
    }

    return t;
}

ProgressItem* ProgressManager::createProgressItemImpl(const QString& parent,
                                                      const QString& id,
                                                      const QString& label,
                                                      const QString& status,
                                                      bool  canBeCanceled,
                                                      bool  hasThumb)
{
    ProgressItem* const p = findItembyId(parent);

    return createProgressItemImpl(p, id, label, status, canBeCanceled, hasThumb);
}

bool ProgressManager::addProgressItem(ProgressItem* const t, ProgressItem* const parent)
{
    if (!instance()->findItembyId(t->id()))
    {
        instance()->addProgressItemImpl(t, parent);

        return true;
    }
    else
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "A tool identified as " << t->id() << " is already running.";
        t->setComplete();

        return false;
    }
}

void ProgressManager::addProgressItemImpl(ProgressItem* const t, ProgressItem* const parent)
{
    if (t->thread() != thread())
    {
        if (t->thread() != QThread::currentThread())
        {
            // we cannot moveToThread this item living in a third thread. Refusing to add.

            qCDebug(DIGIKAM_GENERAL_LOG) << "Refusing to add in thread 1 a ProgressItem created in thread 2 to ProgressManager, living in thread 3";
            return;
        }

        // Move to ProgressManager's thread

        t->moveToThread(thread());
    }

    connect(t, SIGNAL(progressItemCompleted(ProgressItem*)),
            this, SLOT(slotTransactionCompleted(ProgressItem*)), Qt::DirectConnection);

    connect(t, SIGNAL(progressItemProgress(ProgressItem*,uint)),
            this, SIGNAL(progressItemProgress(ProgressItem*,uint)));

    connect(t, SIGNAL(progressItemAdded(ProgressItem*)),
            this, SIGNAL(progressItemAdded(ProgressItem*)));

    connect(t, SIGNAL(progressItemCanceled(ProgressItem*)),
            this, SIGNAL(progressItemCanceled(ProgressItem*)));

    connect(t, SIGNAL(progressItemStatus(ProgressItem*,QString)),
            this, SIGNAL(progressItemStatus(ProgressItem*,QString)));

    connect(t, SIGNAL(progressItemLabel(ProgressItem*,QString)),
            this, SIGNAL(progressItemLabel(ProgressItem*,QString)));

    connect(t, SIGNAL(progressItemUsesBusyIndicator(ProgressItem*,bool)),
            this, SIGNAL(progressItemUsesBusyIndicator(ProgressItem*,bool)));

    connect(t, SIGNAL(progressItemThumbnail(ProgressItem*,QPixmap)),
            this, SIGNAL(progressItemThumbnail(ProgressItem*,QPixmap)));

    d->addItem(t, parent);

    Q_EMIT progressItemAdded(t);
}

void ProgressManager::emitShowProgressViewImpl()
{
    Q_EMIT showProgressView();
}

void ProgressManager::slotTransactionCompleted(ProgressItem* item)
{
    if (!item)
    {
        return;
    }

    d->removeItem(item);

    // move to UI thread

    Q_EMIT completeTransactionDeferred(item);
}

void ProgressManager::slotTransactionCompletedDeferred(ProgressItem* item)
{
    Q_EMIT progressItemCompleted(item);

    item->deleteLater();
}

void ProgressManager::slotStandardCancelHandler(ProgressItem* item)
{
    item->setComplete();
}

ProgressItem* ProgressManager::singleItem() const
{
    QHash<QString, ProgressItem*> hash;

    {
        QMutexLocker lock(&d->mutex);
        hash = d->transactions;
    }

    ProgressItem* item = nullptr;
    QHash<QString, ProgressItem*>::const_iterator it = hash.constBegin();

    while (it != hash.constEnd())
    {
        // No single item for progress possible, as one of them is a busy indicator one.

        if ((*it)->usesBusyIndicator())
        {
            return nullptr;
        }

        // If it's a top level one, only those count.

        if (!(*it)->parent())
        {
            if (item)
            {
                return nullptr; // we found more than one
            }
            else
            {
                item = (*it);
            }
        }

        ++it;
    }

    return item;
}

void ProgressManager::slotAbortAll()
{
    QHash<QString, ProgressItem*> hash;

    {
        QMutexLocker lock(&d->mutex);

        if (d->transactions.isEmpty())
        {
            return;
        }

        hash = d->transactions;
    }

    QHashIterator<QString, ProgressItem*> it(hash);

    while (it.hasNext())
    {
        it.next();
        it.value()->cancel();
    }

    d->waitingLoop->exec(QEventLoop::ExcludeUserInputEvents);
}

void ProgressManager::slotTransactionViewIsEmpty()
{
    d->waitingLoop->quit();
}

void ProgressManager::emitShowProgressView()
{
    instance()->emitShowProgressViewImpl();
}

ProgressItem* ProgressManager::createProgressItem(const QString& label, const QString& status, bool canBeCanceled, bool hasThumb)
{
    return instance()->createProgressItemImpl(nullptr, instance()->getUniqueID(), label, status, canBeCanceled, hasThumb);
}

ProgressItem* ProgressManager::createProgressItem(ProgressItem* parent, const QString& id, const QString& label,
                                                  const QString& status, bool canBeCanceled, bool hasThumb)
{
    return instance()->createProgressItemImpl(parent, id, label, status, canBeCanceled, hasThumb);
}

ProgressItem* ProgressManager::createProgressItem(const QString& parent, const QString& id, const QString& label,
                                                  const QString& status, bool canBeCanceled, bool hasThumb)
{
    return instance()->createProgressItemImpl(parent, id, label, status, canBeCanceled, hasThumb);
}

ProgressItem* ProgressManager::createProgressItem(const QString& id, const QString& label, const QString& status,
                                                  bool canBeCanceled, bool hasThumb)
{
    return instance()->createProgressItemImpl(nullptr, id, label, status, canBeCanceled, hasThumb);
}

} // namespace Digikam

#include "moc_progressmanager.cpp"
