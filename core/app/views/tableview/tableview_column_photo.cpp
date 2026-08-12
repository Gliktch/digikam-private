/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-03-14
 * Description : Table view column helpers: Photo properties
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tableview_column_photo.h"

// Qt includes

#include <QFormLayout>
#include <QComboBox>
#include <QLocale>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "coredbinfocontainers.h"
#include "itempropertiestab.h"
#include "dmetadata.h"
#include "iteminfo.h"

namespace Digikam
{

namespace TableViewColumns
{

ColumnPhotoProperties::ColumnPhotoProperties(TableViewShared* const tableViewShared,
                                             const TableViewColumnConfiguration& pConfiguration,
                                             const SubColumn pSubColumn,
                                             QObject* const parent)
    : TableViewColumn(tableViewShared, pConfiguration, parent),
      subColumn      (pSubColumn)
{
}

QStringList ColumnPhotoProperties::getSubColumns()
{
    QStringList columns;
    columns << QLatin1String("cameramaker")
            << QLatin1String("cameramodel")
            << QLatin1String("lens")
            << QLatin1String("aperture")
            << QLatin1String("focal")
            << QLatin1String("exposure")
            << QLatin1String("sensitivity")
            << QLatin1String("modeprogram")
            << QLatin1String("flash")
            << QLatin1String("whitebalance");

    return columns;
}

TableViewColumnDescription ColumnPhotoProperties::getDescription()
{
    TableViewColumnDescription description(QLatin1String("photo-properties"), i18nc("@info: tableview", "Photo properties"));
    description.setIcon(QLatin1String("camera-photo"));

    description.addSubColumn(TableViewColumnDescription(QLatin1String("cameramaker"),  i18nc("@title: tableview", "Camera maker")));
    description.addSubColumn(TableViewColumnDescription(QLatin1String("cameramodel"),  i18nc("@title: tableview", "Camera model")));
    description.addSubColumn(TableViewColumnDescription(QLatin1String("lens"),         i18nc("@title: tableview", "Lens")));
    description.addSubColumn(TableViewColumnDescription(QLatin1String("aperture"),     i18nc("@title: tableview", "Aperture")));
    description.addSubColumn(TableViewColumnDescription(QLatin1String("focal"),        i18nc("@title: tableview", "Focal length")));
    description.addSubColumn(TableViewColumnDescription(QLatin1String("exposure"),     i18nc("@title: tableview", "Exposure")));
    description.addSubColumn(TableViewColumnDescription(QLatin1String("sensitivity"),  i18nc("@title: tableview", "Sensitivity")));
    description.addSubColumn(TableViewColumnDescription(QLatin1String("modeprogram"),  i18nc("@title: tableview", "Mode/program")));
    description.addSubColumn(TableViewColumnDescription(QLatin1String("flash"),        i18nc("@title: tableview", "Flash")));
    description.addSubColumn(TableViewColumnDescription(QLatin1String("whitebalance"), i18nc("@title: tableview", "White balance")));

    return description;
}

QString ColumnPhotoProperties::getTitle() const
{
    switch (subColumn)
    {
        case SubColumnCameraMaker:
        {
            return i18nc("@title: tableview", "Camera maker");
        }

        case SubColumnCameraModel:
        {
            return i18nc("@title: tableview", "Camera model");
        }

        case SubColumnLens:
        {
            return i18nc("@title: tableview", "Lens");
        }

        case SubColumnAperture:
        {
            return i18nc("@title: tableview", "Aperture");
        }

        case SubColumnFocal:
        {
            return i18nc("@title: tableview", "Focal length");
        }

        case SubColumnExposure:
        {
            return i18nc("@title: tableview", "Exposure");
        }

        case SubColumnSensitivity:
        {
            return i18nc("@title: tableview", "Sensitivity");
        }

        case SubColumnModeProgram:
        {
            return i18nc("@title: tableview", "Mode/program");
        }

        case SubColumnFlash:
        {
            return i18nc("@title: tableview", "Flash");
        }

        case SubColumnWhiteBalance:
        {
            return i18nc("@title: tableview", "White balance");
        }
    }

    return QString();
}

TableViewColumn::ColumnFlags ColumnPhotoProperties::getColumnFlags() const
{
    ColumnFlags flags(ColumnNoFlags);

    if (
        (subColumn == SubColumnAperture)  ||
        (subColumn == SubColumnFocal)     ||
        (subColumn == SubColumnExposure)  ||
        (subColumn == SubColumnSensitivity)
       )
    {
        flags |= ColumnCustomSorting;
    }

    if (subColumn == SubColumnExposure)
    {
        flags |= ColumnHasConfigurationWidget;
    }

    return flags;
}

QVariant ColumnPhotoProperties::data(TableViewModel::Item* const item, const int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    switch (subColumn)
    {
        case SubColumnCameraMaker:
        {
            QString cameraMaker = s->tableViewModel->itemDatabaseFieldRaw(item,
                                  DatabaseFields::Set(DatabaseFields::Make)).toString();
            ItemPropertiesTab::shortenedMakeInfo(cameraMaker);

            return cameraMaker;
        }

        case SubColumnCameraModel:
        {
            QString cameraModel = s->tableViewModel->itemDatabaseFieldRaw(item,
                                  DatabaseFields::Set(DatabaseFields::Model)).toString();
            ItemPropertiesTab::shortenedModelInfo(cameraModel);

            return cameraModel;
        }

        case SubColumnLens:
        {
            const QString cameraLens = s->tableViewModel->itemDatabaseFieldRaw(item,
                                       DatabaseFields::Set(DatabaseFields::Lens)).toString();

            return cameraLens;
        }

        case SubColumnAperture:
        {
            const QVariant apertureVariant = s->tableViewModel->itemDatabaseFieldRaw(item,
                                             DatabaseFields::Set(DatabaseFields::Aperture));
            const QString apertureString   = DMetadata::valueToString(apertureVariant, MetadataInfo::Aperture);

            return apertureString;
        }

        case SubColumnFocal:
        {
            /// @todo Make this configurable

            const DatabaseFields::Set requiredSet                 = DatabaseFields::Set(DatabaseFields::FocalLength | DatabaseFields::FocalLength35);
            const TableViewModel::DatabaseFieldsHashRaw rawFields = s->tableViewModel->itemDatabaseFieldsRaw(item, requiredSet);
            const QVariant focalLengthVariant                     = rawFields.value(DatabaseFields::FocalLength);
            const QString focalLengthString                       = DMetadata::valueToString(focalLengthVariant, MetadataInfo::FocalLength);
            const QVariant focalLength35Variant                   = rawFields.value(DatabaseFields::FocalLength35);
            const QString focalLength35String                     = DMetadata::valueToString(focalLength35Variant, MetadataInfo::FocalLengthIn35mm);

            if (focalLength35String.isEmpty())
            {
                return focalLengthString;
            }

            if (focalLengthString.isEmpty())
            {
                return QString();
            }

            /// @todo What if only 35 mm is set?

            return i18nc("@info: tableview", "%1 (%2)", focalLengthString, focalLength35String);
        }

        case SubColumnExposure:
        {
            /// @todo Add a configuration option for fraction vs number, units s vs ms vs mus

            const QVariant exposureVariant = s->tableViewModel->itemDatabaseFieldRaw(item,
                                             DatabaseFields::Set(DatabaseFields::ExposureTime));

            if (configuration.getSetting(QLatin1String("format"), QLatin1String("fraction")) == QLatin1String("fraction"))
            {
                const QString exposureString = DMetadata::valueToString(exposureVariant, MetadataInfo::ExposureTime);

                return exposureString;
            }

            if (!exposureVariant.isValid())
            {
                return QString();
            }

            const QString unitKey = configuration.getSetting(QLatin1String("unit"), QLatin1String("seconds"));
            double multiplier     = 1.0;
            KLocalizedString exposureTimeLocalizedString = ki18nc("@info: tableview", "%1 s");

            if      (unitKey == QLatin1String("milliseconds"))
            {
                multiplier                  = 1000.0;
                exposureTimeLocalizedString = ki18nc("@info: tableview", "%1 ms");
            }
            else if (unitKey == QLatin1String("microseconds"))
            {
                multiplier                  = 1000000.0;
                exposureTimeLocalizedString = ki18nc("@info: tableview", "%1 µs");
            }

            const double exposureTime = exposureVariant.toDouble() * multiplier;

            /// @todo Seems like we have to check for 0 here, too, as an invalid value

            if (exposureTime == 0)
            {
                return QString();
            }

            /// @todo Remove trailing zeros?
            /// @todo Align right? --> better align at decimal point

            const QString exposureTimeString = exposureTimeLocalizedString.subs(QLocale().toString(exposureTime, 'g', 3)).toString();

            return exposureTimeString;
        }

        case SubColumnSensitivity:
        {
            const QVariant sensitivityVariant = s->tableViewModel->itemDatabaseFieldRaw(item,
                                                DatabaseFields::Set(DatabaseFields::Sensitivity));
            const QString sensitivityString   = DMetadata::valueToString(sensitivityVariant, MetadataInfo::Sensitivity);

            if (sensitivityString.isEmpty())
            {
                return QString();
            }

            return i18nc("@info: tableview", "%1 ISO", sensitivityString);
        }

        case SubColumnModeProgram:
        {
            const DatabaseFields::Set requiredSet                 = DatabaseFields::Set(DatabaseFields::ExposureMode | DatabaseFields::ExposureProgram);
            const TableViewModel::DatabaseFieldsHashRaw rawFields = s->tableViewModel->itemDatabaseFieldsRaw(item, requiredSet);
            const QVariant exposureModeVariant                    = rawFields.value(DatabaseFields::ExposureMode);
            const QString exposureModeString                      = DMetadata::valueToString(exposureModeVariant, MetadataInfo::ExposureMode);
            const QVariant exposureProgramVariant                 = rawFields.value(DatabaseFields::ExposureProgram);
            const QString exposureProgramString                   = DMetadata::valueToString(exposureProgramVariant, MetadataInfo::ExposureProgram);

            if      (exposureModeString.isEmpty() && exposureProgramString.isEmpty())
            {
                return QString();
            }
            else if (!exposureModeString.isEmpty() && exposureProgramString.isEmpty())
            {
                return exposureModeString;
            }
            else if (exposureModeString.isEmpty() && !exposureProgramString.isEmpty())
            {
                return exposureProgramString;
            }

            return QString::fromUtf8("%1 / %2").arg(exposureModeString).arg(exposureProgramString);
        }

        case SubColumnFlash:
        {
            const QVariant flashModeVariant = s->tableViewModel->itemDatabaseFieldRaw(item,
                                              DatabaseFields::Set(DatabaseFields::FlashMode));
            const QString flashModeString   = DMetadata::valueToString(flashModeVariant, MetadataInfo::FlashMode);

            return flashModeString;
        }

        case SubColumnWhiteBalance:
        {
            const QVariant whiteBalanceVariant = s->tableViewModel->itemDatabaseFieldRaw(item,
                                                 DatabaseFields::Set(DatabaseFields::WhiteBalance));
            const QString whiteBalanceString   = DMetadata::valueToString(whiteBalanceVariant, MetadataInfo::WhiteBalance);

            return whiteBalanceString;
        }
    }

    return QVariant();
}

TableViewColumn::ColumnCompareResult ColumnPhotoProperties::compare(TableViewModel::Item* const itemA, TableViewModel::Item* const itemB) const
{
    const ItemInfo infoA = s->tableViewModel->infoFromItem(itemA);
    const ItemInfo infoB = s->tableViewModel->infoFromItem(itemB);

    switch (subColumn)
    {
        case SubColumnAperture:
        {
            const QVariant variantA = s->tableViewModel->itemDatabaseFieldRaw(itemA,
                                      DatabaseFields::Set(DatabaseFields::Aperture));
            const QVariant variantB = s->tableViewModel->itemDatabaseFieldRaw(itemB,
                                      DatabaseFields::Set(DatabaseFields::Aperture));
            const double apertureA  = variantA.toDouble();
            const double apertureB  = variantB.toDouble();

            return compareHelper<double>(apertureA, apertureB);
        }

        case SubColumnFocal:
        {
            /// @todo This just works if both have focal length set, not if focal length 35 has to be used

            const QVariant variantA   = s->tableViewModel->itemDatabaseFieldRaw(itemA,
                                        DatabaseFields::Set(DatabaseFields::FocalLength));
            const QVariant variantB   = s->tableViewModel->itemDatabaseFieldRaw(itemB,
                                        DatabaseFields::Set(DatabaseFields::FocalLength));
            const double focalLengthA = variantA.toDouble();
            const double focalLengthB = variantB.toDouble();

            return compareHelper<double>(focalLengthA, focalLengthB);
        }

        case SubColumnExposure:
        {
            const QVariant variantA    = s->tableViewModel->itemDatabaseFieldRaw(itemA,
                                         DatabaseFields::Set(DatabaseFields::ExposureTime));
            const QVariant variantB    = s->tableViewModel->itemDatabaseFieldRaw(itemB,
                                         DatabaseFields::Set(DatabaseFields::ExposureTime));
            const double exposureTimeA = variantA.toDouble();
            const double exposureTimeB = variantB.toDouble();

            return compareHelper<double>(exposureTimeA, exposureTimeB);
        }

        case SubColumnSensitivity:
        {
            const QVariant variantA   = s->tableViewModel->itemDatabaseFieldRaw(itemA,
                                        DatabaseFields::Set(DatabaseFields::Sensitivity));
            const QVariant variantB   = s->tableViewModel->itemDatabaseFieldRaw(itemB,
                                        DatabaseFields::Set(DatabaseFields::Sensitivity));
            const double sensitivityA = variantA.toDouble();
            const double sensitivityB = variantB.toDouble();

            return compareHelper<double>(sensitivityA, sensitivityB);
        }

        default:
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "item: unimplemented comparison, subColumn=" << subColumn;

            return CmpEqual;
        }
    }
}

TableViewColumnConfigurationWidget* ColumnPhotoProperties::getConfigurationWidget(QWidget* const parentWidget) const
{
    TableViewColumnConfiguration myConfiguration = getConfiguration();

    return (new ColumnPhotoConfigurationWidget(s, myConfiguration, parentWidget));
}

// ---------------------------------------------------------------------------------------------------------------------

ColumnPhotoConfigurationWidget::ColumnPhotoConfigurationWidget(TableViewShared* const sharedObject,
                                                               const TableViewColumnConfiguration& columnConfiguration,
                                                               QWidget* const parentWidget)
    : TableViewColumnConfigurationWidget(sharedObject, columnConfiguration, parentWidget)
{
    ColumnPhotoProperties::getSubColumnIndex<ColumnPhotoProperties>(configuration.columnId, &subColumn);

    switch (subColumn)
    {
        case ColumnPhotoProperties::SubColumnExposure:
        {
            QFormLayout* const box1    = new QFormLayout();
            selectorExposureTimeFormat = new QComboBox(this);
            selectorExposureTimeFormat->addItem(i18nc("@info: tableview", "Fraction"), QLatin1String("fraction"));
            selectorExposureTimeFormat->addItem(i18nc("@info: tableview", "Rational"), QLatin1String("rational"));
            box1->addRow(i18nc("@info: tableview", "Format"), selectorExposureTimeFormat);

            selectorExposureTimeUnit = new QComboBox(this);
            selectorExposureTimeUnit->addItem(i18nc("@info: tableview", "Seconds"),      QLatin1String("seconds"));
            selectorExposureTimeUnit->addItem(i18nc("@info: tableview", "Milliseconds"), QLatin1String("milliseconds"));
            selectorExposureTimeUnit->addItem(i18nc("@info: tableview", "Microseconds"), QLatin1String("microseconds"));
            box1->addRow(i18nc("@info: tableview", "Unit"), selectorExposureTimeUnit);

            setLayout(box1);

            const int indexF = selectorExposureTimeFormat->findData(configuration.getSetting(QLatin1String("format"), QLatin1String("fraction")));
            selectorExposureTimeFormat->setCurrentIndex((indexF >= 0) ? indexF : 0);
            const int indexU = selectorExposureTimeUnit->findData(configuration.getSetting(QLatin1String("unit"), QLatin1String("seconds")));
            selectorExposureTimeUnit->setCurrentIndex((indexU >= 0) ? indexU : 0);

            slotUpdateUI();

            connect(selectorExposureTimeFormat, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(slotUpdateUI()));
            break;
        }

        default:
        {
            break;
        }
    }
}

TableViewColumnConfiguration ColumnPhotoConfigurationWidget::getNewConfiguration()
{
    const QString formatKey = selectorExposureTimeFormat->itemData(selectorExposureTimeFormat->currentIndex()).toString();
    configuration.columnSettings.insert(QLatin1String("format"), formatKey);

    const QString unitKey   = selectorExposureTimeUnit->itemData(selectorExposureTimeUnit->currentIndex()).toString();
    configuration.columnSettings.insert(QLatin1String("unit"), unitKey);

    return configuration;
}

void ColumnPhotoProperties::setConfiguration(const TableViewColumnConfiguration& newConfiguration)
{
    configuration = newConfiguration;

    Q_EMIT signalAllDataChanged();
}

void ColumnPhotoConfigurationWidget::slotUpdateUI()
{
    if (selectorExposureTimeFormat)
    {
        const QString currentKey = selectorExposureTimeFormat->itemData(selectorExposureTimeFormat->currentIndex()).toString();
        const bool needsUnits    = (currentKey == QLatin1String("rational"));

        selectorExposureTimeUnit->setEnabled(needsUnits);
    }
}

} // namespace TableViewColumns

} // namespace Digikam

#include "moc_tableview_column_photo.cpp"
