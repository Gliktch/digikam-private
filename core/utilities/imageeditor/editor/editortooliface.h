/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-08-20
 * Description : Image editor interface used by editor tools.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class EditorTool;
class EditorWindow;
class ExposureSettingsContainer;
class ICCSettingsContainer;
class DCategorizedView;

class DIGIKAM_EXPORT EditorToolIface : public QObject
{
    Q_OBJECT

public:

    static EditorToolIface* editorToolIface();

    explicit EditorToolIface(EditorWindow* const editor);
    ~EditorToolIface() override;

    EditorTool* currentTool() const;

    void loadTool(EditorTool* const tool);
    void unLoadTool();

    void setPreviewModeMask(int mask);
    void setToolsIconView(DCategorizedView* const view);

    void setToolStartProgress(const QString& toolName);
    void setToolProgress(int progress);
    void setToolStopProgress();

    void setToolInfoMessage(const QString& txt);

    void setupICC();

    void themeChanged();
    void updateExposureSettings();
    void updateICCSettings();

Q_SIGNALS:

    void signalPreviewModeChanged();

public Q_SLOTS:

    void slotCloseTool();
    void slotToolAborted();
    void slotApplyTool();
    void slotToolApplied();

private:

    /// @note disabled
    EditorToolIface(QObject*) = delete;

private:

    static EditorToolIface* m_iface;

    class Private;
    Private* const          d = nullptr;
};

} // namespace Digikam
