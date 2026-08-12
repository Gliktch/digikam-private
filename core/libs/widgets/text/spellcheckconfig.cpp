/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-08-16
 * Description : Spell-check Config widget.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "spellcheckconfig.h"
#include "digikam_config.h"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QApplication>
#include <QStyle>
#include <QTreeWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

#ifdef HAVE_SONNET

#   if defined(Q_CC_CLANG)
#       pragma clang diagnostic push
#       pragma clang diagnostic ignored "-Wnonportable-include-path"
#   endif

#       include <sonnet/speller.h>
using namespace Sonnet;

#   if defined(Q_CC_CLANG)
#       pragma clang diagnostic pop
#   endif

#endif

// Local includes

#include "digikam_globals.h"
#include "localizesettings.h"
#include "altlangstredit.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN SpellCheckConfig::Private
{
public:

    Private() = default;

    QCheckBox*   activeSpellCheck   = nullptr;
    QLabel*      spellCheckLabel    = nullptr;
    QLabel*      languageLabel      = nullptr;
    QComboBox*   languageCB         = nullptr;

    QGroupBox*   dictGroup          = nullptr;
    QGroupBox*   backGroup          = nullptr;
    QGroupBox*   ignoreWordsGroup   = nullptr;

    QTreeWidget* dictList           = nullptr;              ///< Dictionaries list
    QTreeWidget* backList           = nullptr;              ///< Backends list
    QPushButton* addWordButton      = nullptr;
    QPushButton* delWordButton      = nullptr;
    QPushButton* repWordButton      = nullptr;

    QListWidget* ignoreWordsBox     = nullptr;

    QLineEdit*   ignoreWordEdit     = nullptr;
};

SpellCheckConfig::SpellCheckConfig(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    const int spacing = layoutSpacing();

    // --------------------------------------------------------

    QGridLayout* const grid = new QGridLayout(this);

    d->activeSpellCheck     = new QCheckBox(this);
    d->activeSpellCheck->setText(i18nc("@option:check", "Activate spellcheck in background when entering text"));

    d->spellCheckLabel      = new QLabel(xi18nc("@info", "<para>Turn on this option to activate the background spellcheck "
                                                         "feature on captions, titles, and other text-edit widgets. "
                                                         "Spellcheck is able to auto-detect the current language used in "
                                                         "text and will propose alternative with miss-spelled words.</para>"
                                                         "<para>With entries where alternative language can be specified, the "
                                                         "contextual language will be used to parse text. Spellcheck "
                                                         "depends of open-source backends, including necessary dictionaries, "
                                                         "to operate sentence analysis in desired languages.</para>"), this);
    d->spellCheckLabel->setWordWrap(true);

    d->languageLabel = new QLabel(i18nc("@label", "Default Language:"), this);
    d->languageCB    = new QComboBox(this);
    d->languageCB->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    d->languageCB->setWhatsThis(i18nc("@info: edit widget default language",
                                      "Select the default language here to use with \"x-default\""
                                      "value for the alternative language strings."));

    d->languageCB->addItem(i18n("Auto-detect"));
    d->languageCB->setItemData(0, i18nc("@info", "Detect automatically the language by parsing string content"), Qt::ToolTipRole);
    d->languageCB->insertSeparator(d->languageCB->count());

    const auto langs = AltLangStrEdit::allLanguagesRFC3066();

    for (const QString& lg : langs)
    {
        d->languageCB->addItem(lg, lg);
        d->languageCB->setItemData(d->languageCB->findText(lg), AltLangStrEdit::languageNameRFC3066(lg), Qt::ToolTipRole);
    }

    // ---

    d->dictGroup               = new QGroupBox(i18nc("@title", "Available Language Dictionaries"), this);
    QVBoxLayout* const dictlay = new QVBoxLayout();
    d->dictGroup->setLayout(dictlay);

    d->dictList                = new QTreeWidget(this);
    d->dictList->setRootIsDecorated(false);
    d->dictList->setItemsExpandable(false);
    d->dictList->setExpandsOnDoubleClick(false);
    d->dictList->setAlternatingRowColors(true);
    d->dictList->setSelectionMode(QAbstractItemView::NoSelection);
    d->dictList->setAllColumnsShowFocus(true);
    d->dictList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->dictList->setColumnCount(2);
    d->dictList->setHeaderLabels(QStringList() << i18nc("@title: dictionary language code", "Code")
                                               << i18nc("@title: dictionary language name", "Name"));
    d->dictList->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    d->dictList->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    dictlay->addWidget(d->dictList);

    // ---

    d->backGroup               = new QGroupBox(i18nc("@title", "Available Backends"), this);
    QVBoxLayout* const backlay = new QVBoxLayout();
    d->backGroup->setLayout(backlay);

    d->backList                = new QTreeWidget(this);
    d->backList->setRootIsDecorated(false);
    d->backList->setItemsExpandable(false);
    d->backList->setExpandsOnDoubleClick(false);
    d->backList->setAlternatingRowColors(true);
    d->backList->setSelectionMode(QAbstractItemView::NoSelection);
    d->backList->setAllColumnsShowFocus(true);
    d->backList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->backList->setColumnCount(1);
    d->backList->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    d->backList->header()->setVisible(false);
    backlay->addWidget(d->backList);

    // ---

    d->ignoreWordsGroup               = new QGroupBox(i18nc("@title", "Ignored Words"), this);
    QGridLayout* const ignoreWordsLay = new QGridLayout();
    d->ignoreWordsGroup->setLayout(ignoreWordsLay);

    d->ignoreWordEdit = new QLineEdit(this);
    d->ignoreWordEdit->setClearButtonEnabled(true);
    d->ignoreWordEdit->setPlaceholderText(i18nc("@info", "Set here a new word to ignore during spellcheck"));

    d->ignoreWordsBox = new QListWidget(this);
    d->ignoreWordsBox->setWhatsThis(i18nc("@info", "You can add or remove words to ignore "
                                          "while spell-checking operations."));

    d->ignoreWordsBox->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    d->addWordButton = new QPushButton(i18nc("@action", "&Add..."),  this);
    d->delWordButton = new QPushButton(i18nc("@action", "&Remove"),  this);
    d->repWordButton = new QPushButton(i18nc("@action", "&Replace"), this);

    d->addWordButton->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    d->delWordButton->setIcon(QIcon::fromTheme(QLatin1String("list-remove")));
    d->repWordButton->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
    d->delWordButton->setEnabled(false);
    d->repWordButton->setEnabled(false);

    ignoreWordsLay->setAlignment(Qt::AlignTop);
    ignoreWordsLay->addWidget(d->ignoreWordEdit, 0, 0, 1, 1);
    ignoreWordsLay->addWidget(d->ignoreWordsBox, 1, 0, 5, 1);
    ignoreWordsLay->addWidget(d->addWordButton,  1, 1, 1, 1);
    ignoreWordsLay->addWidget(d->delWordButton,  2, 1, 1, 1);
    ignoreWordsLay->addWidget(d->repWordButton,  3, 1, 1, 1);
    ignoreWordsLay->setRowStretch(4, 10);
    ignoreWordsLay->setColumnStretch(0, 10);
    ignoreWordsLay->setContentsMargins(spacing, spacing, spacing, spacing);
    ignoreWordsLay->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->activeSpellCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotSpellcheckActivated(bool)));

    connect(d->ignoreWordsBox, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotIgnoreWordSelectionChanged()));

    connect(d->addWordButton, SIGNAL(clicked()),
            this, SLOT(slotAddWord()));

    connect(d->delWordButton, SIGNAL(clicked()),
            this, SLOT(slotDelWord()));

    connect(d->repWordButton, SIGNAL(clicked()),
            this, SLOT(slotRepWord()));

    // ---

#ifdef HAVE_SONNET

    Speller dict;
    QMap<QString, QString> map = dict.availableDictionaries();

    for (QMap<QString, QString>::const_iterator it = map.constBegin() ; it != map.constEnd() ; ++it)
    {
        new QTreeWidgetItem(d->dictList, QStringList() << it.value() << it.key());
    }

    const auto backs = dict.availableBackends();

    for (const QString& b : backs)
    {
        new QTreeWidgetItem(d->backList, QStringList() << b);
    }

#endif

    // ---

    grid->setAlignment(Qt::AlignTop);
    grid->addWidget(d->activeSpellCheck, 0, 0, 1, 2);
    grid->addWidget(d->spellCheckLabel,  1, 0, 1, 2);
    grid->addWidget(d->languageLabel,    2, 0, 1, 1);
    grid->addWidget(d->languageCB,       2, 1, 1, 1);
    grid->addWidget(d->dictGroup,        3, 0, 1, 1);
    grid->addWidget(d->backGroup,        3, 1, 1, 1);
    grid->addWidget(d->ignoreWordsGroup, 4, 0, 1, 2);
    grid->setRowStretch(3, 10);
    grid->setColumnStretch(0, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // --------------------------------------------------------

    readSettings();
}

SpellCheckConfig::~SpellCheckConfig()
{
    delete d;
}

void SpellCheckConfig::applySettings()
{
    LocalizeSettings* const config = LocalizeSettings::instance();

    if (!config)
    {
        return;
    }

    LocalizeContainer set;

    set.enableSpellCheck = d->activeSpellCheck->isChecked();
    set.defaultLanguage  = d->languageCB->currentData().toString();

    QStringList ignoredWords;

    for (int i = 0 ; i < d->ignoreWordsBox->count() ; ++i)
    {
        QListWidgetItem* const item = d->ignoreWordsBox->item(i);
        ignoredWords.append(item->text());
    }

    set.ignoredWords = ignoredWords;

    qCDebug(DIGIKAM_WIDGETS_LOG) << set;

    config->setSettings(set, LocalizeSettings::SpellCheckConfig);
}

void SpellCheckConfig::readSettings()
{
    const LocalizeSettings* const config = LocalizeSettings::instance();

    if (!config)
    {
        return;
    }

    LocalizeContainer set                = config->settings();

    d->activeSpellCheck->setChecked(set.enableSpellCheck);
    d->languageCB->setCurrentIndex(set.defaultLanguage.isEmpty() ? 0 : d->languageCB->findData(set.defaultLanguage));
    d->ignoreWordsBox->insertItems(0, set.ignoredWords);

    slotSpellcheckActivated(d->activeSpellCheck->isChecked());
}

void SpellCheckConfig::slotSpellcheckActivated(bool b)
{
    d->spellCheckLabel->setEnabled(b);
    d->languageLabel->setEnabled(b);
    d->languageCB->setEnabled(b);
    d->dictGroup->setEnabled(b);
    d->backGroup->setEnabled(b);
    d->ignoreWordsGroup->setEnabled(b);
}

void SpellCheckConfig::slotDelWord()
{
    QListWidgetItem* const item = d->ignoreWordsBox->currentItem();

    if (!item)
    {
        return;
    }

    d->ignoreWordsBox->takeItem(d->ignoreWordsBox->row(item));
    delete item;
}

void SpellCheckConfig::slotRepWord()
{
    QString newWord = d->ignoreWordEdit->text();

    if (newWord.isEmpty())
    {
        return;
    }

    if (!d->ignoreWordsBox->selectedItems().isEmpty())
    {
        d->ignoreWordsBox->selectedItems().at(0)->setText(newWord);
        d->ignoreWordEdit->clear();
    }
}

void SpellCheckConfig::slotIgnoreWordSelectionChanged()
{
    if (!d->ignoreWordsBox->selectedItems().isEmpty())
    {
        d->ignoreWordEdit->setText(d->ignoreWordsBox->selectedItems().at(0)->text());
        d->delWordButton->setEnabled(true);
        d->repWordButton->setEnabled(true);
    }
    else
    {
        d->delWordButton->setEnabled(false);
        d->repWordButton->setEnabled(false);
    }
}

void SpellCheckConfig::slotAddWord()
{
    QString newWord = d->ignoreWordEdit->text();

    if (newWord.isEmpty())
    {
        return;
    }

    bool found = false;

    for (int i = 0 ; i < d->ignoreWordsBox->count() ; ++i)
    {
        QListWidgetItem* const item = d->ignoreWordsBox->item(i);

        if (newWord == item->text())
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        d->ignoreWordsBox->insertItem(d->ignoreWordsBox->count(), newWord);
        d->ignoreWordEdit->clear();
    }
}

} // namespace Digikam

#include "moc_spellcheckconfig.cpp"
