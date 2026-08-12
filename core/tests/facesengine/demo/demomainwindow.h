/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-21
 * Description : GUI test program for FacesEngine
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Alex Jironkin <alexjironkin at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMainWindow>

namespace FaceEngineDemo
{

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget* const parent = nullptr);
    ~MainWindow() override;

public Q_SLOTS:

    void slotOpenImage();
    void slotDetectFaces();
    void slotUpdateDatabase();
    void slotRecognise();
    void slotUpdateAccuracy();
    void slotUpdateSensitivity();

protected:

    void changeEvent(QEvent* e) override;

private:

    void clearScene();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace FaceEngineDemo
