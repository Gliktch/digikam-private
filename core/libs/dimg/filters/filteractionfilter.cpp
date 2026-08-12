/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-11-10
 * Description : meta-filter to apply FilterAction
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "filteractionfilter.h"

// Qt includes

#include <QScopedPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dimgbuiltinfilter.h"
#include "dimgfiltermanager.h"

namespace Digikam
{

class Q_DECL_HIDDEN FilterActionFilter::Private
{
public:

    Private() = default;

    bool                continueOnError = false;

    QList<FilterAction> actions;
    QList<FilterAction> appliedActions;

    QString             errorMessage;
};

FilterActionFilter::FilterActionFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d                 (new Private)
{
    initFilter();
}

FilterActionFilter::~FilterActionFilter()
{
    delete d;
}

void FilterActionFilter::setContinueOnError(bool cont)
{
    d->continueOnError = cont;
}

void FilterActionFilter::setFilterActions(const QList<FilterAction>& actions)
{
    d->actions = actions;
}

void FilterActionFilter::addFilterActions(const QList<FilterAction>& actions)
{
    d->actions += actions;
}

void FilterActionFilter::setFilterAction(const FilterAction& action)
{
    d->actions.clear();
    d->actions << action;
}

void FilterActionFilter::addFilterAction(const FilterAction& action)
{
    d->actions << action;
}

QList<FilterAction> FilterActionFilter::filterActions() const
{
    return d->actions;
}

bool FilterActionFilter::isReproducible() const
{
    for (const FilterAction& action : std::as_const(d->actions))
    {
        if (
            !action.isNull() &&
            (action.category() != FilterAction::ReproducibleFilter)
           )
        {   // cppcheck-suppress useStlAlgorithm
            return false;
        }
    }

    return true;
}

bool FilterActionFilter::isComplexAction() const
{
    for (const FilterAction& action : std::as_const(d->actions))
    {
        if (
            !action.isNull()                                        &&
            (action.category() != FilterAction::ReproducibleFilter) &&
            (action.category() != FilterAction::ComplexFilter)
           )
        {   // cppcheck-suppress useStlAlgorithm
            return false;
        }
    }

    return true;
}

bool FilterActionFilter::isSupported() const
{
    for (const FilterAction& action : std::as_const(d->actions))
    {
        if (!action.isNull() && !DImgFilterManager::instance()->isSupported(action.identifier(), action.version()))
        {   // cppcheck-suppress useStlAlgorithm
            return false;
        }
    }

    return true;
}

bool FilterActionFilter::completelyApplied() const
{
    return (d->appliedActions.size() == d->actions.size());
}

QList<FilterAction> FilterActionFilter::appliedFilterActions() const
{
    return d->appliedActions;
}

FilterAction FilterActionFilter::failedAction() const
{
    if (d->appliedActions.size() >= d->actions.size())
    {
        return FilterAction();
    }

    return d->actions.at(d->appliedActions.size());
}

int FilterActionFilter::failedActionIndex() const
{
    return d->appliedActions.size();
}

QString FilterActionFilter::failedActionMessage() const
{
    return d->errorMessage;
}

void FilterActionFilter::filterImage()
{
    d->appliedActions.clear();
    d->errorMessage.clear();
    const float progressIncrement = 1.0 / qMax(1, d->actions.size());
    float progress                = 0;

    postProgress(0);

    DImg img = m_orgImage;

    for (const FilterAction& action : std::as_const(d->actions))
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Replaying action" << action.identifier();

        if (action.isNull())
        {
            continue;
        }

        if (DImgBuiltinFilter::isSupported(action.identifier()))
        {
            DImgBuiltinFilter filter(action);

            if (!filter.isValid())
            {
                d->errorMessage = i18n("Built-in transformation not supported");

                if (d->continueOnError)
                {
                    continue;
                }
                else
                {
                    break;
                }
            }

            filter.apply(img);
            d->appliedActions << filter.filterAction();
        }
        else
        {
            QScopedPointer<DImgThreadedFilter> filter
            (DImgFilterManager::instance()->createFilter(action.identifier(), action.version()));

            if (!filter)
            {
                d->errorMessage = i18n("Filter identifier or version is not supported");

                if (d->continueOnError)
                {
                    continue;
                }
                else
                {
                    break;
                }
            }

            filter->readParameters(action);

            if (!filter->parametersSuccessfullyRead())
            {
                d->errorMessage = filter->readParametersError(action);

                if (d->continueOnError)
                {
                    continue;
                }
                else
                {
                    break;
                }
            }

            // compute

            filter->setupAndStartDirectly(img, this, (int)progress, (int)(progress + progressIncrement));
            img = filter->getTargetImage();
            d->appliedActions << filter->filterAction();
        }

        progress += progressIncrement;
        postProgress((int)progress);
    }

    m_destImage = img;
}

} // namespace Digikam

#include "moc_filteractionfilter.cpp"
