/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-07-07
 * Description : country selector combo-box.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "countryselector.h"

// Qt includes

#include <QMap>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN CountrySelector::Private
{
public:

    Private()
    {
        /**
         * NOTE: We cannot use KLocale::allCountriesList() here because KDE only
         * support 2 characters country codes. XMP require 3 characters country
         * following ISO 3166 (userpage.chemie.fu-berlin.de/diverse/doc/ISO_3166.html)
         */

        // Standard ISO 3166 country codes.

        countryCodeMap.insert(QLatin1String("AFG"), i18nc("@info: country code", "Afghanistan"));
        countryCodeMap.insert(QLatin1String("ALB"), i18nc("@info: country code", "Albania"));
        countryCodeMap.insert(QLatin1String("DZA"), i18nc("@info: country code", "Algeria"));
        countryCodeMap.insert(QLatin1String("ASM"), i18nc("@info: country code", "American Samoa"));
        countryCodeMap.insert(QLatin1String("AND"), i18nc("@info: country code", "Andorra"));
        countryCodeMap.insert(QLatin1String("AGO"), i18nc("@info: country code", "Angola"));
        countryCodeMap.insert(QLatin1String("AIA"), i18nc("@info: country code", "Anguilla"));
        countryCodeMap.insert(QLatin1String("AGO"), i18nc("@info: country code", "Angola"));
        countryCodeMap.insert(QLatin1String("ATA"), i18nc("@info: country code", "Antarctica"));
        countryCodeMap.insert(QLatin1String("ATG"), i18nc("@info: country code", "Antigua and Barbuda"));
        countryCodeMap.insert(QLatin1String("ARG"), i18nc("@info: country code", "Argentina"));
        countryCodeMap.insert(QLatin1String("ARM"), i18nc("@info: country code", "Armenia"));
        countryCodeMap.insert(QLatin1String("ABW"), i18nc("@info: country code", "Aruba"));
        countryCodeMap.insert(QLatin1String("AUS"), i18nc("@info: country code", "Australia"));
        countryCodeMap.insert(QLatin1String("AUT"), i18nc("@info: country code", "Austria"));
        countryCodeMap.insert(QLatin1String("AZE"), i18nc("@info: country code", "Azerbaijan"));
        countryCodeMap.insert(QLatin1String("BHS"), i18nc("@info: country code", "Bahamas"));
        countryCodeMap.insert(QLatin1String("BHR"), i18nc("@info: country code", "Bahrain"));
        countryCodeMap.insert(QLatin1String("BGD"), i18nc("@info: country code", "Bangladesh"));
        countryCodeMap.insert(QLatin1String("BRB"), i18nc("@info: country code", "Barbados"));
        countryCodeMap.insert(QLatin1String("BLR"), i18nc("@info: country code", "Belarus"));
        countryCodeMap.insert(QLatin1String("BEL"), i18nc("@info: country code", "Belgium"));
        countryCodeMap.insert(QLatin1String("BLZ"), i18nc("@info: country code", "Belize"));
        countryCodeMap.insert(QLatin1String("BEN"), i18nc("@info: country code", "Benin"));
        countryCodeMap.insert(QLatin1String("BMU"), i18nc("@info: country code", "Bermuda"));
        countryCodeMap.insert(QLatin1String("BTN"), i18nc("@info: country code", "Bhutan"));
        countryCodeMap.insert(QLatin1String("BOL"), i18nc("@info: country code", "Bolivia"));
        countryCodeMap.insert(QLatin1String("BIH"), i18nc("@info: country code", "Bosnia and Herzegovina"));
        countryCodeMap.insert(QLatin1String("BWA"), i18nc("@info: country code", "Botswana"));
        countryCodeMap.insert(QLatin1String("BVT"), i18nc("@info: country code", "Bouvet Island"));
        countryCodeMap.insert(QLatin1String("BRA"), i18nc("@info: country code", "Brazil"));
        countryCodeMap.insert(QLatin1String("IOT"), i18nc("@info: country code", "British Indian Ocean Territory"));
        countryCodeMap.insert(QLatin1String("VGB"), i18nc("@info: country code", "British Virgin Islands"));
        countryCodeMap.insert(QLatin1String("BRN"), i18nc("@info: country code", "Brunei Darussalam"));
        countryCodeMap.insert(QLatin1String("BGR"), i18nc("@info: country code", "Bulgaria"));
        countryCodeMap.insert(QLatin1String("BFA"), i18nc("@info: country code", "Burkina Faso"));
        countryCodeMap.insert(QLatin1String("BDI"), i18nc("@info: country code", "Burundi"));
        countryCodeMap.insert(QLatin1String("KHM"), i18nc("@info: country code", "Cambodia"));
        countryCodeMap.insert(QLatin1String("CMR"), i18nc("@info: country code", "Cameroon"));
        countryCodeMap.insert(QLatin1String("CAN"), i18nc("@info: country code", "Canada"));
        countryCodeMap.insert(QLatin1String("CPV"), i18nc("@info: country code", "Cape Verde"));
        countryCodeMap.insert(QLatin1String("CYM"), i18nc("@info: country code", "Cayman Islands"));
        countryCodeMap.insert(QLatin1String("CAF"), i18nc("@info: country code", "Central African Republic"));
        countryCodeMap.insert(QLatin1String("TCD"), i18nc("@info: country code", "Chad"));
        countryCodeMap.insert(QLatin1String("CHL"), i18nc("@info: country code", "Chile"));
        countryCodeMap.insert(QLatin1String("CHN"), i18nc("@info: country code", "China"));
        countryCodeMap.insert(QLatin1String("CXR"), i18nc("@info: country code", "Christmas Island "));
        countryCodeMap.insert(QLatin1String("CCK"), i18nc("@info: country code", "Cocos Islands"));
        countryCodeMap.insert(QLatin1String("COL"), i18nc("@info: country code", "Colombia"));
        countryCodeMap.insert(QLatin1String("COM"), i18nc("@info: country code", "Comoros"));
        countryCodeMap.insert(QLatin1String("COD"), i18nc("@info: country code", "Zaire"));
        countryCodeMap.insert(QLatin1String("COG"), i18nc("@info: country code", "Congo"));
        countryCodeMap.insert(QLatin1String("COK"), i18nc("@info: country code", "Cook Islands"));
        countryCodeMap.insert(QLatin1String("CRI"), i18nc("@info: country code", "Costa Rica"));
        countryCodeMap.insert(QLatin1String("CIV"), i18nc("@info: country code", "Ivory Coast"));
        countryCodeMap.insert(QLatin1String("CUB"), i18nc("@info: country code", "Cuba"));
        countryCodeMap.insert(QLatin1String("CYP"), i18nc("@info: country code", "Cyprus"));
        countryCodeMap.insert(QLatin1String("CZE"), i18nc("@info: country code", "Czechia"));
        countryCodeMap.insert(QLatin1String("DNK"), i18nc("@info: country code", "Denmark"));
        countryCodeMap.insert(QLatin1String("DJI"), i18nc("@info: country code", "Djibouti"));
        countryCodeMap.insert(QLatin1String("DMA"), i18nc("@info: country code", "Dominica"));
        countryCodeMap.insert(QLatin1String("DOM"), i18nc("@info: country code", "Dominican Republic"));
        countryCodeMap.insert(QLatin1String("ECU"), i18nc("@info: country code", "Ecuador"));
        countryCodeMap.insert(QLatin1String("EGY"), i18nc("@info: country code", "Egypt"));
        countryCodeMap.insert(QLatin1String("SLV"), i18nc("@info: country code", "El Salvador"));
        countryCodeMap.insert(QLatin1String("GNQ"), i18nc("@info: country code", "Equatorial Guinea"));
        countryCodeMap.insert(QLatin1String("ERI"), i18nc("@info: country code", "Eritrea"));
        countryCodeMap.insert(QLatin1String("EST"), i18nc("@info: country code", "Estonia"));
        countryCodeMap.insert(QLatin1String("ETH"), i18nc("@info: country code", "Ethiopia"));
        countryCodeMap.insert(QLatin1String("FRO"), i18nc("@info: country code", "Faeroe Islands"));        // codespell:ignore fro
        countryCodeMap.insert(QLatin1String("FLK"), i18nc("@info: country code", "Falkland Islands"));
        countryCodeMap.insert(QLatin1String("FJI"), i18nc("@info: country code", "Fiji Islands"));
        countryCodeMap.insert(QLatin1String("FIN"), i18nc("@info: country code", "Finland"));
        countryCodeMap.insert(QLatin1String("FRA"), i18nc("@info: country code", "France"));
        countryCodeMap.insert(QLatin1String("GUF"), i18nc("@info: country code", "French Guiana"));
        countryCodeMap.insert(QLatin1String("PYF"), i18nc("@info: country code", "French Polynesia"));
        countryCodeMap.insert(QLatin1String("ATF"), i18nc("@info: country code", "French Southern Territories"));
        countryCodeMap.insert(QLatin1String("GAB"), i18nc("@info: country code", "Gabon"));
        countryCodeMap.insert(QLatin1String("GMB"), i18nc("@info: country code", "Gambia"));
        countryCodeMap.insert(QLatin1String("GEO"), i18nc("@info: country code", "Georgia"));
        countryCodeMap.insert(QLatin1String("DEU"), i18nc("@info: country code", "Germany"));
        countryCodeMap.insert(QLatin1String("GHA"), i18nc("@info: country code", "Ghana"));
        countryCodeMap.insert(QLatin1String("GIB"), i18nc("@info: country code", "Gibraltar"));
        countryCodeMap.insert(QLatin1String("GRC"), i18nc("@info: country code", "Greece"));
        countryCodeMap.insert(QLatin1String("GRL"), i18nc("@info: country code", "Greenland"));
        countryCodeMap.insert(QLatin1String("GRD"), i18nc("@info: country code", "Grenada"));
        countryCodeMap.insert(QLatin1String("GLP"), i18nc("@info: country code", "Guadaloupe"));
        countryCodeMap.insert(QLatin1String("GUM"), i18nc("@info: country code", "Guam"));
        countryCodeMap.insert(QLatin1String("GTM"), i18nc("@info: country code", "Guatemala"));
        countryCodeMap.insert(QLatin1String("GIN"), i18nc("@info: country code", "Guinea"));
        countryCodeMap.insert(QLatin1String("GNB"), i18nc("@info: country code", "Guinea-Bissau"));
        countryCodeMap.insert(QLatin1String("GUY"), i18nc("@info: country code", "Guyana"));
        countryCodeMap.insert(QLatin1String("HTI"), i18nc("@info: country code", "Haiti"));
        countryCodeMap.insert(QLatin1String("HMD"), i18nc("@info: country code", "Heard and McDonald Islands"));
        countryCodeMap.insert(QLatin1String("VAT"), i18nc("@info: country code", "Vatican"));
        countryCodeMap.insert(QLatin1String("HND"), i18nc("@info: country code", "Honduras"));
        countryCodeMap.insert(QLatin1String("HKG"), i18nc("@info: country code", "Hong Kong"));
        countryCodeMap.insert(QLatin1String("HRV"), i18nc("@info: country code", "Croatia"));
        countryCodeMap.insert(QLatin1String("HUN"), i18nc("@info: country code", "Hungary"));
        countryCodeMap.insert(QLatin1String("ISL"), i18nc("@info: country code", "Iceland"));
        countryCodeMap.insert(QLatin1String("IND"), i18nc("@info: country code", "India"));
        countryCodeMap.insert(QLatin1String("IDN"), i18nc("@info: country code", "Indonesia"));
        countryCodeMap.insert(QLatin1String("IRN"), i18nc("@info: country code", "Iran"));
        countryCodeMap.insert(QLatin1String("IRQ"), i18nc("@info: country code", "Iraq"));
        countryCodeMap.insert(QLatin1String("IRL"), i18nc("@info: country code", "Ireland"));
        countryCodeMap.insert(QLatin1String("ISR"), i18nc("@info: country code", "Israel"));
        countryCodeMap.insert(QLatin1String("ITA"), i18nc("@info: country code", "Italy"));
        countryCodeMap.insert(QLatin1String("JAM"), i18nc("@info: country code", "Jamaica"));
        countryCodeMap.insert(QLatin1String("JPN"), i18nc("@info: country code", "Japan"));
        countryCodeMap.insert(QLatin1String("JOR"), i18nc("@info: country code", "Jordan"));
        countryCodeMap.insert(QLatin1String("KAZ"), i18nc("@info: country code", "Kazakhstan"));
        countryCodeMap.insert(QLatin1String("KEN"), i18nc("@info: country code", "Kenya"));
        countryCodeMap.insert(QLatin1String("KIR"), i18nc("@info: country code", "Kiribati"));
        countryCodeMap.insert(QLatin1String("PRK"), i18nc("@info: country code", "North-Korea"));
        countryCodeMap.insert(QLatin1String("KOR"), i18nc("@info: country code", "South-Korea"));
        countryCodeMap.insert(QLatin1String("KWT"), i18nc("@info: country code", "Kuwait"));
        countryCodeMap.insert(QLatin1String("KGZ"), i18nc("@info: country code", "Kyrgyz Republic"));
        countryCodeMap.insert(QLatin1String("LAO"), i18nc("@info: country code", "Lao"));
        countryCodeMap.insert(QLatin1String("LVA"), i18nc("@info: country code", "Latvia"));
        countryCodeMap.insert(QLatin1String("LBN"), i18nc("@info: country code", "Lebanon"));
        countryCodeMap.insert(QLatin1String("LSO"), i18nc("@info: country code", "Lesotho"));
        countryCodeMap.insert(QLatin1String("LBR"), i18nc("@info: country code", "Liberia"));
        countryCodeMap.insert(QLatin1String("LBY"), i18nc("@info: country code", "Libyan Arab Jamahiriya"));
        countryCodeMap.insert(QLatin1String("LIE"), i18nc("@info: country code", "Liechtenstein"));
        countryCodeMap.insert(QLatin1String("LTU"), i18nc("@info: country code", "Lithuania"));
        countryCodeMap.insert(QLatin1String("LUX"), i18nc("@info: country code", "Luxembourg"));
        countryCodeMap.insert(QLatin1String("MAC"), i18nc("@info: country code", "Macao"));
        countryCodeMap.insert(QLatin1String("MKD"), i18nc("@info: country code", "North Macedonia"));
        countryCodeMap.insert(QLatin1String("MDG"), i18nc("@info: country code", "Madagascar"));
        countryCodeMap.insert(QLatin1String("MWI"), i18nc("@info: country code", "Malawi"));
        countryCodeMap.insert(QLatin1String("MYS"), i18nc("@info: country code", "Malaysia"));
        countryCodeMap.insert(QLatin1String("MDV"), i18nc("@info: country code", "Maldives"));
        countryCodeMap.insert(QLatin1String("MLI"), i18nc("@info: country code", "Mali"));
        countryCodeMap.insert(QLatin1String("MLT"), i18nc("@info: country code", "Malta"));
        countryCodeMap.insert(QLatin1String("MHL"), i18nc("@info: country code", "Marshall Islands"));
        countryCodeMap.insert(QLatin1String("MTQ"), i18nc("@info: country code", "Martinique"));
        countryCodeMap.insert(QLatin1String("MRT"), i18nc("@info: country code", "Mauritania"));
        countryCodeMap.insert(QLatin1String("MUS"), i18nc("@info: country code", "Mauritius"));
        countryCodeMap.insert(QLatin1String("MYT"), i18nc("@info: country code", "Mayotte"));
        countryCodeMap.insert(QLatin1String("MEX"), i18nc("@info: country code", "Mexico"));
        countryCodeMap.insert(QLatin1String("FSM"), i18nc("@info: country code", "Micronesia"));
        countryCodeMap.insert(QLatin1String("MDA"), i18nc("@info: country code", "Moldova"));
        countryCodeMap.insert(QLatin1String("MCO"), i18nc("@info: country code", "Monaco"));
        countryCodeMap.insert(QLatin1String("MNG"), i18nc("@info: country code", "Mongolia"));
        countryCodeMap.insert(QLatin1String("MNE"), i18nc("@info: country code", "Montenegro"));
        countryCodeMap.insert(QLatin1String("MSR"), i18nc("@info: country code", "Montserrat"));
        countryCodeMap.insert(QLatin1String("MAR"), i18nc("@info: country code", "Morocco"));
        countryCodeMap.insert(QLatin1String("MOZ"), i18nc("@info: country code", "Mozambique"));
        countryCodeMap.insert(QLatin1String("MMR"), i18nc("@info: country code", "Myanmar"));
        countryCodeMap.insert(QLatin1String("NAM"), i18nc("@info: country code", "Namibia"));
        countryCodeMap.insert(QLatin1String("NRU"), i18nc("@info: country code", "Nauru"));
        countryCodeMap.insert(QLatin1String("NPL"), i18nc("@info: country code", "Nepal"));
        countryCodeMap.insert(QLatin1String("ANT"), i18nc("@info: country code", "Netherlands Antilles"));
        countryCodeMap.insert(QLatin1String("NLD"), i18nc("@info: country code", "Netherlands"));
        countryCodeMap.insert(QLatin1String("NCL"), i18nc("@info: country code", "New Caledonia"));
        countryCodeMap.insert(QLatin1String("NZL"), i18nc("@info: country code", "New Zealand"));
        countryCodeMap.insert(QLatin1String("NIC"), i18nc("@info: country code", "Nicaragua"));
        countryCodeMap.insert(QLatin1String("NER"), i18nc("@info: country code", "Niger"));
        countryCodeMap.insert(QLatin1String("NGA"), i18nc("@info: country code", "Nigeria"));
        countryCodeMap.insert(QLatin1String("NIU"), i18nc("@info: country code", "Niue"));
        countryCodeMap.insert(QLatin1String("NFK"), i18nc("@info: country code", "Norfolk Island"));
        countryCodeMap.insert(QLatin1String("MNP"), i18nc("@info: country code", "Northern Mariana Islands"));
        countryCodeMap.insert(QLatin1String("NOR"), i18nc("@info: country code", "Norway"));
        countryCodeMap.insert(QLatin1String("OMN"), i18nc("@info: country code", "Oman"));
        countryCodeMap.insert(QLatin1String("PAK"), i18nc("@info: country code", "Pakistan"));
        countryCodeMap.insert(QLatin1String("PLW"), i18nc("@info: country code", "Palau"));
        countryCodeMap.insert(QLatin1String("PSE"), i18nc("@info: country code", "Palestinian Territory"));
        countryCodeMap.insert(QLatin1String("PAN"), i18nc("@info: country code", "Panama"));
        countryCodeMap.insert(QLatin1String("PNG"), i18nc("@info: country code", "Papua New Guinea"));
        countryCodeMap.insert(QLatin1String("PRY"), i18nc("@info: country code", "Paraguay"));
        countryCodeMap.insert(QLatin1String("PER"), i18nc("@info: country code", "Peru"));
        countryCodeMap.insert(QLatin1String("PHL"), i18nc("@info: country code", "Philippines"));
        countryCodeMap.insert(QLatin1String("PCN"), i18nc("@info: country code", "Pitcairn Island"));
        countryCodeMap.insert(QLatin1String("POL"), i18nc("@info: country code", "Poland"));
        countryCodeMap.insert(QLatin1String("PRT"), i18nc("@info: country code", "Portugal"));
        countryCodeMap.insert(QLatin1String("PRI"), i18nc("@info: country code", "Puerto Rico"));
        countryCodeMap.insert(QLatin1String("QAT"), i18nc("@info: country code", "Qatar"));
        countryCodeMap.insert(QLatin1String("REU"), i18nc("@info: country code", "Reunion"));
        countryCodeMap.insert(QLatin1String("ROU"), i18nc("@info: country code", "Romania"));
        countryCodeMap.insert(QLatin1String("RUS"), i18nc("@info: country code", "Russian Federation"));
        countryCodeMap.insert(QLatin1String("RWA"), i18nc("@info: country code", "Rwanda"));
        countryCodeMap.insert(QLatin1String("SHN"), i18nc("@info: country code", "St. Helena"));
        countryCodeMap.insert(QLatin1String("KNA"), i18nc("@info: country code", "St. Kitts and Nevis"));
        countryCodeMap.insert(QLatin1String("LCA"), i18nc("@info: country code", "St. Lucia"));
        countryCodeMap.insert(QLatin1String("SPM"), i18nc("@info: country code", "St. Pierre and Miquelon"));
        countryCodeMap.insert(QLatin1String("VCT"), i18nc("@info: country code", "St. Vincent and the Grenadines"));
        countryCodeMap.insert(QLatin1String("WSM"), i18nc("@info: country code", "Samoa"));
        countryCodeMap.insert(QLatin1String("SMR"), i18nc("@info: country code", "San Marino"));
        countryCodeMap.insert(QLatin1String("STP"), i18nc("@info: country code", "Sao Tome and Principe"));
        countryCodeMap.insert(QLatin1String("SAU"), i18nc("@info: country code", "Saudi Arabia"));
        countryCodeMap.insert(QLatin1String("SEN"), i18nc("@info: country code", "Senegal"));
        countryCodeMap.insert(QLatin1String("SRB"), i18nc("@info: country code", "Serbia"));
        countryCodeMap.insert(QLatin1String("SYC"), i18nc("@info: country code", "Seychelles"));
        countryCodeMap.insert(QLatin1String("SLE"), i18nc("@info: country code", "Sierra Leone"));
        countryCodeMap.insert(QLatin1String("SGP"), i18nc("@info: country code", "Singapore"));
        countryCodeMap.insert(QLatin1String("SVK"), i18nc("@info: country code", "Slovakia"));
        countryCodeMap.insert(QLatin1String("SVN"), i18nc("@info: country code", "Slovenia"));
        countryCodeMap.insert(QLatin1String("SLB"), i18nc("@info: country code", "Solomon Islands"));
        countryCodeMap.insert(QLatin1String("SOM"), i18nc("@info: country code", "Somalia"));               // codespell:ignore som
        countryCodeMap.insert(QLatin1String("ZAF"), i18nc("@info: country code", "South Africa"));
        countryCodeMap.insert(QLatin1String("SGS"), i18nc("@info: country code", "South Georgia and the South Sandwich Islands"));
        countryCodeMap.insert(QLatin1String("ESP"), i18nc("@info: country code", "Spain"));
        countryCodeMap.insert(QLatin1String("LKA"), i18nc("@info: country code", "Sri Lanka"));
        countryCodeMap.insert(QLatin1String("SDN"), i18nc("@info: country code", "Sudan"));
        countryCodeMap.insert(QLatin1String("SUR"), i18nc("@info: country code", "Suriname"));
        countryCodeMap.insert(QLatin1String("SJM"), i18nc("@info: country code", "Svalbard & Jan Mayen Islands"));
        countryCodeMap.insert(QLatin1String("SWZ"), i18nc("@info: country code", "Swaziland"));
        countryCodeMap.insert(QLatin1String("SWE"), i18nc("@info: country code", "Sweden"));
        countryCodeMap.insert(QLatin1String("CHE"), i18nc("@info: country code", "Switzerland"));
        countryCodeMap.insert(QLatin1String("SYR"), i18nc("@info: country code", "Syrian Arab Republic"));
        countryCodeMap.insert(QLatin1String("TWN"), i18nc("@info: country code", "Taiwan"));
        countryCodeMap.insert(QLatin1String("TJK"), i18nc("@info: country code", "Tajikistan"));
        countryCodeMap.insert(QLatin1String("TZA"), i18nc("@info: country code", "Tanzania"));
        countryCodeMap.insert(QLatin1String("THA"), i18nc("@info: country code", "Thailand"));              // codespell:ignore tha
        countryCodeMap.insert(QLatin1String("TLS"), i18nc("@info: country code", "Timor-Leste"));
        countryCodeMap.insert(QLatin1String("TGO"), i18nc("@info: country code", "Togo"));
        countryCodeMap.insert(QLatin1String("TKL"), i18nc("@info: country code", "Tokelau Islands"));
        countryCodeMap.insert(QLatin1String("TON"), i18nc("@info: country code", "Tonga"));
        countryCodeMap.insert(QLatin1String("TTO"), i18nc("@info: country code", "Trinidad and Tobago"));
        countryCodeMap.insert(QLatin1String("TUN"), i18nc("@info: country code", "Tunisia"));
        countryCodeMap.insert(QLatin1String("TUR"), i18nc("@info: country code", "Turkey"));
        countryCodeMap.insert(QLatin1String("TKM"), i18nc("@info: country code", "Turkmenistan"));
        countryCodeMap.insert(QLatin1String("TCA"), i18nc("@info: country code", "Turks and Caicos Islands"));
        countryCodeMap.insert(QLatin1String("TUV"), i18nc("@info: country code", "Tuvalu"));
        countryCodeMap.insert(QLatin1String("VIR"), i18nc("@info: country code", "US Virgin Islands"));
        countryCodeMap.insert(QLatin1String("UGA"), i18nc("@info: country code", "Uganda"));
        countryCodeMap.insert(QLatin1String("UKR"), i18nc("@info: country code", "Ukraine"));
        countryCodeMap.insert(QLatin1String("ARE"), i18nc("@info: country code", "United Arab Emirates"));
        countryCodeMap.insert(QLatin1String("GBR"), i18nc("@info: country code", "United Kingdom"));
        countryCodeMap.insert(QLatin1String("UMI"), i18nc("@info: country code", "United States Minor Outlying Islands"));
        countryCodeMap.insert(QLatin1String("USA"), i18nc("@info: country code", "United States of America"));
        countryCodeMap.insert(QLatin1String("URY"), i18nc("@info: country code", "Uruguay, Eastern Republic of"));
        countryCodeMap.insert(QLatin1String("UZB"), i18nc("@info: country code", "Uzbekistan"));
        countryCodeMap.insert(QLatin1String("VUT"), i18nc("@info: country code", "Vanuatu"));
        countryCodeMap.insert(QLatin1String("VEN"), i18nc("@info: country code", "Venezuela"));
        countryCodeMap.insert(QLatin1String("VNM"), i18nc("@info: country code", "Viet Nam"));
        countryCodeMap.insert(QLatin1String("WLF"), i18nc("@info: country code", "Wallis and Futuna Islands "));
        countryCodeMap.insert(QLatin1String("ESH"), i18nc("@info: country code", "Western Sahara"));
        countryCodeMap.insert(QLatin1String("YEM"), i18nc("@info: country code", "Yemen"));
        countryCodeMap.insert(QLatin1String("ZMB"), i18nc("@info: country code", "Zambia"));
        countryCodeMap.insert(QLatin1String("ZWE"), i18nc("@info: country code", "Zimbabwe"));
        // Supplemental IPTC/IIM country codes.
        countryCodeMap.insert(QLatin1String("XUN"), i18nc("@info: country code", "United Nations"));
        countryCodeMap.insert(QLatin1String("XEU"), i18nc("@info: country code", "European Union"));
        countryCodeMap.insert(QLatin1String("XSP"), i18nc("@info: country code", "Space"));
        countryCodeMap.insert(QLatin1String("XSE"), i18nc("@info: country code", "At Sea"));
        countryCodeMap.insert(QLatin1String("XIF"), i18nc("@info: country code", "In Flight"));
        countryCodeMap.insert(QLatin1String("XEN"), i18nc("@info: country code", "England"));
        countryCodeMap.insert(QLatin1String("XSC"), i18nc("@info: country code", "Scotland"));
        countryCodeMap.insert(QLatin1String("XNI"), i18nc("@info: country code", "Northern Ireland"));
        countryCodeMap.insert(QLatin1String("XWA"), i18nc("@info: country code", "Wales"));
        countryCodeMap.insert(QLatin1String("PSE"), i18nc("@info: country code", "Palestine"));
        countryCodeMap.insert(QLatin1String("GZA"), i18nc("@info: country code", "Gaza"));
        countryCodeMap.insert(QLatin1String("JRO"), i18nc("@info: country code", "Jericho"));
    }

    typedef QMap<QString, QString> CountryCodeMap;

    CountryCodeMap                 countryCodeMap;
};

CountrySelector::CountrySelector(QWidget* const parent)
    : QComboBox(parent),
      d        (new Private)
{
    for (Private::CountryCodeMap::Iterator it = d->countryCodeMap.begin() ;
         it != d->countryCodeMap.end() ; ++it)
    {
        addItem(QString::fromLatin1("%1 - %2").arg(it.key()).arg(it.value()));
    }

    model()->sort(0);

    insertSeparator(count());
    addItem(i18nc("@item: unknown country", "Unknown"));
}

CountrySelector::~CountrySelector()
{
    delete d;
}

void CountrySelector::setCountry(const QString& countryCode)
{
    // NOTE: if countryCode is empty or do not matches code map, unknown is selected from the list.

    int id = count() - 1;

    for (int i = 0 ; i < d->countryCodeMap.count() ; ++i)
    {
        if (itemText(i).left(3) == countryCode)
        {
            id = i;

            break;
        }
    }

    setCurrentIndex(id);

    qCDebug(DIGIKAM_WIDGETS_LOG) << count() << " :: " << id;
}

bool CountrySelector::country(QString& countryCode, QString& countryName) const
{
    // Unknown is selected ?

    if (currentIndex() == count()-1)
    {
        return false;
    }

    countryName = currentText().mid(6);
    countryCode = currentText().left(3);

    return true;
}

QString CountrySelector::countryForCode(const QString& countryCode)
{
    Private priv;

    return (priv.countryCodeMap[countryCode]);
}

} // namespace Digikam

#include "moc_countryselector.cpp"
