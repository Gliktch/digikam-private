/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-02-11
 * Description : a tool to export images to WikiMedia web service
 *
 * SPDX-FileCopyrightText: 2011      by Alexandre Mendes <alex dot mendes1988 at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Parthasarathy Gopavarapu <gparthasarathy93 at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Peter Potrowl <peter dot potrowl at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QStringList>
#include <QTreeWidgetItem>
#include <QUrl>

// Local includes

#include "dinfointerface.h"
#include "dprogresswdg.h"
#include "ditemslist.h"

class KConfigGroup;

using namespace Digikam;

namespace DigikamGenericMediaWikiPlugin
{

enum MediaWikiDownloadType
{
    MediaWikiMyAlbum = 0,
    MediaWikiFriendAlbum,
    MediaWikiPhotosMe,
    MediaWikiPhotosFriend
};

class MediaWikiWidget : public QWidget
{
    Q_OBJECT

public:

    explicit MediaWikiWidget(DInfoInterface* const iface, QWidget* const parent);
    ~MediaWikiWidget() override;

public:

    void updateLabels(const QString& userName = QString(),
                      const QString& wikiName = QString(),
                      const QString& url = QString());
    void invertAccountLoginBox();

    DItemsList*  imagesList()  const;
    DProgressWdg* progressBar() const;

    int  dimension()            const;
    int  quality()              const;
    bool resize()               const;
    bool removeMeta()           const;
    bool removeGeo()            const;

    QString author()            const;
    QString source()            const;
    QString genCategories()     const;
    QString genText()           const;
    QString genComments()       const;
    QString license()           const;
    QString categories()        const;
    QString title()             const;
    QString description()       const;
    QString date()              const;
    QString latitude()          const;
    QString longitude()         const;

    QMap <QString, QMap <QString, QString> > allImagesDesc();

    void clearImagesDesc();
    void readSettings(KConfigGroup& group);
    void saveSettings(KConfigGroup& group);
    void loadItemInfoFirstLoad();
    void loadItemInfo(const QUrl& url);
    void clearEditFields();

Q_SIGNALS:

    void signalChangeUserRequest();
    void signalLoginRequest(const QString& login,
                            const QString& pass,
                            const QString& wikiName,
                            const QUrl& wikiUrl);

private Q_SLOTS:

    void slotResizeChecked();
    void slotRemoveMetaChecked();
    void slotChangeUserClicked();
    void slotLoginClicked();
    void slotNewWikiClicked();
    void slotAddWikiClicked();
    void slotLoadImagesDesc(QTreeWidgetItem* item);
    void slotRemoveImagesDesc();
    void slotRestoreExtension();
    void slotApplyTitle();
    void slotApplyDate();
    void slotApplyCategories();
    void slotApplyDescription();
    void slotApplyLatitude();
    void slotApplyLongitude();

private:

    class Private;
    Private* const d = nullptr;

    friend class WmWindow;
};

} // namespace DigikamGenericMediaWikiPlugin
