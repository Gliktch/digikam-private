/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-06-16
 * Description : Advanced Configuration tab for metadata.
 *
 * SPDX-FileCopyrightText: 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "advancedmetadatatab.h"

// Qt includes

#include <QApplication>
#include <QVBoxLayout>
#include <QComboBox>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QListView>
#include <QStandardPaths>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QLabel>
#include <QUrl>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dmetadatasettings.h"
#include "namespacelistview.h"
#include "namespaceeditdlg.h"
#include "dmessagebox.h"
#include "dfiledialog.h"

namespace Digikam
{

class Q_DECL_HIDDEN AdvancedMetadataTab::Private
{
public:

    Private() = default;

public:

    QComboBox*                  metadataType        = nullptr;
    QComboBox*                  operationType       = nullptr;
    QPushButton*                addButton           = nullptr;
    QPushButton*                editButton          = nullptr;
    QPushButton*                deleteButton        = nullptr;
    QPushButton*                moveUpButton        = nullptr;
    QPushButton*                moveDownButton      = nullptr;
    QPushButton*                revertChanges       = nullptr;
    QPushButton*                saveProfile         = nullptr;
    QPushButton*                loadProfile         = nullptr;
    QPushButton*                resetButton         = nullptr;
    QCheckBox*                  unifyReadWrite      = nullptr;
    QCheckBox*                  readingAllTags      = nullptr;
    QList<QStandardItemModel*>  models;
    NamespaceListView*          namespaceView       = nullptr;
    DMetadataSettingsContainer  container;
    int                         metadataTypeSize    = 0;

    bool                        changed             = false;
};

AdvancedMetadataTab::AdvancedMetadataTab(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    // ---------- Advanced Configuration Panel -----------------------------

    d->container = DMetadataSettings::instance()->settings();
    setUi();
    setModels();
    connectButtons();

    d->unifyReadWrite->setChecked(d->container.unifyReadWrite());
    d->readingAllTags->setChecked(d->container.readingAllTags());

    connect(d->unifyReadWrite, SIGNAL(toggled(bool)),
            this, SLOT(slotUnifyChecked(bool)));

    connect(d->readingAllTags, SIGNAL(toggled(bool)),
            this, SLOT(slotAllTagsChecked(bool)));

    connect(d->metadataType, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotIndexChanged()));

    connect(d->operationType, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotIndexChanged()));

    /**
     * Connect all actions to slotRevertAvailable, which will enable revert to original
     * if an add, edit, delete, or reorder was made
     */
    connect(d->namespaceView, SIGNAL(signalItemsChanged()),
            this, SLOT(slotRevertChangesAvailable()));

    if (d->unifyReadWrite->isChecked())
    {
        d->operationType->setEnabled(false);
    }
}

AdvancedMetadataTab::~AdvancedMetadataTab()
{
    delete d;
}

void AdvancedMetadataTab::slotSaveProfile()
{
    QString metaPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    QString savePath = DFileDialog::getSaveFileName(qApp->activeWindow(),
                                                    i18nc("@title:window", "Save Advanced Metadata Profile"),
                                                    metaPath, QLatin1String("*.dkamp"), nullptr,
                                                    QFileDialog::DontConfirmOverwrite);

    if (savePath.isEmpty())
    {
        return;
    }

    if (!savePath.endsWith(QLatin1String(".dkamp")))
    {
        savePath.append(QLatin1String(".dkamp"));
    }

    KConfig config(savePath);
    KConfigGroup group1 = config.group(QLatin1String("General"));
    KConfigGroup group2 = config.group(QLatin1String("Metadata"));
    group1.writeEntry(QLatin1String("AMPVersion"), 1);
    d->container.writeToConfig(group2);
    config.sync();
}

void AdvancedMetadataTab::slotLoadProfile()
{
    QString metaPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                              QLatin1String("digikam/metadata"),
                                              QStandardPaths::LocateDirectory);

    QString loadPath = DFileDialog::getOpenFileName(qApp->activeWindow(),
                                                    i18nc("@title:window", "Load Advanced Metadata Profile"),
                                                    metaPath, QLatin1String("*.dkamp"));

    if (loadPath.isEmpty())
    {
        return;
    }

    KConfig config(loadPath);
    KConfigGroup group1 = config.group(QLatin1String("General"));
    KConfigGroup group2 = config.group(QLatin1String("Metadata"));
    int version         = group1.readEntry(QLatin1String("AMPVersion"), 0);

    if (version != 1)
    {
        return;
    }

    DMetadataSettingsContainer container;
    container.readFromConfig(group2);

    if (container.mappingKeys().size() != 6)
    {
        return;
    }

    d->container = container;

    d->unifyReadWrite->setChecked(d->container.unifyReadWrite());
    d->readingAllTags->setChecked(d->container.readingAllTags());

    d->revertChanges->setEnabled(false);
    d->changed = false;

    setModels();
}

void AdvancedMetadataTab::slotResetToDefault()
{
    const int result = DMessageBox::showContinueCancel(QMessageBox::Warning,
                                                       this,
                                                       i18nc("@title:window, reset to default warning dialog", "Warning"),
                                                       i18nc("@info", "This option will reset configuration to default\n"
                                                                      "All your changes will be lost.\n "
                                                                      "Do you want to continue?"));

    if (result != QMessageBox::Yes)
    {
        return;
    }

    d->container.defaultValues();
    d->models.at(getModelIndex())->clear();
    setModelData(d->models.at(getModelIndex()), getCurrentContainer());

    d->namespaceView->setModel(d->models.at(getModelIndex()));
}

void AdvancedMetadataTab::slotRevertChanges()
{
    d->models.at(getModelIndex())->clear();
    setModelData(d->models.at(getModelIndex()), getCurrentContainer());

    d->namespaceView->setModel(d->models.at(getModelIndex()));

    d->changed = false;
    d->revertChanges->setEnabled(false);
}

void AdvancedMetadataTab::slotAddNewNamespace()
{
    NamespaceEntry entry;

    // Setting some default parameters;

    if      (d->metadataType->currentData().toString() == NamespaceEntry::DM_TAG_CONTAINER())
    {
        entry.nsType = NamespaceEntry::TAGS;
    }
    else if (d->metadataType->currentData().toString() == NamespaceEntry::DM_TITLE_CONTAINER())
    {
        entry.nsType = NamespaceEntry::TITLE;
    }
    else if (d->metadataType->currentData().toString() == NamespaceEntry::DM_RATING_CONTAINER())
    {
        entry.nsType = NamespaceEntry::RATING;
    }
    else if (d->metadataType->currentData().toString() == NamespaceEntry::DM_COMMENT_CONTAINER())
    {
        entry.nsType = NamespaceEntry::COMMENT;
    }
    else if (d->metadataType->currentData().toString() == NamespaceEntry::DM_PICKLABEL_CONTAINER())
    {
        entry.nsType = NamespaceEntry::PICKLABEL;
    }
    else if (d->metadataType->currentData().toString() == NamespaceEntry::DM_COLORLABEL_CONTAINER())
    {
        entry.nsType = NamespaceEntry::COLORLABEL;
    }

    entry.isDefault  = false;
    entry.subspace   = NamespaceEntry::XMP;

    if (!NamespaceEditDlg::create(qApp->activeWindow(), entry))
    {
        return;
    }

    QStandardItem* const root = d->models.at(getModelIndex())->invisibleRootItem();
    QStandardItem* const item = new QStandardItem(entry.namespaceName);
    setDataToItem(item, entry);

    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
    root->appendRow(item);

    slotRevertChangesAvailable();
}

void AdvancedMetadataTab::slotEditNamespace()
{
    if (!d->namespaceView->currentIndex().isValid())
    {
        return;
    }

    NamespaceEntry entry = getCurrentContainer().at(d->namespaceView->currentIndex().row());

    if (!NamespaceEditDlg::edit(qApp->activeWindow(), entry))
    {
        return;
    }

    const QStandardItem* const root = d->models.at(getModelIndex())->invisibleRootItem();
    QStandardItem* const item       = root->child(d->namespaceView->currentIndex().row());

    setDataToItem(item, entry);

    slotRevertChangesAvailable();
}

void AdvancedMetadataTab::applySettings()
{
    updateContainer();
    DMetadataSettings::instance()->setSettings(d->container);
}

void AdvancedMetadataTab::slotUnifyChecked(bool value)
{
    d->operationType->setDisabled(value);
    d->container.setUnifyReadWrite(value);

    d->operationType->setCurrentIndex(0);

    slotIndexChanged();
}

void AdvancedMetadataTab::slotAllTagsChecked(bool value)
{
    d->container.setReadingAllTags(value);
}

void AdvancedMetadataTab::slotIndexChanged()
{
    d->namespaceView->setModel(d->models.at(getModelIndex()));

    bool val = (d->metadataType->currentData().toString() ==
                NamespaceEntry::DM_TAG_CONTAINER());
    d->readingAllTags->setEnabled(val);
}

void AdvancedMetadataTab::slotRevertChangesAvailable()
{
    if (!d->changed)
    {
        d->revertChanges->setEnabled(true);
        d->changed = true;
    }

    updateContainer();
}

void AdvancedMetadataTab::connectButtons()
{
    connect(d->addButton, SIGNAL(clicked()),
            this, SLOT(slotAddNewNamespace()));

    connect(d->editButton, SIGNAL(clicked()),
            this, SLOT(slotEditNamespace()));

    connect(d->deleteButton, SIGNAL(clicked()),
            d->namespaceView, SLOT(slotDeleteSelected()));

    connect(d->saveProfile, SIGNAL(clicked()),
            this, SLOT(slotSaveProfile()));

    connect(d->loadProfile, SIGNAL(clicked()),
            this, SLOT(slotLoadProfile()));

    connect(d->resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetToDefault()));

    connect(d->revertChanges, SIGNAL(clicked()),
            this, SLOT(slotRevertChanges()));

    connect(d->moveUpButton, SIGNAL(clicked()),
            d->namespaceView, SLOT(slotMoveItemUp()));

    connect(d->moveDownButton, SIGNAL(clicked()),
            d->namespaceView, SLOT(slotMoveItemDown()));
}

void AdvancedMetadataTab::setModelData(QStandardItemModel* const model,
                                       const QList<NamespaceEntry>& container)
{
    QStandardItem* const root = model->invisibleRootItem();

    for (const NamespaceEntry& e : container)
    {
        QStandardItem* const item = new QStandardItem(e.namespaceName);

        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
        setDataToItem(item, e);
        root->appendRow(item);
    }

    connect(model, SIGNAL(itemChanged(QStandardItem*)),
            this, SLOT(slotRevertChangesAvailable()));
}

void AdvancedMetadataTab::setUi()
{
    QVBoxLayout* const advancedConfLayout = new QVBoxLayout(this);
    QGridLayout* const topLayout          = new QGridLayout();
    QHBoxLayout* const bottomLayout       = new QHBoxLayout();

    QLabel* const tipLabel = new QLabel(this);
    tipLabel->setTextFormat(Qt::RichText);
    tipLabel->setWordWrap(true);
    tipLabel->setText(i18n("Advanced configuration menu allow you to manage metadata namespaces"
                           " used by digiKam to store and retrieve tags, rating and comments.<br>"
                           "<b>Note: </b>Order is important when reading metadata"
                          ));

    //--- Top layout ----------------

    d->metadataType   = new QComboBox(this);
    d->operationType  = new QComboBox(this);

    d->operationType->insertItems(0, QStringList() << i18n("Read Options")
                                                   << i18n("Write Options"));

    d->unifyReadWrite = new QCheckBox(i18n("Unify read and write"));
    d->readingAllTags = new QCheckBox(i18n("Read all metadata for tags"));
    d->readingAllTags->setEnabled(false);

    topLayout->addWidget(d->metadataType,   0, 0, 1, 1);
    topLayout->addWidget(d->operationType,  0, 1, 1, 1);
    topLayout->addWidget(d->unifyReadWrite, 0, 2, 1, 1);
    topLayout->addWidget(d->readingAllTags, 1, 2, 1, 1);

    //------------ Bottom Layout-------------

    // View

    d->namespaceView = new NamespaceListView(this);

    // Buttons

    QVBoxLayout* const buttonsLayout = new QVBoxLayout();
    buttonsLayout->setAlignment(Qt::AlignTop);

    d->addButton      = new QPushButton(QIcon::fromTheme(QLatin1String("list-add")),
                                        i18n("Add"));

    d->editButton     = new QPushButton(QIcon::fromTheme(QLatin1String("document-edit")),
                                        i18n("Edit"));

    d->deleteButton   = new QPushButton(QIcon::fromTheme(QLatin1String("window-close")),
                                        i18n("Delete"));

    d->moveUpButton   = new QPushButton(QIcon::fromTheme(QLatin1String("go-up")),
                                        i18n("Move Up"));

    d->moveDownButton = new QPushButton(QIcon::fromTheme(QLatin1String("go-down")),
                                        i18n("Move Down"));

    d->revertChanges  = new QPushButton(QIcon::fromTheme(QLatin1String("edit-undo")),
                                        i18n("Revert Changes"));

    // Revert changes is disabled, until a change is made

    d->revertChanges->setEnabled(false);
    d->saveProfile = new QPushButton(QIcon::fromTheme(QLatin1String("document-save-as")),
                                     i18n("Save Profile"));

    d->loadProfile = new QPushButton(QIcon::fromTheme(QLatin1String("document-open")),
                                     i18n("Load Profile"));

    d->resetButton = new QPushButton(QIcon::fromTheme(QLatin1String("view-refresh")),
                                     i18n("Reset to Default"));

    buttonsLayout->addWidget(d->addButton);
    buttonsLayout->addWidget(d->editButton);
    buttonsLayout->addWidget(d->deleteButton);
    buttonsLayout->addWidget(d->moveUpButton);
    buttonsLayout->addWidget(d->moveDownButton);
    buttonsLayout->addWidget(d->revertChanges);
    buttonsLayout->addWidget(d->saveProfile);
    buttonsLayout->addWidget(d->loadProfile);
    buttonsLayout->addWidget(d->resetButton);

    QVBoxLayout* const vbox = new QVBoxLayout();
    vbox->addWidget(d->namespaceView);

    bottomLayout->addLayout(vbox);
    bottomLayout->addLayout(buttonsLayout);

    advancedConfLayout->addWidget(tipLabel);
    advancedConfLayout->addLayout(topLayout);
    advancedConfLayout->addLayout(bottomLayout);
}

void AdvancedMetadataTab::setDataToItem(QStandardItem* const item, const NamespaceEntry& entry)
{
    item->setData(entry.namespaceName,  Qt::DisplayRole);
    item->setData(entry.namespaceName,  NAME_ROLE);
    item->setData((int)entry.tagPaths,  ISTAG_ROLE);
    item->setData(entry.separator,      SEPARATOR_ROLE);
    item->setData((int)entry.nsType,    NSTYPE_ROLE);

    if (entry.nsType == NamespaceEntry::RATING)
    {
       item->setData(entry.convertRatio.at(0), ZEROSTAR_ROLE);
       item->setData(entry.convertRatio.at(1), ONESTAR_ROLE);
       item->setData(entry.convertRatio.at(2), TWOSTAR_ROLE);
       item->setData(entry.convertRatio.at(3), THREESTAR_ROLE);
       item->setData(entry.convertRatio.at(4), FOURSTAR_ROLE);
       item->setData(entry.convertRatio.at(5), FIVESTAR_ROLE);
    }

    item->setData((int)entry.specialOpts,    SPECIALOPTS_ROLE);
    item->setData(entry.alternativeName,     ALTNAME_ROLE);
    item->setData((int)entry.subspace,       SUBSPACE_ROLE);
    item->setData((int)entry.secondNameOpts, ALTNAMEOPTS_ROLE);
    item->setData(entry.isDefault,           ISDEFAULT_ROLE);

    item->setCheckable(true);

    if (!entry.isDisabled)
    {
        item->setCheckState(Qt::Checked);
    }
}

int AdvancedMetadataTab::getModelIndex()
{
    if (d->unifyReadWrite->isChecked())
    {
        return d->metadataType->currentIndex();
    }
    else
    {
        // for 4 metadata types:
        // read operation  = 4*0 + (0, 1, 2, 3)
        // write operation = 4*1 + (0, 1, 2, 3) = (4, 5, 6, 7)

        return (
                (d->metadataTypeSize * d->operationType->currentIndex()) +
                d->metadataType->currentIndex()
               );
    }
}

QList<NamespaceEntry>& AdvancedMetadataTab::getCurrentContainer()
{
    int currentIndex = getModelIndex();

    if (currentIndex >= d->metadataTypeSize)
    {
        return d->container.getWriteMapping(d->metadataType->currentData().toString());
    }
    else
    {
        return d->container.getReadMapping(d->metadataType->currentData().toString());
    }
}

void AdvancedMetadataTab::setModels()
{
    d->metadataType->blockSignals(true);
    d->metadataType->clear();

    QList<QString> keys = d->container.mappingKeys();

    for (const QString& str : std::as_const(keys))
    {
        d->metadataType->addItem(d->container.translateMappingKey(str), str);
    }

    d->metadataTypeSize = keys.size();

    for (int i = 0 ; i < d->models.size() ; ++i)
    {
        d->models.at(i)->clear();
    }

    d->models.clear();

    for (int i = 0 ; i < keys.size() * 2 ; ++i)
    {
        d->models.append(new QStandardItemModel(this));
    }

    int index = 0;

    for (const QString& str : std::as_const(keys))
    {
        setModelData(d->models.at(index++), d->container.getReadMapping(str));      // krazy:exclude=postfixop
    }

    for (const QString& str : std::as_const(keys))
    {
        setModelData(d->models.at(index++), d->container.getWriteMapping(str));     // krazy:exclude=postfixop
    }

    d->metadataType->setCurrentIndex(0);
    d->metadataType->blockSignals(false);

    slotIndexChanged();
}

void AdvancedMetadataTab::updateContainer()
{
    QList<QString> keys = d->container.mappingKeys();
    int index           = 0;

    for (const QString& str : std::as_const(keys))
    {
        d->container.getReadMapping(str).clear();
        saveModelData(d->models.at(index++), d->container.getReadMapping(str));
    }

    for (const QString& str : std::as_const(keys))
    {
        d->container.getWriteMapping(str).clear();
        saveModelData(d->models.at(index++), d->container.getWriteMapping(str));
    }
}

void AdvancedMetadataTab::saveModelData(QStandardItemModel* const model,
                                        QList<NamespaceEntry>& container)
{
    QStandardItem* const root = model->invisibleRootItem();

    if (!root->hasChildren())
    {
        return;
    }

    for (int i = 0 ; i < root->rowCount() ; ++i)
    {
        NamespaceEntry ns;
        QStandardItem* const current = root->child(i);
        ns.namespaceName             = current->data(NAME_ROLE).toString();
        ns.tagPaths                  = (NamespaceEntry::TagType)current->data(ISTAG_ROLE).toInt();
        ns.separator                 = current->data(SEPARATOR_ROLE).toString();
        ns.nsType                    = (NamespaceEntry::NamespaceType)current->data(NSTYPE_ROLE).toInt();

        if (ns.nsType == NamespaceEntry::RATING)
        {
            ns.convertRatio.append(current->data(ZEROSTAR_ROLE).toInt());
            ns.convertRatio.append(current->data(ONESTAR_ROLE).toInt());
            ns.convertRatio.append(current->data(TWOSTAR_ROLE).toInt());
            ns.convertRatio.append(current->data(THREESTAR_ROLE).toInt());
            ns.convertRatio.append(current->data(FOURSTAR_ROLE).toInt());
            ns.convertRatio.append(current->data(FIVESTAR_ROLE).toInt());
        }

        ns.specialOpts     = (NamespaceEntry::SpecialOptions)current->data(SPECIALOPTS_ROLE).toInt();
        ns.alternativeName = current->data(ALTNAME_ROLE).toString();
        ns.subspace        = (NamespaceEntry::NsSubspace)current->data(SUBSPACE_ROLE).toInt();
        ns.secondNameOpts  = (NamespaceEntry::SpecialOptions)current->data(ALTNAMEOPTS_ROLE).toInt();
        ns.index           = i;
        ns.isDefault       = current->data(ISDEFAULT_ROLE).toBool();

        if (current->checkState() == Qt::Checked)
        {
            ns.isDisabled = false;
        }
        else
        {
            ns.isDisabled = true;
        }

        container.append(ns);
    }
}

} // namespace Digikam

#include "moc_advancedmetadatatab.cpp"
