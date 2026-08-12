/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-07-07
 * Description : a tool to export images to Flickr web service
 *
 * SPDX-FileCopyrightText: 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QTabWidget>
#include <QLineEdit>
#include <QComboBox>

// Local includes

#include "wscomboboxintermediate.h"
#include "flickrlist.h"
#include "wssettingswidget.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericFlickrPlugin
{

class FlickrWidget : public WSSettingsWidget
{
    Q_OBJECT

public:

    explicit FlickrWidget(QWidget* const parent,
                          DInfoInterface* const iface,
                          const QString& serviceName);
    ~FlickrWidget()                                     override;

    void updateLabels(const QString& name = QString(),
                      const QString& url = QString())   override;

private Q_SLOTS:

    void slotPermissionChanged(FlickrList::FieldType, Qt::CheckState);
    void slotSafetyLevelChanged(FlickrList::SafetyLevel);
    void slotContentTypeChanged(FlickrList::ContentType);
    void slotMainPublicToggled(int);
    void slotMainFamilyToggled(int);
    void slotMainFriendsToggled(int);
    void slotMainSafetyLevelChanged(int);
    void slotMainContentTypeChanged(int);
    void slotExtendedPublicationToggled(bool);
    void slotExtendedTagsToggled(bool);
    void slotAddExtraTagsToggled(bool);

private:

    void mainPermissionToggled(FlickrList::FieldType, Qt::CheckState);

private:

    class Private;
    Private* const d = nullptr;

    friend class FlickrWindow;
};

} // namespace DigikamGenericFlickrPlugin
