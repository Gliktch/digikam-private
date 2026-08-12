/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-01-20
 * Description : User interface for searches
 *
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QCache>
#include <QList>
#include <QRect>
#include <QWidget>

class QHBoxLayout;
class QDialogButtonBox;
class QPushButton;

namespace Digikam
{

class SearchGroup;
class SearchViewBottomBar;
class SearchXmlCachingReader;
class SearchXmlWriter;

class SearchViewThemedPartsCache
{
public:

    SearchViewThemedPartsCache()                   = default;
    virtual ~SearchViewThemedPartsCache()          = default;

    virtual QPixmap groupLabelPixmap(int w, int h) = 0;
    virtual QPixmap bottomBarPixmap(int w, int h)  = 0;

private:

    Q_DISABLE_COPY(SearchViewThemedPartsCache)
};

class AbstractSearchGroupContainer : public QWidget
{
    Q_OBJECT

public:

    /**
     * Abstract base class for classes that contain SearchGroups
     * To contain common code of SearchView and SearchGroup,
     * as SearchGroups can have subgroups.
     */
    explicit AbstractSearchGroupContainer(QWidget* const parent = nullptr);

public Q_SLOTS:

    SearchGroup* addSearchGroup();
    void removeSearchGroup(SearchGroup* group);

protected:

    /// Call before reading the XML part that could contain group elements
    void startReadingGroups(SearchXmlCachingReader& reader);

    /// Call when a group element is the current element
    void readGroup(SearchXmlCachingReader& reader);

    /// Call when the XML part is finished
    void finishReadingGroups();

    /// Write contained groups to writer
    void writeGroups(SearchXmlWriter& writer) const;

    /// Collects the data from the same method of all contained groups (position relative to this widget)
    QList<QRect> startupAnimationAreaOfGroups() const;

    /// Re-implement: create and setup a search group
    virtual SearchGroup* createSearchGroup() = 0;

    /// Re-implement: Adds a newly created group to the layout structures
    virtual void addGroupToLayout(SearchGroup* group) = 0;

protected Q_SLOTS:

    void removeSendingSearchGroup();

protected:

    int                 m_groupIndex = 0;
    QList<SearchGroup*> m_groups;
};

// -------------------------------------------------------------------------

class SearchView : public AbstractSearchGroupContainer,
                   public SearchViewThemedPartsCache
{
    Q_OBJECT

public:

    SearchView();
    ~SearchView()                                  override;

    void setup();
    void setBottomBar(SearchViewBottomBar* const bar);

    void read(const QString& search);
    QString write()                          const;

    QPixmap groupLabelPixmap(int w, int h)         override;
    QPixmap bottomBarPixmap(int w, int h)          override;

Q_SIGNALS:

    void searchOk();
    void searchTryout();
    void searchCancel();

protected Q_SLOTS:

    void setTheme();
    void slotAddGroupButton();
    void slotClearAllGroups();
    void slotResetButton();
    void startAnimation();
    void animationFrame(int);
    void timeLineFinished();

protected:

    QPixmap cachedBannerPixmap(int w, int h) const;

    void paintEvent(QPaintEvent* e)                 override;
    void showEvent(QShowEvent* event)               override;

    SearchGroup* createSearchGroup()                override;
    void addGroupToLayout(SearchGroup* group)       override;

private:

    /// @note disabled.
    SearchView(QWidget*)                     = delete;
    SearchView(const SearchView&)            = delete;
    SearchView& operator=(const SearchView&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

// -------------------------------------------------------------------------

class SearchViewBottomBar : public QWidget
{
    Q_OBJECT

public:

    explicit SearchViewBottomBar(SearchViewThemedPartsCache* const cache,
                                 QWidget* const parent = nullptr);

Q_SIGNALS:

    void okPressed();
    void cancelPressed();
    void tryoutPressed();
    void addGroupPressed();
    void resetPressed();
    void clearAllPressed();

protected:

    void paintEvent(QPaintEvent*) override;

protected:

    QHBoxLayout*                m_mainLayout        = nullptr;

    QDialogButtonBox*           m_buttonBox         = nullptr;
    QPushButton*                m_addGroupsButton   = nullptr;
    QPushButton*                m_resetButton       = nullptr;
    QPushButton*                m_clearAllButton    = nullptr;

    SearchViewThemedPartsCache* m_themeCache        = nullptr;

private Q_SLOTS:

    void slotHelp();
};

} // namespace Digikam
