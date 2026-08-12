/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-07-28
 * Description : Common widgets shared by Web Service tools
 *
 * SPDX-FileCopyrightText: 2013      by Pankaj Kumar <me at panks dot me>
 * SPDX-FileCopyrightText: 2015      by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 * SPDX-FileCopyrightText: 2016-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

//Qt includes

#include <QWidget>

// Local includes

#include "digikam_export.h"
#include "ditemslist.h"
#include "dinfointerface.h"
#include "dprogresswdg.h"

class QLabel;
class QSpinBox;
class QCheckBox;
class QButtonGroup;
class QComboBox;
class QPushButton;
class QGroupBox;
class QGridLayout;
class QVBoxLayout;
class QHBoxLayout;

namespace Digikam
{

class DIGIKAM_EXPORT WSSettingsWidget : public QWidget
{
    Q_OBJECT

public:

    explicit WSSettingsWidget(QWidget* const parent,
                              DInfoInterface* const iface,
                              const QString& toolName);
    ~WSSettingsWidget() override;

public:

    void              replaceImageList(QWidget* const widget);
    void              addWidgetToSettingsBox(QWidget* const widget);

    QString           getDestinationPath()   const;
    QString           getAlbumTitle()        const;

    DItemsList*       imagesList()           const;
    DProgressWdg*     progressBar()          const;

    QWidget*          getSettingsBox()       const;
    QVBoxLayout*      getSettingsBoxLayout() const;

    QGroupBox*        getAlbumBox()          const;
    QGridLayout*      getAlbumBoxLayout()    const;

    QGroupBox*        getOptionsBox()        const;
    QGridLayout*      getOptionsBoxLayout()  const;

    QGroupBox*        getUploadBox()         const;
    QVBoxLayout*      getUploadBoxLayout()   const;

    QGroupBox*        getSizeBox()           const;
    QVBoxLayout*      getSizeBoxLayout()     const;

    QGroupBox*        getAccountBox()        const;
    QGridLayout*      getAccountBoxLayout()  const;

    QLabel*           getHeaderLbl()         const;
    QLabel*           getUserNameLabel()     const;
    QPushButton*      getChangeUserBtn()     const;
    QComboBox*        getDimensionCoB()      const;
    QPushButton*      getNewAlbmBtn()        const;
    QPushButton*      getReloadBtn()         const;
    QCheckBox*        getOriginalCheckBox()  const;
    QCheckBox*        getResizeCheckBox()    const;
    QCheckBox*        getPhotoIdCheckBox()   const;
    QSpinBox*         getDimensionSpB()      const;
    QSpinBox*         getImgQualitySpB()     const;
    QComboBox*        getAlbumsCoB()         const;

public:

    virtual void updateLabels(const QString& name = QString(),
                              const QString& url = QString()) = 0;

protected Q_SLOTS:

    void slotResizeChecked();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
