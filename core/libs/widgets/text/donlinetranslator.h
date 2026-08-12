/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-02-22
 * Description : A text translator using web-services.
 *
 * SPDX-FileCopyrightText: 2018-2022 by Hennadii Chernyshchyk <genaloner at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMap>
#include <QPointer>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>

// Local includes

#include "digikam_export.h"

class QStateMachine;
class QState;
class QNetworkAccessManager;
class QNetworkReply;

namespace Digikam
{

/**
 * @brief Contains translation options for a single word
 *
 * Can be obtained from the QOnlineTranslator object.
 *
 * Example:
 * @code
 * QOnlineTranslator translator;
 * // Obtain translation
 *
 * QTextStream out(stdout);
 *
 * for (auto it = translator.translationOptions().cbegin() ; it != translator.translationOptions().cend() ; ++it)
 * {
 *     out << it.key() << ":" << endl;  // Output the type of speech with a colon
 *
 *     for (const auto &[word, gender, translations] : it.value())
 *     {
 *         out << "  " << word << ": "; // Print the word
 *         out << translations;         // Print translations
 *         out << endl;
 *     }
 *
 *     out << endl;
 * }
 * @endcode
 *
 * Possible output:
 * @code
 * // verb:
 * //  sagen: say, tell, speak, mean, utter
 * //  sprechen: speak, talk, say, pronounce, militate, discourse
 * //  meinen: think, mean, believe, say, opine, fancy
 * //  heißen: mean, be called, be named, bid, tell, be titled
 * //  äußern: express, comment, speak, voice, say, utter
 * //  aussprechen: express, pronounce, say, speak, voice, enunciate
 * //  vorbringen: make, put forward, raise, say, put, bring forward
 * //  aufsagen: recite, say, speak
 *
 * // noun:
 * //  Sagen: say
 * //  Mitspracherecht: say
 * @endcode
 */
struct DIGIKAM_EXPORT DOnlineTranslatorOption
{
    /**
     * @brief Word that specified for translation options.
     */
    QString     word;

    /**
     * @brief Gender of the word.
     */
    QString     gender;

    /**
     * @brief Associated translations for the word.
     */
    QStringList translations;

    /**
     * @brief Converts the object to JSON
     *
     * @return JSON representation
     */
    QJsonObject toJson() const
    {
        QJsonObject object
        {
            { QLatin1String("gender"),       gender                                   },
            { QLatin1String("translations"), QJsonArray::fromStringList(translations) },
            { QLatin1String("word"),         word                                     },
        };

        return object;
    }
};

// ------------------------------------------------------------------

/**
 * @brief Provides translation data
 */
class DIGIKAM_EXPORT DOnlineTranslator : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DOnlineTranslator)

public:

    /**
     * @brief Represents all languages for translation
     */
    enum Language
    {
        NoLanguage = -1,
        Auto,
        Afrikaans,
        Albanian,
        Amharic,
        Arabic,
        Armenian,
        Azerbaijani,
        Bashkir,
        Basque,
        Belarusian,
        Bengali,
        Bosnian,
        Bulgarian,
        Cantonese,
        Catalan,
        Cebuano,
        Chichewa,
        Corsican,
        Croatian,
        Czech,
        Danish,
        Dutch,
        English,
        Esperanto,
        Estonian,
        Fijian,
        Filipino,
        Finnish,
        French,
        Frisian,
        Galician,
        Georgian,
        German,
        Greek,
        Gujarati,
        HaitianCreole,
        Hausa,
        Hawaiian,
        Hebrew,
        HillMari,
        Hindi,
        Hmong,
        Hungarian,
        Icelandic,
        Igbo,
        Indonesian,
        Irish,
        Italian,
        Japanese,
        Javanese,
        Kannada,
        Kazakh,
        Khmer,
        Kinyarwanda,
        Klingon,
        KlingonPlqaD,
        Korean,
        Kurdish,
        Kyrgyz,
        Lao,
        Latin,
        Latvian,
        LevantineArabic,
        Lithuanian,
        Luxembourgish,
        Macedonian,
        Malagasy,
        Malay,
        Malayalam,
        Maltese,
        Maori,
        Marathi,
        Mari,
        Mongolian,
        Myanmar,
        Nepali,
        Norwegian,
        Oriya,
        Papiamento,
        Pashto,
        Persian,
        Polish,
        Portuguese,
        Punjabi,
        QueretaroOtomi,
        Romanian,
        Russian,
        Samoan,
        ScotsGaelic,
        SerbianCyrillic,
        SerbianLatin,
        Sesotho,
        Shona,
        SimplifiedChinese,
        Sindhi,
        Sinhala,
        Slovak,
        Slovenian,
        Somali,
        Spanish,
        Sundanese,
        Swahili,
        Swedish,
        Tagalog,
        Tahitian,
        Tajik,
        Tamil,
        Tatar,
        Telugu,
        Thai,
        Tongan,
        TraditionalChinese,
        Turkish,
        Turkmen,
        Udmurt,
        Uighur,
        Ukrainian,
        Urdu,
        Uzbek,
        Vietnamese,
        Welsh,
        Xhosa,
        Yiddish,
        Yoruba,
        YucatecMaya,
        Zulu
    };
    Q_ENUM(Language)

    /**
     * @brief Represents online engines
     */
    enum Engine
    {
        Google,
        Yandex,
        Bing,
        Lingva,
        LibreTranslate
    };
    Q_ENUM(Engine)

    /**
     * @brief Indicates all possible error conditions found during the processing of the translation
     */
    enum TranslationError
    {
        /**
         * No error condition
         */
        NoError,

        /**
         * Unsupported combination of parameters
         */
        ParametersError,

        /**
         * Network error
         */
        NetworkError,

        /**
         * Service unavailable or maximum number of requests
         */
        ServiceError,

        /**
         * The request could not be parsed (report a bug if you see this)
         */
        ParsingError
    };
    Q_ENUM(TranslationError)

    /**
     * @brief Create object
     *
     * Constructs an object with empty data and with parent.
     * You can use translate() to send text to object.
     *
     * @param parent the parent object
     */
    explicit DOnlineTranslator(QObject* const parent = nullptr);
    ~DOnlineTranslator() override;

    /**
     * @brief Translate text
     *
     * @param text the text to translate
     * @param engine online engine to use
     * @param translationLang language to translation
     * @param sourceLang language of the passed text
     * @param uiLang ui language to use for display
     */
    void translate(const QString& text,
                   Engine engine = Google,
                   Language translationLang = Auto,
                   Language sourceLang = Auto,
                   Language uiLang = Auto);

    /**
     * @brief Detect language
     *
     * @param text the text for language detection
     * @param engine the engine to use
     */
    void detectLanguage(const QString& text,
                        Engine engine = Google);

    /**
     * @brief Cancel translation operation (if any).
     */
    void abort();

    /**
     * @brief Check translation progress
     *
     * @return `true` when the translation is still processing and has not finished or was aborted yet.
     */
    bool isRunning() const;

    /**
     * @brief Converts the object to JSON
     *
     * @return JSON representation
     */
    QJsonDocument toJson() const;

    /**
     * @brief Source text
     *
     * @return source text
     */
    QString source() const;

    /**
     * @brief Source transliteration
     *
     * @return transliteration of the source text
     */
    QString sourceTranslit() const;

    /**
     * @brief Source transcription
     *
     * @return transcription of the source text
     */
    QString sourceTranscription() const;

    /**
     * @brief Source language name
     *
     * @return language name of the source text
     */
    QString sourceLanguageName() const;

    /**
     * @brief Source language
     *
     * @return language of the source text
     */
    Language sourceLanguage() const;

    /**
     * Return the engine literal name.
     */
    static QString engineName(Engine engine);

    //@{
    // Properties methods (donlinetranslator_properties.cpp)

    /**
     * Convert language RFC3066 to supported language code
     */
    static QString fromRFC3066(Engine engine, const QString& langCodeRFC3066);

    /**
     * Return a list of all supported language in RFC3066.
     */
    static QStringList supportedRFC3066(Engine engine);

    /**
     * @brief Translated text
     *
     * @return translated text.
     */
    QString translation() const;

    /**
     * @brief Translation transliteration
     *
     * @return transliteration of the translated text
     */
    QString translationTranslit() const;

    /**
     * @brief Translation language name
     *
     * @return language name of the translated text
     */
    QString translationLanguageName() const;

    /**
     * @brief Translation language
     *
     * @return language of the translated text
     */
    Language translationLanguage() const;

    /**
     * @brief Translation options
     *
     * @return QMap whose key represents the type of speech, and the value is a QVector of translation options
     * @sa DOnlineTranslatorOption
     */
    QMap<QString, QVector<DOnlineTranslatorOption> > translationOptions() const;

    /**
     * @brief Language code
     *
     * @param lang language
     * @return language code
     */
    static QString languageCode(Language lang);

    /**
     * @brief Returns general language code
     *
     * @param langCode code
     * @return language
     */
    static Language language(const QString& langCode);

    /**
     * @brief Last error
     *
     * Error that was found during the processing of the last translation.
     * If no error was found, returns DOnlineTranslator::NoError.
     * The text of the error can be obtained by errorString().
     *
     * @return last error
     */
    TranslationError error() const;

    /**
     * @brief Last error string
     *
     * A human-readable description of the last translation error that occurred.
     *
     * @return last error string
     */
    QString errorString() const;

    /**
     * @brief Check if source transliteration is enabled
     *
     * @return `true` if source transliteration is enabled
     */
    bool isSourceTranslitEnabled() const;

    /**
     * @brief Check if translation transliteration is enabled
     *
     * @return `true` if translation transliteration is enabled
     */
    bool isTranslationTranslitEnabled() const;

    /**
     * @brief Check if source transcription is enabled
     *
     * @return `true` if source transcription is enabled
     */
    bool isSourceTranscriptionEnabled() const;

    /**
     * @brief Check if translation options are enabled
     *
     * @return `true` if translation options are enabled
     * @sa DOnlineTranslatorOption
     */
    bool isTranslationOptionsEnabled() const;

    /**
     * @brief Enable or disable source transliteration
     *
     * @param enable whether to enable source transliteration
     */
    void setSourceTranslitEnabled(bool enable);

    /**
     * @brief Enable or disable translation transliteration
     *
     * @param enable whether to enable translation transliteration
     */
    void setTranslationTranslitEnabled(bool enable);

    /**
     * @brief Enable or disable source transcription
     *
     * @param enable whether to enable source transcription
     */
    void setSourceTranscriptionEnabled(bool enable);

    /**
     * @brief Enable or disable translation options
     *
     * @param enable whether to enable translation options
     * @sa DOnlineTranslatorOption
     */
    void setTranslationOptionsEnabled(bool enable);

    /**
     * @brief Set the URL engine
     *
     * Only affects LibreTranslate and Lingva because these engines have multiple instances.
     * You need to call this function to specify the URL of an instance for them.
     *
     * @param engine the engine to use
     * @param url engine url
     */
    void setEngineUrl(Engine engine, const QString& url);

    /**
     * @brief Set api key for engine
     *
     * Affects only LibreTranslate.
     *
     * @param engine the engine to use
     * @param apiKey your key for this particular instance
     */
    void setEngineApiKey(Engine engine, const QByteArray& apiKey);

    //@}

    //@{
    // Conversion tables (donlinetranslator_tables.cpp)

    /**
     * @brief Language
     *
     * @param locale the locale to use
     * @return language
     */
    static Language language(const QLocale& locale);

    /**
     * @brief Language name
     *
     * @param lang language
     * @return language name
     */
    static QString languageName(Language lang);

    /**
     * @brief Check if transliteration is supported
     *
     * @param engine the engine to use
     * @param lang language
     * @return `true` if the specified engine supports transliteration for specified language
     */
    static bool isSupportTranslation(Engine engine, Language lang);

private:

    /**
     * Check for service support
     */
    static bool isSupportTranslit(Engine engine, Language lang);
    static bool isSupportDictionary(Engine engine, Language sourceLang, Language translationLang);

    //@}

Q_SIGNALS:

    /**
     * @brief Translation finished
     *
     * This signal is emitted when the translation is complete.
     */
    void signalFinished();

private Q_SLOTS:

    void slotSkipGarbageText();

    /**
     * @note Engines have translation limit, so need to split all text into parts and make request sequentially.
     * Also Yandex and Bing requires several requests to get dictionary, transliteration etc.
     * We use state machine to rely async computation with signals and slots.
     */

    //@{
    /// Google Web service specific methods (donlinetranslator_google.cpp).

private Q_SLOTS:

    void slotRequestGoogleTranslate();
    void slotParseGoogleTranslate();

private:

    void buildGoogleStateMachine();
    void buildGoogleDetectStateMachine();

    //@}

    //@{
    /// Yandex Web service specific methods (donlinetranslator_yandex.cpp).

private Q_SLOTS:

    void slotRequestYandexKey();
    void slotParseYandexKey();

    void slotRequestYandexTranslate();
    void slotParseYandexTranslate();

    void slotRequestYandexSourceTranslit();
    void slotParseYandexSourceTranslit();

    void slotRequestYandexTranslationTranslit();
    void slotParseYandexTranslationTranslit();

    void slotRequestYandexDictionary();
    void slotParseYandexDictionary();

private:

    void buildYandexStateMachine();
    void buildYandexDetectStateMachine();

    /**
     * Helper functions for Yandex transliteration
     */
    void requestYandexTranslit(Language language);
    void parseYandexTranslit(QString& text);

    //@}

    //@{
    /// Bing Web service specific methods (donlinntranslator_bing.cpp).

private Q_SLOTS:

    void slotRequestBingCredentials();
    void slotParseBingCredentials();

    void slotRequestBingTranslate();
    void slotParseBingTranslate();

    void slotRequestBingDictionary();
    void slotParseBingDictionary();

private:

    void buildBingStateMachine();
    void buildBingDetectStateMachine();

    //@}

    //@{
    /// LibreTranslate Web service specific methods ()donlinetranslator_libretr.cpp).

private Q_SLOTS:

    void slotRequestLibreLangDetection();
    void slotParseLibreLangDetection();

    void slotRequestLibreTranslate();
    void slotParseLibreTranslate();

private:

    void buildLibreStateMachine();
    void buildLibreDetectStateMachine();

    //@}

    //@{
    /// Lingva Web service specific methods (donlinetranslator_lingva.cpp).

private Q_SLOTS:

    void slotRequestLingvaTranslate();
    void slotParseLingvaTranslate();

private:

    void buildLingvaStateMachine();
    void buildLingvaDetectStateMachine();

    //@}

private:

    /**
     * Helper functions to build nested states
     */
    void buildSplitNetworkRequest(QState* const parent,
                                  void (DOnlineTranslator::*requestMethod)(),
                                  void (DOnlineTranslator::*parseMethod)(),
                                  const QString& text,
                                  int textLimit);
    void buildNetworkRequestState(QState* const parent,
                                  void (DOnlineTranslator::*requestMethod)(),
                                  void (DOnlineTranslator::*parseMethod)(),
                                  const QString& text = QString());


    void resetData(TranslationError error = NoError, const QString& errorString = QString());


    // Other

    /**
     * Returns engine-specific language code for translation
     */
    static QString languageApiCode(Engine engine, Language lang);

    /**
     * Parse language from response language code
     */
    static Language language(Engine engine, const QString& langCode);
    /**
     * Get split index of the text according to the limit
     */
    static int getSplitIndex(const QString& untranslatedText, int limit);

    static bool isContainsSpace(const QString& text);

    static void addSpaceBetweenParts(QString& text);

private:

    class Private;
    Private* const d = nullptr;

    friend class DOnlineTts;
};

// -------------------------------------------------------------------------------------------

/**
 * @brief Provides TTS URL generation
 *
 * Example:
 * @code
 *
 * DOnlineTts tts;
 * tts.generateUrls(QLatin1String("Hello World!"), DOnlineTranslator::Google, DOnlineTranslator::English);
 *
 * // Get list of Urls to play with media player.
 * QList<QUrl> urls = tts.media();
 *
 * @endcode
 */
class DIGIKAM_EXPORT DOnlineTts : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DOnlineTts)

public:

    /**
     * @brief Defines voice to use
     *
     * Used only by Yandex.
     */
    enum Voice
    {
        // All
        NoVoice = -1,

        // Yandex
        Zahar,
        Ermil,
        Jane,
        Oksana,
        Alyss,
        Omazh
    };
    Q_ENUM(Voice)

    /**
     * @brief Defines emotion to use
     *
     * Used only by Yandex.
     */
    enum Emotion
    {
        // All
        NoEmotion = -1,

        // Yandex
        Neutral,
        Good,
        Evil
    };
    Q_ENUM(Emotion)

    /**
     * @brief Indicates all possible error conditions found during the processing of the URLs generation
     */
    enum TtsError
    {
        /**
         * No error condition
         */
        NoError,
        /**
         * Specified engine does not support TTS
         */
        UnsupportedEngine,
        /**
         * Unsupported language by specified engine
         */
        UnsupportedLanguage,
        /**
         * Unsupported voice by specified engine
         */
        UnsupportedVoice,
        /**
         * Unsupported emotion by specified engine
         */
        UnsupportedEmotion,
    };
    Q_ENUM(TtsError)

    /**
     * @brief Create object
     *
     * Constructs an object with empty data and with parent.
     * You can use generateUrls() to create URLs for use in QMediaPlayer.
     *
     * @param parent the parent object
     */
    explicit DOnlineTts(QObject* const parent = nullptr);
    ~DOnlineTts() override;

    /**
     * @brief Create TTS urls
     *
     * Splits text into parts (engines have a limited number of characters per request) and returns list with the generated API URLs to play.
     *
     * @param text the text to speak
     * @param engine online translation engine
     * @param lang text language
     * @param voice the voice to use (used only by Yandex)
     * @param emotion the emotion to use (used only by Yandex)
     */
    void generateUrls(const QString& text,
                      DOnlineTranslator::Engine engine,
                      DOnlineTranslator::Language lang,
                      Voice voice = NoVoice,
                      Emotion emotion = NoEmotion);

    /**
     * @brief Generated media
     *
     * @return List of generated URLs
     */
    QList<QUrl> media() const;

    /**
     * @brief Last error
     *
     * Error that was found during the generating tts.
     * If no error was found, returns TtsError::NoError.
     * The text of the error can be obtained by errorString().
     *
     * @return last error
     */
    TtsError error() const;

    /**
     * @brief Last error string
     *
     * A human-readable description of the last tts URL generation error that occurred.
     *
     * @return last error string
     */
    QString errorString() const;

    /**
     * @brief Code of the voice
     *
     * @param voice the voice to use
     * @return code for voice
     */
    static QString voiceCode(Voice voice);

    /**
     * @brief Code of the emotion
     *
     * Used only by Yandex.
     *
     * @param emotion the emotion to use
     * @return code for emotion
     */
    static QString emotionCode(Emotion emotion);

    /**
     * @brief Emotion from code
     *
     * Used only by Yandex.
     *
     * @param emotionCode emotion code
     * @return corresponding emotion
     */
    static Emotion emotion(const QString& emotionCode);

    /**
     * @brief Voice from code
     *
     * Used only by Yandex.
     *
     * @param voiceCode voice code
     * @return corresponding voice
     */
    static Voice voice(const QString& voiceCode);

private:

    void setError(TtsError error, const QString& errorString);

    QString languageApiCode(DOnlineTranslator::Engine engine, DOnlineTranslator::Language lang);
    QString voiceApiCode(DOnlineTranslator::Engine engine, Voice voice);
    QString emotionApiCode(DOnlineTranslator::Engine engine, Emotion emotion);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
