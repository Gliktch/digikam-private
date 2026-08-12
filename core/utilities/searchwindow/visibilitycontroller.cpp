/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-03-14
 * Description : User interface for searches
 *
 * SPDX-FileCopyrightText: 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "visibilitycontroller.h"

namespace Digikam
{

class Q_DECL_HIDDEN VisibilityController::Private
{
public:

    Private() = default;

    VisibilityController::Status status             = VisibilityController::Unknown;
    QList<VisibilityObject*>     objects;
    QWidget*                     containerWidget    = nullptr;
};

class Q_DECL_HIDDEN VisibilityWidgetWrapper : public QObject,
                                              public VisibilityObject
{
    Q_OBJECT

public:

    VisibilityWidgetWrapper(VisibilityController* const parent, QWidget* const widget)
        : QObject (parent),
          m_widget(widget)
    {
    }

    void setVisible(bool visible) override
    {
        m_widget->setVisible(visible);
    }

    bool isVisible() override
    {
        return m_widget->isVisible();
    }

    QWidget* m_widget = nullptr;
};

// -------------------------------------------------------------------------------------

VisibilityController::VisibilityController(QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
}

VisibilityController::~VisibilityController()
{
    delete d;
}

void VisibilityController::addObject(VisibilityObject* const object)
{
    d->objects << object;

    // create clean state

    if (d->status == Unknown)
    {
        if (object->isVisible())
        {
            d->status = Shown;
        }
        else
        {
            d->status = Hidden;
        }
    }

    // set state on object

    if ((d->status == Shown) || (d->status == Showing))
    {
        object->setVisible(true);
    }
    else
    {
        object->setVisible(false);
    }
}

void VisibilityController::addWidget(QWidget* const widget)
{
    addObject(new VisibilityWidgetWrapper(this, widget));
}

void VisibilityController::setContainerWidget(QWidget* const widget)
{
    d->containerWidget = widget;
}

void VisibilityController::setVisible(bool shallBeVisible)
{
    if (shallBeVisible)
    {
        if ((d->status == Shown) || (d->status == Showing))
        {
            return;
        }

        d->status = Showing;
        beginStatusChange();
    }
    else
    {
        if ((d->status == Hidden) || (d->status == Hiding))
        {
            return;
        }

        d->status = Hiding;
        beginStatusChange();
    }
}

void VisibilityController::show()
{
    setVisible(true);
}

void VisibilityController::hide()
{
    setVisible(false);
}

void VisibilityController::triggerVisibility()
{
    if ((d->status == Shown) || (d->status == Showing) || (d->status == Unknown))
    {
        setVisible(false);
    }
    else
    {
        setVisible(true);
    }
}

bool VisibilityController::isVisible() const
{
    if ((d->status == Shown) || (d->status == Showing))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void VisibilityController::beginStatusChange()
{
    allSteps();
}

void VisibilityController::step()
{
    if      (d->status == Showing)
    {
        for (VisibilityObject* const o : std::as_const(d->objects))
        {
            if (!o->isVisible())
            {   // cppcheck-suppress useStlAlgorithm
                o->setVisible(true);
                return;
            }
        }
    }
    else if (d->status == Hiding)
    {
        for (VisibilityObject* const o : std::as_const(d->objects))
        {
            if (o->isVisible())
            {   // cppcheck-suppress useStlAlgorithm
                o->setVisible(false);
                return;
            }
        }
    }
}

void VisibilityController::allSteps()
{
    if      (d->status == Showing)
    {
        if (d->containerWidget)
        {
            d->containerWidget->setUpdatesEnabled(false);
        }

        for (VisibilityObject* const o : std::as_const(d->objects))
        {
            o->setVisible(true);
        }

        if (d->containerWidget)
        {
            d->containerWidget->setUpdatesEnabled(true);
        }
    }
    else if (d->status == Hiding)
    {
        if (d->containerWidget)
        {
            d->containerWidget->setUpdatesEnabled(false);
        }

        for (VisibilityObject* const o : std::as_const(d->objects))
        {
            o->setVisible(false);
        }

        if (d->containerWidget)
        {
            d->containerWidget->setUpdatesEnabled(true);
        }
    }
}

} // namespace Digikam

#include "visibilitycontroller.moc"

#include "moc_visibilitycontroller.cpp"
