/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-08-20
 * Description : Localization of Strings Config widget.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "localizeconfig.h"

// Qt includes

#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QApplication>
#include <QStyle>
#include <QHeaderView>
#include <QPushButton>
#include <QCheckBox>
#include <QIcon>
#include <QTreeWidgetItemIterator>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "localizesettings.h"
#include "altlangstredit.h"
#include "digikam_debug.h"
#include "dlayoutbox.h"
#include "dexpanderbox.h"

namespace Digikam
{

LanguagesList::LanguagesList(QWidget* const parent)
    : QTreeWidget(parent)
{
    setRootIsDecorated(false);
    setItemsExpandable(false);
    setExpandsOnDoubleClick(false);
    setAlternatingRowColors(true);
    setSelectionMode(QAbstractItemView::NoSelection);
    setAllColumnsShowFocus(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setColumnCount(2);
    setHeaderLabels(QStringList() << i18nc("@title: translator language code", "Code (Language-Country)")
                                  << i18nc("@title: translator language name", "Name"));
    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(1, QHeaderView::Stretch);
}

void LanguagesList::slotSearchTextChanged(const SearchTextSettings& settings)
{
    int found = 0;

    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        if (
            (*it)->text(0).contains(settings.text, settings.caseSensitive) ||
            (*it)->text(1).contains(settings.text, settings.caseSensitive)
           )
        {
            found++;
            (*it)->setHidden(false);
        }
        else
        {
            (*it)->setHidden(true);
        }

        ++it;
    }

    Q_EMIT signalSearchResult(found);
}

// --------------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN LanguagesView : public QGroupBox
{
    Q_OBJECT

public:

    explicit LanguagesView(QWidget* const parent)
        : QGroupBox(parent)
    {
        QVBoxLayout* const vlay = new QVBoxLayout();
        setLayout(vlay);

        m_langList              = new LanguagesList(this);
        DHBox* const hbox       = new DHBox(this);
        m_showSelected          = new QCheckBox(hbox);
        m_langFilter            = new SearchTextBar(hbox, QLatin1String("TranslatorLangSearchBar"),
                                                    i18nc("@info", "Enter here a string to search in languages list..."));

        vlay->addWidget(m_langList);
        vlay->addWidget(hbox);

        connect(m_showSelected, SIGNAL(toggled(bool)),
                this, SLOT(slotShowSelected(bool)));

        connect(m_langFilter, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
                m_langList, SLOT(slotSearchTextChanged(SearchTextSettings)));

        connect(m_langList, SIGNAL(signalSearchResult(int)),
                this, SLOT(slotSearchResult(int)));

        connect(m_langList, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
                this, SLOT(slotItemClicked(QTreeWidgetItem*,int)));

        slotItemClicked(nullptr, 0);
    }

    void updateStats()
    {
        int count = 0;
        QTreeWidgetItemIterator it(m_langList);

        while (*it)
        {
            if ((*it)->checkState(0) == Qt::Checked)
            {
                count++;
            }

            ++it;
        }

        m_showSelected->setText(i18nc("@info", "Show only selected items (%1/%2)",
                                count, m_langList->topLevelItemCount()));
    }

private Q_SLOTS:

    void slotItemClicked(QTreeWidgetItem*, int column)
    {
        if (column == 0)
        {
            updateStats();
        }
    }

    void slotShowSelected(bool b)
    {
        QTreeWidgetItemIterator it(m_langList);

        while (*it)
        {
            if (b)
            {
                (*it)->setHidden((*it)->checkState(0) != Qt::Checked);
            }
            else
            {
                (*it)->setHidden(false);
            }

            ++it;
        }
    }

    void slotSearchResult(int found)
    {
        m_langFilter->slotSearchResult(found ? true : false);
    }

public:

    LanguagesList* m_langList       = nullptr;
    QCheckBox*     m_showSelected   = nullptr;
    SearchTextBar* m_langFilter     = nullptr;
};

class Q_DECL_HIDDEN LocalizeConfig::Private
{
public:

    Private() = default;

    LanguagesList*  altLangList     = nullptr;
    LanguagesView*  altLangGroup    = nullptr;

    QStringList     trLangEnabled;
    QComboBox*      translatorCB    = nullptr;
    QLabel*         translatorLabel = nullptr;
    LanguagesList*  trLangList      = nullptr;
    LanguagesView*  trLangGroup     = nullptr;
};

LocalizeConfig::LocalizeConfig(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    const int spacing = layoutSpacing();

    QGridLayout* const grid = new QGridLayout(this);

    // --------------------------------------------------------

    d->altLangGroup         = new LanguagesView(this);
    d->altLangGroup->setTitle(i18nc("@title", "Select Alternative Languages To Edit Text"));
    d->altLangList          = d->altLangGroup->m_langList;

    // --------------------------------------------------------

    QLabel* const trLabel   = new QLabel(i18nc("@label", "Online Translator:"), this);
    d->translatorCB         = new QComboBox(this);
    d->translatorCB->setToolTip(i18nc("@option", "Select here the online engine to translate text"));
    d->translatorCB->insertItem(DOnlineTranslator::Google,         QLatin1String("Google"));
    d->translatorCB->insertItem(DOnlineTranslator::Yandex,         QLatin1String("Yandex"));
    d->translatorCB->insertItem(DOnlineTranslator::Bing,           QLatin1String("Bing"));
//    d->translatorCB->insertItem(DOnlineTranslator::LibreTranslate, QLatin1String("Libre Translate"));
    d->translatorCB->insertItem(DOnlineTranslator::Lingva,         QLatin1String("Lingva"));

    d->translatorLabel      = new QLabel(xi18nc("@info", "<para>The translator engine is an online Web-service used to translate "
                                                         "text from one language to another one. This feature is available on "
                                                         "metadata option where alternative language is supported, as Title, "
                                                         "Caption, etc. The translation process is fully automated but it requires "
                                                         "an Internet connection to work.</para>"
                                                         "<para>User can select the more appropriate translator engine depending of "
                                                         "desired target language, as not all world-wide languages are supported "
                                                         "by online Web-services.</para>"), this);
    d->translatorLabel->setWordWrap(true);

    d->trLangGroup             = new LanguagesView(this);
    d->trLangList              = d->trLangGroup->m_langList;

    // --------------------------------------------------------

    grid->setAlignment(Qt::AlignTop);
    grid->addWidget(d->altLangGroup,                       0, 0, 1, 3);
    grid->addWidget(new DLineWidget(Qt::Horizontal, this), 1, 0, 1, 3);
    grid->addWidget(trLabel,                               2, 0, 1, 1);
    grid->addWidget(d->translatorCB,                       2, 1, 1, 1);
    grid->addWidget(d->translatorLabel,                    3, 0, 1, 3);
    grid->addWidget(d->trLangGroup,                        4, 0, 1, 3);
    grid->setRowStretch(4, 10);
    grid->setColumnStretch(2, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // --------------------------------------------------------

    populateAltLanguages();
    populateTranslatorLanguages();
    readSettings();
    slotTranslatorChanged();

    connect(d->translatorCB, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotTranslatorChanged()));
}

LocalizeConfig::~LocalizeConfig()
{
    delete d;
}

void LocalizeConfig::populateAltLanguages()
{
    d->altLangList->clear();
    const auto langs = AltLangStrEdit::allLanguagesRFC3066();

    for (const QString& code : langs)
    {
        new QTreeWidgetItem(d->altLangList,
                            QStringList() << code
                                          << AltLangStrEdit::languageNameRFC3066(code));
    }
}

void LocalizeConfig::populateTranslatorLanguages()
{
    d->trLangList->clear();
    const auto langs = DOnlineTranslator::supportedRFC3066((DOnlineTranslator::Engine)d->translatorCB->currentIndex());

    for (const QString& code : langs)
    {
        new QTreeWidgetItem(d->trLangList,
                            QStringList() << code
                                          << AltLangStrEdit::languageNameRFC3066(code));
    }
}

void LocalizeConfig::slotTranslatorChanged()
{
    int count       = 0;

    QTreeWidgetItemIterator it(d->trLangList);

    while (*it)
    {
        (*it)->setDisabled(!DOnlineTranslator::isSupportTranslation(
            (DOnlineTranslator::Engine)d->translatorCB->currentIndex(),
            DOnlineTranslator::language(DOnlineTranslator::fromRFC3066((DOnlineTranslator::Engine)d->translatorCB->currentIndex(), (*it)->text(0))))
        );

        if (!(*it)->isDisabled())
        {
            count++;
        }

        ++it;
    }

    d->trLangGroup->setTitle(i18nc("@title", "Available Translator Languages (%1/%2)",
                             count, d->trLangList->topLevelItemCount()));
}

void LocalizeConfig::applySettings()
{
    LocalizeSettings* const config = LocalizeSettings::instance();

    if (!config)
    {
        return;
    }

    LocalizeContainer set;

    set.alternativeLang << QLatin1String("x-default");  // This first entry must always be present on the list.

    QTreeWidgetItemIterator it(d->altLangList);

    while (*it)
    {
        if (((*it)->checkState(0) == Qt::Checked))
        {
            set.alternativeLang << (*it)->text(0);
        }

        ++it;
    }

    set.translatorEngine = (DOnlineTranslator::Engine)d->translatorCB->currentIndex();

    QTreeWidgetItemIterator it2(d->trLangList);

    while (*it2)
    {
        if (((*it2)->checkState(0) == Qt::Checked) && !(*it2)->isDisabled())
        {
            set.translatorLang << (*it2)->text(0);
        }

        ++it2;
    }

    config->setSettings(set, LocalizeSettings::LocalizeConfig);
}

void LocalizeConfig::readSettings()
{
    const LocalizeSettings* const config = LocalizeSettings::instance();

    if (!config)
    {
        return;
    }

    LocalizeContainer set                = config->settings();

    QTreeWidgetItemIterator it(d->altLangList);

    while (*it)
    {
        if (set.translatorLang.contains((*it)->text(0)))
        {
            (*it)->setCheckState(0, Qt::Checked);
        }
        else
        {
            (*it)->setCheckState(0, Qt::Unchecked);
        }

        ++it;
    }

    d->translatorCB->setCurrentIndex(set.translatorEngine);

    QTreeWidgetItemIterator it2(d->trLangList);

    while (*it2)
    {
        if (set.translatorLang.contains((*it2)->text(0)))
        {
            (*it2)->setCheckState(0, Qt::Checked);
        }
        else
        {
            (*it2)->setCheckState(0, Qt::Unchecked);
        }

        ++it2;
    }

    d->altLangGroup->updateStats();
    d->trLangGroup->updateStats();
}

} // namespace Digikam

#include "localizeconfig.moc"

#include "moc_localizeconfig.cpp"
