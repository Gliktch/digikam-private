/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-10-23
 * Description : unit test for DImg image loader with multithreading
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QObject>

// Local includes

#include "dimg.h"
#include "loadingdescription.h"
#include "loadsavethread.h"

using namespace Digikam;

class LoadSaveThreadTest : public QObject
{
    Q_OBJECT

public:

    explicit LoadSaveThreadTest(QObject* const parent = nullptr);

private Q_SLOTS:

    void testLoadSaveThread();

    void slotImageLoaded(const LoadingDescription&, const DImg&);
    void slotImageSaved(const QString&, bool);
    void slotLoadingProgress(const LoadingDescription&, float);
    void slotSavingProgress(const QString&, float);

private:

    LoadSaveThread* m_thread         = nullptr;
    QString         m_fname;
    QString         m_outFilePath;
    bool            m_loadedProgress = false;
    bool            m_savedProgress  = false;
    bool            m_loaded         = false;
    bool            m_saved          = false;
};
