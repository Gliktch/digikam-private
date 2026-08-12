/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-02-26
 * Description : Upper widget in the search sidebar
 *
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "coredbalbuminfo.h"

namespace Digikam
{

class Album;
class SAlbum;
class SearchWindow;

class SearchTabHeader : public QWidget
{
    Q_OBJECT

public:

    explicit SearchTabHeader(QWidget* const parent);
    ~SearchTabHeader() override;

public Q_SLOTS:

    void selectedSearchChanged(Album* album);
    void copySearch(SAlbum* album);
    void editSearch(SAlbum* album);
    void newKeywordSearch();
    void newAdvancedSearch();

Q_SIGNALS:

    void searchShallBeSelected(const QList<Album*>& albums);

private Q_SLOTS:

    void keywordChanged();
    void keywordChangedTimer();
    void slotEditCurrentSearch();
    void saveSearch();
    void storedKeywordChanged();
    void editStoredAdvancedSearch();
    void advancedSearchEdited(int id, const QString& query);

private:

    void          setCurrentSearch(DatabaseSearch::Type type, const QString& query, bool selectCurrentAlbum = true);
    QString       queryFromKeywords(const QString& keywords) const;
    QString       keywordsFromQuery(const QString& query) const;
    SearchWindow* searchWindow() const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
