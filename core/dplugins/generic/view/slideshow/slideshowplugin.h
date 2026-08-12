/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to render slideshow.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2020      by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dplugingeneric.h"
#include "iteminfolist.h"

#define DPLUGIN_IID "org.kde.digikam.plugin.generic.SlideShow"

using namespace Digikam;

namespace DigikamGenericSlideShowPlugin
{

class SlideShowSettings;

class SlideShowPlugin : public DPluginGeneric
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DPLUGIN_IID)
    Q_INTERFACES(Digikam::DPluginGeneric)

public:

    explicit SlideShowPlugin(QObject* const parent = nullptr);
    ~SlideShowPlugin()                   override = default;

    QString name()                 const override;
    QString iid()                  const override;
    QIcon   icon()                 const override;
    QString details()              const override;
    QString description()          const override;
    QList<DPluginAuthor> authors() const override;
    QString handbookSection()      const override;
    QString handbookChapter()      const override;

    void setup(QObject* const)           override;

    void addConnectionSlideEnd(QObject* const obj);

private Q_SLOTS:

    void slotMenuSlideShow();
    void slotMenuSlideShowAll();
    void slotMenuSlideShowSelection();
    void slotMenuSlideShowRecursive();
    void slotShowRecursive(const QList<QUrl>& imageList);
    void slotShowManual();

private:

    void slideshow(SlideShowSettings* const settings,
                   bool autoPlayEnabled = true,
                   const QUrl& startFrom = QUrl());
};

} // namespace DigikamGenericSlideShowPlugin
