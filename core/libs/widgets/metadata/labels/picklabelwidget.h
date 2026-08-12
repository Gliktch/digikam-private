/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-02-14
 * Description : pick label widget
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QColor>
#include <QPushButton>
#include <QEvent>
#include <QList>
#include <QMetaType>
#include <QMenu>

// Local includes

#include "dlayoutbox.h"
#include "digikam_globals.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT PickLabelWidget : public DVBox
{
    Q_OBJECT

public:

    explicit PickLabelWidget(QWidget* const parent = nullptr);
    ~PickLabelWidget() override;

    /**
     * Show or not on the bottom view the description of label with shortcuts.
     */
    void setDescriptionBoxVisible(bool b);

    /**
     * Set all Color Label buttons exclusive or not. Default is true as only one can be selected.
     * Non-exclusive mode is dedicated for Advanced Search tool.
     */
    void setButtonsExclusive(bool b);

    /**
     * Turn on Color Label buttons using list. Pass an empty list to clear all selection.
     */
    void setPickLabels(const QList<PickLabel>& list);

    /**
     * Return the list of Pick Label buttons turned on or an empty list of none.
     */
    QList<PickLabel> pickLabels() const;

public:

    static QString labelPickName(PickLabel label);

    static QIcon buildIcon(PickLabel label);

Q_SIGNALS:

    void signalPickLabelChanged(int);

protected:

    bool eventFilter(QObject* obj, QEvent* ev) override;

private:

    void updateDescription(PickLabel label);

private:

    class Private;
    Private* const d = nullptr;
};

// ------------------------------------------------------------------------------

class DIGIKAM_EXPORT PickLabelSelector : public QPushButton
{
    Q_OBJECT

public:

    explicit PickLabelSelector(QWidget* const parent = nullptr);
    ~PickLabelSelector() override;

    void setPickLabel(PickLabel label);
    PickLabel pickLabel();

    PickLabelWidget* pickLabelWidget() const;

Q_SIGNALS:

    void signalPickLabelChanged(int);

private Q_SLOTS:

    void slotPickLabelChanged(int);

private:

    class Private;
    Private* const d = nullptr;
};

// ------------------------------------------------------------------------------

class DIGIKAM_EXPORT PickLabelMenuAction : public QMenu
{
    Q_OBJECT

public:

    explicit PickLabelMenuAction(QMenu* const parent = nullptr);
    ~PickLabelMenuAction() override = default;

Q_SIGNALS:

    void signalPickLabelChanged(int);
};

} // namespace Digikam

Q_DECLARE_METATYPE(QList<Digikam::PickLabel>)
