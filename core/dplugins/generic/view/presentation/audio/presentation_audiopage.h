/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-14
 * Description : a presentation tool.
 *
 * SPDX-FileCopyrightText: 2008-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "digikam_config.h"

// Qt includes

#include <QTime>
#include <QMutex>
#include <QUrl>
#include <QDialog>

// Local includes

#include "ui_presentation_audiopage.h"
#include "presentationaudiowidget.h"
#include "presentationaudiolist.h"

namespace DigikamGenericPresentationPlugin
{

class PresentationAudioWidget;
class PresentationContainer;

class SoundtrackPreview : public QDialog
{
    Q_OBJECT

public:

    explicit SoundtrackPreview(QWidget* const,
                               const QList<QUrl>&, PresentationContainer* const);
    ~SoundtrackPreview() override = default;

private:

    PresentationAudioWidget* m_playbackWidget = nullptr;
};

// ----------------------------------------------------------------------

class PresentationAudioPage : public QWidget,
                              public Ui::PresentationAudioPage
{
    Q_OBJECT

public:

    explicit PresentationAudioPage(QWidget* const parent,
                                   PresentationContainer* const sharedData);
    ~PresentationAudioPage() override;

    void readSettings();
    void saveSettings();

private:

    void addItems(const QList<QUrl>& fileList);
    void updateTracksNumber();
    void updateFileList();
    void compareTimes();

private Q_SLOTS:

    void slotAddDropItems(const QList<QUrl>& filesUrl);
    void slotSoundFilesButtonAdd();
    void slotSoundFilesButtonDelete();
    void slotSoundFilesButtonUp();
    void slotSoundFilesButtonDown();
    void slotSoundFilesButtonLoad();
    void slotSoundFilesButtonSave();
    void slotSoundFilesButtonReset();
    void slotSoundFilesSelected(int);
    void slotPreviewButtonClicked();
    void slotImageTotalTimeChanged(const QTime&);
    void slotAddNewTime(const QUrl&, const QTime&);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericPresentationPlugin
