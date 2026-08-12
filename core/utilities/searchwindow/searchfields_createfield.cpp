/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-01-20
 * Description : User interface for searches
 *
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2026      by Srirupa Datta <srirupa dot sps at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "searchfields_p.h"

namespace Digikam
{

SearchField* SearchField::createField(const QString& name, SearchFieldGroup* const parent)
{
    if      (name == QLatin1String("albumid"))
    {
        SearchFieldAlbum* const field = new SearchFieldAlbum(parent, SearchFieldAlbum::TypeAlbum);
        field->setFieldName(name);
        field->setText(i18n("Album"), i18n("Search items located in"));

        return field;
    }
    else if (name == QLatin1String("albumname"))
    {
        SearchFieldText* const field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Album"), i18n("The album name contains"));

        return field;
    }
    else if (name == QLatin1String("albumcaption"))
    {
        SearchFieldText* const field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Album"), i18n("The album caption contains"));

        return field;
    }
    else if (name == QLatin1String("albumcollection"))
    {
        SearchFieldChoice* const field            = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Album"), i18n("The album category is"));
        const ApplicationSettings* const settings = ApplicationSettings::instance();

        if (settings)
        {
            QStringList Categories = settings->getAlbumCategoryNames();
            int size               = Categories.size();
            QStringList categorychoices;

            for (int i = 0 ; i < size ; ++i)
            {
                categorychoices << Categories.at(i) << Categories.at(i);
            }

            field->setChoice(categorychoices);
        }

        return field;
    }
    else if (name == QLatin1String("tagid"))
    {
        SearchFieldAlbum* const field = new SearchFieldAlbum(parent, SearchFieldAlbum::TypeTag);
        field->setFieldName(name);
        field->setText(i18n("Tags"), i18n("Return items with tag"));

        return field;
    }
    else if (name == QLatin1String("tagname"))
    {
        SearchFieldText* const field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Tags"), i18n("A tag of the item contains"));

        return field;
    }
    else if (name == QLatin1String("nottagged"))
    {
        /**
         * @todo Merge a "Not tagged" field into TagModel together with AND/OR control
         * for checked tags and logical connections (AND and Not Tagged checked => all other tags disabled)
         */

        SearchFieldCheckBox* const field = new SearchFieldCheckBox(parent);
        field->setFieldName(name);
        field->setText(i18n("Tags"), i18n("Return items without tags"));
        field->setLabel(i18n("Not Tagged"));

        return field;
    }
    else if (name == QLatin1String("filename"))
    {
        SearchFieldText* const field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("File Name"), i18n("Return items whose file name contains"));

        return field;
    }
    else if (name == QLatin1String("modificationdate"))
    {
        SearchFieldRangeDate* const field = new SearchFieldRangeDate(parent, SearchFieldRangeDate::DateTime);
        field->setFieldName(name);
        field->setText(i18n("Modification"), i18n("Return items modified between"));
        field->setBetweenText(i18nc("'Return items modified between...and...", "and"));

        return field;
    }
    else if (name == QLatin1String("filesize"))
    {
        SearchFieldRangeDouble* const field = new SearchFieldRangeDouble(parent);
        field->setFieldName(name);
        field->setText(i18n("File Size"), i18n("Size of the file in megabytes"));
        field->setBetweenText(i18nc("Size of the file ...-...", "-"));
        field->setNumberPrefixAndSuffix(QString(), QLatin1String("MiB"));
        field->setBoundary(0, 1000000, 1, 0.5);
        field->setFactor(1024 * 1024);

        return field;
    }
    else if (name == QLatin1String("bytesize"))
    {
        SearchFieldRangeInt* const field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("File Size"), i18n("Size of the file in bytes"));
        field->setBetweenText(i18nc("Size of the file ...-...", "-"));
        field->setNumberPrefixAndSuffix(QString(), QLatin1String("B"));
        field->setBoundary(0, 1024 * 1024, 100);

        return field;
    }
    else if (name == QLatin1String("monthday"))
    {
        SearchFieldMonthDay* const field = new SearchFieldMonthDay(parent);
        field->setFieldName(name);
        field->setText(i18n("Month/Day"), i18n("Return items from a month or a day of the month"));

        return field;
    }
    else if (name == QLatin1String("labels"))
    {
        SearchFieldLabels* const field = new SearchFieldLabels(parent);
        field->setFieldName(name);
        field->setText(i18n("Labels"), i18n("Return items with labels"));

        return field;
    }
    else if (name == QLatin1String("rating"))
    {
        SearchFieldRating* const field = new SearchFieldRating(parent);
        field->setFieldName(name);
        field->setText(i18n("Rating"), i18n("Return items rated at least"));
        field->setBetweenText(i18nc("Return items rated at least...at most...", "at most"));

        return field;
    }
    else if (name == QLatin1String("creationdate"))
    {
        SearchFieldRangeDate* const field = new SearchFieldRangeDate(parent, SearchFieldRangeDate::DateTime);
        field->setFieldName(name);
        field->setText(i18n("Date"), i18n("Return items created between"));
        field->setBetweenText(i18nc("'Return items created between...and...", "and"));

        return field;
    }
    else if (name == QLatin1String("digitizationdate"))
    {
        SearchFieldRangeDate* const field = new SearchFieldRangeDate(parent, SearchFieldRangeDate::DateTime);
        field->setFieldName(name);
        field->setText(i18n("Digitization"), i18n("Return items digitized between"));
        field->setBetweenText(i18nc("'Return items digitized between...and...", "and"));

        return field;
    }
    else if (name == QLatin1String("modifiedrecent"))
    {
        SearchFieldRecentModified* const field = new SearchFieldRecentModified(parent);
        field->setFieldName(name);
        field->setText(i18n("Modified last"),
                    i18n("Return items modified within the last"));
        return field;
    }
    else if (name == QLatin1String("creationtime"))
    {
        SearchFieldRangeTime* const field = new SearchFieldRangeTime(parent);
        field->setFieldName(name);
        field->setText(i18n("Time"), i18n("Return items with created time between"));
        field->setBetweenText(i18nc("'Return items with created time between...and...", "and"));

        return field;
    }
    else if (name == QLatin1String("orientation"))
    {
        // choice

        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Exif Orientation"), i18n("Find items with orientation flag"));
        QMap<int, QString> map = DMetadata::possibleValuesForEnumField(MetadataInfo::Orientation);
        field->setChoice(map);

        return field;
    }
    else if (name == QLatin1String("dimension"))
    {
        // "width", "height", "pixels"
    }
    else if (name == QLatin1String("width"))
    {
        SearchFieldRangeInt* const field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Width"), i18n("Find items with a width between"));
        field->setBetweenText(i18nc("Find items with a width between...and...", "and"));
        field->setNumberPrefixAndSuffix(QString(), i18nc("Pixels", "px"));
        field->setBoundary(1, 1000000, 250);
        field->setSuggestedValues(QList<int>()
                                  << 50 << 100 << 200 << 300 << 400 << 500 << 600 << 700 << 800 << 900
                                  << 1000 << 1250 << 1500 << 1750 << 2000 << 3000 << 4000
                                  << 5000 << 6000 << 7000 << 8000 << 9000 << 10000
                                 );
        field->setSuggestedInitialValue(1000);
        field->setSingleSteps(50, 1000);

        return field;
    }
    else if (name == QLatin1String("height"))
    {
        SearchFieldRangeInt* const field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Height"), i18n("Find items with a height between"));
        field->setBetweenText(i18nc("Find items with a height between...and...", "and"));
        field->setNumberPrefixAndSuffix(QString(), i18nc("Pixels", "px"));
        field->setBoundary(1, 1000000, 250);
        field->setSuggestedValues(QList<int>()
                                  << 50 << 100 << 200 << 300 << 400 << 500 << 600 << 700 << 800 << 900
                                  << 1000 << 1250 << 1500 << 1750 << 2000 << 3000 << 4000
                                  << 5000 << 6000 << 7000 << 8000 << 9000 << 10000
                                 );
        field->setSuggestedInitialValue(1000);
        field->setSingleSteps(50, 1000);

        return field;
    }
    else if (name == QLatin1String("pageorientation"))
    {
        SearchFieldPageOrientation* const field = new SearchFieldPageOrientation(parent);
        field->setFieldName(name);
        field->setText(i18n("Orientation"), i18nc("Find items with any orientation / landscape / portrait orientation...",
                                                  "Find items with"));
        return field;
    }
    else if (name == QLatin1String("format"))
    {
        // choice

        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("File Format"), i18n("Return items with the file format"));
        QStringList formats;

        {
            const auto fmts = CoreDbAccess().db()->getFormatStatistics(DatabaseItem::Image).keys();

            for (const QString& fmt : fmts)
            {
                formats << fmt << i18nc("@label: file format", "%1 [Image]", fmt);
            }
        }

        {
            const auto fmts = CoreDbAccess().db()->getFormatStatistics(DatabaseItem::Video).keys();

            for (const QString& fmt : fmts)
            {
                formats << fmt << i18nc("@label: file format", "%1 [Video]", fmt);
            }
        }

        {
            const auto fmts = CoreDbAccess().db()->getFormatStatistics(DatabaseItem::Audio).keys();

            for (const QString& fmt : fmts)
            {
                formats << fmt << i18nc("@label: file format", "%1 [Audio]", fmt);
            }
        }
/*
        FIXME: This can report 2 times JPG : one as image, one as other. Where is the problem ?

        {
            const auto fmts = CoreDbAccess().db()->getFormatStatistics(DatabaseItem::Other).keys();

            for (const QString& fmt : fmts)
            {
                formats << fmt << i18n("%1 [Other]", fmt);
            }
        }
*/
        formats.sort();

        qCDebug(DIGIKAM_GENERAL_LOG) << formats;

        field->setChoice(formats);

        return field;
    }
    else if (name == QLatin1String("colordepth"))
    {
        // choice

        SearchFieldColorDepth* const field = new SearchFieldColorDepth(parent);
        field->setFieldName(name);
        field->setText(i18n("Color Depth"), i18nc("Find items with any color depth / 8 bits per channel...", "Find items with"));

        return field;
    }
    else if (name == QLatin1String("colormodel"))
    {
        // choice

        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Color Model"), i18n("Find items with the color model"));
        QMap<int, QString> map;

        // Images

        map.insert(DImg::COLORMODELUNKNOWN,          i18nc("@label: color model", "%1 [Image]", DImg::colorModelToString(DImg::COLORMODELUNKNOWN)));
        map.insert(DImg::RGB,                        i18nc("@label: color model", "%1 [Image]", DImg::colorModelToString(DImg::RGB)));
        map.insert(DImg::GRAYSCALE,                  i18nc("@label: color model", "%1 [Image]", DImg::colorModelToString(DImg::GRAYSCALE)));
        map.insert(DImg::MONOCHROME,                 i18nc("@label: color model", "%1 [Image]", DImg::colorModelToString(DImg::MONOCHROME)));
        map.insert(DImg::INDEXED,                    i18nc("@label: color model", "%1 [Image]", DImg::colorModelToString(DImg::INDEXED)));
        map.insert(DImg::YCBCR,                      i18nc("@label: color model", "%1 [Image]", DImg::colorModelToString(DImg::YCBCR)));
        map.insert(DImg::CMYK,                       i18nc("@label: color model", "%1 [Image]", DImg::colorModelToString(DImg::CMYK)));
        map.insert(DImg::CIELAB,                     i18nc("@label: color model", "%1 [Image]", DImg::colorModelToString(DImg::CIELAB)));
        map.insert(DImg::COLORMODELRAW,              i18nc("@label: color model", "%1 [Image]", DImg::colorModelToString(DImg::COLORMODELRAW)));

        // Video

        map.insert(DMetadata::VIDEOCOLORMODEL_SRGB,  i18nc("@label: color model", "%1 [Video]", DMetadata::videoColorModelToString(DMetadata::VIDEOCOLORMODEL_SRGB)));
        map.insert(DMetadata::VIDEOCOLORMODEL_BT709, i18nc("@label: color model", "%1 [Video]", DMetadata::videoColorModelToString(DMetadata::VIDEOCOLORMODEL_BT709)));
        map.insert(DMetadata::VIDEOCOLORMODEL_BT601, i18nc("@label: color model", "%1 [Video]", DMetadata::videoColorModelToString(DMetadata::VIDEOCOLORMODEL_BT601)));
        map.insert(DMetadata::VIDEOCOLORMODEL_OTHER, i18nc("@label: color model", "%1 [Video]", DMetadata::videoColorModelToString(DMetadata::VIDEOCOLORMODEL_OTHER)));
        field->setChoice(map);

        return field;
    }
    else if (name == QLatin1String("make"))
    {
        // choice

        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Camera"), i18n("The make of the camera"));

        QStringList make = CoreDbAccess().db()->getListFromImageMetadata(DatabaseFields::Make);
        QString wildcard = QLatin1String("*%1*");
        QMap<QString, QString> makeMap;

        for (int i = 0 ; i < make.count() ; ++i)
        {
            QString shortName = make[i];
            ItemPropertiesTab::shortenedMakeInfo(shortName);

            if (!shortName.isEmpty())
            {
                makeMap.insert(shortName, wildcard.arg(shortName));
            }
        }

        make.clear();
        
        QMap<QString, QString>::const_iterator it;

        for (it = makeMap.constBegin() ; it != makeMap.constEnd() ; ++it)
        {
            make << it.value() << it.key();
        }

        field->setChoice(make);

        return field;
    }
    else if (name == QLatin1String("model"))
    {
        // choice

        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Camera"), i18n("The model of the camera"));

        QStringList model = CoreDbAccess().db()->getListFromImageMetadata(DatabaseFields::Model);
        QString wildcard  = QLatin1String("*%1*");
        QMap<QString, QString> modelMap;

        for (int i = 0 ; i < model.count() ; ++i)
        {
            QString shortName = model[i];
            ItemPropertiesTab::shortenedModelInfo(shortName);

            if (!shortName.isEmpty())
            {
                modelMap.insert(shortName, wildcard.arg(model[i]));
            }
        }

        model.clear();

        QMap<QString, QString>::const_iterator it;

        for (it = modelMap.constBegin() ; it != modelMap.constEnd() ; ++it)
        {
            model << it.value() << it.key();
        }

        field->setChoice(model);

        return field;
    }
    else if (name == QLatin1String("lenses"))
    {
        // choice

        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Lens"), i18n("The type of the lens"));
        QStringList lens = CoreDbAccess().db()->getListFromImageMetadata(DatabaseFields::Lens);
        lens += lens;
        lens.sort();
        field->setChoice(lens);

        return field;
    }
    else if (name == QLatin1String("nocamera"))
    {
        // choice

        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Camera"), i18n("Return items without a camera ..."));

        QStringList noCamera;
        noCamera << QLatin1String("make")  << i18nc("Camera", "Make")
                 << QLatin1String("model") << i18nc("Camera", "Model")
                 << QLatin1String("lens")  << i18nc("Camera", "Lens Type");

        field->setChoice(noCamera);

        return field;
    }
    else if (name == QLatin1String("aperture"))
    {
        // double

        SearchFieldRangeDouble* const field = new SearchFieldRangeDouble(parent);
        field->setFieldName(name);
        field->setText(i18n("Aperture"), i18n("Lens aperture as f-number"));
        field->setBetweenText(i18nc("Lens aperture as f-number ...-...", "-"));
        field->setNoValueText(QLatin1String("f/#"));
        field->setNumberPrefixAndSuffix(QLatin1String("f/"), QString());
        field->setBoundary(0.3, 65536, 1, 0.1);
        field->setSuggestedValues(QList<double>()
                                  << 0.5 << 0.7 << 1.0 << 1.4 << 2 << 2.8 << 4 << 5.6
                                  << 8 << 11 << 16 << 22 << 32 << 45 << 64 << 90 << 128
                                 );
        field->setSuggestedInitialValue(1.0);
        field->setSingleSteps(0.1, 10);

        return field;
    }
    else if (name == QLatin1String("focallength"))
    {
        // double

        SearchFieldRangeInt* const field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Focal length"), i18n("Focal length of the lens"));
        field->setBetweenText(i18nc("Focal length of the lens ...-...", "-"));
        field->setNumberPrefixAndSuffix(QString(), QLatin1String("mm"));
        field->setBoundary(0, 200000, 10);
        field->setSuggestedValues(QList<int>()
                                  << 10 << 15 << 20 << 25 << 30 << 40 << 50 << 60 << 70 << 80 << 90
                                  << 100 << 150 << 200 << 250 << 300 << 400 << 500 << 750 << 1000
                                 );
        field->setSuggestedInitialValue(30);
        field->setSingleSteps(2, 500);

        return field;
    }
    else if (name == QLatin1String("focallength35"))
    {
        // double

        SearchFieldRangeInt* const field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Focal length"), i18n("35mm equivalent focal length"));
        field->setBetweenText(i18nc("35mm equivalent focal length ...-...", "-"));
        field->setNumberPrefixAndSuffix(QString(), QLatin1String("mm"));
        field->setBoundary(0, 200000, 10);
        field->setSuggestedValues(QList<int>()
                                  << 8 << 10 << 15 << 16 << 20 << 28 << 30 << 40 << 50 << 60 << 70 << 80
                                  << 90 << 100 << 150 << 200 << 250 << 300 << 400 << 500 << 750 << 1000
                                 );
        field->setSuggestedInitialValue(28);
        field->setSingleSteps(2, 500);

        return field;
    }
    else if (name == QLatin1String("exposuretime"))
    {
        // double

        SearchFieldRangeInt* const field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Exposure"), i18n("Exposure time"));
        field->setBetweenText(i18nc("Exposure time ...-...", "-"));
        field->setNumberPrefixAndSuffix(QString(), QLatin1String("s"));
        field->enableFractionMagic(QLatin1String("1/"));    // it's 1/250, not 250 as in the spin box
        field->setBoundary(86400, -1024000, 10);            // negative is 1/
        field->setSuggestedValues(QList<int>()
                                  << 30 << 15 << 8 << 4 << 2 << 1 << -2 << -4 << -8 << -15
                                  << -30 << -50 << -60 << -100 << -125 << -150 << -200
                                  << -250 << -500 << -750 << -1000 << -2000 << -4000 << -8000 << -16000
                                 );
        field->setSuggestedInitialValue(-200);
        field->setSingleSteps(2000, 5);

        return field;
    }
    else if (name == QLatin1String("exposureprogram"))
    {
        // choice

        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Exposure"), i18n("Automatic exposure program"));
        QMap<int, QString> map = DMetadata::possibleValuesForEnumField(MetadataInfo::ExposureProgram);
        field->setChoice(map);

        return field;
    }
    else if (name == QLatin1String("exposuremode"))
    {
        // choice

        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Exposure"), i18n("Automatic or manual exposure"));
        QMap<int, QString> map = DMetadata::possibleValuesForEnumField(MetadataInfo::ExposureMode);
        field->setChoice(map);
        return field;
    }
    else if (name == QLatin1String("sensitivity"))
    {
        // int

        SearchFieldRangeInt* const field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Sensitivity"), i18n("ISO film speed (linear scale, ASA)"));
        field->setBetweenText(i18nc("ISO film speed (linear scale, ASA) ...-...", "-"));
        field->setBoundary(0, 2000000, 50);
        field->setSuggestedValues(QList<int>()
                                  << 6 << 8 << 10 << 12 << 16 << 20 << 25 << 32 << 40 << 50 << 64
                                  << 80 << 100 << 125 << 160 << 200 << 250 << 320 << 400 << 500
                                  << 640 << 800 << 1000 << 1250 << 1600 << 2000 << 2500 << 3200
                                  << 4000 << 5000 << 6400
                                 );
        field->setSuggestedInitialValue(200);
        field->setSingleSteps(1, 400);

        return field;
    }
    else if (name == QLatin1String("flashmode"))
    {
        // choice

        /**
         * @todo This is a bitmask, and gives some more information
         */

        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18nc("@label: flash is on or off", "Flash"), i18nc("@label: flash mode used", "Flash mode"));
        QMap<int, QString> map = DMetadata::possibleValuesForEnumField(MetadataInfo::FlashMode);
        field->setChoice(map);

        return field;
    }
    else if (name == QLatin1String("whitebalance"))
    {
        // choice

        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("White Balance"), i18n("Automatic or manual white balance"));
        QMap<int, QString> map = DMetadata::possibleValuesForEnumField(MetadataInfo::WhiteBalance);
        field->setChoice(map);

        return field;
    }
    else if (name == QLatin1String("whitebalancecolortemperature"))
    {
        // int
        SearchFieldRangeInt* const field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("White balance"), i18n("Color temperature used for white balance"));
        field->setBetweenText(i18nc("Color temperature used for white balance ...-...", "-"));
        field->setNumberPrefixAndSuffix(QString(), QLatin1String("K"));
        field->setBoundary(1, 100000, 100);

        return field;
    }
    else if (name == QLatin1String("meteringmode"))
    {
        // choice

        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Metering Mode"), i18n("Method to determine the exposure"));
        QMap<int, QString> map = DMetadata::possibleValuesForEnumField(MetadataInfo::MeteringMode);
        field->setChoice(map);

        return field;
    }
    else if (name == QLatin1String("subjectdistance"))
    {
        // double

        SearchFieldRangeDouble* const field = new SearchFieldRangeDouble(parent);
        field->setFieldName(name);
        field->setText(i18n("Subject Distance"), i18n("Distance of the subject from the lens"));
        field->setBetweenText(i18nc("Distance of the subject from the lens ...-...", "-"));
        field->setNumberPrefixAndSuffix(QString(), QLatin1String("m"));
        field->setBoundary(0, 50000, 1, 0.1);

        return field;
    }
    else if (name == QLatin1String("subjectdistancecategory"))
    {
        // choice

        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Subject Distance"), i18n("Macro, close or distant view"));
        QMap<int, QString> map = DMetadata::possibleValuesForEnumField(MetadataInfo::SubjectDistanceCategory);
        field->setChoice(map);

        return field;
    }
    else if (name == QLatin1String("nocapturesettings"))
    {
        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Capture Settings"), i18n("Return items without selected capture settings ..."));

        QStringList noCaptureSettings;
        noCaptureSettings
        << QLatin1String("aperture")   << i18n("Aperture")
        << QLatin1String("focallength")<< i18n("Focal length")
        << QLatin1String("focallength35") << i18n("35mm equivalent focal length")
        << QLatin1String("exposuretime")  << i18n("Exposure time")
        << QLatin1String("exposureprogram") << i18n("Automatic exposure program")
        << QLatin1String("exposuremode")    << i18n("Automatic or manual exposure")
        << QLatin1String("sensitivity")     << i18n("ISO film speed")
        << QLatin1String("meteringmode")    << i18n("Metering mode")
        << QLatin1String("flashmode")       << i18nc("@label: flash mode used", "Flash mode")
        << QLatin1String("whitebalance")    << i18n("White balance")
        << QLatin1String("whitebalancecolortemperature") << i18n("White balance color temperature")
        << QLatin1String("subjectdistance") << i18n("Subject Distance")
        << QLatin1String("subjectdistancecategory") << i18n("Subject Distance Category");


        field->setChoice(noCaptureSettings);

        return field;
    }


    else if (name == QLatin1String("latitude"))
    {
    }
    else if (name == QLatin1String("longitude"))
    {
    }
    else if (name == QLatin1String("altitude"))
    {
        SearchFieldRangeDouble* const field = new SearchFieldRangeDouble(parent);
        field->setFieldName(name);
        field->setText(i18n("GPS"), i18n("Altitude range"));
        field->setBetweenText(i18nc("Altitude range ...-...", "-"));
        field->setNumberPrefixAndSuffix(QString(), QLatin1String("m"));
        field->setBoundary(0, 10000, 4, 1);

        return field;
    }
    else if (name == QLatin1String("positionorientation"))
    {
    }
    else if (name == QLatin1String("positiontilt"))
    {
    }
    else if (name == QLatin1String("positionroll"))
    {
    }
    else if (name == QLatin1String("positiondescription"))
    {
    }
    else if (name == QLatin1String("nogps"))
    {
        SearchFieldCheckBox* const field = new SearchFieldCheckBox(parent);
        field->setFieldName(name);
        field->setText(i18n("GPS"), i18n("Item has no GPS info"));
        field->setLabel(i18n("Not Geo-located"));

        return field;
    }
    else if (name == QLatin1String("country"))
    {
        // choice

        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Country"), i18n("Country shown in the item"));

        QStringList countries = CoreDbAccess().db()->getAllImagePropertiesByName(name);
        countries            += countries;
        countries.sort();

        field->setChoice(countries);

        return field;
    }
    else if (name == QLatin1String("provinceState"))
    {
        // choice

        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Province"), i18n("Province shown in the item"));

        QStringList provinces = CoreDbAccess().db()->getAllImagePropertiesByName(name);
        provinces            += provinces;
        provinces.sort();

        field->setChoice(provinces);

        return field;
    }
    else if (name == QLatin1String("city"))
    {
        // choice

        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("City"), i18n("City shown in the item"));

        QStringList cities = CoreDbAccess().db()->getAllImagePropertiesByName(name);
        cities            += cities;
        cities.sort();

        field->setChoice(cities);

        return field;
    }
    else if (name == QLatin1String("location"))
    {
        // choice

        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Sublocation"), i18n("Sublocation shown in the item"));

        QStringList locations = CoreDbAccess().db()->getAllImagePropertiesByName(name);
        locations            += locations;
        locations.sort();

        field->setChoice(locations);

        return field;
    }
    else if (name == QLatin1String("noplaces"))
    {
        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);

        field->setText(i18n("Place"), i18n("Return items without selected place information ..."));

        QStringList noPlaceFields;
        noPlaceFields
            << QLatin1String("country")       << i18n("Country")
            << QLatin1String("provinceState") << i18n("State/Province")
            << QLatin1String("city")          << i18n("City")
            << QLatin1String("location")      << i18n("Sublocation");

        field->setChoice(noPlaceFields);

        return field;
    }
    else if (name == QLatin1String("creator"))
    {
        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18nc("@label: item creator", "Creator"),
                    i18n("Return items created by"));

        QStringList creators = CoreDbAccess().db()->getAllImageCopyrightValues(QLatin1String("creator"));
        creators += creators;
        creators.sort();
        field->setChoice(creators);

        return field;
    }
    else if (name == QLatin1String("comment"))
    {
        SearchFieldText* const field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18nc("@label: item comment", "Caption"), i18n("Return items whose comment contains"));

        return field;
    }
    else if (name == QLatin1String("commentauthor"))
    {
        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Author"),
                    i18n("Return items commented by"));

        QStringList authors = CoreDbAccess().db()->getAllCommentAuthors(DatabaseComment::Comment);
        authors += authors;
        authors.sort();
        field->setChoice(authors);

        return field;
    }
    else if (name == QLatin1String("headline"))
    {
        SearchFieldText* const field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Headline"), i18n("Return items with the IPTC headline"));

        return field;
    }
    else if (name == QLatin1String("title"))
    {
        SearchFieldText* const field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18nc("@label: item title", "Title"), i18n("Return items with the IPTC title"));

        return field;
    }
    else if (name == QLatin1String("keyword"))
    {
        SearchFieldText* const field = new SearchFieldKeyword(parent);
        field->setFieldName(name);
        field->setText(QString(), i18n("Find items that have associated all these words:"));

        return field;
    }
    else if (name == QLatin1String("emptytext"))
    {
        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Empty Text"), i18n("Return items without text in"));
        QStringList comments;
        comments << QLatin1String("creator")  << i18nc("@label: search items without creator property",  "Creator");
        comments << QLatin1String("comment")  << i18nc("@label: search items without caption property",  "Caption");
        comments << QLatin1String("author")   << i18nc("@label: search items without author property",   "Author");
        comments << QLatin1String("headline") << i18nc("@label: search items without headline property", "Headline");
        comments << QLatin1String("title")    << i18nc("@label: search items without title property",    "Title");

        field->setChoice(comments);

        return field;
    }
    else if (name == QLatin1String("aspectratioimg"))
    {
        SearchFieldText* const field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Aspect Ratio"), i18n("Return items with the aspect ratio"));

        return field;
    }
    else if (name == QLatin1String("pixelsize"))
    {
        SearchFieldRangeInt* const field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Pixel Size"), i18n("Value of (Width * Height) between"));
        field->setBetweenText(i18nc("Value of (Width * Height) between...and...", "and"));
        field->setNumberPrefixAndSuffix(QString(), QLatin1String("px"));
        field->setBoundary(1, 2000000000, 100);

        return field;
    }
    else if (name == QLatin1String("videoaspectratio"))
    {
        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Aspect Ratio"), i18n("Return video with the frame aspect ratio"));
        QStringList ratio;
        ratio << QLatin1String("4:3")  << QLatin1String("4:3");
        ratio << QLatin1String("3:2")  << QLatin1String("3:2");
        ratio << QLatin1String("16:9") << QLatin1String("16:9");
        ratio << QLatin1String("2:1")  << QLatin1String("2:1");

        // TODO: add more possible aspect ratio

        field->setChoice(ratio);

        return field;
    }
    else if (name == QLatin1String("videoduration"))
    {
        SearchFieldRangeInt* const field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Duration"), i18n("Length of the video"));
        field->setBetweenText(i18nc("Find video with a length between...and...", "and"));
        field->setNumberPrefixAndSuffix(QString(), i18nc("Seconds", "s"));
        field->setBoundary(1, 10000, 100);
        field->setSuggestedValues(QList<int>()
                                  << 10 << 30 << 60 << 90 << 120 << 240 << 360 << 500 << 1000 << 2000
                                  << 3000 << 4000 << 5000 << 6000 << 7000 << 8000 << 9000 << 10000
                                  // TODO : adjust default values
                                 );
        field->setSuggestedInitialValue(10);
        field->setSingleSteps(10, 100);

        return field;
    }
    else if (name == QLatin1String("videoframerate"))
    {
        SearchFieldRangeInt* const field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Frame Rate"), i18n("Return video with the frame rate"));
        field->setBetweenText(i18nc("Find video with frame rate between...and...", "and"));
        field->setNumberPrefixAndSuffix(QString(), i18nc("Frames per Second", "fps"));
        field->setBoundary(10, 600, 5);
        field->setSuggestedValues(QList<int>()
                                  << 10 << 15 << 20 << 25 << 30 << 35 << 40 << 45 << 55 << 60 << 120
                                  << 180 << 240 << 300 << 360 << 420 << 480 << 540 << 600
                                  // TODO : adjust default values
                                 );
        field->setSuggestedInitialValue(10);
        field->setSingleSteps(5, 60);

        return field;
    }
    else if (name == QLatin1String("videocodec"))
    {
        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Codec"), i18n("Return video codec"));
        QStringList codec;

        // List of most common video codecs supported by FFMpeg (see "ffpmpeg -codecs" for details)
        //
        //       FFMpeg codec name                 FFMpeg codec description

        codec << QLatin1String("8bps")          << QLatin1String("QuickTime 8BPS video");
        codec << QLatin1String("amv")           << QLatin1String("AMV Video");
        codec << QLatin1String("avs")           << QLatin1String("AVS (Audio Video Standard) video");
        codec << QLatin1String("cavs")          << QLatin1String("Chinese AVS (Audio Video Standard) (AVS1-P2, JiZhun profile)");
        codec << QLatin1String("cinepak")       << QLatin1String("Cinepak");
        codec << QLatin1String("dirac")         << QLatin1String("Dirac");
        codec << QLatin1String("flv1")          << QLatin1String("FLV / Sorenson Spark / Sorenson H.263 (Flash Video)");
        codec << QLatin1String("h261")          << QLatin1String("H.261");
        codec << QLatin1String("h263")          << QLatin1String("H.263 / H.263-1996, H.263+ / H.263-1998 / H.263 version 2");
        codec << QLatin1String("h263i")         << QLatin1String("Intel H.263");
        codec << QLatin1String("h263p")         << QLatin1String("H.263+ / H.263-1998 / H.263 version 2");
        codec << QLatin1String("h264")          << QLatin1String("H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10");
        codec << QLatin1String("hevc")          << QLatin1String("H.265 / HEVC (High Efficiency Video Coding)");
        codec << QLatin1String("jpeg2000")      << QLatin1String("JPEG 2000");
        codec << QLatin1String("mjpeg")         << QLatin1String("Motion JPEG");
        codec << QLatin1String("mjpegb")        << QLatin1String("Apple MJPEG-B");
        codec << QLatin1String("mpeg1video")    << QLatin1String("MPEG-1 video");
        codec << QLatin1String("mpeg2video")    << QLatin1String("MPEG-2 video");
        codec << QLatin1String("mpeg4")         << QLatin1String("MPEG-4 part 2");
        codec << QLatin1String("msmpeg4v1")     << QLatin1String("MPEG-4 part 2 Microsoft variant version 1");
        codec << QLatin1String("msmpeg4v2")     << QLatin1String("MPEG-4 part 2 Microsoft variant version 2");
        codec << QLatin1String("msmpeg4v3")     << QLatin1String("MPEG-4 part 2 Microsoft variant version 3");
        codec << QLatin1String("msvideo1")      << QLatin1String("Microsoft Video 1");
        codec << QLatin1String("msrle")         << QLatin1String("Microsoft RLE");
        codec << QLatin1String("mvc1")          << QLatin1String("Silicon Graphics Motion Video Compressor 1");
        codec << QLatin1String("mvc2")          << QLatin1String("Silicon Graphics Motion Video Compressor 2");
        codec << QLatin1String("qtrle")         << QLatin1String("QuickTime Animation (RLE) video");
        codec << QLatin1String("rawvideo")      << QLatin1String("Raw video");
        codec << QLatin1String("rpza")          << QLatin1String("QuickTime video (RPZA)");
        codec << QLatin1String("rv10")          << QLatin1String("RealVideo 1.0");
        codec << QLatin1String("rv20")          << QLatin1String("RealVideo 2.0");
        codec << QLatin1String("rv30")          << QLatin1String("RealVideo 3.0");
        codec << QLatin1String("rv40")          << QLatin1String("RealVideo 4.0");
        codec << QLatin1String("smc")           << QLatin1String("QuickTime Graphics (SMC)");
        codec << QLatin1String("snow")          << QLatin1String("Snow");
        codec << QLatin1String("svq1")          << QLatin1String("Sorenson Vector Quantizer 1 / Sorenson Video 1 / SVQ1");
        codec << QLatin1String("svq3")          << QLatin1String("Sorenson Vector Quantizer 3 / Sorenson Video 3 / SVQ3");
        codec << QLatin1String("theora")        << QLatin1String("Theora");
        codec << QLatin1String("vc1")           << QLatin1String("SMPTE VC-1");
        codec << QLatin1String("vc1image")      << QLatin1String("Windows Media Video 9 Image v2");
        codec << QLatin1String("vp3")           << QLatin1String("On2 VP3");
        codec << QLatin1String("vp5")           << QLatin1String("On2 VP5");
        codec << QLatin1String("vp6")           << QLatin1String("On2 VP6");
        codec << QLatin1String("vp6a")          << QLatin1String("On2 VP6 (Flash version, with alpha channel)");
        codec << QLatin1String("vp6f")          << QLatin1String("On2 VP6 (Flash version)");
        codec << QLatin1String("vp7")           << QLatin1String("On2 VP7");
        codec << QLatin1String("vp8")           << QLatin1String("On2 VP8");
        codec << QLatin1String("vp9")           << QLatin1String("Google VP9");
        codec << QLatin1String("wmv1")          << QLatin1String("Windows Media Video 7");
        codec << QLatin1String("wmv2")          << QLatin1String("Windows Media Video 8");
        codec << QLatin1String("wmv3")          << QLatin1String("Windows Media Video 9");
        codec << QLatin1String("wmv3image")     << QLatin1String("Windows Media Video 9 Image");

        // TODO: add more possible codec

        field->setChoice(codec);

        return field;
    }
    else if (name == QLatin1String("videoaudiobitrate"))
    {
        SearchFieldRangeInt* const field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Audio Bit Rate"), i18n("Return Audio Bits Rate"));
        field->setBetweenText(i18nc("Find files with audio bit rate between...and...", "and"));
        field->setNumberPrefixAndSuffix(QString(), i18nc("Bits per Second", "bps"));
        field->setBoundary(1000, 100000, 1000);
        field->setSuggestedValues(QList<int>()
                                  << 1000 << 4000 << 8000 << 12000 << 16000 << 20000 << 30000 << 40000 << 50000
                                  << 60000 << 700000 << 800000 << 900000 << 100000
                                  // TODO : adjust default values
                                 );
        field->setSuggestedInitialValue(1000);
        field->setSingleSteps(1000, 1000);

        return field;
    }
    else if (name == QLatin1String("videoaudiochanneltype"))
    {
        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Audio Channel Type"), i18n("Return Audio Channel Type"));
        QStringList type;
        type << QLatin1String("Mono")       << i18nc("@label: audio channel type", "Mono");
        type << QLatin1String("Stereo")     << i18nc("@label: audio channel type", "Stereo");
        type << QLatin1String("5.1")        << i18nc("@label: audio channel type", "5.1 Surround Sound");
        type << QLatin1String("7.1")        << i18nc("@label: audio channel type", "7.1 Surround Sound");
        type << QLatin1String("16 Channel") << i18nc("@label: audio channel type", "16 Channels Sequence");
        type << QLatin1String("Other")      << i18nc("@label: audio channel type", "Other Channel Type");

        // TODO: add more possible audio channel type

        field->setChoice(type);

        return field;
    }
    else if (name == QLatin1String("videoaudioCodec"))
    {
        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Audio Codec"), i18n("Return Audio Codec"));
        QStringList type;

        // List of most common audio codecs supported by FFMpeg (see "ffpmpeg -codecs" for details)
        //
        //      FFMpeg codec name                      FFMpeg codec description

        type << QLatin1String("aac")                << QLatin1String("AAC (Advanced Audio Coding)");
        type << QLatin1String("aac_latm")           << QLatin1String("AAC LATM (Advanced Audio Coding LATM syntax)");
        type << QLatin1String("ac3")                << QLatin1String("ATSC A/52A (AC-3)");
        type << QLatin1String("adpcm_g722")         << QLatin1String("G.722 ADPCM");
        type << QLatin1String("adpcm_g726")         << QLatin1String("G.726 ADPCM");
        type << QLatin1String("adpcm_g726le")       << QLatin1String("G.726 ADPCM little-endian");
        type << QLatin1String("adpcm_ima_wav")      << QLatin1String("ADPCM IMA WAV");
        type << QLatin1String("adpcm_ima_qt")       << QLatin1String("ADPCM IMA QuickTime");
        type << QLatin1String("adpcm_swf")          << QLatin1String("ADPCM Shockwave Flash");
        type << QLatin1String("alac")               << QLatin1String("ALAC (Apple Lossless Audio Codec)");
        type << QLatin1String("amr_nb")             << QLatin1String("AMR-NB (Adaptive Multi-Rate NarrowBand)");
        type << QLatin1String("amr_wb")             << QLatin1String("AMR-WB (Adaptive Multi-Rate WideBand)");
        type << QLatin1String("ape")                << QLatin1String("Monkey's Audio");
        type << QLatin1String("atrac1")             << QLatin1String("ATRAC1 (Adaptive TRansform Acoustic Coding)");
        type << QLatin1String("atrac3")             << QLatin1String("ATRAC3 (Adaptive TRansform Acoustic Coding 3)");
        type << QLatin1String("atrac3al")           << QLatin1String("ATRAC3 AL (Adaptive TRansform Acoustic Coding 3 Advanced Lossless)");
        type << QLatin1String("atrac3p")            << QLatin1String("ATRAC3+ (Adaptive TRansform Acoustic Coding 3+)");
        type << QLatin1String("atrac3pal")          << QLatin1String("ATRAC3+ AL (Adaptive TRansform Acoustic Coding 3+ Advanced Lossless)");
        type << QLatin1String("celt")               << QLatin1String("Constrained Energy Lapped Transform (CELT)");
        type << QLatin1String("cook")               << QLatin1String("Cook / Cooker / Gecko (RealAudio G2)");
        type << QLatin1String("dts")                << QLatin1String("DCA (DTS Coherent Acoustics)");
        type << QLatin1String("eac3")               << QLatin1String("ATSC A/52B (AC-3, E-AC-3)");
        type << QLatin1String("flac")               << QLatin1String("FLAC (Free Lossless Audio Codec)");
        type << QLatin1String("g723_1")             << QLatin1String("G.723.1");
        type << QLatin1String("g729")               << QLatin1String("G.729");
        type << QLatin1String("mp1")                << QLatin1String("MP1 (MPEG audio layer 1)");
        type << QLatin1String("mp2")                << QLatin1String("MP2 (MPEG audio layer 2)");
        type << QLatin1String("mp3")                << QLatin1String("MP3 (MPEG audio layer 3)");
        type << QLatin1String("mp3adu")             << QLatin1String("ADU (Application Data Unit) MP3 (MPEG audio layer 3)");
        type << QLatin1String("mp3on4")             << QLatin1String("MP3 on MP4");
        type << QLatin1String("mp4als")             << QLatin1String("MPEG-4 Audio Lossless Coding (ALS)");     // codespell:ignore als
        type << QLatin1String("musepack7")          << QLatin1String("Musepack SV7");
        type << QLatin1String("musepack8")          << QLatin1String("Musepack SV8");
        type << QLatin1String("nellymoser")         << QLatin1String("Nellymoser Asao");
        type << QLatin1String("opus")               << QLatin1String("Opus (Opus Interactive Audio Codec)");
        type << QLatin1String("pcm_alaw")           << QLatin1String("PCM A-law / G.711 A-law");
        type << QLatin1String("pcm_bluray")         << QLatin1String("PCM signed 16|20|24-bit big-endian for Blu-ray media");
        type << QLatin1String("pcm_dvd")            << QLatin1String("PCM signed 20|24-bit big-endian");
        type << QLatin1String("pcm_f16le")          << QLatin1String("PCM 16.8 floating point little-endian");
        type << QLatin1String("pcm_f24le")          << QLatin1String("PCM 24.0 floating point little-endian");
        type << QLatin1String("pcm_f32be")          << QLatin1String("PCM 32-bit floating point big-endian");
        type << QLatin1String("pcm_f32le")          << QLatin1String("PCM 32-bit floating point little-endian");
        type << QLatin1String("pcm_f64be")          << QLatin1String("PCM 64-bit floating point big-endian");
        type << QLatin1String("pcm_f64le")          << QLatin1String("PCM 64-bit floating point little-endian");
        type << QLatin1String("pcm_lxf")            << QLatin1String("PCM signed 20-bit little-endian planar");
        type << QLatin1String("pcm_mulaw")          << QLatin1String("PCM mu-law / G.711 mu-law");
        type << QLatin1String("pcm_s16be")          << QLatin1String("PCM signed 16-bit big-endian");
        type << QLatin1String("pcm_s16be_planar")   << QLatin1String("PCM signed 16-bit big-endian planar");
        type << QLatin1String("pcm_s16le")          << QLatin1String("PCM signed 16-bit little-endian");
        type << QLatin1String("pcm_s16le_planar")   << QLatin1String("PCM signed 16-bit little-endian planar");
        type << QLatin1String("pcm_s24be")          << QLatin1String("PCM signed 24-bit big-endian");
        type << QLatin1String("pcm_s24daud")        << QLatin1String("PCM D-Cinema audio signed 24-bit");
        type << QLatin1String("pcm_s24le")          << QLatin1String("PCM signed 24-bit little-endian");
        type << QLatin1String("pcm_s24le_planar")   << QLatin1String("PCM signed 24-bit little-endian planar");
        type << QLatin1String("pcm_s32be")          << QLatin1String("PCM signed 32-bit big-endian");
        type << QLatin1String("pcm_s32le")          << QLatin1String("PCM signed 32-bit little-endian");
        type << QLatin1String("pcm_s32le_planar")   << QLatin1String("PCM signed 32-bit little-endian planar");
        type << QLatin1String("pcm_s64be")          << QLatin1String("PCM signed 64-bit big-endian");
        type << QLatin1String("pcm_s64le")          << QLatin1String("PCM signed 64-bit little-endian");
        type << QLatin1String("pcm_s8")             << QLatin1String("PCM signed 8-bit");
        type << QLatin1String("pcm_s8_planar")      << QLatin1String("PCM signed 8-bit planar");
        type << QLatin1String("pcm_u16be")          << QLatin1String("PCM unsigned 16-bit big-endian");
        type << QLatin1String("pcm_u16le")          << QLatin1String("PCM unsigned 16-bit little-endian");
        type << QLatin1String("pcm_u24be")          << QLatin1String("PCM unsigned 24-bit big-endian");
        type << QLatin1String("pcm_u24le")          << QLatin1String("PCM unsigned 24-bit little-endian");
        type << QLatin1String("pcm_u32be")          << QLatin1String("PCM unsigned 32-bit big-endian");
        type << QLatin1String("pcm_u32le")          << QLatin1String("PCM unsigned 32-bit little-endian");
        type << QLatin1String("pcm_u8")             << QLatin1String("PCM unsigned 8-bit");
        type << QLatin1String("pcm_zork")           << QLatin1String("PCM Zork");
        type << QLatin1String("ra_144")             << QLatin1String("RealAudio 1.0 (14.4K)");
        type << QLatin1String("ra_288")             << QLatin1String("RealAudio 2.0 (28.8K)");
        type << QLatin1String("ralf")               << QLatin1String("RealAudio Lossless");
        type << QLatin1String("sipr")               << QLatin1String("RealAudio SIPR / ACELP.NET");
        type << QLatin1String("speex")              << QLatin1String("Speex");
        type << QLatin1String("tak")                << QLatin1String("TAK (Tom's lossless Audio Kompressor)");
        type << QLatin1String("wavpack")            << QLatin1String("WavPack");
        type << QLatin1String("wmalossless")        << QLatin1String("Windows Media Audio Lossless");
        type << QLatin1String("wmapro")             << QLatin1String("Windows Media Audio 9 Professional");
        type << QLatin1String("wmav1")              << QLatin1String("Windows Media Audio 1");
        type << QLatin1String("wmav2")              << QLatin1String("Windows Media Audio 2");
        type << QLatin1String("wmavoice")           << QLatin1String("Windows Media Audio Voice");

        // TODO: add more possible audio Codec

        field->setChoice(type);

        return field;
    }
    else if (name == QLatin1String("faceregionscount"))
    {
        SearchFieldRangeInt* const field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Faces"), i18n("Return items by number of face regions"));
        field->setBetweenText(i18nc("Return items by number of face regions between...and...", "and"));
        field->setBoundary(0, 1000, 1);

        return field;
    }
    else if (name == QLatin1String("confirmedfaces"))
    {
        SearchFieldCheckBox* const field = new SearchFieldCheckBox(parent);
        field->setFieldName(name);
        field->setText(i18n("Faces"), i18n("Return items with confirmed face regions"));
        field->setLabel(i18n("Confirmed Face Regions"));

        return field;
    }
    else if (name == QLatin1String("nofaceregions"))
    {
        SearchFieldCheckBox* const field = new SearchFieldCheckBox(parent);
        field->setFieldName(name);
        field->setText(i18n("Faces"), i18n("Return items without face regions"));
        field->setLabel(i18n("No Face Regions"));

        return field;
    }
    else
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "SearchField::createField: cannot create SearchField for" << name;
    }

    return nullptr;
}

} // namespace Digikam
