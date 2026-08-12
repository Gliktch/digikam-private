/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-03-14
 * Description : User interface for searches
 *
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QString>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>

// Local includes

#include "comboboxutilities.h"

class QTextEdit;
class QPushButton;

namespace Digikam
{

class AnimatedClearButton : public QWidget
{
    Q_OBJECT

public:

    explicit AnimatedClearButton(QWidget* const parent = nullptr);

    QSize sizeHint() const override;

    void setPixmap(const QPixmap& p);
    QPixmap pixmap() const;

    /**
     * Sets a primary condition for the button to be shown.
     * If false, animateVisible() will have no effect.
     */
    void setShallBeShown(bool show);

    /** This parameter determines the behavior when the animation
     *  to hide the widget has finished:
     *  If stayVisible is true, the widget remains visible,
     *  but paints nothing.
     *  If stayVisible is false, setVisible(false) is called,
     *  which removes the widget for layouting etc.
     *  Default: false
     */
    void stayVisibleWhenAnimatedOut(bool stayVisible);

public Q_SLOTS:

    /// Set visible, possibly with animation
    void animateVisible(bool visible);

    /// Set visible without animation
    void setDirectlyVisible(bool visible);

    /// Set enabled state for drawing the pixmap
    void slotPixmapEnabled(bool b);

Q_SIGNALS:

    void clicked();
    void visibleChanged(bool v);

protected:

    void paintEvent(QPaintEvent* event)        override;
    void mouseReleaseEvent(QMouseEvent* event) override;

protected Q_SLOTS:

    void slotVisibleChanged();
    void slotUpdateAnimationSettings();

private:

    class Private;
    Private* const d = nullptr;
};

// -------------------------------------------------------------------------

class CustomStepsDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

public:

    /**
     * This is a normal QDoubleSpinBox which allows to
     * customize the stepping behavior, for cases where
     * linear steps are not applicable
     */
    explicit CustomStepsDoubleSpinBox(QWidget* const parent = nullptr);
    ~CustomStepsDoubleSpinBox() override;

    /**
     * Set a list of values that are usually applicable for the
     * type of data of the combo box. The user can still type in
     * any other value. Boundaries are not touched.
     * Up or below the min and max values of the list given,
     * default stepping is used.
     */
    void setSuggestedValues(const QList<double>& values);

    /**
     * Sets the value that should be set as first value
     * when first moving away from the minimum value.
     */
    void setSuggestedInitialValue(double initialValue);

    /**
     * Allows to set to different default single steps,
     * for the range below m_values, the other for above.
     */
    void setSingleSteps(double smaller, double larger);

    void setInvertStepping(bool invert);

    /**
     * Resets to minimum value.
     */
    void reset();

    void stepBy(int steps) override;

private Q_SLOTS:

    void slotValueChanged(double val);

private:

    class Private;
    Private* const d = nullptr;
};

// -------------------------------------------------------------------------

class CustomStepsIntSpinBox : public QSpinBox
{
    Q_OBJECT

public:

    /**
     * This is a normal QIntSpinBox which allows to
     * customize the stepping behavior, for cases where
     * linear steps are not applicable
     */
    explicit CustomStepsIntSpinBox(QWidget* const parent = nullptr);
    ~CustomStepsIntSpinBox() override;

    /**
     * Set a list of values that are usually applicable for the
     * type of data of the combo box. The user can still type in
     * any other value. Boundaries are not touched.
     * Up or below the min and max values of the list given,
     * default stepping is used.
     */
    void setSuggestedValues(const QList<int>& values);

    /**
     * Sets the value that should be set as first value
     * when first moving away from the minimum value.
     */
    void setSuggestedInitialValue(int initialValue);

    /**
     * Allows to set to different default single steps,
     * for the range below m_values, the other for above.
     */
    void setSingleSteps(int smaller, int larger);

    void setInvertStepping(bool invert);

    /**
     * Call this with a fraction prefix (like "1/") to enable
     * magic handling of the value as fraction denominator.
     */
    void enableFractionMagic(const QString& prefix);

    /**
     * Resets to minimum value
     */
    void reset();

    /**
     * value() and setValue() for fraction magic value.
     */
    double fractionMagicValue() const;
    void   setFractionMagicValue(double value);

    void stepBy(int steps) override;

protected:

    QString textFromValue(int value) const override;
    int valueFromText(const QString& text) const override;
    StepEnabled stepEnabled() const override;

private Q_SLOTS:

    void slotValueChanged(int val);

private:

    class Private;
    Private* const d = nullptr;
};

// -------------------------------------------------------------------------

class StyleSheetDebugger : public QWidget
{
    Q_OBJECT

public:

    /**
     * This widget is for development purpose only:
     * It allows the developer to change the style sheet
     * on a widget dynamically.
     * If you want to develop or debug the stylesheet on your widget,
     * add temporary code:
     * new StyleSheetDebugger(myWidget);
     * That's all. Change the style sheet by editing it and pressing Ok.
     */
    explicit StyleSheetDebugger(QWidget* const object);

protected Q_SLOTS:

    void buttonClicked();

protected:

    QTextEdit*   m_edit         = nullptr;
    QPushButton* m_okButton     = nullptr;
    QWidget*     m_widget       = nullptr;
};

} // namespace Digikam
