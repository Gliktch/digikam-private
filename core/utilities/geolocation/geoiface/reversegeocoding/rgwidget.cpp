/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-12
 * Description : A widget to apply Reverse Geocoding
 *
 * SPDX-FileCopyrightText: 2010 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2010 by Gabriel Voicu <ping dot gabi at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "rgwidget.h"

// Qt includes

#include <QCheckBox>
#include <QContextMenuEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMap>
#include <QPointer>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QMenu>
#include <QInputDialog>
#include <QAction>
#include <QComboBox>
#include <QApplication>
#include <QMessageBox>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "gpsundocommand.h"
#include "gpsitemmodel.h"
#include "backend-geonames-rg.h"
#include "backend-osm-rg.h"
#include "backend-geonamesUS-rg.h"
#include "parsetagstring.h"
#include "rgtagmodel.h"
#include "simpletreemodel.h"
#include "dmessagebox.h"
#include "dexpanderbox.h"
#include "dmetadata.h"
#include "digikam_debug.h"

#ifdef GPSSYNC_MODELTEST
#   include <modeltest.h>
#endif // GPSSYNC_MODELTEST

namespace Digikam
{

/**
 * @class RGWidget
 *
 * @brief The RGWidget class represents the main widget for reverse geocoding.
 */
class Q_DECL_HIDDEN RGWidget::Private
{
public:

    Private() = default;

public:

    bool                   currentlyAskingCancelQuestion    = false;
    bool                   hideOptions                      = true;
    bool                   UIEnabled                        = true;
    QLabel*                label                            = nullptr;
    GPSItemModel*          imageModel                       = nullptr;
    QItemSelectionModel*   selectionModel                   = nullptr;
    QPushButton*           buttonRGSelected                 = nullptr;

    GPSUndoCommand*        undoCommand                      = nullptr;
    QModelIndex            currentTagTreeIndex;

    QComboBox*             serviceComboBox                  = nullptr;
    QComboBox*             languageEdit                     = nullptr;
    QList<RGInfo>          photoList;
    QList<RGBackend*>      backendRGList;
    RGBackend*             currentBackend                   = nullptr;
    int                    requestedRGCount                 = 0;
    int                    receivedRGCount                  = 0;
    QPushButton*           buttonHideOptions                = nullptr;
    QCheckBox*             tagsLoc                          = nullptr;
    QCheckBox*             metaLoc                          = nullptr;
    QWidget*               UGridContainer                   = nullptr;
    QWidget*               LGridContainer                   = nullptr;
    QLabel*                serviceLabel                     = nullptr;
    QLabel*                languageLabel                    = nullptr;
    DLineWidget*           separator                        = nullptr;

    RGTagModel*            tagModel                         = nullptr;
    QTreeView*             tagTreeView                      = nullptr;

    QItemSelectionModel*   tagSelectionModel                = nullptr;
    QAction*               actionAddCountry                 = nullptr;
    QAction*               actionAddState                   = nullptr;
    QAction*               actionAddStateDistrict           = nullptr;
    QAction*               actionAddCounty                  = nullptr;
    QAction*               actionAddCity                    = nullptr;
    QAction*               actionAddCityDistrict            = nullptr;
    QAction*               actionAddSuburb                  = nullptr;
    QAction*               actionAddTown                    = nullptr;
    QAction*               actionAddVillage                 = nullptr;
    QAction*               actionAddHamlet                  = nullptr;
    QAction*               actionAddStreet                  = nullptr;
    QAction*               actionAddHouseNumber             = nullptr;
    QAction*               actionAddPlace                   = nullptr;
    QAction*               actionAddLAU2                    = nullptr;
    QAction*               actionAddLAU1                    = nullptr;
    QAction*               actionAddCustomizedSpacer        = nullptr;
    QAction*               actionRemoveTag                  = nullptr;
    QAction*               actionRemoveAllSpacers           = nullptr;
    QAction*               actionAddAllAddressElementsToTag = nullptr;

    QMap<QString, QString> countryCode;
};

/**
 * @brief Constructor
 *
 * @param imageModel     The image model
 * @param selectionModel The image selection model
 * @param externTagModel The tag model
 * @param parent         The parent object
 */
RGWidget::RGWidget(GPSItemModel* const imageModel,
                   QItemSelectionModel* const selectionModel,
                   QAbstractItemModel* externTagModel,
                   QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    d->imageModel     = imageModel;
    d->selectionModel = selectionModel;

    // we need to have a main layout and add QVBoxLayout

    QVBoxLayout* const vBoxLayout = new QVBoxLayout(this);
    d->UGridContainer             = new QWidget(this);

    vBoxLayout->addWidget(d->UGridContainer);
    d->tagTreeView                = new QTreeView(this);
    d->tagTreeView->setHeaderHidden(true);
    vBoxLayout->addWidget(d->tagTreeView);

    Q_ASSERT(d->tagTreeView != nullptr);

    if (!externTagModel)
    {
        externTagModel = new SimpleTreeModel(1, this);
    }

    d->tagModel = new RGTagModel(externTagModel, this);
    d->tagTreeView->setModel(d->tagModel);

#ifdef GPSSYNC_MODELTEST

    new ModelTest(externTagModel, d->tagTreeView);
    new ModelTest(d->tagModel, d->tagTreeView);

#endif // GPSSYNC_MODELTEST

    d->tagSelectionModel         = new QItemSelectionModel(d->tagModel);
    d->tagTreeView->setSelectionModel(d->tagSelectionModel);

    d->actionAddCountry          = new QAction(i18n("Add country tag"), this);
    d->actionAddCountry->setData(QLatin1String("{Country}"));
    d->actionAddState            = new QAction(i18n("Add state tag"), this);
    d->actionAddState->setData(QLatin1String("{State}"));
    d->actionAddStateDistrict    = new QAction(i18n("Add state district tag"), this);
    d->actionAddStateDistrict->setData(QLatin1String("{State district}"));
    d->actionAddCounty           = new QAction(i18n("Add county tag"), this);
    d->actionAddCounty->setData(QLatin1String("{County}"));
    d->actionAddCity             = new QAction(i18n("Add city tag"), this);
    d->actionAddCity->setData(QLatin1String("{City}"));
    d->actionAddCityDistrict     = new QAction(i18n("Add city district tag"), this);
    d->actionAddCityDistrict->setData(QLatin1String("{City district}"));
    d->actionAddSuburb           = new QAction(i18n("Add suburb tag"), this);
    d->actionAddSuburb->setData(QLatin1String("{Suburb}"));
    d->actionAddTown             = new QAction(i18n("Add town tag"), this);
    d->actionAddTown->setData(QLatin1String("{Town}"));
    d->actionAddVillage          = new QAction(i18n("Add village tag"), this);
    d->actionAddVillage->setData(QLatin1String("{Village}"));
    d->actionAddHamlet           = new QAction(i18n("Add hamlet tag"), this);
    d->actionAddHamlet->setData(QLatin1String("{Hamlet}"));
    d->actionAddStreet           = new QAction(i18n("Add street"), this);
    d->actionAddStreet->setData(QLatin1String("{Street}"));
    d->actionAddHouseNumber      = new QAction(i18n("Add house number tag"), this);
    d->actionAddHouseNumber->setData(QLatin1String("{House number}"));
    d->actionAddPlace            = new QAction(i18n("Add place"), this);
    d->actionAddPlace->setData(QLatin1String("{Place}"));
    d->actionAddLAU2             = new QAction(i18n("Add Local Administrative Area 2"), this);
    d->actionAddLAU2->setData(QLatin1String("{LAU2}"));
    d->actionAddLAU1             = new QAction(i18n("Add Local Administrative Area 1"), this);
    d->actionAddLAU1->setData(QLatin1String("{LAU1}"));
    d->actionAddCustomizedSpacer = new QAction(i18n("Add new tag"), this);
    d->actionRemoveTag           = new QAction(i18n("Remove selected tag"), this);
    d->actionRemoveAllSpacers    = new QAction(i18n("Remove all control tags below this tag"), this);
    d->actionRemoveAllSpacers->setData(QLatin1String("Remove all spacers"));

    d->actionAddAllAddressElementsToTag = new QAction(i18n("Add all address elements"), this);
    QGridLayout* const gridLayout       = new QGridLayout(d->UGridContainer);

    d->languageLabel = new QLabel(i18n("Select language:"), d->UGridContainer);
    d->languageEdit  = new QComboBox(d->UGridContainer);

    DMetadata::CountryCodeMap map = DMetadata::countryCodeMap();

    for (DMetadata::CountryCodeMap::Iterator it = map.begin() ; it != map.end() ; ++it)
    {
        d->languageEdit->addItem(QString::fromUtf8("%1 - %2").arg(it.key()).arg(it.value()), it.key().toLower());
    }

    d->serviceLabel    = new QLabel(i18n("Select service:"), d->UGridContainer);
    d->serviceComboBox = new QComboBox(d->UGridContainer);

    d->serviceComboBox->addItem(i18n("Open Street Map"));
    d->serviceComboBox->addItem(i18n("Geonames.org place name (non-US)"));
    d->serviceComboBox->addItem(i18n("Geonames.org full address (US only)"));

    gridLayout->addWidget(d->serviceLabel,    0, 0, 1, 2);
    gridLayout->addWidget(d->serviceComboBox, 1, 0, 1, 2);
    gridLayout->addWidget(d->languageLabel,   2, 0, 1, 1);
    gridLayout->addWidget(d->languageEdit,    2, 1, 1, 1);

    d->UGridContainer->setLayout(gridLayout);

    d->separator         = new DLineWidget(Qt::Horizontal, this);
    vBoxLayout->addWidget(d->separator);

    d->buttonHideOptions = new QPushButton(i18n("Less options"), this);
    vBoxLayout->addWidget(d->buttonHideOptions);

    d->LGridContainer              = new QWidget(this);
    vBoxLayout->addWidget(d->LGridContainer);
    QGridLayout* const LGridLayout = new QGridLayout(d->LGridContainer);

    d->tagsLoc = new QCheckBox(i18n("Write locations as tags"), d->LGridContainer);
    d->metaLoc = new QCheckBox(i18n("Write locations metadata"), d->LGridContainer);

    LGridLayout->addWidget(d->tagsLoc, 0, 0, 1, 3);
    LGridLayout->addWidget(d->metaLoc, 1, 0, 1, 3);

    d->LGridContainer->setLayout(LGridLayout);

    d->buttonRGSelected = new QPushButton(i18n("Process reverse geocoding"), this);
    vBoxLayout->addWidget(d->buttonRGSelected);

    d->backendRGList.append(new BackendOsmRG(this));
    d->backendRGList.append(new BackendGeonamesRG(this));
    d->backendRGList.append(new BackendGeonamesUSRG(this));

    d->tagTreeView->installEventFilter(this);

    createCountryCodeMap();

    updateUIState();

    connect(d->buttonRGSelected, SIGNAL(clicked()),
            this, SLOT(slotButtonRGSelected()));

    connect(d->buttonHideOptions, SIGNAL(clicked()),
            this, SLOT(slotHideOptions()));

    connect(d->selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(updateUIState()));

    connect(d->actionAddCountry, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddState, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddStateDistrict, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddCounty, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddCity, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddCityDistrict, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddSuburb, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddTown, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddVillage, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddHamlet, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddHouseNumber, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddStreet, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddPlace, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddLAU2, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddLAU1, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSingleSpacer()));

    connect(d->actionAddCustomizedSpacer, SIGNAL(triggered(bool)),
            this, SLOT(slotAddCustomizedSpacer()));

    connect(d->actionAddAllAddressElementsToTag, SIGNAL(triggered(bool)),
            this, SLOT(slotAddAllAddressElementsToTag()));

    connect(d->imageModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotRegenerateNewTags()));

    connect(d->actionRemoveTag, SIGNAL(triggered(bool)),
            this, SLOT(slotRemoveTag()));

    connect(d->actionRemoveAllSpacers, SIGNAL(triggered(bool)),
            this, SLOT(slotRemoveAllSpacers()));

    for (int i = 0 ; i < d->backendRGList.count() ; ++i)
    {
        connect(d->backendRGList[i], SIGNAL(signalRGReady(QList<RGInfo>)),
                this, SLOT(slotRGReady(QList<RGInfo>)));
    }

    int currentServiceIndex = d->serviceComboBox->currentIndex();
    d->currentBackend       = d->backendRGList[currentServiceIndex];
}

/**
 * @brief Destructor
 */
RGWidget::~RGWidget()
{
    delete d;
}

/**
 * @brief Enables or disables the containing widgets.
 */
void RGWidget::updateUIState()
{
    const bool haveSelection = d->selectionModel->hasSelection();

    d->buttonRGSelected->setEnabled(d->UIEnabled && haveSelection);
    d->serviceLabel->setEnabled(d->UIEnabled);
    d->serviceComboBox->setEnabled(d->UIEnabled);
    d->languageLabel->setEnabled(d->UIEnabled);
    d->languageEdit->setEnabled(d->UIEnabled);
    d->buttonHideOptions->setEnabled(d->UIEnabled);
    d->tagsLoc->setEnabled(d->UIEnabled);
    d->metaLoc->setEnabled(d->UIEnabled);
}

/**
 * @brief This slot triggers when the button that start the reverse geocoding process is pressed.
 */
void RGWidget::slotButtonRGSelected()
{
    // get the selected images

    const QModelIndexList selectedItems = d->selectionModel->selectedRows();
    int currentServiceIndex             = d->serviceComboBox->currentIndex();
    d->currentBackend                   = d->backendRGList[currentServiceIndex];
    d->undoCommand                      = new GPSUndoCommand();
    d->undoCommand->setText(i18n("Image tags are changed."));

    QList<RGInfo> photoList;
    QString wantedLanguage                 = d->languageEdit->itemData(d->languageEdit->currentIndex()).toString();

    for (int i = 0 ; i < selectedItems.count() ; ++i)
    {
        const QPersistentModelIndex itemIndex = selectedItems.at(i);
        GPSItemContainer* const selectedItem  = d->imageModel->itemFromIndex(itemIndex);
        const GPSDataContainer gpsData        = selectedItem->gpsData();

        if (!gpsData.hasCoordinates())
        {
            continue;
        }

        const qreal latitude  = gpsData.getCoordinates().lat();
        const qreal longitude = gpsData.getCoordinates().lon();

        RGInfo photoObj;
        photoObj.id           = itemIndex;
        photoObj.coordinates  = GeoCoordinates(latitude, longitude);

        photoList << photoObj;

        selectedItem->writeTagsToXmp(d->tagsLoc->isChecked());
        selectedItem->writeLocations(d->metaLoc->isChecked());
    }

    if (!photoList.isEmpty())
    {
        d->receivedRGCount  = 0;
        d->requestedRGCount = photoList.count();

        Q_EMIT signalSetUIEnabledWithCancel(false, this, QString::fromUtf8(SLOT(slotRGCanceled())));
        Q_EMIT signalProgressSetup(d->requestedRGCount, i18n("Retrieving RG info -"));

        d->currentBackend->callRGBackend(photoList, wantedLanguage);
    }
}

/**
 * @brief Hide or shows the extra options.
 */
void RGWidget::slotHideOptions()
{
    if (d->hideOptions)
    {
        d->LGridContainer->hide();
        d->hideOptions = false;
        d->buttonHideOptions->setText(i18n("More options"));
    }
    else
    {
        d->LGridContainer->show();
        d->hideOptions = true;
        d->buttonHideOptions->setText(i18n("Less options"));
    }
}

/**
 * @brief The data has returned from backend and now it's processed here.
 * @param returnedRGList Contains the data returned by backend.
 */
void RGWidget::slotRGReady(const QList<RGInfo>& returnedRGList)
{
    const QString errorString = d->currentBackend->getErrorMessage();

    if (!errorString.isEmpty())
    {
        /// @todo This collides with the message box displayed if the user aborts the RG process

        QMessageBox::critical(QApplication::activeWindow(), i18nc("@title:window", "Reverse Geo-coding Error"), errorString);

        d->receivedRGCount += returnedRGList.count();

        Q_EMIT signalSetUIEnabled(true);

        return;
    }

    for (int i = 0 ; i < returnedRGList.count() ; ++i)
    {
        QPersistentModelIndex currentImageIndex = returnedRGList[i].id;

        if (!returnedRGList[i].rgData.isEmpty())
        {
            QString addressElementsWantedFormat;

            if      (d->currentBackend->backendName() == QLatin1String("Geonames"))
            {
                addressElementsWantedFormat.append(QLatin1String("/{Country}/{Country code}/{Place}"));
            }
            else if (d->currentBackend->backendName() == QLatin1String("GeonamesUS"))
            {
                addressElementsWantedFormat.append(QLatin1String("/{LAU2}/{LAU1}/{City}"));
            }
            else
            {
                addressElementsWantedFormat.append(QLatin1String("/{Country}/{Country code}/{State}/{State district}"
                                                                 "/{County}/{City}/{City district}/{Suburb}/{Town}"
                                                                 "/{Village}/{Hamlet}/{Street}/{House number}"));
            }

            QStringList combinedResult = makeTagString(returnedRGList[i],
                                                       addressElementsWantedFormat,
                                                       d->currentBackend->backendName());

            QString addressFormat      = combinedResult[0];
            QString addressElements    = combinedResult[1];

            // removes first "/" from tag addresses

            addressFormat.remove(0, 1);
            addressElements.remove(0, 1);
            addressElementsWantedFormat.remove(0, 1);

            const QStringList listAddressElementsWantedFormat = addressElementsWantedFormat.split(QLatin1Char('/'));
            const QStringList listAddressElements             = addressElements.split(QLatin1Char('/'));
            const QStringList listAddressFormat               = addressFormat.split(QLatin1Char('/'));
            QStringList elements, resultedData;

            for (int j = 0 ; j < listAddressElementsWantedFormat.count() ; ++j)
            {
                QString currentAddressFormat = listAddressElementsWantedFormat.at(j);
                int currentIndexFormat       = listAddressFormat.indexOf(currentAddressFormat);

                if (currentIndexFormat != -1)
                {
                    elements << currentAddressFormat;
                    resultedData << listAddressElements.at(currentIndexFormat);
                }
            }

            QList<QList<TagData> > returnedTags = d->tagModel->addNewData(elements, resultedData);

            int countryCodeIndex = listAddressFormat.indexOf(QLatin1String("{Country code}"));

            if (countryCodeIndex != -1)
            {
                TagData countryCode;
                countryCode.tagName = d->countryCode.value(listAddressElements.at(countryCodeIndex).toUpper());
                countryCode.tipName = QLatin1String("{Country code}");
                returnedTags << (QList<TagData>() << countryCode);
            }

            GPSItemContainer* const currentItem = d->imageModel->itemFromIndex(currentImageIndex);

            GPSUndoCommand::UndoInfo undoInfo(currentImageIndex);
            undoInfo.readOldDataFromItem(currentItem);

            currentItem->setTagList(returnedTags);

            undoInfo.readNewDataFromItem(currentItem);
            d->undoCommand->addUndoInfo(undoInfo);
        }
    }

    d->receivedRGCount += returnedRGList.count();

    if (d->receivedRGCount >= d->requestedRGCount)
    {
        if (d->currentlyAskingCancelQuestion)
        {
            // if the user is currently answering the cancel question, do nothing, only report progress

            Q_EMIT signalProgressChanged(d->receivedRGCount);
        }
        else
        {
            Q_EMIT signalUndoCommand(d->undoCommand);

            d->undoCommand = nullptr;

            Q_EMIT signalSetUIEnabled(true);
        }
    }
    else
    {
        Q_EMIT signalProgressChanged(d->receivedRGCount);
    }
}

/**
 * @brief Sets whether the containing widgets are enabled or disabled.
 * @param state If true, the controls are enabled.
 */
void RGWidget::setUIEnabled(const bool state)
{
    d->UIEnabled = state;
    updateUIState();
}

/**
 * @brief Here are filtered the events.
 */
bool RGWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == d->tagTreeView)
    {
        if ((event->type() == QEvent::ContextMenu) && d->UIEnabled)
        {
            QMenu* const menu             = new QMenu(d->tagTreeView);
            const int currentServiceIndex = d->serviceComboBox->currentIndex();
            d->currentBackend             = d->backendRGList[currentServiceIndex];
            QString backendName           = d->currentBackend->backendName();
            QContextMenuEvent* const e    = static_cast<QContextMenuEvent*>(event);
            d->currentTagTreeIndex        = d->tagTreeView->indexAt(e->pos());
            const Type tagType            = d->tagModel->getTagType(d->currentTagTreeIndex);

            if      (backendName == QLatin1String("OSM"))
            {
                menu->addAction(d->actionAddAllAddressElementsToTag);
                menu->addSeparator();
                menu->addAction(d->actionAddCountry);
                menu->addAction(d->actionAddState);
                menu->addAction(d->actionAddStateDistrict);
                menu->addAction(d->actionAddCounty);
                menu->addAction(d->actionAddCity);
                menu->addAction(d->actionAddCityDistrict);
                menu->addAction(d->actionAddSuburb);
                menu->addAction(d->actionAddTown);
                menu->addAction(d->actionAddVillage);
                menu->addAction(d->actionAddHamlet);
                menu->addAction(d->actionAddStreet);
                menu->addAction(d->actionAddHouseNumber);
            }
            else if (backendName == QLatin1String("Geonames"))
            {
                menu->addAction(d->actionAddAllAddressElementsToTag);
                menu->addAction(d->actionAddCountry);
                menu->addAction(d->actionAddPlace);
            }
            else if (backendName == QLatin1String("GeonamesUS"))
            {
                menu->addAction(d->actionAddAllAddressElementsToTag);
                menu->addAction(d->actionAddLAU2);
                menu->addAction(d->actionAddLAU1);
                menu->addAction(d->actionAddCity);
            }

            menu->addSeparator();
            menu->addAction(d->actionAddCustomizedSpacer);
            menu->addSeparator();

            if (tagType == TypeSpacer)
            {
                menu->addAction(d->actionRemoveTag);
            }

            menu->addAction(d->actionRemoveAllSpacers);
            menu->exec(e->globalPos());
            delete menu;
        }
    }

    return QObject::eventFilter(watched, event);
}

/**
 * @brief Saves the settings of widgets contained in reverse geocoding widget.
 * @param group Here are stored the settings.
 */
void RGWidget::saveSettingsToGroup(KConfigGroup* const group)
{
    group->writeEntry("RG Backend",        d->serviceComboBox->currentIndex());
    group->writeEntry("Language",          d->languageEdit->currentIndex());
    group->writeEntry("Hide options",      d->hideOptions);
    group->writeEntry("Tags location",     d->tagsLoc->isChecked());
    group->writeEntry("Metadata location", d->metaLoc->isChecked());

    QList<QList<TagData> > currentSpacerList = d->tagModel->getSpacers();
    const int spacerCount                    = currentSpacerList.count();
    group->writeEntry("Spacers count", spacerCount);

    for (int i = 0 ; i < currentSpacerList.count() ; ++i)
    {
        QString spacerName;
        spacerName.append(QString::fromLatin1("Spacerlistname %1").arg(i));
        QString spacerType;
        spacerType.append(QString::fromLatin1("Spacerlisttype %1").arg(i));

        QStringList spacerTagNames;
        QStringList spacerTypes;

        for (int j = 0 ; j < currentSpacerList[i].count() ; ++j)
        {
            spacerTagNames.append(currentSpacerList[i].at(j).tagName);

            if      (currentSpacerList[i].at(j).tagType == TypeSpacer)
            {
                spacerTypes.append(QLatin1String("Spacer"));
            }
            else if (currentSpacerList[i].at(j).tagType == TypeNewChild)
            {
                spacerTypes.append(QLatin1String("NewChild"));
            }
            else
            {
                spacerTypes.append(QLatin1String("OldChild"));
            }
        }

        group->writeEntry(spacerName, spacerTagNames);
        group->writeEntry(spacerType, spacerTypes);
    }
}

/**
 * @brief Restores the settings of widgets contained in reverse geocoding widget.
 * @param group Here are stored the settings.
 */
void RGWidget::readSettingsFromGroup(const KConfigGroup* const group)
{
    const int spacerCount = group->readEntry("Spacers count", 0);
    QList<QList<TagData> > spacersList;

    for (int i = 0 ; i < spacerCount ; ++i)
    {
        QStringList spacerTagNames = group->readEntry(QString::fromLatin1("Spacerlistname %1").arg(i), QStringList());
        QStringList spacerTypes    = group->readEntry(QString::fromLatin1("Spacerlisttype %1").arg(i), QStringList());
        QList<TagData> currentSpacerAddress;

        for (int j = 0 ; j < spacerTagNames.count() ; ++j)
        {
            TagData currentTagData;
            currentTagData.tagName = spacerTagNames.at(j);
            QString currentTagType = spacerTypes.at(j);

            if      (currentTagType == QLatin1String("Spacer"))
            {
                currentTagData.tagType = TypeSpacer;
            }
            else if (currentTagType == QLatin1String("NewChild"))
            {
                currentTagData.tagType = TypeNewChild;
            }
            else if (currentTagType == QLatin1String("OldChild"))
            {
                currentTagData.tagType = TypeChild;
            }
            else
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "Unknown tag type" << currentTagType;
                continue;
            }

            currentSpacerAddress.append(currentTagData);
        }

        spacersList.append(currentSpacerAddress);
    }

    // this make sure that all external tags are added to tag tree view before spacers are re-added

    d->tagModel->addAllExternalTagsToTreeView();
    d->tagModel->readdNewTags(spacersList);

    d->serviceComboBox->setCurrentIndex(group->readEntry("RG Backend", 0));
    d->languageEdit->setCurrentIndex(group->readEntry("Language",      0));

    d->hideOptions = !(group->readEntry("Hide options",                false));
    slotHideOptions();

    d->tagsLoc->setChecked(group->readEntry("Tags location",           true));
    d->metaLoc->setChecked(group->readEntry("Metadata location",       false));
}

/**
 * @brief Adds a tag to tag tree.
 */
void RGWidget::slotAddSingleSpacer()
{
/*
    const QModelIndex baseIndex = d->tagSelectionModel->currentIndex();
*/
    QModelIndex baseIndex;

    if (!d->currentTagTreeIndex.isValid())
    {
        baseIndex = d->currentTagTreeIndex;
    }
    else
    {
        baseIndex = d->tagSelectionModel->currentIndex();
    }

    QAction* const senderAction = qobject_cast<QAction*>(sender());
    QString currentSpacerName   = senderAction->data().toString();

    d->tagModel->addSpacerTag(baseIndex, currentSpacerName);
}

/**
 * @brief Adds a new tag to the tag tree.
 */
void RGWidget::slotAddCustomizedSpacer()
{
    QModelIndex baseIndex;

    if (!d->currentTagTreeIndex.isValid())
    {
        baseIndex = d->currentTagTreeIndex;
    }
    else
    {
        baseIndex = d->tagSelectionModel->currentIndex();
    }

    bool ok            = false;
    QString textString = QInputDialog::getText(this, i18nc("@title:window", "Add New Tag"),
                                               i18n("Select a name for the new tag:"),
                                               QLineEdit::Normal, QString(), &ok);

    if (ok && !textString.isEmpty())
    {
        d->tagModel->addSpacerTag(baseIndex, textString);
    }
}

/**
 * @brief Removes a tag from tag tree.
 * @note If the tag is an external, it is no more deleted.
 */
void RGWidget::slotRemoveTag()
{
    const QModelIndex baseIndex = d->tagSelectionModel->currentIndex();
    d->tagModel->deleteTag(baseIndex);
}

/**
 * @brief Removes all spacers.
 */
void RGWidget::slotRemoveAllSpacers()
{
    QModelIndex baseIndex;

    if (!d->currentTagTreeIndex.isValid())
    {
        baseIndex = d->currentTagTreeIndex;
    }
    else
    {
        baseIndex = d->tagSelectionModel->currentIndex();
    }

    d->tagModel->deleteAllSpacersOrNewTags(baseIndex, TypeSpacer);
}

/**
 * @brief Re-adds all deleted tags based on Undo/Redo widget.
 */
void RGWidget::slotReaddNewTags()
{
    for (int row = 0 ; row < d->imageModel->rowCount() ; ++row)
    {
        const GPSItemContainer* const currentItem = d->imageModel->itemFromIndex(d->imageModel->index(row, 0));
        QList<QList<TagData> > tagAddresses       = currentItem->getTagList();

        if (!tagAddresses.isEmpty())
        {
            d->tagModel->readdNewTags(tagAddresses);
        }
    }
}

/**
 * @brief Deletes and re-adds all new added tags.
 */
void RGWidget::slotRegenerateNewTags()
{
    QModelIndex baseIndex = QModelIndex();
    d->tagModel->deleteAllSpacersOrNewTags(baseIndex, TypeNewChild);

    slotReaddNewTags();
}

/**
 * @brief Adds all address elements below the selected tag. The address elements are ordered by area size.
 * For example: country > state > state district > city ...
 */
void RGWidget::slotAddAllAddressElementsToTag()
{
    QModelIndex baseIndex;

    if (!d->currentTagTreeIndex.isValid())
    {
        baseIndex = d->currentTagTreeIndex;
    }
    else
    {
        baseIndex = d->tagSelectionModel->currentIndex();
    }

    QStringList spacerList;

    if      (d->currentBackend->backendName() == QLatin1String("OSM"))
    {
        /// @todo Why are these wrapped in QString?

        spacerList.append(QLatin1String("{Country}"));
        spacerList.append(QLatin1String("{State}"));
        spacerList.append(QLatin1String("{State district}"));
        spacerList.append(QLatin1String("{County}"));
        spacerList.append(QLatin1String("{City}"));
        spacerList.append(QLatin1String("{City district}"));
        spacerList.append(QLatin1String("{Suburb}"));
        spacerList.append(QLatin1String("{Town}"));
        spacerList.append(QLatin1String("{Village}"));
        spacerList.append(QLatin1String("{Hamlet}"));
        spacerList.append(QLatin1String("{Street}"));
        spacerList.append(QLatin1String("{House number}"));
    }
    else if (d->currentBackend->backendName() == QLatin1String("Geonames"))
    {
        spacerList.append(QLatin1String("{Country}"));
        spacerList.append(QLatin1String("{Place}"));
    }
    else
    {
        spacerList.append(QLatin1String("{LAU1}"));
        spacerList.append(QLatin1String("{LAU2}"));
        spacerList.append(QLatin1String("{City}"));
    }

    d->tagModel->addAllSpacersToTag(baseIndex, spacerList,0);
}

void RGWidget::slotRGCanceled()
{
    if (!d->undoCommand)
    {
        // the undo command object is not available, therefore
        // RG has probably been finished already

        return;
    }

    if (d->receivedRGCount > 0)
    {
        // Before we abort, ask the user whether he wants to discard
        // the information obtained so far.

        // ATTENTION: While we ask the question, the RG backend continues running
        //            and sends information about new images to this widget.
        //            This means that RG might finish while we ask the question!!!

        d->currentlyAskingCancelQuestion = true;

        QString question;

        if (d->receivedRGCount > 1)
        {
            question = i18n("%1 out of %2 images have been reverse geocoded. "
                            "Would you like to keep the tags which were "
                            "already obtained?",
                            d->receivedRGCount, d->requestedRGCount);
        }
        else
        {
            question = i18n("1 image have been reverse geocoded. "
                            "Would you like to keep the tags which were "
                            "already obtained?");
        }

        const int result = DMessageBox::showYesNo(QMessageBox::Warning, this,
                                                  i18nc("@title:window", "Abort reverse geocoding?"),
                                                  question);

        d->currentlyAskingCancelQuestion = false;

        if (result == QMessageBox::Cancel)
        {
            // continue

            // did RG finish while we asked the question?

            if (d->receivedRGCount==d->requestedRGCount)
            {
                // the undo data was delayed, now send it

                if (d->undoCommand)
                {
                    Q_EMIT signalUndoCommand(d->undoCommand);

                    d->undoCommand = nullptr;
                }

                // unlock the UI

                Q_EMIT signalSetUIEnabled(true);
            }

            return;
        }

        if (result == QMessageBox::No)
        {
            // discard the tags

            d->undoCommand->undo();
        }

        if (result == QMessageBox::Yes)
        {
            if (d->undoCommand)
            {
                Q_EMIT signalUndoCommand(d->undoCommand);

                d->undoCommand = nullptr;
            }
        }
    }

    // clean up the RG request:

    d->currentBackend->cancelRequests();

    if (d->undoCommand)
    {
        delete d->undoCommand;
        d->undoCommand = nullptr;
    }

    Q_EMIT signalSetUIEnabled(true);
}

void RGWidget::createCountryCodeMap()
{
    // All ISO 3166-1 country codes to convert alpha2 to alpha3
    // https://en.wikipedia.org/wiki/ISO_3166-1
    // https://www.iban.com/country-codes

    d->countryCode.insert(QLatin1String("AF"), QLatin1String("AFG"));
    d->countryCode.insert(QLatin1String("AL"), QLatin1String("ALB"));
    d->countryCode.insert(QLatin1String("DZ"), QLatin1String("DZA"));
    d->countryCode.insert(QLatin1String("AS"), QLatin1String("ASM"));
    d->countryCode.insert(QLatin1String("AD"), QLatin1String("AND"));
    d->countryCode.insert(QLatin1String("AO"), QLatin1String("AGO"));
    d->countryCode.insert(QLatin1String("AI"), QLatin1String("AIA"));
    d->countryCode.insert(QLatin1String("AQ"), QLatin1String("ATA"));
    d->countryCode.insert(QLatin1String("AG"), QLatin1String("ATG"));
    d->countryCode.insert(QLatin1String("AR"), QLatin1String("ARG"));
    d->countryCode.insert(QLatin1String("AM"), QLatin1String("ARM"));
    d->countryCode.insert(QLatin1String("AW"), QLatin1String("ABW"));
    d->countryCode.insert(QLatin1String("AU"), QLatin1String("AUS"));
    d->countryCode.insert(QLatin1String("AT"), QLatin1String("AUT"));
    d->countryCode.insert(QLatin1String("AZ"), QLatin1String("AZE"));
    d->countryCode.insert(QLatin1String("BS"), QLatin1String("BHS"));
    d->countryCode.insert(QLatin1String("BH"), QLatin1String("BHR"));
    d->countryCode.insert(QLatin1String("BD"), QLatin1String("BGD"));
    d->countryCode.insert(QLatin1String("BB"), QLatin1String("BRB"));
    d->countryCode.insert(QLatin1String("BY"), QLatin1String("BLR"));
    d->countryCode.insert(QLatin1String("BE"), QLatin1String("BEL"));
    d->countryCode.insert(QLatin1String("BZ"), QLatin1String("BLZ"));
    d->countryCode.insert(QLatin1String("BJ"), QLatin1String("BEN"));
    d->countryCode.insert(QLatin1String("BM"), QLatin1String("BMU"));
    d->countryCode.insert(QLatin1String("BT"), QLatin1String("BTN"));
    d->countryCode.insert(QLatin1String("BO"), QLatin1String("BOL"));
    d->countryCode.insert(QLatin1String("BQ"), QLatin1String("BES"));
    d->countryCode.insert(QLatin1String("BA"), QLatin1String("BIH"));
    d->countryCode.insert(QLatin1String("BW"), QLatin1String("BWA"));
    d->countryCode.insert(QLatin1String("BV"), QLatin1String("BVT"));
    d->countryCode.insert(QLatin1String("BR"), QLatin1String("BRA"));
    d->countryCode.insert(QLatin1String("IO"), QLatin1String("IOT"));
    d->countryCode.insert(QLatin1String("BN"), QLatin1String("BRN"));
    d->countryCode.insert(QLatin1String("BG"), QLatin1String("BGR"));
    d->countryCode.insert(QLatin1String("BF"), QLatin1String("BFA"));
    d->countryCode.insert(QLatin1String("BI"), QLatin1String("BDI"));
    d->countryCode.insert(QLatin1String("CV"), QLatin1String("CPV"));
    d->countryCode.insert(QLatin1String("KH"), QLatin1String("KHM"));
    d->countryCode.insert(QLatin1String("CM"), QLatin1String("CMR"));
    d->countryCode.insert(QLatin1String("CA"), QLatin1String("CAN"));
    d->countryCode.insert(QLatin1String("KY"), QLatin1String("CYM"));
    d->countryCode.insert(QLatin1String("CF"), QLatin1String("CAF"));
    d->countryCode.insert(QLatin1String("TD"), QLatin1String("TCD"));
    d->countryCode.insert(QLatin1String("CL"), QLatin1String("CHL"));
    d->countryCode.insert(QLatin1String("CN"), QLatin1String("CHN"));
    d->countryCode.insert(QLatin1String("CX"), QLatin1String("CXR"));
    d->countryCode.insert(QLatin1String("CC"), QLatin1String("CCK"));
    d->countryCode.insert(QLatin1String("CO"), QLatin1String("COL"));
    d->countryCode.insert(QLatin1String("KM"), QLatin1String("COM"));
    d->countryCode.insert(QLatin1String("CD"), QLatin1String("COD"));
    d->countryCode.insert(QLatin1String("CG"), QLatin1String("COG"));
    d->countryCode.insert(QLatin1String("CK"), QLatin1String("COK"));
    d->countryCode.insert(QLatin1String("CR"), QLatin1String("CRI"));
    d->countryCode.insert(QLatin1String("HR"), QLatin1String("HRV"));
    d->countryCode.insert(QLatin1String("CU"), QLatin1String("CUB"));
    d->countryCode.insert(QLatin1String("CW"), QLatin1String("CUW"));
    d->countryCode.insert(QLatin1String("CY"), QLatin1String("CYP"));
    d->countryCode.insert(QLatin1String("CZ"), QLatin1String("CZE"));
    d->countryCode.insert(QLatin1String("CI"), QLatin1String("CIV"));
    d->countryCode.insert(QLatin1String("DK"), QLatin1String("DNK"));
    d->countryCode.insert(QLatin1String("DJ"), QLatin1String("DJI"));
    d->countryCode.insert(QLatin1String("DM"), QLatin1String("DMA"));
    d->countryCode.insert(QLatin1String("DO"), QLatin1String("DOM"));
    d->countryCode.insert(QLatin1String("EC"), QLatin1String("ECU"));
    d->countryCode.insert(QLatin1String("EG"), QLatin1String("EGY"));
    d->countryCode.insert(QLatin1String("SV"), QLatin1String("SLV"));
    d->countryCode.insert(QLatin1String("GQ"), QLatin1String("GNQ"));
    d->countryCode.insert(QLatin1String("ER"), QLatin1String("ERI"));
    d->countryCode.insert(QLatin1String("EE"), QLatin1String("EST"));
    d->countryCode.insert(QLatin1String("SZ"), QLatin1String("SWZ"));
    d->countryCode.insert(QLatin1String("ET"), QLatin1String("ETH"));
    d->countryCode.insert(QLatin1String("FK"), QLatin1String("FLK"));
    d->countryCode.insert(QLatin1String("FO"), QLatin1String("FRO"));       // codespell:ignore fro
    d->countryCode.insert(QLatin1String("FJ"), QLatin1String("FJI"));
    d->countryCode.insert(QLatin1String("FI"), QLatin1String("FIN"));
    d->countryCode.insert(QLatin1String("FR"), QLatin1String("FRA"));
    d->countryCode.insert(QLatin1String("GF"), QLatin1String("GUF"));
    d->countryCode.insert(QLatin1String("PF"), QLatin1String("PYF"));
    d->countryCode.insert(QLatin1String("TF"), QLatin1String("ATF"));
    d->countryCode.insert(QLatin1String("GA"), QLatin1String("GAB"));
    d->countryCode.insert(QLatin1String("GM"), QLatin1String("GMB"));
    d->countryCode.insert(QLatin1String("GE"), QLatin1String("GEO"));
    d->countryCode.insert(QLatin1String("DE"), QLatin1String("DEU"));
    d->countryCode.insert(QLatin1String("GH"), QLatin1String("GHA"));
    d->countryCode.insert(QLatin1String("GI"), QLatin1String("GIB"));
    d->countryCode.insert(QLatin1String("GR"), QLatin1String("GRC"));
    d->countryCode.insert(QLatin1String("GL"), QLatin1String("GRL"));
    d->countryCode.insert(QLatin1String("GD"), QLatin1String("GRD"));
    d->countryCode.insert(QLatin1String("GP"), QLatin1String("GLP"));
    d->countryCode.insert(QLatin1String("GU"), QLatin1String("GUM"));
    d->countryCode.insert(QLatin1String("GT"), QLatin1String("GTM"));
    d->countryCode.insert(QLatin1String("GG"), QLatin1String("GGY"));
    d->countryCode.insert(QLatin1String("GN"), QLatin1String("GIN"));
    d->countryCode.insert(QLatin1String("GW"), QLatin1String("GNB"));
    d->countryCode.insert(QLatin1String("GY"), QLatin1String("GUY"));
    d->countryCode.insert(QLatin1String("HT"), QLatin1String("HTI"));
    d->countryCode.insert(QLatin1String("HM"), QLatin1String("HMD"));
    d->countryCode.insert(QLatin1String("VA"), QLatin1String("VAT"));
    d->countryCode.insert(QLatin1String("HN"), QLatin1String("HND"));
    d->countryCode.insert(QLatin1String("HK"), QLatin1String("HKG"));
    d->countryCode.insert(QLatin1String("HU"), QLatin1String("HUN"));
    d->countryCode.insert(QLatin1String("IS"), QLatin1String("ISL"));
    d->countryCode.insert(QLatin1String("IN"), QLatin1String("IND"));
    d->countryCode.insert(QLatin1String("ID"), QLatin1String("IDN"));
    d->countryCode.insert(QLatin1String("IR"), QLatin1String("IRN"));
    d->countryCode.insert(QLatin1String("IQ"), QLatin1String("IRQ"));
    d->countryCode.insert(QLatin1String("IE"), QLatin1String("IRL"));
    d->countryCode.insert(QLatin1String("IM"), QLatin1String("IMN"));
    d->countryCode.insert(QLatin1String("IL"), QLatin1String("ISR"));
    d->countryCode.insert(QLatin1String("IT"), QLatin1String("ITA"));
    d->countryCode.insert(QLatin1String("JM"), QLatin1String("JAM"));
    d->countryCode.insert(QLatin1String("JP"), QLatin1String("JPN"));
    d->countryCode.insert(QLatin1String("JE"), QLatin1String("JEY"));
    d->countryCode.insert(QLatin1String("JO"), QLatin1String("JOR"));
    d->countryCode.insert(QLatin1String("KZ"), QLatin1String("KAZ"));
    d->countryCode.insert(QLatin1String("KE"), QLatin1String("KEN"));
    d->countryCode.insert(QLatin1String("KI"), QLatin1String("KIR"));
    d->countryCode.insert(QLatin1String("KP"), QLatin1String("PRK"));
    d->countryCode.insert(QLatin1String("KR"), QLatin1String("KOR"));
    d->countryCode.insert(QLatin1String("KW"), QLatin1String("KWT"));
    d->countryCode.insert(QLatin1String("KG"), QLatin1String("KGZ"));
    d->countryCode.insert(QLatin1String("LA"), QLatin1String("LAO"));
    d->countryCode.insert(QLatin1String("LV"), QLatin1String("LVA"));
    d->countryCode.insert(QLatin1String("LB"), QLatin1String("LBN"));
    d->countryCode.insert(QLatin1String("LS"), QLatin1String("LSO"));
    d->countryCode.insert(QLatin1String("LR"), QLatin1String("LBR"));
    d->countryCode.insert(QLatin1String("LY"), QLatin1String("LBY"));
    d->countryCode.insert(QLatin1String("LI"), QLatin1String("LIE"));
    d->countryCode.insert(QLatin1String("LT"), QLatin1String("LTU"));
    d->countryCode.insert(QLatin1String("LU"), QLatin1String("LUX"));
    d->countryCode.insert(QLatin1String("MO"), QLatin1String("MAC"));
    d->countryCode.insert(QLatin1String("MG"), QLatin1String("MDG"));
    d->countryCode.insert(QLatin1String("MW"), QLatin1String("MWI"));
    d->countryCode.insert(QLatin1String("MY"), QLatin1String("MYS"));
    d->countryCode.insert(QLatin1String("MV"), QLatin1String("MDV"));
    d->countryCode.insert(QLatin1String("ML"), QLatin1String("MLI"));
    d->countryCode.insert(QLatin1String("MT"), QLatin1String("MLT"));
    d->countryCode.insert(QLatin1String("MH"), QLatin1String("MHL"));
    d->countryCode.insert(QLatin1String("MQ"), QLatin1String("MTQ"));
    d->countryCode.insert(QLatin1String("MR"), QLatin1String("MRT"));
    d->countryCode.insert(QLatin1String("MU"), QLatin1String("MUS"));
    d->countryCode.insert(QLatin1String("YT"), QLatin1String("MYT"));
    d->countryCode.insert(QLatin1String("MX"), QLatin1String("MEX"));
    d->countryCode.insert(QLatin1String("FM"), QLatin1String("FSM"));
    d->countryCode.insert(QLatin1String("MD"), QLatin1String("MDA"));
    d->countryCode.insert(QLatin1String("MC"), QLatin1String("MCO"));
    d->countryCode.insert(QLatin1String("MN"), QLatin1String("MNG"));
    d->countryCode.insert(QLatin1String("ME"), QLatin1String("MNE"));
    d->countryCode.insert(QLatin1String("MS"), QLatin1String("MSR"));
    d->countryCode.insert(QLatin1String("MA"), QLatin1String("MAR"));
    d->countryCode.insert(QLatin1String("MZ"), QLatin1String("MOZ"));
    d->countryCode.insert(QLatin1String("MM"), QLatin1String("MMR"));
    d->countryCode.insert(QLatin1String("NA"), QLatin1String("NAM"));
    d->countryCode.insert(QLatin1String("NR"), QLatin1String("NRU"));
    d->countryCode.insert(QLatin1String("NP"), QLatin1String("NPL"));
    d->countryCode.insert(QLatin1String("NL"), QLatin1String("NLD"));
    d->countryCode.insert(QLatin1String("NC"), QLatin1String("NCL"));
    d->countryCode.insert(QLatin1String("NZ"), QLatin1String("NZL"));
    d->countryCode.insert(QLatin1String("NI"), QLatin1String("NIC"));
    d->countryCode.insert(QLatin1String("NE"), QLatin1String("NER"));
    d->countryCode.insert(QLatin1String("NG"), QLatin1String("NGA"));
    d->countryCode.insert(QLatin1String("NU"), QLatin1String("NIU"));
    d->countryCode.insert(QLatin1String("NF"), QLatin1String("NFK"));
    d->countryCode.insert(QLatin1String("MP"), QLatin1String("MNP"));
    d->countryCode.insert(QLatin1String("NO"), QLatin1String("NOR"));
    d->countryCode.insert(QLatin1String("OM"), QLatin1String("OMN"));
    d->countryCode.insert(QLatin1String("PK"), QLatin1String("PAK"));
    d->countryCode.insert(QLatin1String("PW"), QLatin1String("PLW"));
    d->countryCode.insert(QLatin1String("PS"), QLatin1String("PSE"));
    d->countryCode.insert(QLatin1String("PA"), QLatin1String("PAN"));
    d->countryCode.insert(QLatin1String("PG"), QLatin1String("PNG"));
    d->countryCode.insert(QLatin1String("PY"), QLatin1String("PRY"));
    d->countryCode.insert(QLatin1String("PE"), QLatin1String("PER"));
    d->countryCode.insert(QLatin1String("PH"), QLatin1String("PHL"));
    d->countryCode.insert(QLatin1String("PN"), QLatin1String("PCN"));
    d->countryCode.insert(QLatin1String("PL"), QLatin1String("POL"));
    d->countryCode.insert(QLatin1String("PT"), QLatin1String("PRT"));
    d->countryCode.insert(QLatin1String("PR"), QLatin1String("PRI"));
    d->countryCode.insert(QLatin1String("QA"), QLatin1String("QAT"));
    d->countryCode.insert(QLatin1String("MK"), QLatin1String("MKD"));
    d->countryCode.insert(QLatin1String("RO"), QLatin1String("ROU"));
    d->countryCode.insert(QLatin1String("RU"), QLatin1String("RUS"));
    d->countryCode.insert(QLatin1String("RW"), QLatin1String("RWA"));
    d->countryCode.insert(QLatin1String("RE"), QLatin1String("REU"));
    d->countryCode.insert(QLatin1String("BL"), QLatin1String("BLM"));
    d->countryCode.insert(QLatin1String("SH"), QLatin1String("SHN"));
    d->countryCode.insert(QLatin1String("KN"), QLatin1String("KNA"));
    d->countryCode.insert(QLatin1String("LC"), QLatin1String("LCA"));
    d->countryCode.insert(QLatin1String("MF"), QLatin1String("MAF"));
    d->countryCode.insert(QLatin1String("PM"), QLatin1String("SPM"));
    d->countryCode.insert(QLatin1String("VC"), QLatin1String("VCT"));
    d->countryCode.insert(QLatin1String("WS"), QLatin1String("WSM"));
    d->countryCode.insert(QLatin1String("SM"), QLatin1String("SMR"));
    d->countryCode.insert(QLatin1String("ST"), QLatin1String("STP"));
    d->countryCode.insert(QLatin1String("SA"), QLatin1String("SAU"));
    d->countryCode.insert(QLatin1String("SN"), QLatin1String("SEN"));
    d->countryCode.insert(QLatin1String("RS"), QLatin1String("SRB"));
    d->countryCode.insert(QLatin1String("SC"), QLatin1String("SYC"));
    d->countryCode.insert(QLatin1String("SL"), QLatin1String("SLE"));
    d->countryCode.insert(QLatin1String("SG"), QLatin1String("SGP"));
    d->countryCode.insert(QLatin1String("SX"), QLatin1String("SXM"));
    d->countryCode.insert(QLatin1String("SK"), QLatin1String("SVK"));
    d->countryCode.insert(QLatin1String("SI"), QLatin1String("SVN"));
    d->countryCode.insert(QLatin1String("SB"), QLatin1String("SLB"));
    d->countryCode.insert(QLatin1String("SO"), QLatin1String("SOM"));       // codespell:ignore som
    d->countryCode.insert(QLatin1String("ZA"), QLatin1String("ZAF"));
    d->countryCode.insert(QLatin1String("GS"), QLatin1String("SGS"));
    d->countryCode.insert(QLatin1String("SS"), QLatin1String("SSD"));
    d->countryCode.insert(QLatin1String("ES"), QLatin1String("ESP"));
    d->countryCode.insert(QLatin1String("LK"), QLatin1String("LKA"));
    d->countryCode.insert(QLatin1String("SD"), QLatin1String("SDN"));
    d->countryCode.insert(QLatin1String("SR"), QLatin1String("SUR"));
    d->countryCode.insert(QLatin1String("SJ"), QLatin1String("SJM"));
    d->countryCode.insert(QLatin1String("SE"), QLatin1String("SWE"));
    d->countryCode.insert(QLatin1String("CH"), QLatin1String("CHE"));
    d->countryCode.insert(QLatin1String("SY"), QLatin1String("SYR"));
    d->countryCode.insert(QLatin1String("TW"), QLatin1String("TWN"));
    d->countryCode.insert(QLatin1String("TJ"), QLatin1String("TJK"));
    d->countryCode.insert(QLatin1String("TZ"), QLatin1String("TZA"));
    d->countryCode.insert(QLatin1String("TH"), QLatin1String("THA"));       // codespell:ignore tha
    d->countryCode.insert(QLatin1String("TL"), QLatin1String("TLS"));
    d->countryCode.insert(QLatin1String("TG"), QLatin1String("TGO"));
    d->countryCode.insert(QLatin1String("TK"), QLatin1String("TKL"));
    d->countryCode.insert(QLatin1String("TO"), QLatin1String("TON"));
    d->countryCode.insert(QLatin1String("TT"), QLatin1String("TTO"));
    d->countryCode.insert(QLatin1String("TN"), QLatin1String("TUN"));
    d->countryCode.insert(QLatin1String("TR"), QLatin1String("TUR"));
    d->countryCode.insert(QLatin1String("TM"), QLatin1String("TKM"));
    d->countryCode.insert(QLatin1String("TC"), QLatin1String("TCA"));
    d->countryCode.insert(QLatin1String("TV"), QLatin1String("TUV"));
    d->countryCode.insert(QLatin1String("UG"), QLatin1String("UGA"));
    d->countryCode.insert(QLatin1String("UA"), QLatin1String("UKR"));
    d->countryCode.insert(QLatin1String("AE"), QLatin1String("ARE"));
    d->countryCode.insert(QLatin1String("GB"), QLatin1String("GBR"));
    d->countryCode.insert(QLatin1String("UM"), QLatin1String("UMI"));
    d->countryCode.insert(QLatin1String("US"), QLatin1String("USA"));
    d->countryCode.insert(QLatin1String("UY"), QLatin1String("URY"));
    d->countryCode.insert(QLatin1String("UZ"), QLatin1String("UZB"));
    d->countryCode.insert(QLatin1String("VU"), QLatin1String("VUT"));
    d->countryCode.insert(QLatin1String("VE"), QLatin1String("VEN"));
    d->countryCode.insert(QLatin1String("VN"), QLatin1String("VNM"));
    d->countryCode.insert(QLatin1String("VG"), QLatin1String("VGB"));
    d->countryCode.insert(QLatin1String("VI"), QLatin1String("VIR"));
    d->countryCode.insert(QLatin1String("WF"), QLatin1String("WLF"));
    d->countryCode.insert(QLatin1String("EH"), QLatin1String("ESH"));
    d->countryCode.insert(QLatin1String("YE"), QLatin1String("YEM"));
    d->countryCode.insert(QLatin1String("ZM"), QLatin1String("ZMB"));
    d->countryCode.insert(QLatin1String("ZW"), QLatin1String("ZWE"));
    d->countryCode.insert(QLatin1String("AX"), QLatin1String("ALA"));
}

} // namespace Digikam

#include "moc_rgwidget.cpp"
