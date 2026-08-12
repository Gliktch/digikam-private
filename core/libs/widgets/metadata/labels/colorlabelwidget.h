/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-01-28
 * Description : color label widget
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
#include <QMap>

// Local includes

#include "dlayoutbox.h"
#include "digikam_globals.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ColorLabelWidget : public DVBox
{
    Q_OBJECT

public:

    explicit ColorLabelWidget(QWidget* const parent = nullptr);
    ~ColorLabelWidget()                             override;

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
    void setColorLabels(const QList<ColorLabel>& list);

    /**
     * Return the list of Color Label buttons turned on or an empty list of none.
     */
    QList<ColorLabel> colorLabels() const;

public:

    static QColor  labelColor(ColorLabel label);
    static QString labelColorName(ColorLabel label);

    static QIcon buildIcon(ColorLabel label, int size = 12);

Q_SIGNALS:

    void signalColorLabelChanged(int);

protected:

    bool eventFilter(QObject* obj, QEvent* ev)      override;

private:

    void updateDescription(ColorLabel label);

private:

    class Private;
    Private* const d = nullptr;
};

// ------------------------------------------------------------------------------

class DIGIKAM_EXPORT ColorLabelSelector : public QPushButton
{
    Q_OBJECT

public:

    explicit ColorLabelSelector(QWidget* const parent = nullptr);
    ~ColorLabelSelector() override;

    void setColorLabel(ColorLabel label);
    ColorLabel colorLabel();

    ColorLabelWidget* colorLabelWidget() const;

Q_SIGNALS:

    void signalColorLabelChanged(int);

private Q_SLOTS:

    void slotColorLabelChanged(int);

private:

    class Private;
    Private* const d = nullptr;
};

// ------------------------------------------------------------------------------

class DIGIKAM_EXPORT ColorLabelMenuAction : public QMenu
{
    Q_OBJECT

public:

    explicit ColorLabelMenuAction(QMenu* const parent = nullptr);
    ~ColorLabelMenuAction() override = default;

Q_SIGNALS:

    void signalColorLabelChanged(int);
};

} // namespace Digikam

Q_DECLARE_METATYPE(QList<Digikam::ColorLabel>)
