/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-02-06
 * Description : an image editor actions undo/redo manager
 *
 * SPDX-FileCopyrightText: 2005-2006 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2006 by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QStringList>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DImageHistory;
class DImg;
class EditorCore;
class UndoAction;
class UndoMetadataContainer;

class DIGIKAM_EXPORT UndoManager
{

public:

    explicit UndoManager(EditorCore* const core);
    ~UndoManager();

    void addAction(UndoAction* const action);
    void undo();
    void redo();
    void rollbackToOrigin();
    bool putImageDataAndHistory(DImg* const img, int stepsBack) const;

    void clear(bool clearCache = true);

    bool          anyMoreUndo()                                 const;
    bool          anyMoreRedo()                                 const;
    int           availableUndoSteps()                          const;
    int           availableRedoSteps()                          const;
    QStringList   getUndoHistory()                              const;
    QStringList   getRedoHistory()                              const;
    bool          isAtOrigin()                                  const;
    void          setOrigin()                                   const;
    bool          hasChanges()                                  const;

    /// The history if all available redo steps are redone
    DImageHistory getImageHistoryOfFullRedo()                   const;

    void clearPreviousOriginData();

private:

    void clearUndoActions();
    void clearRedoActions();
    void undoStep(bool saveRedo, bool execute, bool flyingRollback);
    void redoStep(bool execute, bool flyingRollback);
    void makeSnapshot(int index);
    void restoreSnapshot(int index, const UndoMetadataContainer& c);
    void getSnapshot(int index, DImg* const img)                const;

private:

    /// @note disabled
    UndoManager(const UndoManager&)            = delete;
    UndoManager& operator=(const UndoManager&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
