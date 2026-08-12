/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-08-20
 * Description : editor tool template class.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QString>
#include <QPixmap>
#include <QPoint>

// Local includes

#include "digikam_export.h"
#include "dcolor.h"
#include "previewtoolbar.h"
#include "filteraction.h"
#include "dplugineditor.h"

namespace Digikam
{

class DImgThreadedFilter;
class DImgThreadedAnalyser;
class EditorToolSettings;

class DIGIKAM_EXPORT EditorTool : public QObject
{
    Q_OBJECT

public:

    explicit EditorTool(QObject* const parent);
    ~EditorTool()                               override;

    void setPlugin(DPlugin* const _plugin);
    DPlugin* plugin()                     const;

    /**
     * Called by editor tool interface to initialized tool when all is ready, through slotInit().
     */
    void init();

    /**
     * Set this option to on if you want to call slotPreview() in slotInit() at tool startup.
     */
    void setInitPreview(bool b);

    QString                toolHelp()     const;
    QString                toolName()     const;
    int                    toolVersion()  const;
    QIcon                  toolIcon()     const;
    QWidget*               toolView()     const;
    EditorToolSettings*    toolSettings() const;
    FilterAction::Category toolCategory() const;

    virtual void setBackgroundColor(const QColor& bg);
    virtual void ICCSettingsChanged();
    virtual void exposureSettingsChanged();

public Q_SLOTS:

    void slotUpdateSpotInfo(const Digikam::DColor& col, const QPoint& point);
    void slotPreviewModeChanged();

    virtual void slotCloseTool();
    virtual void slotApplyTool();

Q_SIGNALS:

    void okClicked();
    void cancelClicked();

protected:

    void setToolInfoMessage(const QString& txt);
    void setToolHelp(const QString& anchor);
    void setToolName(const QString& name);
    void setToolVersion(const int version);
    void setToolIcon(const QIcon& icon);
    void setPreviewModeMask(int mask);
    void setToolCategory(const FilterAction::Category category);

    virtual void setToolView(QWidget* const view);
    virtual void setToolSettings(EditorToolSettings* const settings);
    virtual void setBusy(bool);
    virtual void readSettings();
    virtual void writeSettings();
    virtual void finalRendering()     {};

protected Q_SLOTS:

    void slotTimer();

    virtual void slotOk();
    virtual void slotCancel();
    virtual void slotInit();
    virtual void slotResetSettings();
    virtual void slotLoadSettings()   {};
    virtual void slotSaveAsSettings() {};
    virtual void slotPreview()        {};
    virtual void slotChannelChanged() {};
    virtual void slotScaleChanged()   {};

private:

    class Private;
    Private* const d = nullptr;
};

// -----------------------------------------------------------------

class DIGIKAM_EXPORT EditorToolThreaded : public EditorTool
{
    Q_OBJECT

public:

    enum RenderingMode
    {
        NoneRendering = 0,
        PreviewRendering,
        FinalRendering
    };

public:

    explicit EditorToolThreaded(QObject* const parent);
    ~EditorToolThreaded()                                     override;

    /**
     * Set the small text to show in editor status progress bar during
     * tool computation. If it's not set, tool name is used instead.
     */
    void setProgressMessage(const QString& mess);

    /**
     * return the current tool rendering mode.
     */
    RenderingMode renderingMode()                       const;

public Q_SLOTS:

    virtual void slotAbort();

protected:

    /**
     * Manage filter instance plugged in tool interface
     */
    DImgThreadedFilter* filter()                        const;
    void setFilter(DImgThreadedFilter* const _filter);

    /**
     * Manage analyser instance plugged in tool interface
     */
    DImgThreadedAnalyser* analyser()                    const;
    void setAnalyser(DImgThreadedAnalyser* const _analyser);

    /**
     * If true, delete filter instance when preview or final rendering is processed.
     * If false, filter instance will be managed outside for ex. with ContentAwareResizing tool.
     */
    void deleteFilterInstance(bool b = true);

    virtual void preparePreview()    {};
    virtual void prepareFinal()      {};
    virtual void setPreviewImage()   {};
    virtual void setFinalImage()     {};
    virtual void renderingFinished() {};
    virtual void analyserCompleted() {};

protected Q_SLOTS:

    /**
     * Manage start and end events from filter
     */
    void slotFilterStarted();
    void slotFilterFinished(bool success);

    /**
     * Manage start and end events from analyser
     */
    void slotAnalyserStarted();
    void slotAnalyserFinished(bool success);

    /**
     * Dispatch progress event from filter and analyser
     */
    void slotProgress(int progress);

    void slotInit()                                           override;
    void slotOk()                                             override;
    void slotCancel()                                         override;
    void slotPreview()                                        override;

private Q_SLOTS:

    void slotResized();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
