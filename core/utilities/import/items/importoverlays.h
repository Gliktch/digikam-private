/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-08-21
 * Description : Overlays for the import interface
 *
 * SPDX-FileCopyrightText: 2012      by Islam Wazery <wazery at ubuntu dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAbstractButton>
#include <QAbstractItemView>

// Local includes

#include "itemviewhoverbutton.h"
#include "itemdelegateoverlay.h"
#include "itemviewimportdelegate.h"
#include "ratingwidget.h"

namespace Digikam
{

class ImportOverlayWidget : public QAbstractButton
{
    Q_OBJECT

public:

    explicit ImportOverlayWidget(QWidget* const parent = nullptr);

protected:

    void paintEvent(QPaintEvent*) override;
};

// --------------------------------------------------------------------

class ImportCoordinatesOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT
    REQUIRE_DELEGATE(ItemViewImportDelegate)

public:

    explicit ImportCoordinatesOverlay(QObject* const parent);
    ImportOverlayWidget* buttonWidget() const;

protected:

    void updatePosition();

    QWidget* createWidget()                         override;
    void setActive(bool active)                     override;
    void visualChange()                             override;
    bool checkIndex(const QModelIndex& index) const override;
    void slotEntered(const QModelIndex& index)      override;

protected:

    QPersistentModelIndex m_index;
};

// --------------------------------------------------------------------

class ImportLockOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT
    REQUIRE_DELEGATE(ItemViewImportDelegate)

public:

    explicit ImportLockOverlay(QObject* const parent);
    ImportOverlayWidget* buttonWidget()       const;

protected:

    void updatePosition();

    QWidget* createWidget()                         override;
    void setActive(bool active)                     override;
    void visualChange()                             override;
    bool checkIndex(const QModelIndex& index) const override;
    void slotEntered(const QModelIndex& index)      override;

protected:

    QPersistentModelIndex m_index;
};

// --------------------------------------------------------------------

class ImportDownloadOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT
    REQUIRE_DELEGATE(ItemViewImportDelegate)

public:

    explicit ImportDownloadOverlay(QObject* const parent);
    ImportOverlayWidget* buttonWidget()       const;

protected:

    void updatePosition();

    QWidget* createWidget()                         override;
    void setActive(bool active)                     override;
    void visualChange()                             override;
    bool checkIndex(const QModelIndex& index) const override;
    void slotEntered(const QModelIndex& index)      override;

protected:

    QPersistentModelIndex m_index;
};

// ------------------------------------------------------------------------------------------------

class ImportRatingOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT
    REQUIRE_DELEGATE(ItemViewImportDelegate)

public:

    explicit ImportRatingOverlay(QObject* const parent);
    RatingWidget* ratingWidget() const;

Q_SIGNALS:

    void ratingEdited(const QList<QModelIndex>& indexes, int rating);

protected Q_SLOTS:

    void slotRatingChanged(int);
    void slotDataChanged(const QModelIndex&, const QModelIndex&);

protected:

    QWidget* createWidget()                    override;
    void setActive(bool)                       override;
    void visualChange()                        override;
    void hide()                                override;
    void slotEntered(const QModelIndex& index) override;
    void widgetEnterEvent()                    override;
    void widgetLeaveEvent()                    override;

    void updatePosition();
    void updateRating();

protected:

    QPersistentModelIndex m_index;
};

// ------------------------------------------------------------------------------------------------

enum ImportRotateOverlayDirection
{
    ImportRotateOverlayLeft,
    ImportRotateOverlayRight
};

class ImportRotateOverlayButton : public ItemViewHoverButton
{
    Q_OBJECT

public:

    explicit ImportRotateOverlayButton(ImportRotateOverlayDirection dir, QAbstractItemView* const parentView);
    QSize sizeHint() const  override;

protected:

    QIcon icon()            override;
    void updateToolTip()    override;

protected:

    ImportRotateOverlayDirection const m_direction;
};

// --------------------------------------------------------------------

class ImportRotateOverlay : public HoverButtonDelegateOverlay
{
    Q_OBJECT

public:

    explicit ImportRotateOverlay(ImportRotateOverlayDirection dir, QObject* const parent);
    void setActive(bool active)                       override;

    ImportRotateOverlayDirection direction()    const { return m_direction;                             }
    bool isLeft()                               const { return m_direction == ImportRotateOverlayLeft;  }
    bool isRight()                              const { return m_direction == ImportRotateOverlayRight; }

    static ImportRotateOverlay* left (QObject* const parent) { return new ImportRotateOverlay(ImportRotateOverlayLeft,  parent); }
    static ImportRotateOverlay* right(QObject* const parent) { return new ImportRotateOverlay(ImportRotateOverlayRight, parent); }

Q_SIGNALS:

    void signalRotate(const QList<QModelIndex>& indexes);

protected:

    ItemViewHoverButton* createButton()               override;
    void updateButton(const QModelIndex& index)       override;
    bool checkIndex(const QModelIndex& index)   const override;
    void widgetEnterEvent()                           override;
    void widgetLeaveEvent()                           override;

private Q_SLOTS:

    void slotClicked();

private:

    ImportRotateOverlayDirection const m_direction;
};

} // namespace Digikam
