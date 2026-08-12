/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-15
 * Description : localize selector widget
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "localizeselector.h"

// Qt includes

#include <QLabel>
#include <QIcon>
#include <QMenu>
#include <QAction>
#include <QListWidget>
#include <QWidgetAction>
#include <QStyle>
#include <QToolButton>
#include <QApplication>
#include <QScrollBar>
#include <QListWidgetItem>
#include <QGridLayout>
#include <QScopedPointer>
#include <QEventLoop>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "digikam_debug.h"
#include "localizeconfig.h"
#include "localizesettings.h"
#include "donlinetranslator.h"
#include "altlangstredit.h"

namespace Digikam
{

class Q_DECL_HIDDEN TranslateAction : public QWidgetAction
{
    Q_OBJECT

public:

    explicit TranslateAction(QObject* const parent)
        : QWidgetAction(parent)
    {
    }

    QWidget* createWidget(QWidget* parent) override
    {
        m_list         = new QListWidget(parent);
        QFontMetrics fontMt(m_list->font());
        QRect fontRect = fontMt.boundingRect(0, 0, m_list->width(), m_list->height(), 0, QLatin1String("XX-XX"));
        int width      =  m_list->contentsMargins().left() + m_list->contentsMargins().right();
        width         += fontRect.width() + m_list->verticalScrollBar()->height();
        m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_list->setFixedWidth(width);

        return m_list;
    }

public:

    QListWidget* m_list = nullptr;
};

class Q_DECL_HIDDEN LocalizeSelector::Private
{
public:

    Private() = default;

public:

    QToolButton*                   translateButton  = nullptr;
    QToolButton*                   settingsButton   = nullptr;

    TranslateAction*               translateAction  = nullptr;
};

LocalizeSelector::LocalizeSelector(QWidget* const parent)
    : DHBox(parent),
      d      (new Private)
{
    d->translateButton = new QToolButton(this);
    d->translateButton->setIcon(QIcon::fromTheme(QLatin1String("language-chooser")));
    d->translateButton->setPopupMode(QToolButton::MenuButtonPopup);
    QMenu* const menu  = new QMenu(d->translateButton);
    d->translateAction = new TranslateAction(d->translateButton);
    menu->addAction(d->translateAction);
    d->translateButton->setMenu(menu);

    d->settingsButton  = new QToolButton(this);
    d->settingsButton->setIcon(QIcon::fromTheme(QLatin1String("configure")));
    d->settingsButton->setToolTip(i18nc("@info", "Open localize setup"));

    setContentsMargins(QMargins());
    setSpacing(layoutSpacing());

    slotLocalizeChanged();

    // --------------------------------------------------------

    connect(d->settingsButton, &QToolButton::clicked,
            this, &LocalizeSelector::slotOpenLocalizeSetup);

    connect(d->translateAction->m_list, &QListWidget::itemClicked,
            this, &LocalizeSelector::slotTranslate);

    connect(d->translateButton, &QToolButton::pressed,
            d->translateButton, &QToolButton::showMenu);

    connect(LocalizeSettings::instance(), &LocalizeSettings::signalSettingsChanged,
            this, &LocalizeSelector::slotLocalizeChanged);
}

LocalizeSelector::~LocalizeSelector()
{
    delete d;
}

void LocalizeSelector::populateTranslationEntries()
{
    d->translateAction->m_list->clear();

    QStringList allRFC3066  = DOnlineTranslator::supportedRFC3066(LocalizeSettings::instance()->settings().translatorEngine);
    Q_UNUSED(allRFC3066);
    LocalizeContainer set   = LocalizeSettings::instance()->settings();

    for (const QString& lg : std::as_const(set.translatorLang))
    {
        QListWidgetItem* const item = new QListWidgetItem(d->translateAction->m_list);
        item->setText(lg);
        item->setToolTip(i18nc("@info", "Translate to %1", AltLangStrEdit::languageNameRFC3066(lg)));
        d->translateAction->m_list->addItem(item);
    }
}

void LocalizeSelector::slotOpenLocalizeSetup()
{
    LocalizeSettings::instance()->openLocalizeSetup();
}

void LocalizeSelector::slotLocalizeChanged()
{
    populateTranslationEntries();
    d->translateButton->setToolTip(i18nc("@info: language edit widget", "Select language to translate with %1",
                                   DOnlineTranslator::engineName(LocalizeSettings::instance()->settings().translatorEngine)));
}

void LocalizeSelector::slotTranslate(QListWidgetItem* item)
{
    d->translateButton->menu()->close();

    if (item)
    {
        Q_EMIT signalTranslate(item->text());
    }
}

// ------------------------------------------------------------------------

class Q_DECL_HIDDEN LocalizeSelectorList::Private
{
public:

    Private() = default;

public:

    QLabel*           trLabel       = nullptr;
    LocalizeSelector* trSelector    = nullptr;
    LanguagesList*    trList        = nullptr;
};

LocalizeSelectorList::LocalizeSelectorList(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    QGridLayout* const grid  = new QGridLayout(this);

    d->trLabel               = new QLabel(this);
    d->trSelector            = new LocalizeSelector(this);
    d->trList                = new LanguagesList(this);
    d->trList->setContextMenuPolicy(Qt::CustomContextMenu);

    grid->addWidget(d->trLabel,      0, 0, 1, 1);
    grid->addWidget(d->trSelector,   0, 1, 1, 1);
    grid->addWidget(d->trList,       1, 0, 1, 2);
    grid->setColumnStretch(0, 10);
    grid->setRowStretch(2, 10);
    grid->setContentsMargins(0, 0, 0, 0);

    connect(d->trSelector, SIGNAL(signalTranslate(QString)),
            this, SLOT(slotAppendTranslation(QString)));

    connect(d->trList, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotShowContextMenu(QPoint)));
}

LocalizeSelectorList::~LocalizeSelectorList()
{
    delete d;
}

void LocalizeSelectorList::setTitle(const QString& title)
{
    d->trLabel->setText(title);
}

void LocalizeSelectorList::clearLanguages()
{
    d->trList->clear();
}

void LocalizeSelectorList::addLanguage(const QString& code)
{
    d->trList->addTopLevelItem(new QTreeWidgetItem(d->trList, QStringList() << code << AltLangStrEdit::languageNameRFC3066(code)));
}

QStringList LocalizeSelectorList::languagesList() const
{
    QStringList codes;

    QTreeWidgetItemIterator it(d->trList);

    while (*it)
    {
        QString lang = (*it)->text(0);

        if (!lang.isEmpty())
        {
            codes << lang;
        }

        ++it;
    }

    return codes;
}

void LocalizeSelectorList::slotShowContextMenu(const QPoint& pos)
{
    QPoint globalPos        = d->trList->mapToGlobal(pos);

    QMenu menu;
    const QAction* const rm = menu.addAction(i18nc("@action", "Remove this entry"));
    const QAction* const cl = menu.addAction(i18nc("@action", "Clear List"));
    const QAction* const ac = menu.exec(globalPos);

    if      (ac == rm)
    {
        delete d->trList->currentItem();
    }
    else if (ac == cl)
    {
        d->trList->clear();
    }

    Q_EMIT signalSettingsChanged();
}

void LocalizeSelectorList::slotAppendTranslation(const QString& lang)
{
    QTreeWidgetItemIterator it(d->trList);

    while (*it)
    {
        if ((*it)->text(0).startsWith(lang))
        {
            return;
        }

        ++it;
    }

    addLanguage(lang);

    Q_EMIT signalSettingsChanged();
}

// ------------------------------------------------------------------------

bool s_inlineTranslateString(const QString& text, const QString& trCode, QString& tr, QString& error)
{
    QScopedPointer<DOnlineTranslator> trengine(new DOnlineTranslator);
    QScopedPointer<QEventLoop> waitingLoop(new QEventLoop);

    DOnlineTranslator::Language srcLang = DOnlineTranslator::Auto;
    DOnlineTranslator::Language trLang  = DOnlineTranslator::language(DOnlineTranslator::fromRFC3066(LocalizeSettings::instance()->settings().translatorEngine, trCode));

    qCDebug(DIGIKAM_WIDGETS_LOG) << "Request to translate with Web-service:";
    qCDebug(DIGIKAM_WIDGETS_LOG) << "Text to translate        :" << text;
    qCDebug(DIGIKAM_WIDGETS_LOG) << "To target language       :" << trLang;
    qCDebug(DIGIKAM_WIDGETS_LOG) << "With source language     :" << srcLang;

    QObject::connect(trengine.data(), &DOnlineTranslator::signalFinished,
                     waitingLoop.data(), &QEventLoop::quit);

    trengine->translate(text,                                                            // String to translate
                        LocalizeSettings::instance()->settings().translatorEngine,       // Web service
                        trLang,                                                          // Target language
                        srcLang,                                                         // Source language
                        DOnlineTranslator::Auto);

    waitingLoop->exec(QEventLoop::ExcludeUserInputEvents);

    if (trengine->error() == DOnlineTranslator::NoError)
    {
        tr = trengine->translation();
        qCDebug(DIGIKAM_WIDGETS_LOG) << "Text translated          :" << tr;

        return true;
    }
    else
    {
        error = QChar::fromLatin1(trengine->error());
    }

    return false;
}

} // namespace Digikam

#include "localizeselector.moc"

#include "moc_localizeselector.cpp"
