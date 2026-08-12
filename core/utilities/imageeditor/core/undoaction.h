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

#pragma once

// Local includes

#include "digikam_export.h"
#include "dimagehistory.h"
#include "dimg.h"
#include "dimgbuiltinfilter.h"

namespace Digikam
{

class EditorCore;

class DIGIKAM_EXPORT UndoMetadataContainer
{
public:

    /**
     * Fill a container from the DImg
     */
    static UndoMetadataContainer fromImage(const DImg& img);

    /**
     * Write this container's values to the DImg
     */
    void toImage(DImg& img) const;

    bool changesIccProfile(const DImg& target) const;

public:

    DImageHistory history;
    IccProfile    profile;
};

// -------------------------------------------------------------------

class DIGIKAM_EXPORT UndoAction
{

public:

    explicit UndoAction(const EditorCore* const core);
    virtual ~UndoAction();

    void                  setTitle(const QString& title);
    QString               getTitle()                  const;

    void                  setMetadata(const UndoMetadataContainer&);
    UndoMetadataContainer getMetadata()               const;

    void                  setFileOriginData(const QVariant& data,
                                            const DImageHistory& resolvedInitialHistory);
    bool                  hasFileOriginData()         const;
    QVariant              fileOriginData()            const;
    DImageHistory         fileOriginResolvedHistory() const;

private:

    /// @note disabled
    UndoAction(const UndoAction&)            = delete;
    UndoAction& operator=(const UndoAction&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

// --------------------------------------------------------------------

class DIGIKAM_EXPORT UndoActionReversible : public UndoAction
{

public:

    explicit UndoActionReversible(const EditorCore* const core,
                                  const DImgBuiltinFilter& reversibleFilter);

    DImgBuiltinFilter getFilter()        const;
    DImgBuiltinFilter getReverseFilter() const;

protected:

    DImgBuiltinFilter m_filter;
};

// --------------------------------------------------------------------

class DIGIKAM_EXPORT UndoActionIrreversible : public UndoAction
{

public:

    explicit UndoActionIrreversible(const EditorCore* const core,
                                    const QString& title = QString());
    ~UndoActionIrreversible() override = default;
};

} // namespace Digikam
