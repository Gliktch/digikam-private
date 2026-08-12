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

#include "searchtabheader.h"

// Qt includes

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedLayout>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>
#include <QLineEdit>
#include <QInputDialog>
#include <QIcon>
#include <QMenu>
#include <QContextMenuEvent>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Local includes

#include "digikam_globals.h"
#include "digikam_debug.h"
#include "album.h"
#include "albummanager.h"
#include "searchfolderview.h"
#include "searchwindow.h"
#include "coredbsearchxml.h"
#include "dexpanderbox.h"

namespace Digikam
{

class Q_DECL_HIDDEN KeywordLineEdit : public QLineEdit
{
    Q_OBJECT

public:

    explicit KeywordLineEdit(QWidget* const parent = nullptr)
        : QLineEdit(parent)
    {
        KSharedConfig::Ptr config = KSharedConfig::openConfig();
        KConfigGroup group        = config->group(QLatin1String("KeywordSearchEdit Settings"));
        m_autoSearch              = group.readEntry(QLatin1String("Autostart Search"), false);
        m_i18nSearch              = i18n("(Advanced Search)");
    }

    void showAdvancedSearch(bool hasAdvanced)
    {
        if (m_hasAdvanced == hasAdvanced)
        {
            return;
        }

        m_hasAdvanced = hasAdvanced;
        adjustStatus(m_hasAdvanced);
    }

    void focusInEvent(QFocusEvent* e) override
    {
        if (m_hasAdvanced)
        {
            adjustStatus(false);
        }

        QLineEdit::focusInEvent(e);
    }

    void focusOutEvent(QFocusEvent* e) override
    {
        QLineEdit::focusOutEvent(e);

        if (m_hasAdvanced)
        {
            adjustStatus(true);
        }
    }

    void contextMenuEvent(QContextMenuEvent* e) override
    {
        QAction* const action = new QAction(i18nc("@action:inmenu",
                                                  "Autostart Search"), this);
        action->setCheckable(true);
        action->setChecked(m_autoSearch);

        connect(action, &QAction::triggered,
                this, &KeywordLineEdit::toggleAutoSearch);

        QMenu* const menu = createStandardContextMenu();
        menu->addSeparator();
        menu->addAction(action);
        menu->exec(e->globalPos());
        delete menu;
    }

    bool autoSearchEnabled() const
    {
        return m_autoSearch;
    }

    void adjustStatus(bool adv)
    {
        if (adv)
        {
            QPalette p = palette();
            p.setColor(QPalette::Text, p.color(QPalette::Disabled, QPalette::Text));
            setPalette(p);

            setText(m_i18nSearch);
        }
        else
        {
            setPalette(QPalette());

            if (text() == m_i18nSearch)
            {
                setText(QString());
            }
        }
    }

    QString getText() const
    {
        if (text() == m_i18nSearch)
        {
            return QString();
        }

        return text();
    }

public Q_SLOTS:

    void toggleAutoSearch()
    {
        m_autoSearch              = !m_autoSearch;

        KSharedConfig::Ptr config = KSharedConfig::openConfig();
        KConfigGroup group        = config->group(QLatin1String("KeywordSearchEdit Settings"));
        group.writeEntry(QLatin1String("Autostart Search"), m_autoSearch);
    }

protected:

    bool    m_hasAdvanced   = false;
    bool    m_autoSearch    = false;
    QString m_i18nSearch;
};

// -------------------------------------------------------------------------

class Q_DECL_HIDDEN SearchTabHeader::Private
{
public:

    Private() = default;

    QGroupBox*          newSearchWidget         = nullptr;
    QGroupBox*          saveAsWidget            = nullptr;
    QGroupBox*          editSimpleWidget        = nullptr;
    QGroupBox*          editAdvancedWidget      = nullptr;

    QStackedLayout*     lowerArea               = nullptr;

    KeywordLineEdit*    keywordEdit             = nullptr;
    QPushButton*        advancedNewSearch       = nullptr;
    QPushButton*        advancedEditSearch      = nullptr;

    QLineEdit*          saveNameEdit            = nullptr;
    QToolButton*        saveButton              = nullptr;

    DAdjustableLabel*   storedKeywordEditName   = nullptr;
    QLineEdit*          storedKeywordEdit       = nullptr;
    DAdjustableLabel*   storedAdvancedEditName  = nullptr;
    QPushButton*        storedAdvancedEditLabel = nullptr;

    QTimer*             keywordEditTimer        = nullptr;
    QTimer*             storedKeywordEditTimer  = nullptr;

    SearchWindow*       searchWindow            = nullptr;

    SAlbum*             currentAlbum            = nullptr;

    QString             oldKeywordContent;
    QString             oldStoredKeywordContent;
};

SearchTabHeader::SearchTabHeader(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    const int spacing = layoutSpacing();

    QVBoxLayout* const mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(QMargins());
    setLayout(mainLayout);

    // upper part

    d->newSearchWidget      = new QGroupBox(this);
    mainLayout->addWidget(d->newSearchWidget);

    // lower part

    d->lowerArea            = new QStackedLayout;
    mainLayout->addLayout(d->lowerArea);

    d->saveAsWidget         = new QGroupBox(this);
    d->editSimpleWidget     = new QGroupBox(this);
    d->editAdvancedWidget   = new QGroupBox(this);
    d->lowerArea->addWidget(d->saveAsWidget);
    d->lowerArea->addWidget(d->editSimpleWidget);
    d->lowerArea->addWidget(d->editAdvancedWidget);

    // ------------------- //

    // upper part

    d->newSearchWidget->setTitle(i18n("New Search"));
    QGridLayout* const grid1  = new QGridLayout;
    QLabel* const searchLabel = new QLabel(i18nc("@label: quick search properties", "Search:"), this);
    d->keywordEdit            = new KeywordLineEdit(this);
    d->keywordEdit->setClearButtonEnabled(true);
    d->keywordEdit->setPlaceholderText(i18n("Enter keywords here..."));

    d->advancedNewSearch      = new QPushButton(i18n("New Advanced Search..."), this);
    d->advancedEditSearch     = new QPushButton(i18n("Edit Current Search..."), this);

    grid1->addWidget(searchLabel,           0, 0, 1, 1);
    grid1->addWidget(d->keywordEdit,        0, 1, 1, 1);
    grid1->addWidget(d->advancedNewSearch,  1, 0, 1, 2);
    grid1->addWidget(d->advancedEditSearch, 2, 0, 1, 2);
    grid1->setContentsMargins(spacing, spacing, spacing, spacing);
    grid1->setSpacing(spacing);

    d->newSearchWidget->setLayout(grid1);

    // ------------------- //

    // lower part, variant 1

    d->saveAsWidget->setTitle(i18n("Save Current Search"));

    QHBoxLayout* const hbox1 = new QHBoxLayout;
    d->saveNameEdit          = new QLineEdit(this);
    d->saveNameEdit->setWhatsThis(i18n("Enter a name for the current search to save it in the "
                                       "\"Searches\" view"));

    d->saveButton            = new QToolButton(this);
    d->saveButton->setIcon(QIcon::fromTheme(QLatin1String("document-save")));
    d->saveButton->setToolTip(i18n("Save current search to a new virtual Album"));
    d->saveButton->setWhatsThis(i18n("If you press this button, the current search "
                                     "will be saved to a new virtual Search Album using the name "
                                     "set on the left side."));

    hbox1->addWidget(d->saveNameEdit);
    hbox1->addWidget(d->saveButton);
    hbox1->setContentsMargins(spacing, spacing, spacing, spacing);
    hbox1->setSpacing(spacing);

    d->saveAsWidget->setLayout(hbox1);

    // ------------------- //

    // lower part, variant 2

    d->editSimpleWidget->setTitle(i18n("Edit Stored Search"));

    QVBoxLayout* const vbox1 = new QVBoxLayout;
    d->storedKeywordEditName = new DAdjustableLabel(this);

    if (layoutDirection() == Qt::RightToLeft)
    {
        d->storedKeywordEditName->setElideMode(Qt::ElideRight);
    }
    else
    {
        d->storedKeywordEditName->setElideMode(Qt::ElideLeft);
    }

    d->storedKeywordEdit     = new QLineEdit(this);

    vbox1->addWidget(d->storedKeywordEditName);
    vbox1->addWidget(d->storedKeywordEdit);
    vbox1->setContentsMargins(spacing, spacing, spacing, spacing);
    vbox1->setSpacing(spacing);

    d->editSimpleWidget->setLayout(vbox1);

    // ------------------- //

    // lower part, variant 3

    d->editAdvancedWidget->setTitle(i18n("Edit Stored Search"));

    QVBoxLayout* const vbox2   = new QVBoxLayout;

    d->storedAdvancedEditName  = new DAdjustableLabel(this);

    if (layoutDirection() == Qt::RightToLeft)
    {
        d->storedAdvancedEditName->setElideMode(Qt::ElideRight);
    }
    else
    {
        d->storedAdvancedEditName->setElideMode(Qt::ElideLeft);
    }

    d->storedAdvancedEditLabel = new QPushButton(i18n("Edit..."), this);

    vbox2->addWidget(d->storedAdvancedEditName);
    vbox2->addWidget(d->storedAdvancedEditLabel);
    d->editAdvancedWidget->setLayout(vbox2);

    // ------------------- //

    // timers

    d->keywordEditTimer       = new QTimer(this);
    d->keywordEditTimer->setSingleShot(true);
    d->keywordEditTimer->setInterval(800);

    d->storedKeywordEditTimer = new QTimer(this);
    d->storedKeywordEditTimer->setSingleShot(true);
    d->storedKeywordEditTimer->setInterval(800);

    // ------------------- //

    connect(d->keywordEdit, SIGNAL(textEdited(QString)),
            d->keywordEditTimer, SLOT(start()));

    connect(d->keywordEditTimer, SIGNAL(timeout()),
            this, SLOT(keywordChangedTimer()));

    connect(d->keywordEdit, SIGNAL(editingFinished()),
            this, SLOT(keywordChanged()));

    connect(d->advancedNewSearch, SIGNAL(clicked()),
            this, SLOT(newAdvancedSearch()));

    connect(d->advancedEditSearch, SIGNAL(clicked()),
            this, SLOT(slotEditCurrentSearch()));

    connect(d->saveNameEdit, SIGNAL(returnPressed()),
            this, SLOT(saveSearch()));

    connect(d->saveButton, SIGNAL(clicked()),
            this, SLOT(saveSearch()));

    connect(d->storedKeywordEditTimer, SIGNAL(timeout()),
            this, SLOT(storedKeywordChanged()));

    connect(d->storedKeywordEdit, SIGNAL(editingFinished()),
            this, SLOT(storedKeywordChanged()));

    connect(d->storedAdvancedEditLabel, SIGNAL(clicked()),
            this, SLOT(editStoredAdvancedSearch()));
}

SearchTabHeader::~SearchTabHeader()
{
    delete d->searchWindow;
    delete d;
}

SearchWindow* SearchTabHeader::searchWindow() const
{
    if (!d->searchWindow)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Creating search window";

        // Create the advanced search edit window, deferred from constructor

        d->searchWindow = new SearchWindow;

        connect(d->searchWindow, SIGNAL(searchEdited(int,QString)),
                this, SLOT(advancedSearchEdited(int,QString)),
                Qt::QueuedConnection);
    }

    return d->searchWindow;
}

void SearchTabHeader::selectedSearchChanged(Album* a)
{
    SAlbum* album = dynamic_cast<SAlbum*>(a);

    // Signal from SearchFolderView that a search has been selected.
    // Don't check on d->currentAlbum == album, rather update status (which may have changed on same album)

    d->currentAlbum = album;

    qCDebug(DIGIKAM_GENERAL_LOG) << "changing to SAlbum " << album;

    if (!album)
    {
        d->lowerArea->setCurrentWidget(d->saveAsWidget);
        d->lowerArea->setEnabled(false);
    }
    else
    {
        d->lowerArea->setEnabled(true);

        if      (album->title() == SAlbum::getTemporaryTitle(DatabaseSearch::AdvancedSearch))
        {
            d->lowerArea->setCurrentWidget(d->saveAsWidget);

            if (album->isKeywordSearch())
            {
                d->keywordEdit->setText(keywordsFromQuery(album->query()));
                d->keywordEdit->showAdvancedSearch(false);
            }
            else
            {
                d->keywordEdit->showAdvancedSearch(true);
            }
        }
        else if (album->isKeywordSearch())
        {
            d->lowerArea->setCurrentWidget(d->editSimpleWidget);
            d->storedKeywordEditName->setAdjustedText(album->title());
            d->storedKeywordEdit->setText(keywordsFromQuery(album->query()));
            d->keywordEdit->showAdvancedSearch(false);
        }
        else
        {
            d->lowerArea->setCurrentWidget(d->editAdvancedWidget);
            d->storedAdvancedEditName->setAdjustedText(album->title());
            d->keywordEdit->showAdvancedSearch(false);
        }
    }
}

void SearchTabHeader::copySearch(SAlbum* album)
{
    if (!album)
    {
        return;
    }

    if  (album->isAdvancedSearch())
    {
        SAlbum* const salbum = AlbumManager::instance()->findSAlbum(SAlbum::getTemporaryTitle(DatabaseSearch::AdvancedSearch));

        if (salbum)
        {
            AlbumManager::instance()->updateSAlbum(salbum, album->query());
            SearchWindow* const window = searchWindow();

            window->reset();
            window->readSearch(salbum->id(), salbum->query());
            window->show();
            window->raise();
        }
    }
}

void SearchTabHeader::editSearch(SAlbum* album)
{
    if (!album)
    {
        return;
    }

    if      (album->isAdvancedSearch())
    {
        SearchWindow* const window = searchWindow();
        window->reset();
        window->readSearch(album->id(), album->query());
        window->show();
        window->raise();
    }
    else if (album->isKeywordSearch())
    {
        d->storedKeywordEdit->selectAll();
    }
}

void SearchTabHeader::newKeywordSearch()
{
    d->keywordEdit->clear();
    d->keywordEdit->setFocus();
}

void SearchTabHeader::newAdvancedSearch()
{
    SearchWindow* const window = searchWindow();
    window->reset();
    window->show();
    window->raise();
}

void SearchTabHeader::keywordChanged()
{
    QString keywords = d->keywordEdit->getText();

    qCDebug(DIGIKAM_GENERAL_LOG) << "keywords changed to '" << keywords << "'";

    if ((d->oldKeywordContent == keywords) || (keywords.trimmed().isEmpty()))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "same keywords as before, ignoring...";

        return;
    }
    else
    {
        d->oldKeywordContent = keywords;
    }

    setCurrentSearch(DatabaseSearch::KeywordSearch, queryFromKeywords(keywords));
    d->keywordEdit->setFocus();
}

void SearchTabHeader::keywordChangedTimer()
{
    if (d->keywordEdit->autoSearchEnabled())
    {
        keywordChanged();
    }
}

void SearchTabHeader::slotEditCurrentSearch()
{
    const SAlbum* const album  = AlbumManager::instance()->findSAlbum(SAlbum::getTemporaryTitle(DatabaseSearch::AdvancedSearch));
    SearchWindow* const window = searchWindow();

    if (album)
    {
        window->readSearch(album->id(), album->query());
    }
    else
    {
        window->reset();
    }

    window->show();
    window->raise();
}

void SearchTabHeader::saveSearch()
{
    // Only applicable if:
    // 1. current album is Search View Current Album Save this album as a user names search album.
    // 2. user as processed a search before to save it.

    QString name = d->saveNameEdit->text();

    qCDebug(DIGIKAM_GENERAL_LOG) << "name = " << name;

    if (name.isEmpty() || !d->currentAlbum)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "no current album, returning";

        // passive popup

        return;
    }

    const SAlbum* oldAlbum = AlbumManager::instance()->findSAlbum(name);

    while (oldAlbum)
    {
        QString label    = i18n("Search name already exists.\n"
                                "Please enter a new name:");
        bool ok;
        QString newTitle = QInputDialog::getText(this,
                                                 i18nc("@title:window", "Name Exists"),
                                                 label,
                                                 QLineEdit::Normal,
                                                 name,
                                                 &ok);

        if (!ok)
        {
            return;
        }

        name     = newTitle;
        oldAlbum = AlbumManager::instance()->findSAlbum(name);
    }

    // cppcheck-suppress constVariablePointer
    SAlbum* const newAlbum = AlbumManager::instance()->createSAlbum(name, d->currentAlbum->searchType(),
                                                                    d->currentAlbum->query());
    Q_EMIT searchShallBeSelected(QList<Album*>() << newAlbum);
}

void SearchTabHeader::storedKeywordChanged()
{
    QString keywords = d->storedKeywordEdit->text();

    if (d->oldStoredKeywordContent == keywords)
    {
        return;
    }
    else
    {
        d->oldStoredKeywordContent = keywords;
    }

    if (d->currentAlbum)
    {
        AlbumManager::instance()->updateSAlbum(d->currentAlbum, queryFromKeywords(keywords));

        Q_EMIT searchShallBeSelected(QList<Album*>() << d->currentAlbum);
    }
}

void SearchTabHeader::editStoredAdvancedSearch()
{
    if (d->currentAlbum)
    {
        SearchWindow* const window = searchWindow();
        window->reset();
        window->readSearch(d->currentAlbum->id(), d->currentAlbum->query());
        window->show();
        window->raise();
    }
}

void SearchTabHeader::advancedSearchEdited(int id, const QString& query)
{
    // if the user just pressed the button, but did not change anything in the window,
    // the search is effectively still a keyword search.
    // We go the hard way and check this case.

    KeywordSearchReader check(query);
    DatabaseSearch::Type type = check.isSimpleKeywordSearch() ? DatabaseSearch::KeywordSearch
                                                              : DatabaseSearch::AdvancedSearch;

    if (id == -1)
    {
        setCurrentSearch(type, query);
    }
    else
    {
        SAlbum* const album = AlbumManager::instance()->findSAlbum(id);

        if (album)
        {
            AlbumManager::instance()->updateSAlbum(album, query, album->title(), type);

            Q_EMIT searchShallBeSelected(QList<Album*>() << album);
        }
    }
}

void SearchTabHeader::setCurrentSearch(DatabaseSearch::Type type, const QString& query, bool selectCurrentAlbum)
{
    SAlbum* album = AlbumManager::instance()->findSAlbum(SAlbum::getTemporaryTitle(DatabaseSearch::KeywordSearch));

    if (album)
    {
        AlbumManager::instance()->updateSAlbum(album, query,
                                               SAlbum::getTemporaryTitle(DatabaseSearch::KeywordSearch),
                                               type);
    }
    else
    {
        album = AlbumManager::instance()->createSAlbum(SAlbum::getTemporaryTitle(DatabaseSearch::KeywordSearch),
                                                       type, query);
    }

    if (selectCurrentAlbum)
    {
        Q_EMIT searchShallBeSelected(QList<Album*>() << album);
    }
}

QString SearchTabHeader::queryFromKeywords(const QString& keywords) const
{
    QStringList keywordList = KeywordSearch::split(keywords);

    // create xml

    KeywordSearchWriter writer;

    return writer.xml(keywordList);
}

QString SearchTabHeader::keywordsFromQuery(const QString& query) const
{
    KeywordSearchReader reader(query);
    QStringList keywordList = reader.keywords();

    return KeywordSearch::merge(keywordList);
}

} // namespace Digikam

#include "searchtabheader.moc"

#include "moc_searchtabheader.cpp"
