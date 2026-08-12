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

#pragma once

// Qt includes

#include <QScrollArea>

// Local includes

#include "overlaywidget.h"
#include "digikam_export.h"

namespace Digikam
{
class ProgressItem;

class TransactionItem : public DVBox
{
    Q_OBJECT

public:

    explicit TransactionItem(QWidget* const parent, ProgressItem* const item, bool first);
    ~TransactionItem() override;

    void hideHLine();

    void setProgress(int progress);
    void setLabel(const QString&);
    void setThumbnail(const QPixmap&);

    /**
     * @note the given text is interpreted as RichText, so you might need to
     *       use .toHtmlEscaped() it before passing.
     */
    void setStatus(const QString&);

    void setTotalSteps(int totalSteps);

    ProgressItem* item() const;

    void addSubTransaction(ProgressItem* const item);

    /**
     * @brief The progressitem is deleted immediately, we take 5s to go out,
     * so better not use mItem during this time.
     */
    void setItemComplete();

public Q_SLOTS:

    void slotItemCanceled();

private:

    class Private;
    Private* const d = nullptr;
};

// --------------------------------------------------------------------------------

class TransactionItemView : public QScrollArea
{
    Q_OBJECT

public:

    explicit TransactionItemView(QWidget* const parent = nullptr, const QString& name=QString());
    ~TransactionItemView() override = default;

    TransactionItem* addTransactionItem(ProgressItem* item, bool first);

    QSize sizeHint()        const override;
    QSize minimumSizeHint() const override;

public Q_SLOTS:

    void slotLayoutFirstItem();

Q_SIGNALS:

    void signalTransactionViewIsEmpty();

protected:

    void resizeEvent(QResizeEvent* event) override;

private:

    DVBox* m_bigBox = nullptr;
};

// --------------------------------------------------------------------------------

class DIGIKAM_EXPORT ProgressView : public OverlayWidget
{
    Q_OBJECT

public:

    explicit ProgressView(QWidget* const alignWidget, QWidget* const parent,
                          const QString& name = QString());
    ~ProgressView()                 override;

    void setVisible(bool b)         override;

public Q_SLOTS:

    void slotToggleVisibility();

Q_SIGNALS:

    void visibilityChanged(bool);

protected Q_SLOTS:

    void slotTransactionAdded(ProgressItem*);
    void slotTransactionCompleted(ProgressItem*);
    void slotTransactionCanceled(ProgressItem*);
    void slotTransactionProgress(ProgressItem*, unsigned int progress);
    void slotTransactionStatus(ProgressItem*, const QString&);
    void slotTransactionLabel(ProgressItem*, const QString&);
    void slotTransactionUsesBusyIndicator(ProgressItem*, bool);
    void slotTransactionThumbnail(ProgressItem*, const QPixmap&);
    void slotClose();
    void slotShow();
    void slotHide();

protected:

    void closeEvent(QCloseEvent*)   override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
