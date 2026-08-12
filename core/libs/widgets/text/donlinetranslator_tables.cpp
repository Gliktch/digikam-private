/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-02-22
 * Description : A text translator using web-services - Conversion tables.
 *
 * SPDX-FileCopyrightText: 2018-2022 by Hennadii Chernyshchyk <genaloner at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "donlinetranslator_p.h"

namespace Digikam
{

QString DOnlineTranslator::fromRFC3066(Engine engine, const QString& langCodeRFC3066)
{
    QString lg;

    switch (engine)
    {
        case Google:
        case LibreTranslate:
        {
            lg = DOnlineTranslator::Private::s_rfc3066LanguageCodesGoogle[langCodeRFC3066];
            break;
        }

        case Yandex:
        {
            lg = DOnlineTranslator::Private::s_rfc3066LanguageCodesYandex[langCodeRFC3066];
            break;
        }

        case Bing:
        {
            lg = DOnlineTranslator::Private::s_rfc3066LanguageCodesBing[langCodeRFC3066];
            break;
        }

        case Lingva:
        {
            lg = DOnlineTranslator::Private::s_rfc3066LanguageCodesLingva[langCodeRFC3066];
            break;
        }
    }

    if (lg.isEmpty())
    {
        lg = DOnlineTranslator::Private::s_rfc3066LanguageCodesGeneric[langCodeRFC3066];
    }

    return lg;
}

QStringList DOnlineTranslator::supportedRFC3066(Engine engine)
{
    QStringList lst = DOnlineTranslator::Private::s_rfc3066LanguageCodesGeneric.keys();

    switch (engine)
    {
        case Google:
        case LibreTranslate:
        {
            lst << DOnlineTranslator::Private::s_rfc3066LanguageCodesGoogle.keys();
            break;
        }

        case Yandex:
        {
            lst << DOnlineTranslator::Private::s_rfc3066LanguageCodesYandex.keys();
            break;
        }

        case Bing:
        {
            lst << DOnlineTranslator::Private::s_rfc3066LanguageCodesBing.keys();
            break;
        }

        case Lingva:
        {
            lst << DOnlineTranslator::Private::s_rfc3066LanguageCodesLingva.keys();
            break;
        }
    }

    lst.removeDuplicates();

    return lst;
}

QString DOnlineTranslator::languageName(Language lang)
{
    switch (lang)
    {
        case Auto:
            return i18nc("@info: language name", "Automatically detect");

        case Afrikaans:
            return i18nc("@info: language name", "Afrikaans");

        case Albanian:
            return i18nc("@info: language name", "Albanian");

        case Amharic:
            return i18nc("@info: language name", "Amharic");

        case Arabic:
            return i18nc("@info: language name", "Arabic");

        case Armenian:
            return i18nc("@info: language name", "Armenian");

        case Azerbaijani:
            return i18nc("@info: language name", "Azeerbaijani");

        case Basque:
            return i18nc("@info: language name", "Basque");

        case Bashkir:
            return i18nc("@info: language name", "Bashkir");

        case Belarusian:
            return i18nc("@info: language name", "Belarusian");

        case Bengali:
            return i18nc("@info: language name", "Bengali");

        case Bosnian:
            return i18nc("@info: language name", "Bosnian");

        case Bulgarian:
            return i18nc("@info: language name", "Bulgarian");

        case Catalan:
            return i18nc("@info: language name", "Catalan");

        case Cantonese:
            return i18nc("@info: language name", "Cantonese");

        case Cebuano:
            return i18nc("@info: language name", "Cebuano");

        case SimplifiedChinese:
            return i18nc("@info: language name", "Chinese (Simplified)");

        case TraditionalChinese:
            return i18nc("@info: language name", "Chinese (Traditional)");

        case Corsican:
            return i18nc("@info: language name", "Corsican");

        case Croatian:
            return i18nc("@info: language name", "Croatian");

        case Czech:
            return i18nc("@info: language name", "Czech");

        case Danish:
            return i18nc("@info: language name", "Danish");

        case Dutch:
            return i18nc("@info: language name", "Dutch");

        case English:
            return i18nc("@info: language name", "English");

        case Esperanto:
            return i18nc("@info: language name", "Esperanto");

        case Estonian:
            return i18nc("@info: language name", "Estonian");

        case Fijian:
            return i18nc("@info: language name", "Fijian");

        case Filipino:
            return i18nc("@info: language name", "Filipino");

        case Finnish:
            return i18nc("@info: language name", "Finnish");

        case French:
            return i18nc("@info: language name", "French");

        case Frisian:
            return i18nc("@info: language name", "Frisian");

        case Galician:
            return i18nc("@info: language name", "Galician");

        case Georgian:
            return i18nc("@info: language name", "Georgian");

        case German:
            return i18nc("@info: language name", "German");

        case Greek:
            return i18nc("@info: language name", "Greek");

        case Gujarati:
            return i18nc("@info: language name", "Gujarati");

        case HaitianCreole:
            return i18nc("@info: language name", "Haitian Creole");

        case Hausa:
            return i18nc("@info: language name", "Hausa");

        case Hawaiian:
            return i18nc("@info: language name", "Hawaiian");

        case Hebrew:
            return i18nc("@info: language name", "Hebrew");

        case HillMari:
            return i18nc("@info: language name", "Hill Mari");

        case Hindi:
            return i18nc("@info: language name", "Hindi");

        case Hmong:
            return i18nc("@info: language name", "Hmong");

        case Hungarian:
            return i18nc("@info: language name", "Hungarian");

        case Icelandic:
            return i18nc("@info: language name", "Icelandic");

        case Igbo:
            return i18nc("@info: language name", "Igbo");

        case Indonesian:
            return i18nc("@info: language name", "Indonesian");

        case Irish:
            return i18nc("@info: language name", "Irish");

        case Italian:
            return i18nc("@info: language name", "Italian");

        case Japanese:
            return i18nc("@info: language name", "Japanese");

        case Javanese:
            return i18nc("@info: language name", "Javanese");

        case Kannada:
            return i18nc("@info: language name", "Kannada");

        case Kazakh:
            return i18nc("@info: language name", "Kazakh");

        case Khmer:
            return i18nc("@info: language name", "Khmer");

        case Kinyarwanda:
            return i18nc("@info: language name", "Kinyarwanda");

        case Klingon:
            return i18nc("@info: language name", "Klingon");

        case KlingonPlqaD:
            return i18nc("@info: language name", "Klingon (PlqaD)");

        case Korean:
            return i18nc("@info: language name", "Korean");

        case Kurdish:
            return i18nc("@info: language name", "Kurdish");

        case Kyrgyz:
            return i18nc("@info: language name", "Kyrgyz");

        case Lao:
            return i18nc("@info: language name", "Lao");

        case Latin:
            return i18nc("@info: language name", "Latin");

        case Latvian:
            return i18nc("@info: language name", "Latvian");

        case LevantineArabic:
            return i18nc("@info: language name", "Levantine Arabic");

        case Lithuanian:
            return i18nc("@info: language name", "Lithuanian");

        case Luxembourgish:
            return i18nc("@info: language name", "Luxembourgish");

        case Macedonian:
            return i18nc("@info: language name", "Macedonian");

        case Malagasy:
            return i18nc("@info: language name", "Malagasy");

        case Malay:
            return i18nc("@info: language name", "Malay");

        case Malayalam:
            return i18nc("@info: language name", "Malayalam");

        case Maltese:
            return i18nc("@info: language name", "Maltese");

        case Maori:
            return i18nc("@info: language name", "Maori");

        case Marathi:
            return i18nc("@info: language name", "Marathi");

        case Mari:
            return i18nc("@info: language name", "Mari");

        case Mongolian:
            return i18nc("@info: language name", "Mongolian");

        case Myanmar:
            return i18nc("@info: language name", "Myanmar");

        case Nepali:
            return i18nc("@info: language name", "Nepali");

        case Norwegian:
            return i18nc("@info: language name", "Norwegian");

        case Oriya:
            return i18nc("@info: language name", "Oriya");

        case Chichewa:
            return i18nc("@info: language name", "Chichewa");

        case Papiamento:
            return i18nc("@info: language name", "Papiamento");

        case Pashto:
            return i18nc("@info: language name", "Pashto");

        case Persian:
            return i18nc("@info: language name", "Persian");

        case Polish:
            return i18nc("@info: language name", "Polish");

        case Portuguese:
            return i18nc("@info: language name", "Portuguese");

        case Punjabi:
            return i18nc("@info: language name", "Punjabi");

        case QueretaroOtomi:
            return i18nc("@info: language name", "Queretaro Otomi");

        case Romanian:
            return i18nc("@info: language name", "Romanian");

        case Russian:
            return i18nc("@info: language name", "Russian");

        case Samoan:
            return i18nc("@info: language name", "Samoan");

        case ScotsGaelic:
            return i18nc("@info: language name", "Scots Gaelic");

        case SerbianCyrillic:
            return i18nc("@info: language name", "Serbian (Cyrillic)");

        case SerbianLatin:
            return i18nc("@info: language name", "Serbian (Latin)");

        case Sesotho:
            return i18nc("@info: language name", "Sesotho");

        case Shona:
            return i18nc("@info: language name", "Shona");

        case Sindhi:
            return i18nc("@info: language name", "Sindhi");

        case Sinhala:
            return i18nc("@info: language name", "Sinhala");

        case Slovak:
            return i18nc("@info: language name", "Slovak");

        case Slovenian:
            return i18nc("@info: language name", "Slovenian");

        case Somali:
            return i18nc("@info: language name", "Somali");

        case Spanish:
            return i18nc("@info: language name", "Spanish");

        case Sundanese:
            return i18nc("@info: language name", "Sundanese");

        case Swahili:
            return i18nc("@info: language name", "Swahili");

        case Swedish:
            return i18nc("@info: language name", "Swedish");

        case Tagalog:
            return i18nc("@info: language name", "Tagalog");

        case Tahitian:
            return i18nc("@info: language name", "Tahitian");

        case Tajik:
            return i18nc("@info: language name", "Tajik");

        case Tamil:
            return i18nc("@info: language name", "Tamil");

        case Tatar:
            return i18nc("@info: language name", "Tatar");

        case Telugu:
            return i18nc("@info: language name", "Telugu");

        case Thai:
            return i18nc("@info: language name", "Thai");

        case Tongan:
            return i18nc("@info: language name", "Tongan");

        case Turkish:
            return i18nc("@info: language name", "Turkish");

        case Turkmen:
            return i18nc("@info: language name", "Turkmen");

        case Udmurt:
            return i18nc("@info: language name", "Udmurt");

        case Uighur:
            return i18nc("@info: language name", "Uighur");

        case Ukrainian:
            return i18nc("@info: language name", "Ukrainian");

        case Urdu:
            return i18nc("@info: language name", "Urdu");

        case Uzbek:
            return i18nc("@info: language name", "Uzbek");

        case Vietnamese:
            return i18nc("@info: language name", "Vietnamese");

        case Welsh:
            return i18nc("@info: language name", "Welsh");

        case Xhosa:
            return i18nc("@info: language name", "Xhosa");

        case Yiddish:
            return i18nc("@info: language name", "Yiddish");

        case Yoruba:
            return i18nc("@info: language name", "Yoruba");

        case YucatecMaya:
            return i18nc("@info: language name", "Yucatec Maya");

        case Zulu:
            return i18nc("@info: language name", "Zulu");

        default:
            return QString();
    }
}

DOnlineTranslator::Language DOnlineTranslator::language(const QLocale& locale)
{
    switch (locale.language())
    {
        case QLocale::Afrikaans:
            return Afrikaans;

        case QLocale::Albanian:
            return Albanian;

        case QLocale::Amharic:
            return Amharic;

        case QLocale::Arabic:
            return Arabic;

        case QLocale::Armenian:
            return Armenian;

        case QLocale::Azerbaijani:
            return Azerbaijani;

        case QLocale::Basque:
            return Basque;

        case QLocale::Belarusian:
            return Belarusian;

        case QLocale::Bengali:
            return Bengali;

        case QLocale::Bosnian:
            return Bosnian;

        case QLocale::Bulgarian:
            return Bulgarian;

        case QLocale::Catalan:
            return Catalan;

        case QLocale::Chinese:
            return SimplifiedChinese;

        case QLocale::LiteraryChinese:
            return TraditionalChinese;

        case QLocale::Corsican:
            return Corsican;

        case QLocale::Croatian:
            return Croatian;

        case QLocale::Czech:
            return Czech;

        case QLocale::Danish:
            return Danish;

        case QLocale::Dutch:
            return Dutch;

        case QLocale::Esperanto:
            return Esperanto;

        case QLocale::Estonian:
            return Estonian;

        case QLocale::Finnish:
            return Finnish;

        case QLocale::French:
            return French;

        case QLocale::Frisian:
            return Frisian;

        case QLocale::Galician:
            return Galician;

        case QLocale::Georgian:
            return Georgian;

        case QLocale::German:
            return German;

        case QLocale::Greek:
            return Greek;

        case QLocale::Gujarati:
            return Gujarati;

        case QLocale::Haitian:
            return HaitianCreole;

        case QLocale::Hausa:
            return Hausa;

        case QLocale::Hawaiian:
            return Hawaiian;

        case QLocale::Hebrew:
            return Hebrew;

        case QLocale::Hindi:
            return Hindi;

        case QLocale::Hungarian:
            return Hungarian;

        case QLocale::Icelandic:
            return Icelandic;

        case QLocale::Igbo:
            return Igbo;

        case QLocale::Indonesian:
            return Indonesian;

        case QLocale::Irish:
            return Irish;

        case QLocale::Italian:
            return Italian;

        case QLocale::Japanese:
            return Japanese;

        case QLocale::Javanese:
            return Javanese;

        case QLocale::Kannada:
            return Kannada;

        case QLocale::Kazakh:
            return Kazakh;

        case QLocale::Khmer:
            return Khmer;

        case QLocale::Kinyarwanda:
            return Kinyarwanda;

        case QLocale::Korean:
            return Korean;

        case QLocale::Kurdish:
            return Kurdish;

        case QLocale::Lao:
            return Lao;

        case QLocale::Latin:
            return Latin;

        case QLocale::Latvian:
            return Latvian;

        case QLocale::Lithuanian:
            return Lithuanian;

        case QLocale::Luxembourgish:
            return Luxembourgish;

        case QLocale::Macedonian:
            return Macedonian;

        case QLocale::Malagasy:
            return Malagasy;

        case QLocale::Malay:
            return Malay;

        case QLocale::Malayalam:
            return Malayalam;

        case QLocale::Maltese:
            return Maltese;

        case QLocale::Maori:
            return Maori;

        case QLocale::Marathi:
            return Marathi;

        case QLocale::Mongolian:
            return Mongolian;

        case QLocale::Nepali:
            return Nepali;

        case QLocale::NorwegianBokmal:
            return Norwegian;

        case QLocale::Oriya:
            return Oriya;

        case QLocale::Pashto:
            return Pashto;

        case QLocale::Persian:
            return Persian;

        case QLocale::Polish:
            return Polish;

        case QLocale::Portuguese:
            return Portuguese;

        case QLocale::Punjabi:
            return Punjabi;

        case QLocale::Romanian:
            return Romanian;

        case QLocale::Russian:
            return Russian;

        case QLocale::Samoan:
            return Samoan;

        case QLocale::Gaelic:
            return ScotsGaelic;

        case QLocale::Serbian:
            return SerbianCyrillic;

        case QLocale::Shona:
            return Shona;

        case QLocale::Sindhi:
            return Sindhi;

        case QLocale::Sinhala:
            return Sinhala;

        case QLocale::Slovak:
            return Slovak;

        case QLocale::Slovenian:
            return Slovenian;

        case QLocale::Somali:
            return Somali;

        case QLocale::Spanish:
            return Spanish;

        case QLocale::Sundanese:
            return Sundanese;

        case QLocale::Swahili:
            return Swahili;

        case QLocale::Swedish:
            return Swedish;

        case QLocale::Filipino:
            return Filipino;

        case QLocale::Tajik:
            return Tajik;

        case QLocale::Tamil:
            return Tamil;

        case QLocale::Tatar:
            return Tatar;

        case QLocale::Telugu:
            return Telugu;

        case QLocale::Thai:
            return Thai;

        case QLocale::Turkish:
            return Turkish;

        case QLocale::Turkmen:
            return Turkmen;

        case QLocale::Uighur:
            return Uighur;

        case QLocale::Ukrainian:
            return Ukrainian;

        case QLocale::Urdu:
            return Urdu;

        case QLocale::Uzbek:
            return Uzbek;

        case QLocale::Vietnamese:
            return Vietnamese;

        case QLocale::Welsh:
            return Welsh;

        case QLocale::Xhosa:
            return Xhosa;

        case QLocale::Yiddish:
            return Yiddish;

        case QLocale::Yoruba:
            return Yoruba;

        case QLocale::Zulu:
            return Zulu;

        default:
            return English;
    }
}

bool DOnlineTranslator::isSupportTranslation(Engine engine, Language lang)
{
    bool isSupported = false;

    switch (engine)
    {
        case Google:
        case Lingva: // Lingva is a frontend to Google Translate

            switch (lang)
            {
                case NoLanguage:
                case Bashkir:
                case Cantonese:
                case Fijian:
                case Filipino:
                case Georgian:
                case HillMari:
                case Klingon:
                case KlingonPlqaD:
                case LevantineArabic:
                case Mari:
                case Papiamento:
                case QueretaroOtomi:
                case SerbianLatin:
                case Tahitian:
                case Tongan:
                case Udmurt:
                case YucatecMaya:
                {
                    isSupported = false;
                    break;
                }

                default:
                {
                    isSupported = true;
                    break;
                }
            }

            break;

        case Yandex:

            switch (lang)
            {
                case NoLanguage:
                case Cantonese:
                case Chichewa:
                case Corsican:
                case Fijian:
                case Filipino:
                case Frisian:
                case Hausa:
                case Hawaiian:
                case Igbo:
                case Kinyarwanda:
                case Klingon:
                case KlingonPlqaD:
                case Kurdish:
                case LevantineArabic:
                case Oriya:
                case Pashto:
                case QueretaroOtomi:
                case Samoan:
                case SerbianLatin:
                case Sesotho:
                case Shona:
                case Sindhi:
                case Somali:
                case Tahitian:
                case Tongan:
                case Turkmen:
                case Uighur:
                case Yoruba:
                case YucatecMaya:
                case Zulu:
                {
                    isSupported = false;
                    break;
                }

                default:
                {
                    isSupported = true;
                    break;
                }
            }

            break;

        case Bing:

            switch (lang)
            {
                case NoLanguage:
                case Albanian:
                case Amharic:
                case Armenian:
                case Azerbaijani:
                case Basque:
                case Bashkir:
                case Belarusian:
                case Cebuano:
                case Corsican:
                case Esperanto:
                case Frisian:
                case Galician:
                case Georgian:
                case Gujarati:
                case Hausa:
                case Hawaiian:
                case HillMari:
                case Igbo:
                case Irish:
                case Javanese:
                case Kannada:
                case Kazakh:
                case Khmer:
                case Kinyarwanda:
                case Kurdish:
                case Kyrgyz:
                case Lao:
                case Latin:
                case Luxembourgish:
                case Macedonian:
                case Malayalam:
                case Maori:
                case Marathi:
                case Mari:
                case Mongolian:
                case Myanmar:
                case Nepali:
                case Oriya:
                case Chichewa:
                case Papiamento:
                case Pashto:
                case Punjabi:
                case ScotsGaelic:
                case Sesotho:
                case Shona:
                case Sindhi:
                case Sinhala:
                case Somali:
                case Sundanese:
                case Tagalog:
                case Tajik:
                case Tatar:
                case Turkmen:
                case Uighur:
                case Udmurt:
                case Uzbek:
                case Xhosa:
                case Yiddish:
                case Yoruba:
                case Zulu:
                {
                    isSupported = false;
                    break;
                }

                default:
                {
                    isSupported = true;
                    break;
                }
            }

            break;

        case LibreTranslate:

            switch (lang)
            {
                case NoLanguage:
                case Afrikaans:
                case Albanian:
                case Amharic:
                case Armenian:
                case Azerbaijani:
                case Bashkir:
                case Basque:
                case Belarusian:
                case Bengali:
                case Bosnian:
                case Bulgarian:
                case Cantonese:
                case Catalan:
                case Cebuano:
                case Chichewa:
                case Corsican:
                case Croatian:
                case Czech:
                case Danish:
                case Dutch:
                case Esperanto:
                case Estonian:
                case Fijian:
                case Filipino:
                case Finnish:
                case Frisian:
                case Galician:
                case Georgian:
                case Greek:
                case Gujarati:
                case HaitianCreole:
                case Hausa:
                case Hawaiian:
                case Hebrew:
                case HillMari:
                case Hmong:
                case Hungarian:
                case Icelandic:
                case Igbo:
                case Javanese:
                case Kannada:
                case Kazakh:
                case Khmer:
                case Kinyarwanda:
                case Klingon:
                case KlingonPlqaD:
                case Kurdish:
                case Kyrgyz:
                case Lao:
                case Latin:
                case Latvian:
                case LevantineArabic:
                case Lithuanian:
                case Luxembourgish:
                case Macedonian:
                case Malagasy:
                case Malay:
                case Malayalam:
                case Maltese:
                case Maori:
                case Marathi:
                case Mari:
                case Mongolian:
                case Myanmar:
                case Nepali:
                case Norwegian:
                case Oriya:
                case Papiamento:
                case Pashto:
                case Persian:
                case Punjabi:
                case QueretaroOtomi:
                case Romanian:
                case Samoan:
                case ScotsGaelic:
                case SerbianCyrillic:
                case SerbianLatin:
                case Sesotho:
                case Shona:
                case SimplifiedChinese:
                case Sindhi:
                case Sinhala:
                case Slovak:
                case Slovenian:
                case Somali:
                case Sundanese:
                case Swahili:
                case Swedish:
                case Tagalog:
                case Tahitian:
                case Tajik:
                case Tamil:
                case Tatar:
                case Telugu:
                case Thai:
                case Tongan:
                case Turkmen:
                case Udmurt:
                case Uighur:
                case Ukrainian:
                case Urdu:
                case Uzbek:
                case Welsh:
                case Xhosa:
                case Yiddish:
                case Yoruba:
                case YucatecMaya:
                case Zulu:
                {
                    isSupported = false;
                    break;
                }

                default:
                {
                    isSupported = true;
                    break;
                }
            }

            break;
    }

    return isSupported;
}

bool DOnlineTranslator::isSupportTranslit(Engine engine, Language lang)
{
    switch (engine)
    {
        case Google:
        {
            // Google supports transliteration for all supported languages

            return isSupportTranslation(Google, lang);
        }

        case Yandex:
        {
            switch (lang)
            {
                case Amharic:
                case Armenian:
                case Bengali:
                case SimplifiedChinese:
                case Georgian:
                case Greek:
                case Gujarati:
                case Hebrew:
                case Hindi:
                case Japanese:
                case Kannada:
                case Korean:
                case Malayalam:
                case Marathi:
                case Nepali:
                case Punjabi:
                case Russian:
                case Sinhala:
                case Tamil:
                case Telugu:
                case Thai:
                case Yiddish:
                {
                    return true;
                }

                default:
                {
                    return false;
                }
            }
        }

        case Bing:
        {
            switch (lang)
            {
                case Arabic:
                case Bengali:
                case Gujarati:
                case Hebrew:
                case Hindi:
                case Japanese:
                case Kannada:
                case Malayalam:
                case Marathi:
                case Punjabi:
                case SerbianCyrillic:
                case SerbianLatin:
                case Tamil:
                case Telugu:
                case Thai:
                case SimplifiedChinese:
                case TraditionalChinese:
                {
                    return true;
                }

                default:
                {
                    return false;
                }
            }
        }

        case LibreTranslate:    // LibreTranslate doesn't support translit
        case Lingva:            // Although Lingvo is a frontend to Google Translate, it doesn't support transliteration
        {
            return false;
        }
    }

    return false;
}

bool DOnlineTranslator::isSupportDictionary(Engine engine, Language sourceLang, Language translationLang)
{
    switch (engine)
    {
        case Google:
        {
            // Google supports dictionary for all supported languages

            return isSupportTranslation(Google, sourceLang) && isSupportTranslation(Google, translationLang);
        }

        case Yandex:
        {
            switch (sourceLang)
            {
                case Belarusian:
                {
                    switch (translationLang)
                    {
                        case Belarusian:
                        case Russian:
                        {
                            return true;
                        }

                        default:
                        {
                            return false;
                        }
                    }
                }

                case Bulgarian:
                {
                    switch (translationLang)
                    {
                        case Russian:
                        {
                            return true;
                        }

                        default:
                        {
                            return false;
                        }
                    }
                }

                case Czech:
                case Danish:
                case Dutch:
                case Estonian:
                case Greek:
                case Latvian:
                case Norwegian:
                case Portuguese:
                case Slovak:
                case Swedish:
                {
                    switch (translationLang)
                    {
                        case English:
                        case Russian:
                        {
                            return true;
                        }

                        default:
                        {
                            return false;
                        }
                    }
                }

                case German:
                {
                    switch (translationLang)
                    {
                        case German:
                        case English:
                        case Russian:
                        case Turkish:
                        {
                            return true;
                        }

                        default:
                        {
                            return false;
                        }
                    }
                }

                case English:
                {
                    switch (translationLang)
                    {
                        case Czech:
                        case Danish:
                        case German:
                        case Greek:
                        case English:
                        case Spanish:
                        case Estonian:
                        case Finnish:
                        case French:
                        case Italian:
                        case Lithuanian:
                        case Latvian:
                        case Dutch:
                        case Norwegian:
                        case Portuguese:
                        case Russian:
                        case Slovak:
                        case Swedish:
                        case Turkish:
                        case Ukrainian:
                        {
                            return true;
                        }

                        default:
                        {
                            return false;
                        }
                    }
                }

                case Spanish:
                {
                    switch (translationLang)
                    {
                        case English:
                        case Spanish:
                        case Russian:
                        {
                            return true;
                        }

                        default:
                        {
                            return false;
                        }
                    }
                }

                case Finnish:
                {
                    switch (translationLang)
                    {
                        case English:
                        case Russian:
                        case Finnish:
                        {
                            return true;
                        }

                        default:
                        {
                            return false;
                        }
                    }
                }

                case French:
                {
                    switch (translationLang)
                    {
                        case French:
                        case English:
                        case Russian:
                        {
                            return true;
                        }

                        default:
                        {
                            return false;
                        }
                    }
                }

                case Hungarian:
                {
                    switch (translationLang)
                    {
                        case Hungarian:
                        case Russian:
                        {
                            return true;
                        }

                        default:
                        {
                            return false;
                        }
                    }
                }

                case Italian:
                {
                    switch (translationLang)
                    {
                        case English:
                        case Italian:
                        case Russian:
                        {
                            return true;
                        }

                        default:
                        {
                            return false;
                        }
                    }
                }

                case Lithuanian:
                {
                    switch (translationLang)
                    {
                        case English:
                        case Lithuanian:
                        case Russian:
                        {
                            return true;
                        }

                        default:
                        {
                            return false;
                        }
                    }
                }

                case Mari:
                case HillMari:
                case Polish:
                case Tatar:
                {
                    switch (translationLang)
                    {
                        case Russian:
                        {
                            return true;
                        }

                        default:
                        {
                            return false;
                        }
                    }
                }

                case Russian:
                {
                    switch (translationLang)
                    {
                        case Belarusian:
                        case Bulgarian:
                        case Czech:
                        case Danish:
                        case German:
                        case Greek:
                        case English:
                        case Spanish:
                        case Estonian:
                        case Finnish:
                        case French:
                        case Italian:
                        case Lithuanian:
                        case Latvian:
                        case Mari:
                        case HillMari:
                        case Dutch:
                        case Norwegian:
                        case Portuguese:
                        case Russian:
                        case Slovak:
                        case Swedish:
                        case Turkish:
                        case Tatar:
                        case Ukrainian:
                        {
                            return true;
                        }

                        default:
                        {
                            return false;
                        }
                    }

                    case Turkish:
                    {
                        switch (translationLang)
                        {
                            case German:
                            case English:
                            case Russian:
                            {
                                return true;
                            }

                            default:
                            {
                                return false;
                            }
                        }
                    }

                    case Ukrainian:
                    {
                        switch (translationLang)
                        {
                            case English:
                            case Russian:
                            case Ukrainian:
                            {
                                return true;
                            }

                            default:
                            {
                                return false;
                            }
                        }
                    }

                    default:
                    {
                        return false;
                    }
                }
            }
        }

        case Bing:
        {
            // Bing support dictionary only to or from English

            Language secondLang;

            if      (sourceLang == English)
            {
                secondLang = translationLang;
            }
            else if (translationLang == English)
            {
                secondLang = sourceLang;
            }
            else
            {
                return false;
            }

            switch (secondLang)
            {
                case Afrikaans:
                case Arabic:
                case Bengali:
                case Bosnian:
                case Bulgarian:
                case Catalan:
                case SimplifiedChinese:
                case Croatian:
                case Czech:
                case Danish:
                case Dutch:
                case Estonian:
                case Finnish:
                case French:
                case German:
                case Greek:
                case HaitianCreole:
                case Hebrew:
                case Hindi:
                case Hmong:
                case Hungarian:
                case Icelandic:
                case Indonesian:
                case Italian:
                case Japanese:
                case Swahili:
                case Klingon:
                case Korean:
                case Latvian:
                case Lithuanian:
                case Malay:
                case Maltese:
                case Norwegian:
                case Persian:
                case Polish:
                case Portuguese:
                case Romanian:
                case Russian:
                case SerbianLatin:
                case Slovak:
                case Slovenian:
                case Spanish:
                case Swedish:
                case Tamil:
                case Thai:
                case Turkish:
                case Ukrainian:
                case Urdu:
                case Vietnamese:
                case Welsh:
                {
                    return true;
                }

                default:
                {
                    return false;
                }
            }
        }

        case LibreTranslate: // LibreTranslate doesn't support dictinaries
        case Lingva:         // Although Lingvo is a frontend to Google Translate, it doesn't support dictionaries
        {
            return false;
        }
    }

    return false;
}

} // namespace Digikam
