/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-15
 * Description : multi-languages string editor
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QMap>

// Local includes

#include "digikam_export.h"
#include "dmetadata.h"
#include "dtextedit.h"

namespace Digikam
{

class DIGIKAM_EXPORT AltLangStrEdit : public QWidget
{
    Q_OBJECT

public:

    /**
     * Default constructor. Use lines to use a specific number of lines with text editor.
     */
    explicit AltLangStrEdit(QWidget* const parent, unsigned int lines = 3);
    ~AltLangStrEdit()                               override;

    /**
     * Create a title widget with a QLabel and relevant text.
     * If a title widget already exists, it's replaced.
     */
    void setTitle(const QString& title);

    /**
     * Create a title with a specific widget instance (aka a QCheckBox for ex).
     * If a title widget already exists, it's replaced.
     */
    void setTitleWidget(QWidget* const twdg);

    /**
     * Return the current title widget instance.
     * If no previous call of setTitle() or setWidgetTitle(), this function will return nullptr.
     */
    QWidget* titleWidget()                   const;

    void setPlaceholderText(const QString& msg);

    void    setCurrentLanguageCode(const QString& lang);
    QString currentLanguageCode()            const;

    QString languageCode(int index)          const;

    /**
     * Fix lines visible in text editor to lines. If zero, do not fix layout to number of lines visible.
     */
    void setLinesVisible(uint lines);
    uint linesVisible()                      const;

    QString defaultAltLang()                 const;
    bool    asDefaultAltLang()               const;

    /**
     * Reset widget, clear all entries
     */
    void reset();

    /**
     * Ensure that the current language is added to the list of entries,
     * even if the text is empty.
     * signalValueAdded() will be emitted.
     */
    void addCurrent();

    DTextEdit* textEdit()                    const;

    MetaEngine::AltLangMap& values()         const;

    virtual void setValues(const MetaEngine::AltLangMap& values);

    /**
     * Return the literal name of RFC 3066 language code (format FR-fr for ex).
     */
    static QString languageNameRFC3066(const QString& code);

    /**
     * Return all language codes available following the RFC 3066.
     */
    static QStringList allLanguagesRFC3066();

Q_SIGNALS:

    /**
     * Emitted when the user changes the text for the current language.
     */
    void signalChanged(const QString& lang, const QString& text);

    /**
     * Emitted when the current language changed.
     */
    void signalSelectionChanged(const QString& lang);

    /**
     * Emitted when an entry for a new language is added.
     */
    void signalValueAdded(const QString& lang, const QString& text);

    /**
     * Emitted when the entry for a language is removed.
     */
    void signalValueDeleted(const QString& lang);

public Q_SLOTS:

    /**
     * Can be used to turn on/off visibility of internal widgets.
     * This do not includes the title widget.
     */
    void slotEnabledInternalWidgets(bool);

protected Q_SLOTS:

    void slotTextChanged();
    void slotSelectionChanged();
    void slotDeleteValue();

private Q_SLOTS:

    /**
     * Perform text translation with Web-service.
     */
    void slotTranslate(const QString& lang);

    void slotTranslationFinished();

protected:

    void populateLangAltListEntries();

    void changeEvent(QEvent* e)                 override;

private:

    class Private;
    Private* const d = nullptr;

    friend class Private;
};

} // namespace Digikam
