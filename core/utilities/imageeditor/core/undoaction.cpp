/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-02-06
 * Description : undo actions manager for image editor.
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005      by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "undoaction.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "editorcore.h"

namespace Digikam
{

UndoMetadataContainer UndoMetadataContainer::fromImage(const DImg& img)
{
    UndoMetadataContainer container;
    container.history = img.getItemHistory();
    container.profile = img.getIccProfile();

    return container;
}

void UndoMetadataContainer::toImage(DImg& img) const
{
    img.setItemHistory(history);
    img.setIccProfile(profile);
}

bool UndoMetadataContainer::changesIccProfile(const DImg& target) const
{
    return !(profile == target.getIccProfile());
}

// ---------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN UndoAction::Private
{
public:

    Private() = default;

    QString               title;
    QVariant              fileOrigin;

    UndoMetadataContainer container;
    DImageHistory         fileOriginResolvedHistory;
};

UndoAction::UndoAction(const EditorCore* const core)
    : d(new Private)
{
    d->container = UndoMetadataContainer::fromImage(*core->getImg());
}

UndoAction::~UndoAction()
{
    delete d;
}

void UndoAction::setTitle(const QString& title)
{
    d->title = title;
}

QString UndoAction::getTitle() const
{
    return d->title;
}

void UndoAction::setMetadata(const UndoMetadataContainer& c)
{
    d->container = c;
}

UndoMetadataContainer UndoAction::getMetadata() const
{
    return d->container;
}

bool UndoAction::hasFileOriginData() const
{
    return (!d->fileOrigin.isNull());
}

void UndoAction::setFileOriginData(const QVariant& data,
                                   const DImageHistory& resolvedInitialHistory)
{
    d->fileOrigin                = data;
    d->fileOriginResolvedHistory = resolvedInitialHistory;
}

QVariant UndoAction::fileOriginData() const
{
    return d->fileOrigin;
}

DImageHistory UndoAction::fileOriginResolvedHistory() const
{
    return d->fileOriginResolvedHistory;
}

// ---------------------------------------------------------------------------------------------

UndoActionReversible::UndoActionReversible(const EditorCore* const core,
                                           const DImgBuiltinFilter& reversibleFilter)
    : UndoAction(core),
      m_filter  (reversibleFilter)
{
    setTitle(m_filter.i18nDisplayableName());
}

DImgBuiltinFilter UndoActionReversible::getFilter() const
{
    return m_filter;
}

DImgBuiltinFilter UndoActionReversible::getReverseFilter() const
{
    return m_filter.reverseFilter();
}

// ---------------------------------------------------------------------------------------------

UndoActionIrreversible::UndoActionIrreversible(const EditorCore* const core,
                                               const QString& title)
    : UndoAction(core)
{
    setTitle(title.isNull() ? i18nc("@title: menu entry to undo unknown previous action", "Unknown")
                            : title);
}

} // namespace Digikam
