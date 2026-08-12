/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-01-10
 * Description : a tool bar for preview mode
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QAction>

// Local includes

#include "digikam_export.h"

class KConfigGroup;

namespace Digikam
{

class EditorWindow;

class DIGIKAM_EXPORT PreviewToolBar : public QWidget
{
    Q_OBJECT

public:

    enum PreviewMode
    {
        PreviewOriginalImage      = 0x00000001,      ///< Original image only.
        PreviewBothImagesHorz     = 0x00000002,      ///< Horizontal with original and target duplicated.
        PreviewBothImagesVert     = 0x00000004,      ///< Vertical with original and target duplicated.
        PreviewBothImagesHorzCont = 0x00000008,      ///< Horizontal with original and target in contiguous.
        PreviewBothImagesVertCont = 0x00000010,      ///< Vertical with original and target in contiguous.
        PreviewTargetImage        = 0x00000020,      ///< Target image only.
        PreviewToggleOnMouseOver  = 0x00000040,      ///< Original image if mouse is over image area, else target image.
        NoPreviewMode             = 0x00000080,      ///< Target image only without information displayed.

        AllPreviewModes           = PreviewOriginalImage      | PreviewBothImagesHorz     |
                                    PreviewBothImagesVert     | PreviewBothImagesHorzCont |
                                    PreviewBothImagesVertCont | PreviewTargetImage        |
                                    PreviewToggleOnMouseOver,

        UnSplitPreviewModes       = PreviewOriginalImage | PreviewTargetImage | PreviewToggleOnMouseOver
    };

public:

    explicit PreviewToolBar(QWidget* const parent = nullptr);
    ~PreviewToolBar()               override;

    void setPreviewModeMask(int mask);

    void setPreviewMode(PreviewMode mode);
    PreviewMode previewMode() const;

    void readSettings(const KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    void registerMenuActionGroup(EditorWindow* const editor);

Q_SIGNALS:

    void signalPreviewModeChanged(int);

private Q_SLOTS:

    void slotButtonReleased(int);
    void slotActionTriggered(QAction*);

private:

    void setCheckedAction(int id);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
