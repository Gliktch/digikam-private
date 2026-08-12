/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-09
 * Description : a presentation tool.
 *
 * SPDX-FileCopyrightText: 2008-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * SPDX-FileCopyrightText:      2009 by Andi Clemens <andi dot clemens at googlemail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2021 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QPixmap>

// Local includes

#include "ui_presentation_mainpage.h"

class QTreeWidgetItem;

namespace DigikamGenericPresentationPlugin
{

class PresentationContainer;
class LoadingDescription;

class PresentationMainPage : public QWidget,
                             public Ui::PresentationMainPage
{
    Q_OBJECT

public:

    explicit PresentationMainPage(QWidget* const parent,
                                  PresentationContainer* const sharedData);
    ~PresentationMainPage() override;

    void readSettings();
    void saveSettings();
    bool updateUrlList();

    void removeImageFromList(const QUrl& url);

Q_SIGNALS:

    void signalTotalTimeChanged(const QTime&);

private Q_SLOTS:

    void slotOpenGLToggled();
    void slotEffectChanged();
    void slotDelayChanged(int);
    void slotOffAutoDelay();
    void slotPrintCommentsToggled();
    void slotUseMillisecondsToggled();
    void slotThumbnail(const LoadingDescription&, const QPixmap&);
    void slotImageListChanged();

    void slotPortfolioDurationChanged(int);
    void slotImagesFilesSelected(QTreeWidgetItem* item);

private:

    void setupConnections();
    void loadEffectNames();
    void loadEffectNamesGL();
    void showNumberImages();
    void addItems(const QList<QUrl>& fileList);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericPresentationPlugin
