/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-02-22
 * Description : A text translator using web-services - Private members.
 *
 * SPDX-FileCopyrightText: 2018-2022 by Hennadii Chernyshchyk <genaloner at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "donlinetranslator.h"

// Qt includes

#include <QCoreApplication>
#include <QFinalState>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QStateMachine>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "networkmanager.h"

namespace Digikam
{

class Q_DECL_HIDDEN DOnlineTranslator::Private
{
public:

    explicit Private(DOnlineTranslator* const parent);

public:

    QStateMachine*                                          stateMachine                 = nullptr;
    QNetworkAccessManager*                                  networkManager               = nullptr;
    QPointer<QNetworkReply>                                 currentReply;

    Language                                                sourceLang                   = NoLanguage;
    Language                                                translationLang              = NoLanguage;
    Language                                                uiLang                       = NoLanguage;
    TranslationError                                        error                        = NoError;

    QString                                                 source;
    QString                                                 sourceTranslit;
    QString                                                 sourceTranscription;
    QString                                                 translation;
    QString                                                 translationTranslit;
    QString                                                 errorString;

    /**
     * @brief self-hosted engines settings.
     * Can be empty, since free instances ignore api key parameter.
     */

    QByteArray                                              libreApiKey;
    QString                                                 libreUrl;
    QString                                                 lingvaUrl;

    QMap<QString, QVector<DOnlineTranslatorOption> >        translationOptions;

    bool                                                    sourceTranslitEnabled        = true;
    bool                                                    translationTranslitEnabled   = true;
    bool                                                    sourceTranscriptionEnabled   = true;
    bool                                                    translationOptionsEnabled    = true;

    bool                                                    onlyDetectLanguage           = false;

    /**
     *@brief credentials that are parsed from the web version to receive the translation using the API.
     */
    static inline QString                                   s_yandexKey;
    static inline QByteArray                                s_bingKey;
    static inline QByteArray                                s_bingToken;
    static inline QString                                   s_bingIg;
    static inline QString                                   s_bingIid;

    /**
     * @brief these properties are used to store useful information in states.
     */
    static constexpr char                                   s_textProperty[]               = "Text";

    /**
     * @brief Engines have a limit of characters per translation request.
     * If the query is larger, then it should be split into several with @method getSplitIndex() helper function.
     */
    static constexpr int                                    s_googleTranslateLimit         = 5000;
    static constexpr int                                    s_yandexTranslateLimit         = 150;
    static constexpr int                                    s_yandexTranslitLimit          = 180;
    static constexpr int                                    s_bingTranslateLimit           = 5001;
    static constexpr int                                    s_libreTranslateLimit          = 120;

    static const QMap<DOnlineTranslator::Language, QString> s_genericLanguageCodes;
    static const QMap<DOnlineTranslator::Language, QString> s_googleLanguageCodes;
    static const QMap<DOnlineTranslator::Language, QString> s_yandexLanguageCodes;
    static const QMap<DOnlineTranslator::Language, QString> s_bingLanguageCodes;
    static const QMap<DOnlineTranslator::Language, QString> s_lingvaLanguageCodes;

    static const QMap<QString, QString>                     s_rfc3066LanguageCodesGeneric;
    static const QMap<QString, QString>                     s_rfc3066LanguageCodesGoogle;
    static const QMap<QString, QString>                     s_rfc3066LanguageCodesYandex;
    static const QMap<QString, QString>                     s_rfc3066LanguageCodesBing;
    static const QMap<QString, QString>                     s_rfc3066LanguageCodesLingva;
};

// ---------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN DOnlineTts::Private
{
public:

    Private() = default;

public:

    QList<QUrl>                             media;
    QString                                 errorString;
    TtsError                                error            = NoError;

    static const QMap<Emotion, QString>     s_emotionCodes;
    static const QMap<Voice, QString>       s_voiceCodes;

    static constexpr int                    s_googleTtsLimit = 200;
    static constexpr int                    s_yandexTtsLimit = 1400;
};

} // namespace Digikam
