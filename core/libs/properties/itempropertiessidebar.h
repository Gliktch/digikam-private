/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : item properties side bar (without support of digiKam database).
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QWidget>
#include <QRect>
#include <QStackedWidget>

// Local includes

#include "digikam_config.h"
#include "sidebar.h"
#include "digikam_export.h"
#include "searchtextbar.h"

namespace Digikam
{

class DImg;
class SidebarSplitter;
class ItemPropertiesTab;
class ItemPropertiesMetadataTab;
class ItemPropertiesColorsTab;

#ifdef HAVE_GEOLOCATION

class ItemPropertiesGPSTab;

#endif // HAVE_GEOLOCATION

class DIGIKAM_EXPORT ItemPropertiesSideBar : public Sidebar
{
    Q_OBJECT

public:
    explicit ItemPropertiesSideBar(QWidget* const parent,
                                   SidebarSplitter* const splitter,
                                   Qt::Edge side = Qt::LeftEdge,
                                   bool mimimizedDefault = false);
    ~ItemPropertiesSideBar() override = default;

    virtual void itemChanged(const QUrl& url, const QRect& rect = QRect(), DImg* const img = nullptr);

Q_SIGNALS:

    void signalSetupMetadataFilters(int);
    void signalSetupExifTool();

public Q_SLOTS:

    void slotLoadMetadataFilters();
    void slotImageSelectionChanged(const QRect& rect);
    virtual void slotNoCurrentItem();

protected Q_SLOTS:

    virtual void slotChangedTab(QWidget* tab);

private Q_SLOTS:

    void slotFileButtonPressed(int index);

protected:

    /**
     * load the last view state from disk - called by StateSavingObject#loadState()
     */
    void doLoadState() override;

    /**
     * save the view state to disk - called by StateSavingObject#saveState()
     */
    void doSaveState() override;

    virtual void setImagePropertiesInformation(const QUrl& url);

protected:

    bool                        m_dirtyPropertiesTab        = false;
    bool                        m_dirtyMetadataTab          = false;
    bool                        m_dirtyColorTab             = false;
    bool                        m_dirtyGpsTab               = false;
    bool                        m_dirtyHistoryTab           = false;

    QRect                       m_currentRect;

    QUrl                        m_currentUrl;

    DImg*                       m_image                     = nullptr;

    QStackedWidget*             m_propertiesStackedView     = nullptr;

    ItemPropertiesTab*          m_propertiesTab             = nullptr;
    ItemPropertiesMetadataTab*  m_metadataTab               = nullptr;
    ItemPropertiesColorsTab*    m_colorTab                  = nullptr;

#ifdef HAVE_GEOLOCATION

    ItemPropertiesGPSTab*       m_gpsTab                    = nullptr;

#endif // HAVE_GEOLOCATION

};

} // namespace Digikam
