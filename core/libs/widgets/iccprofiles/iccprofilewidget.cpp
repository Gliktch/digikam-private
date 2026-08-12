/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-06-23
 * Description : a tab widget to display ICC profile infos
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iccprofilewidget.h"

// Qt includes

#include <QComboBox>
#include <QFile>
#include <QGroupBox>
#include <QLabel>
#include <QMap>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "iccprofile.h"
#include "digikam_debug.h"
#include "cietonguewidget.h"
#include "metadatalistview.h"

namespace
{

static const char* ICCHumanList[] =
{
    "Icc.Header.ColorSpace",
    "Icc.Header.Copyright",
    "Icc.Header.DeviceClass",
    "Icc.Header.Name",
    "Icc.Header.Description",
    "Icc.Header.RenderingIntent",
    "-1"
};

/**
 * NOTE: This entry list is only require for compatibility with MetadataWidget implementation.
 */
static const char* ICCEntryList[] =
{
    "Header",
    "-1"
};

} // namespace

namespace Digikam
{

class Q_DECL_HIDDEN ICCTagInfo
{

public:

    ICCTagInfo() = default;

    ICCTagInfo(const QString& title, const QString& description)
        : m_title      (title),
          m_description(description)
    {
    }

    QString title()       const
    {
        return m_title;
    }

    QString description() const
    {
        return m_description;
    }

private:

    QString m_title;
    QString m_description;
};

typedef QMap<QString, ICCTagInfo> ICCTagInfoMap;

// ---------------------------------------------------------------------------------------

class Q_DECL_HIDDEN ICCProfileWidget::Private
{

public:

    Private() = default;

    IccProfile       profile;

    QStringList      keysFilter;

    CIETongueWidget* cieTongue  = nullptr;

    ICCTagInfoMap    iccTagsDescription;
};

ICCProfileWidget::ICCProfileWidget(QWidget* const parent, int w, int h)
    : MetadataWidget(parent),
      d             (new Private)
{
    setup();
    dkCmsErrorAction(LCMS_ERROR_SHOW);

    // Set the translated ICC tags titles/descriptions list

    d->iccTagsDescription[QLatin1String("Icc.Header.Name")]            = ICCTagInfo(i18nc("@item: icc profile properties", "Name"),
                                                                                    i18nc("@info: icc profile properties", "The ICC profile product name"));
    d->iccTagsDescription[QLatin1String("Icc.Header.Description")]     = ICCTagInfo(i18nc("@item: icc profile properties", "Description"),
                                                                                    i18nc("@info: icc profile properties", "The ICC profile product description"));
    d->iccTagsDescription[QLatin1String("Icc.Header.Information")]     = ICCTagInfo(i18nc("@item: icc profile properties", "Information"),
                                                                                    i18nc("@info: icc profile properties", "Additional ICC profile information"));
    d->iccTagsDescription[QLatin1String("Icc.Header.Manufacturer")]    = ICCTagInfo(i18nc("@item: icc profile properties", "Manufacturer"),
                                                                                    i18nc("@info: icc profile properties", "Raw information about the ICC profile manufacturer"));
    d->iccTagsDescription[QLatin1String("Icc.Header.Model")]           = ICCTagInfo(i18nc("@item: icc profile properties", "Model"),
                                                                                    i18nc("@info: icc profile properties", "Raw information about the ICC profile model"));
    d->iccTagsDescription[QLatin1String("Icc.Header.Copyright")]       = ICCTagInfo(i18nc("@item: icc profile properties", "Copyright"),
                                                                                    i18nc("@info: icc profile properties", "Raw information about the ICC profile copyright"));
    d->iccTagsDescription[QLatin1String("Icc.Header.ProfileID")]       = ICCTagInfo(i18nc("@item: icc profile properties", "Profile ID"),
                                                                                    i18nc("@info: icc profile properties", "The ICC profile ID number"));
    d->iccTagsDescription[QLatin1String("Icc.Header.ColorSpace")]      = ICCTagInfo(i18nc("@item: icc profile properties", "Color Space"),
                                                                                    i18nc("@info: icc profile properties", "The color space used by the ICC profile"));
    d->iccTagsDescription[QLatin1String("Icc.Header.ConnectionSpace")] = ICCTagInfo(i18nc("@item: icc profile properties", "Connection Space"),
                                                                                    i18nc("@info: icc profile properties", "The connection space used by the ICC profile"));
    d->iccTagsDescription[QLatin1String("Icc.Header.DeviceClass")]     = ICCTagInfo(i18nc("@item: icc profile properties", "Device Class"),
                                                                                    i18nc("@info: icc profile properties", "The ICC profile device class"));
    d->iccTagsDescription[QLatin1String("Icc.Header.RenderingIntent")] = ICCTagInfo(i18nc("@item: icc profile properties", "Rendering Intent"),
                                                                                    i18nc("@info: icc profile properties", "The ICC profile rendering intent"));
    d->iccTagsDescription[QLatin1String("Icc.Header.ProfileVersion")]  = ICCTagInfo(i18nc("@item: icc profile properties", "Profile Version"),
                                                                                    i18nc("@info: icc profile properties", "The ICC version used to record the profile"));
    d->iccTagsDescription[QLatin1String("Icc.Header.CMMFlags")]        = ICCTagInfo(i18nc("@item: icc profile properties", "CMM Flags"),
                                                                                    i18nc("@info: icc profile properties", "The ICC profile color management flags"));

    // Set the list of keys and tags filters.

    for (int i = 0 ; QLatin1String(ICCEntryList[i]) != QLatin1String("-1") ; ++i)
    {
        d->keysFilter << QLatin1String(ICCEntryList[i]);
    }

    QStringList tagsFilter;

    for (int i = 0 ; QLatin1String(ICCHumanList[i]) != QLatin1String("-1") ; ++i)
    {
        tagsFilter << QLatin1String(ICCHumanList[i]);
    }

    setTagsFilter(tagsFilter);

    // Add CIE tongue graph to the widget area

    d->cieTongue = new CIETongueWidget(w, h, this);
    d->cieTongue->setWhatsThis(i18nc("@info: icc profile properties",
                                     "This area contains a CIE or chromaticity diagram.\n"
                                     "A CIE diagram is a representation of all the colors\n"
                                     "that a person with normal vision can see. This is represented\n"
                                     "by the colored sail-shaped area. In addition you will see a\n"
                                     "triangle that is superimposed on the diagram outlined in white.\n"
                                     "This triangle represents the outer boundaries of the color space\n"
                                     "of the device that is characterized by the inspected profile.\n"
                                     "This is called the device gamut.\n"
                                     "In addition there are black dots and yellow lines on the diagram.\n"
                                     "Each black dot represents one of the measurement points that were\n"
                                     "used to create this profile. The yellow line represents the\n"
                                     "amount that each point is corrected by the profile, and the\n"
                                     "direction of this correction."));

    setUserAreaWidget(d->cieTongue);
    this->decodeMetadata();
}

ICCProfileWidget::~ICCProfileWidget()
{
    delete d;
}

bool ICCProfileWidget::setProfile(const IccProfile& profile)
{
    // Cleanup all metadata contents.

    setMetadataMap();

    d->profile = profile;

    if (!d->profile.open())
    {
        setMetadataEmpty();
        d->cieTongue->setProfileData();
        d->profile = IccProfile();

        return false;
    }

    // Try to decode current metadata.

    enabledToolButtons(decodeMetadata());

    // Refresh view using decoded metadata.

    buildView();

    return true;
}

IccProfile ICCProfileWidget::getProfile() const
{
    return d->profile;
}

void ICCProfileWidget::setDataLoading()
{
    d->cieTongue->loadingStarted();
}

void ICCProfileWidget::setLoadingFailed()
{
    d->cieTongue->loadingFailed();
}

void ICCProfileWidget::setUncalibratedColor()
{
    d->cieTongue->uncalibratedColor();
}

QString ICCProfileWidget::getMetadataTitle() const
{
    return i18nc("@title: icc profile properties", "ICC Color Profile Information");
}

bool ICCProfileWidget::loadFromURL(const QUrl& url)
{
    setFileName(url.toLocalFile());

    if (url.isEmpty())
    {
        setProfile(IccProfile());
        d->cieTongue->setProfileData();

        return false;
    }
    else
    {
        IccProfile profile(url.toLocalFile());

        if (!setProfile(profile))
        {
            setProfile(IccProfile());
            d->cieTongue->setProfileData();

            return false;
        }
    }

    return true;
}

bool ICCProfileWidget::loadFromProfileData(const QString& fileName, const QByteArray& data)
{
    setFileName(fileName);

    return(setProfile(IccProfile(data)));
}

bool ICCProfileWidget::loadProfile(const QString& fileName, const IccProfile& profile)
{
    setFileName(fileName);

    return(setProfile(profile));
}

bool ICCProfileWidget::decodeMetadata()
{
    if (!d->profile.isOpen())
    {
        return false;
    }

    d->cieTongue->setProfileData(d->profile.data());

    LcmsLock lock;
    cmsHPROFILE hProfile = d->profile.handle();

    if (!hProfile)
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << "Cannot parse ICC profile tags using LCMS";
        return false;
    }

    DMetadata::MetaDataMap metaDataMap;

    if (!QString(dkCmsTakeProductName(hProfile)).isEmpty())
    {
        metaDataMap.insert(QLatin1String("Icc.Header.Name"), dkCmsTakeProductName(hProfile).replace(QLatin1Char('\n'), QLatin1Char(' ')));
    }

    if (!dkCmsTakeProductDesc(hProfile).isEmpty())
    {
        metaDataMap.insert(QLatin1String("Icc.Header.Description"), dkCmsTakeProductDesc(hProfile).replace(QLatin1Char('\n'), QLatin1Char(' ')));
    }

    if (!dkCmsTakeProductInfo(hProfile).isEmpty())
    {
        metaDataMap.insert(QLatin1String("Icc.Header.Information"), dkCmsTakeProductInfo(hProfile).replace(QLatin1Char('\n'), QLatin1Char(' ')));
    }

    if (!dkCmsTakeManufacturer(hProfile).isEmpty())
    {
        metaDataMap.insert(QLatin1String("Icc.Header.Manufacturer"), dkCmsTakeManufacturer(hProfile).replace(QLatin1Char('\n'), QLatin1Char(' ')));
    }

    if (!dkCmsTakeModel(hProfile).isEmpty())
    {
        metaDataMap.insert(QLatin1String("Icc.Header.Model"), dkCmsTakeModel(hProfile).replace(QLatin1Char('\n'), QLatin1Char(' ')));
    }

    if (!dkCmsTakeCopyright(hProfile).isEmpty())
    {
        metaDataMap.insert(QLatin1String("Icc.Header.Copyright"), dkCmsTakeCopyright(hProfile).replace(QLatin1Char('\n'), QLatin1Char(' ')));
    }

    metaDataMap.insert(QLatin1String("Icc.Header.ProfileID"),      QString::number((uint)*dkCmsTakeProfileID(hProfile)));
    metaDataMap.insert(QLatin1String("Icc.Header.ProfileVersion"), QString::number((uint)dkCmsGetProfileICCversion(hProfile)));
    metaDataMap.insert(QLatin1String("Icc.Header.CMMFlags"),       QString::number((uint)dkCmsTakeHeaderFlags(hProfile)));

    QString colorSpace;

    switch (dkCmsGetColorSpace(hProfile))
    {
        case icSigLabData:
        {
            colorSpace = i18nc("@info: icc profile color space", "Lab");
            break;
        }

        case icSigLuvData:
        {
            colorSpace = i18nc("@info: icc profile color space", "Luv");
            break;
        }

        case icSigRgbData:
        {
            colorSpace = i18nc("@info: icc profile color space", "RGB");
            break;
        }

        case icSigGrayData:
        {
            colorSpace = i18nc("@info: icc profile color space", "GRAY");
            break;
        }

        case icSigHsvData:
        {
            colorSpace = i18nc("@info: icc profile color space", "HSV");
            break;
        }

        case icSigHlsData:
        {
            colorSpace = i18nc("@info: icc profile color space", "HLS");
            break;
        }

        case icSigCmykData:
        {
            colorSpace = i18nc("@info: icc profile color space", "CMYK");
            break;
        }

        case icSigCmyData:
        {
            colorSpace= i18nc("@info: icc profile color space", "CMY");
            break;
        }

        default:
        {
            colorSpace = i18nc("@info: icc profile color space", "Unknown");
            break;
        }
    }

    metaDataMap.insert(QLatin1String("Icc.Header.ColorSpace"), colorSpace);

    QString connectionSpace;

    switch (dkCmsGetPCS(hProfile))
    {
        case icSigLabData:
        {
            connectionSpace = i18nc("@info: icc profile color space", "Lab");
            break;
        }

        case icSigLuvData:
        {
            connectionSpace = i18nc("@info: icc profile color space", "Luv");
            break;
        }

        case icSigRgbData:
        {
            connectionSpace = i18nc("@info: icc profile color space", "RGB");
            break;
        }

        case icSigGrayData:
        {
            connectionSpace = i18nc("@info: icc profile color space", "GRAY");
            break;
        }

        case icSigHsvData:
        {
            connectionSpace = i18nc("@info: icc profile color space", "HSV");
            break;
        }

        case icSigHlsData:
        {
            connectionSpace = i18nc("@info: icc profile color space", "HLS");
            break;
        }

        case icSigCmykData:
        {
            connectionSpace = i18nc("@info: icc profile color space", "CMYK");
            break;
        }

        case icSigCmyData:
        {
            connectionSpace= i18nc("@info: icc profile color space", "CMY");
            break;
        }

        default:
        {
            connectionSpace = i18nc("@info: icc profile color space", "Unknown");
            break;
        }
    }

    metaDataMap.insert(QLatin1String("Icc.Header.ConnectionSpace"), connectionSpace);

    QString device;

    switch ((int)dkCmsGetDeviceClass(hProfile))
    {
        case icSigInputClass:
        {
            device = i18nc("@info: icc profile class", "Input device");
            break;
        }

        case icSigDisplayClass:
        {
            device = i18nc("@info: icc profile class", "Display device");
            break;
        }

        case icSigOutputClass:
        {
            device = i18nc("@info: icc profile class", "Output device");
            break;
        }

        case icSigColorSpaceClass:
        {
            device = i18nc("@info: icc profile class", "Color space");
            break;
        }

        case icSigLinkClass:
        {
            device = i18nc("@info: icc profile class", "Link device");
            break;
        }

        case icSigAbstractClass:
        {
            device = i18nc("@info: icc profile class", "Abstract");
            break;
        }

        case icSigNamedColorClass:
        {
            device = i18nc("@info: icc profile class", "Named color");
            break;
        }

        default:
        {
            device = i18nc("@info: icc profile class", "Unknown");
            break;
        }
    }

    metaDataMap.insert(QLatin1String("Icc.Header.DeviceClass"), device);

    QString intent;

    switch (dkCmsTakeRenderingIntent(hProfile))
    {
        case 0:
        {
            intent = i18nc("@info: icc profile redering", "Perceptual");
            break;
        }

        case 1:
        {
            intent = i18nc("@info: icc profile redering", "Relative Colorimetric");
            break;
        }

        case 2:
        {
            intent = i18nc("@info: icc profile redering", "Saturation");
            break;
        }

        case 3:
        {
            intent = i18nc("@info: icc profile redering", "Absolute Colorimetric");
            break;
        }

        default:
        {
            intent = i18nc("@info: icc profile redering", "Unknown");
            break;
        }
    }

    metaDataMap.insert(QLatin1String("Icc.Header.RenderingIntent"), intent);

    // Update all metadata contents.

    setMetadataMap(metaDataMap);

    return true;
}

void ICCProfileWidget::buildView()
{
    if (getMode() == CUSTOM)
    {
        setIfdList(getMetadataMap(), d->keysFilter, getTagsFilter());
    }
    else
    {
        setIfdList(getMetadataMap(), d->keysFilter, QStringList() << QLatin1String("FULL"));
    }

    MetadataWidget::buildView();
}

QString ICCProfileWidget::getTagTitle(const QString& key)
{
    ICCTagInfoMap::const_iterator it = d->iccTagsDescription.constFind(key);

    if (it != d->iccTagsDescription.constEnd())
    {
        return(it.value().title());
    }

    return key.section(QLatin1Char('.'), 2, 2);
}

void ICCProfileWidget::slotSaveMetadataToFile()
{
    QUrl url = saveMetadataToFile(i18nc("@title: file open dialog", "ICC color profile File to Save"),
                                  QString(QLatin1String("*.icc *.icm|") +
                                  i18nc("@info: file open filters", "ICC Files (*.icc; *.icm)")));
    storeMetadataToFile(url, d->profile.data());
}

QString ICCProfileWidget::getTagDescription(const QString& key)
{
    ICCTagInfoMap::const_iterator it = d->iccTagsDescription.constFind(key);

    if (it != d->iccTagsDescription.constEnd())
    {
        return(it.value().description());
    }

    return key.section(QLatin1Char('.'), 2, 2);
}

} // namespace Digikam

#include "moc_iccprofilewidget.cpp"
