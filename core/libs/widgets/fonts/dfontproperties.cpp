/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-12-23
 * Description : a widget to change font properties.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 1996      by Bernd Johannes Wuebben  <wuebben at kde dot org>
 * SPDX-FileCopyrightText: 1999      by Preston Brown <pbrown at kde dot org>
 * SPDX-FileCopyrightText: 1999      by Mario Weilguni <mweilguni at kde dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dfontproperties.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QGuiApplication>
#include <QLabel>
#include <QLayout>
#include <QLocale>
#include <QSplitter>
#include <QScrollBar>
#include <QFontDatabase>
#include <QGroupBox>
#include <QListWidget>
#include <QCoreApplication>
#include <QString>
#include <QHash>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dtextedit.h"

namespace Digikam
{

static bool localeLessThan(const QString& a, const QString& b)
{
    return (QString::localeAwareCompare(a, b) < 0);
}

static int minimumListWidth(const QListWidget* list)
{
    int w = 0;

    for (int i = 0 ; i < list->count() ; ++i)
    {
        int itemWidth = list->visualItemRect(list->item(i)).width();

        // ...and add a space on both sides for not too tight look.

        itemWidth    += list->fontMetrics().horizontalAdvance(QLatin1Char(' ')) * 2;

        w             = qMax(w, itemWidth);
    }

    if (w == 0)
    {
        w = 40;
    }

    w += list->frameWidth() * 2;
    w += list->verticalScrollBar()->sizeHint().width();

    return w;
}

static int minimumListHeight(const QListWidget* list, int numVisibleEntry)
{
    int w = (list->count() > 0) ? list->visualItemRect(list->item(0)).height()
                                : list->fontMetrics().lineSpacing();

    if (w < 0)
    {
        w = 10;
    }

    if (numVisibleEntry <= 0)
    {
        numVisibleEntry = 4;
    }

    return (w * numVisibleEntry + 2 * list->frameWidth());
}

static QString formatFontSize(qreal size)
{
    return QLocale::system().toString(size, 'f', (size == floor(size)) ? 0 : 1);
}

// -----------------------------------------------------------------------------------

class Q_DECL_HIDDEN DFontProperties::Private
{
public:

    explicit Private(DFontProperties* const qq)
        : q(qq)
    {
        palette.setColor(QPalette::Active, QPalette::Text, Qt::black);
        palette.setColor(QPalette::Active, QPalette::Base, Qt::white);
    }

    void    setFamilyBoxItems(const QStringList& fonts);
    void    fillFamilyListBox(bool onlyFixedFonts = false);
    int     nearestSizeRow(qreal val, bool customize);
    qreal   fillSizeList(const QList<qreal>& sizes = QList<qreal>());
    qreal   setupSizeListBox(const QString& family, const QString& style);
    void    setupDisplay();
    QString styleIdentifier(const QFont& font);

    /**
     * Split the compound raw font name into family and foundry.
     *
     * @param name the raw font name reported by Qt
     * @param family the storage for family name
     * @param foundry the storage for foundry name
     */
    void splitFontString(const QString& name, QString* family, QString* foundry = nullptr);

    /**
     * Translate the font name for the user.
     * Primarily for generic fonts like Serif, Sans-Serif, etc.
     *
     * @param name the raw font name reported by Qt
     * @return translated font name
     */
    QString translateFontName(const QString& name);

    /**
     * Compose locale-aware sorted list of translated font names,
     * with generic fonts handled in a special way.
     * The mapping of translated to raw names can be reported too if required.
     *
     * @param names raw font names as reported by Qt
     * @param trToRawNames storage for mapping of translated to raw names
     * @return sorted list of translated font names
     */
    QStringList translateFontNameList(const QStringList& names, QHash<QString, QString>* trToRawNames = nullptr);

    void _d_toggled_checkbox();
    void _d_family_chosen_slot(const QString&);
    void _d_size_chosen_slot(const QString&);
    void _d_style_chosen_slot(const QString&);
    void _d_displaySample(const QFont& font);
    void _d_size_value_slot(double);

public:

    DFontProperties*        q                       = nullptr;

    QPalette                palette;
    QDoubleSpinBox*         sizeOfFont              = nullptr;
    DTextEdit*              sampleEdit              = nullptr;

    QLabel*                 familyLabel             = nullptr;
    QLabel*                 styleLabel              = nullptr;
    QCheckBox*              familyCheckbox          = nullptr;
    QCheckBox*              styleCheckbox           = nullptr;
    QCheckBox*              sizeCheckbox            = nullptr;
    QLabel*                 sizeLabel               = nullptr;
    QListWidget*            familyListBox           = nullptr;
    QListWidget*            styleListBox            = nullptr;
    QListWidget*            sizeListBox             = nullptr;
    QCheckBox*              sizeIsRelativeCheckBox  = nullptr;

    QFont                   selFont;

    QString                 selectedStyle;
    qreal                   selectedSize            = -1.0;

    QString                 standardSizeAtCustom;
    int                     customSizeRow           = -1;

    bool                    signalsAllowed          = true;
    bool                    usingFixed              = true;

    /**
     * Mappings of translated to Qt originated family and style strings.
     */
    QHash<QString, QString> qtFamilies;
    QHash<QString, QString> qtStyles;

    /**
     * Mapping of translated style strings to internal style identifiers.
     */
    QHash<QString, QString> styleIDs;
};

DFontProperties::DFontProperties(QWidget* const parent,
                                 const DisplayFlags& flags,
                                 const QStringList& fontList,
                                 int visibleListSize,
                                 Qt::CheckState* const sizeIsRelativeState)
    : QWidget(parent),
      d      (new DFontProperties::Private(this))
{
    d->usingFixed = flags & FixedFontsOnly;
    setWhatsThis(i18nc("@title: font properties", "Here you can choose the font to be used."));

    // The top layout is divided vertically into a splitter with font
    // attribute widgets and preview on the top, and XLFD data at the bottom.

    QVBoxLayout* const topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins(QMargins());
    const int spacingHint        = layoutSpacing();
    int checkBoxGap              = spacingHint / 2;

    // The splitter contains font attribute widgets in the top part,
    // and the font preview in the bottom part.
    // The splitter is there to allow the user to resize the font preview.

    QSplitter* const splitter    = new QSplitter(Qt::Vertical, this);
    splitter->setChildrenCollapsible(false);
    topLayout->addWidget(splitter);

    // Build the grid of font attribute widgets for the upper splitter part.

    QWidget* page           = nullptr;
    QGridLayout* gridLayout = nullptr;
    int row                 = 0;

    if (flags & DisplayFrame)
    {
        page       = new QGroupBox(i18nc("@title: font properties", "Requested Font"), this);
        splitter->addWidget(page);
        gridLayout = new QGridLayout(page);
        row        = 1;
    }
    else
    {
        page       = new QWidget(this);
        splitter->addWidget(page);
        gridLayout = new QGridLayout(page);
        gridLayout->setContentsMargins(QMargins());
    }

    // First, create the labels across the top.

    QHBoxLayout* const familyLayout = new QHBoxLayout();
    familyLayout->addSpacing(checkBoxGap);

    if (flags & ShowDifferences)
    {
        d->familyCheckbox = new QCheckBox(i18nc("@title: font properties", "Font"), page);

        connect(d->familyCheckbox, SIGNAL(toggled(bool)),
                this, SLOT(_d_toggled_checkbox()));

        familyLayout->addWidget(d->familyCheckbox, 0, Qt::AlignLeft);
        d->familyCheckbox->setWhatsThis(i18nc("@info: font properties", "Enable this checkbox to change the font family settings."));
        d->familyCheckbox->setToolTip(i18nc("@info: font properties", "Change font family?"));
        d->familyLabel = nullptr;
    }
    else
    {
        d->familyCheckbox = nullptr;
        d->familyLabel    = new QLabel(i18nc("@label: font properties", "Font:"), page);
        familyLayout->addWidget(d->familyLabel, 1, Qt::AlignLeft);
    }

    gridLayout->addLayout(familyLayout, row, 0);

    QHBoxLayout* const styleLayout = new QHBoxLayout();

    if (flags & ShowDifferences)
    {
        d->styleCheckbox = new QCheckBox(i18nc("@title: font properties", "Font style"), page);

        connect(d->styleCheckbox, SIGNAL(toggled(bool)),
                this, SLOT(_d_toggled_checkbox()));

        styleLayout->addWidget(d->styleCheckbox, 0, Qt::AlignLeft);
        d->styleCheckbox->setWhatsThis(i18nc("@info: font properties", "Enable this checkbox to change the font style settings."));
        d->styleCheckbox->setToolTip(i18nc("@info: font properties", "Change font style?"));
        d->styleLabel = nullptr;
    }
    else
    {
        d->styleCheckbox = nullptr;
        d->styleLabel    = new QLabel(i18nc("@label: font properties", "Font style:"), page);
        styleLayout->addWidget(d->styleLabel, 1, Qt::AlignLeft);
    }

    styleLayout->addSpacing(checkBoxGap);
    gridLayout->addLayout(styleLayout, row, 1);

    QHBoxLayout* const sizeLayout = new QHBoxLayout();

    if (flags & ShowDifferences)
    {
        d->sizeCheckbox = new QCheckBox(i18nc("@title: font properties", "Size"), page);

        connect(d->sizeCheckbox, SIGNAL(toggled(bool)),
                this, SLOT(_d_toggled_checkbox()));

        sizeLayout->addWidget(d->sizeCheckbox, 0, Qt::AlignLeft);
        d->sizeCheckbox->setWhatsThis(i18nc("@info: font properties", "Enable this checkbox to change the font size settings."));
        d->sizeCheckbox->setToolTip(i18nc("@info: font properties", "Change font size?"));
        d->sizeLabel = nullptr;
    }
    else
    {
        d->sizeCheckbox = nullptr;
        d->sizeLabel    = new QLabel(i18nc("@label:listbox Font size", "Size:"), page);
        sizeLayout->addWidget(d->sizeLabel, 1, Qt::AlignLeft);
    }

    sizeLayout->addSpacing(checkBoxGap);
    sizeLayout->addSpacing(checkBoxGap);        // prevent label from eating border
    gridLayout->addLayout(sizeLayout, row, 2);

    row ++;

    // Now create the actual boxes that hold the info.

    d->familyListBox = new QListWidget(page);
    d->familyListBox->setEnabled(flags ^ ShowDifferences);
    gridLayout->addWidget(d->familyListBox, row, 0);
    QString fontFamilyWhatsThisText(i18nc("@info: font properties", "Here you can choose the font family to be used."));
    d->familyListBox->setWhatsThis(fontFamilyWhatsThisText);

    if (flags & ShowDifferences)
    {
        d->familyCheckbox->setWhatsThis(fontFamilyWhatsThisText);
    }
    else
    {
        d->familyLabel->setWhatsThis(fontFamilyWhatsThisText);
    }

    connect(d->familyListBox, SIGNAL(currentTextChanged(QString)),
            this, SLOT(_d_family_chosen_slot(QString)));

    if (!fontList.isEmpty())
    {
        d->setFamilyBoxItems(fontList);
    }
    else
    {
        d->fillFamilyListBox(flags & FixedFontsOnly);
    }

    d->familyListBox->setMinimumWidth(minimumListWidth(d->familyListBox));
    d->familyListBox->setMinimumHeight(minimumListHeight(d->familyListBox, visibleListSize));

    d->styleListBox = new QListWidget(page);
    d->styleListBox->setEnabled(flags ^ ShowDifferences);
    gridLayout->addWidget(d->styleListBox, row, 1);
    d->styleListBox->setWhatsThis(i18nc("@info: font properties", "Here you can choose the font style to be used."));

    if (flags & ShowDifferences)
    {
        (static_cast<QWidget*>(d->styleCheckbox))->setWhatsThis(fontFamilyWhatsThisText);
    }
    else
    {
        (static_cast<QWidget*>(d->styleLabel))->setWhatsThis(fontFamilyWhatsThisText);
    }

    // Populate usual styles, to determine minimum list width;
    // will be replaced later with correct styles.

    d->styleListBox->addItem(i18nc("@item: font style", "Normal"));
    d->styleListBox->addItem(i18nc("@item: font style", "Italic"));
    d->styleListBox->addItem(i18nc("@item: font style", "Oblique"));
    d->styleListBox->addItem(i18nc("@item: font style", "Bold"));
    d->styleListBox->addItem(i18nc("@item: font style", "Bold Italic"));
    d->styleListBox->setMinimumWidth(minimumListWidth(d->styleListBox));
    d->styleListBox->setMinimumHeight(minimumListHeight(d->styleListBox, visibleListSize));

    connect(d->styleListBox, SIGNAL(currentTextChanged(QString)),
            this, SLOT(_d_style_chosen_slot(QString)));

    d->sizeListBox = new QListWidget(page);
    d->sizeOfFont  = new QDoubleSpinBox(page);
    d->sizeOfFont->setMinimum(4);
    d->sizeOfFont->setMaximum(999);
    d->sizeOfFont->setDecimals(1);
    d->sizeOfFont->setSingleStep(1);

    d->sizeListBox->setEnabled(flags ^ ShowDifferences);
    d->sizeOfFont->setEnabled(flags ^ ShowDifferences);

    if (sizeIsRelativeState)
    {
        QString sizeIsRelativeCBText          = i18nc("@item: font size", "Relative");
        QString sizeIsRelativeCBToolTipText   = i18nc("@info: font properties",
                                                      "Font size \"fixed\" or \"relative\" to environment");
        QString sizeIsRelativeCBWhatsThisText = i18nc("@info: font properties", "Here you can switch between fixed font size and font size "
                                                      "to be calculated dynamically and adjusted to changing "
                                                      "environment (e.g. widget dimensions, paper size).");
        d->sizeIsRelativeCheckBox             = new QCheckBox(sizeIsRelativeCBText, page);
        d->sizeIsRelativeCheckBox->setTristate(flags & ShowDifferences);
        QGridLayout* const sizeLayout2        = new QGridLayout();
        sizeLayout2->setSpacing(spacingHint / 2);
        gridLayout->addLayout(sizeLayout2, row, 2);
        sizeLayout2->setColumnStretch(1, 1);                    // to prevent text from eating the right border
        sizeLayout2->addWidget(d->sizeOfFont,  0, 0, 1, 2);
        sizeLayout2->addWidget(d->sizeListBox, 1, 0, 1, 2);
        sizeLayout2->addWidget(d->sizeIsRelativeCheckBox, 2, 0, Qt::AlignLeft);
        d->sizeIsRelativeCheckBox->setWhatsThis(sizeIsRelativeCBWhatsThisText);
        d->sizeIsRelativeCheckBox->setToolTip(sizeIsRelativeCBToolTipText);
    }
    else
    {
        d->sizeIsRelativeCheckBox      = nullptr;
        QGridLayout* const sizeLayout2 = new QGridLayout();
        sizeLayout2->setSpacing(spacingHint / 2);
        gridLayout->addLayout(sizeLayout2, row, 2);
        sizeLayout2->addWidget(d->sizeOfFont,  0, 0);
        sizeLayout2->addWidget(d->sizeListBox, 1, 0);
    }

    QString fontSizeWhatsThisText = i18nc("@info: font properties", "Here you can choose the font size to be used.");
    d->sizeListBox->setWhatsThis(fontSizeWhatsThisText);

    if (flags & ShowDifferences)
    {
        (static_cast<QWidget*>(d->sizeCheckbox))->setWhatsThis(fontSizeWhatsThisText);
    }
    else
    {
        (static_cast<QWidget*>(d->sizeLabel))->setWhatsThis(fontSizeWhatsThisText);
    }

    // Populate with usual sizes, to determine minimum list width;
    // will be replaced later with correct sizes.

    d->fillSizeList();
    d->sizeListBox->setMinimumWidth(minimumListWidth(d->sizeListBox) + d->sizeListBox->fontMetrics().maxWidth());
    d->sizeListBox->setMinimumHeight(minimumListHeight(d->sizeListBox, visibleListSize));

    connect(d->sizeOfFont, SIGNAL(valueChanged(double)),
            this, SLOT(_d_size_value_slot(double)));

    connect(d->sizeListBox, SIGNAL(currentTextChanged(QString)),
            this, SLOT(_d_size_chosen_slot(QString)));

    row ++;

    // Completed the font attribute grid.
    // Add the font preview into the lower part of the splitter.

    d->sampleEdit = new DTextEdit(0, page);
    d->sampleEdit->setAcceptRichText(false);
    QFont tmpFont(font().family(), 64, QFont::Black);
    d->sampleEdit->setFont(tmpFont);
    d->sampleEdit->setMinimumHeight(d->sampleEdit->fontMetrics().lineSpacing());

    // tr: A classical test phrase, with all letters of the English alphabet.
    // Replace it with a sample text in your language, such that it is
    // representative of language's writing system.
    // If you wish, you can input several lines of text separated by \n.

    setSampleText(i18nc("@info: font properties", "The Quick Brown Fox Jumps Over The Lazy Dog"));
    d->sampleEdit->setTextCursor(QTextCursor(d->sampleEdit->document()));
    QString sampleEditWhatsThisText = i18nc("@info: font properties", "This sample text illustrates the current settings. "
                                            "You may edit it to test special characters.");
    d->sampleEdit->setWhatsThis(sampleEditWhatsThisText);

    connect(this, SIGNAL(fontSelected(QFont)),
            this, SLOT(_d_displaySample(QFont)));

    splitter->addWidget(d->sampleEdit);

    // Finished setting up the splitter.
    // Finished setting up the chooser layout.
    // Lets initialize the display if possible.

    if (d->usingFixed)
    {
        setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont), d->usingFixed);
    }
    else
    {
        setFont(QGuiApplication::font(), d->usingFixed);
    }

    // Check or uncheck or gray out the "relative" checkbox.

    if (sizeIsRelativeState && d->sizeIsRelativeCheckBox)
    {
        setSizeIsRelative(*sizeIsRelativeState);
    }

    // Set focus to the size list as this is the most commonly changed property.

    d->sizeListBox->setFocus();
}

DFontProperties::~DFontProperties()
{
    delete d;
}

void DFontProperties::setColor(const QColor& col)
{
    d->palette.setColor(QPalette::Active, QPalette::Text, col);
    QPalette pal       = d->sampleEdit->palette();
    pal.setColor(QPalette::Active, QPalette::Text, col);
    d->sampleEdit->setPalette(pal);
    QTextCursor cursor = d->sampleEdit->textCursor();
    d->sampleEdit->selectAll();
    d->sampleEdit->setTextColor(col);
    d->sampleEdit->setTextCursor(cursor);
}

QColor DFontProperties::color() const
{
    return d->palette.color(QPalette::Active, QPalette::Text);
}

void DFontProperties::setBackgroundColor(const QColor& col)
{
    d->palette.setColor(QPalette::Active, QPalette::Base, col);
    QPalette pal = d->sampleEdit->palette();
    pal.setColor(QPalette::Active, QPalette::Base, col);
    d->sampleEdit->setPalette(pal);
}

QColor DFontProperties::backgroundColor() const
{
    return d->palette.color(QPalette::Active, QPalette::Base);
}

void DFontProperties::setSizeIsRelative(Qt::CheckState relative)
{
    // Check or uncheck or gray out the "relative" checkbox.

    if (d->sizeIsRelativeCheckBox)
    {
        if (Qt::PartiallyChecked == relative)
        {
            d->sizeIsRelativeCheckBox->setCheckState(Qt::PartiallyChecked);
        }
        else
        {
            d->sizeIsRelativeCheckBox->setCheckState((Qt::Checked == relative) ? Qt::Checked : Qt::Unchecked);
        }
    }
}

Qt::CheckState DFontProperties::sizeIsRelative() const
{
    return d->sizeIsRelativeCheckBox ? d->sizeIsRelativeCheckBox->checkState()
                                     : Qt::PartiallyChecked;
}

QString DFontProperties::sampleText() const
{
    return d->sampleEdit->toPlainText();
}

void DFontProperties::setSampleText(const QString& text)
{
    d->sampleEdit->setPlainText(text);
}

void DFontProperties::setSampleBoxVisible(bool visible)
{
    d->sampleEdit->setVisible(visible);
}

QSize DFontProperties::sizeHint(void) const
{
    return minimumSizeHint();
}

void DFontProperties::enableColumn(int column, bool state)
{
    if (column & FamilyList)
    {
        d->familyListBox->setEnabled(state);
    }

    if (column & StyleList)
    {
        d->styleListBox->setEnabled(state);
    }

    if (column & SizeList)
    {
        d->sizeListBox->setEnabled(state);
        d->sizeOfFont->setEnabled(state);
    }
}

void DFontProperties::makeColumnVisible(int column, bool state)
{
    if (column & FamilyList)
    {
        d->familyListBox->setVisible(state);
        d->familyLabel->setVisible(state);
    }

    if (column & StyleList)
    {
        d->styleListBox->setVisible(state);
        d->styleLabel->setVisible(state);
    }

    if (column & SizeList)
    {
        d->sizeListBox->setVisible(state);
        d->sizeOfFont->setVisible(state);
        d->sizeLabel->setVisible(state);
    }

}
void DFontProperties::setFont(const QFont& aFont, bool onlyFixed)
{
    d->selFont      = aFont;
    d->selectedSize = aFont.pointSizeF();

    if (d->selectedSize == -1)
    {
        d->selectedSize = QFontInfo(aFont).pointSizeF();
    }

    if (onlyFixed != d->usingFixed)
    {
        d->usingFixed = onlyFixed;
        d->fillFamilyListBox(d->usingFixed);
    }

    d->setupDisplay();
}

DFontProperties::FontDiffFlags DFontProperties::fontDiffFlags() const
{
    FontDiffFlags diffFlags = NoFontDiffFlags;

    if (d->familyCheckbox && d->familyCheckbox->isChecked())
    {
        diffFlags |= FontDiffFamily;
    }

    if (d->styleCheckbox && d->styleCheckbox->isChecked())
    {
        diffFlags |= FontDiffStyle;
    }

    if (d->sizeCheckbox && d->sizeCheckbox->isChecked())
    {
        diffFlags |= FontDiffSize;
    }

    return diffFlags;
}

QFont DFontProperties::font() const
{
    return d->selFont;
}

void DFontProperties::Private::_d_toggled_checkbox()
{
    familyListBox->setEnabled(familyCheckbox->isChecked());
    styleListBox->setEnabled(styleCheckbox->isChecked());
    sizeListBox->setEnabled(sizeCheckbox->isChecked());
    sizeOfFont->setEnabled(sizeCheckbox->isChecked());
}

void DFontProperties::Private::_d_family_chosen_slot(const QString& family)
{
    if (!signalsAllowed)
    {
        return;
    }

    signalsAllowed = false;

    QString currentFamily;

    if (family.isEmpty())
    {
        Q_ASSERT(familyListBox->currentItem());

        if (familyListBox->currentItem())
        {
            currentFamily = qtFamilies[familyListBox->currentItem()->text()];
        }
    }
    else
    {
        currentFamily = qtFamilies[family];
    }

    // Get the list of styles available in this family.

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    QStringList styles = QFontDatabase::styles(currentFamily);

#else

    QStringList styles = QFontDatabase().styles(currentFamily);

#endif

    if (styles.isEmpty())
    {
        styles.append(i18nc("@info: font style", "Normal"));
    }

    // Filter style strings and add to the listbox.

    QString pureFamily;
    splitFontString(family, &pureFamily);
    QStringList filteredStyles;
    qtStyles.clear();
    styleIDs.clear();

    for (const QString& style : std::as_const(styles))
    {
        // Sometimes the font database will report an invalid style,
        // that falls back to another when set.
        // Remove such styles, by checking set/get round-trip.

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        QFont testFont = QFontDatabase::font(currentFamily, style, 10);

        if (QFontDatabase::styleString(testFont) != style)

#else

        QFont testFont = QFontDatabase().font(currentFamily, style, 10);

        if (QFontDatabase().styleString(testFont) != style)

#endif

        {
            styles.removeAll(style);
            continue;
        }

        QString fstyle = style;

        if (!filteredStyles.contains(fstyle))
        {
            filteredStyles.append(fstyle);
            qtStyles.insert(fstyle, style);
            styleIDs.insert(fstyle, styleIdentifier(testFont));
        }
    }

    styleListBox->clear();
    styleListBox->addItems(filteredStyles);

    // Try to set the current style in the listbox to that previous.

    int listPos = filteredStyles.indexOf(selectedStyle.isEmpty() ? i18nc("@info: font style", "Normal")
                                                                 : selectedStyle);

    if (listPos < 0)
    {
        // Make extra effort to have Italic selected when Oblique was chosen,
        // and vice versa, as that is what the user would probably want.

        QString styleIt = i18nc("@info: font style", "Italic");
        QString styleOb = i18nc("@info: font style", "Oblique");

        for (int i = 0 ; i < 2 ; ++i)
        {
            int pos = selectedStyle.indexOf(styleIt);

            if (pos >= 0)
            {
                QString style = selectedStyle;
                style.replace(pos, styleIt.length(), styleOb);
                listPos       = filteredStyles.indexOf(style);

                if (listPos >= 0)
                {
                    break;
                }
            }

            std::swap(styleIt, styleOb);
        }
    }

    styleListBox->setCurrentRow((listPos >= 0) ? listPos : 0);
    QString currentStyle = qtStyles[styleListBox->currentItem()->text()];

    // Recompute the size listbox for this family/style.

    qreal currentSize = setupSizeListBox(currentFamily, currentStyle);
    sizeOfFont->setValue(currentSize);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    selFont           = QFontDatabase::font(currentFamily, currentStyle, int(currentSize));

    if (QFontDatabase::isSmoothlyScalable(currentFamily, currentStyle) && (selFont.pointSize() == floor(currentSize)))

#else

    selFont           = QFontDatabase().font(currentFamily, currentStyle, int(currentSize));

    if (QFontDatabase().isSmoothlyScalable(currentFamily, currentStyle) && (selFont.pointSize() == floor(currentSize)))

#endif

    {
        selFont.setPointSizeF(currentSize);
    }

    Q_EMIT q->fontSelected(selFont);

    signalsAllowed = true;
}

void DFontProperties::Private::_d_style_chosen_slot(const QString& style)
{
    if (!signalsAllowed)
    {
        return;
    }

    signalsAllowed = false;

    QString currentFamily = qtFamilies[familyListBox->currentItem()->text()];
    QString currentStyle;

    if (style.isEmpty())
    {
        currentStyle = qtStyles[styleListBox->currentItem()->text()];
    }
    else
    {
        currentStyle = qtStyles[style];
    }

    // Recompute the size listbox for this family/style.

    qreal currentSize = setupSizeListBox(currentFamily, currentStyle);
    sizeOfFont->setValue(currentSize);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    selFont           = QFontDatabase::font(currentFamily, currentStyle, int(currentSize));

    if (QFontDatabase::isSmoothlyScalable(currentFamily, currentStyle) && (selFont.pointSize() == floor(currentSize)))

#else

    selFont           = QFontDatabase().font(currentFamily, currentStyle, int(currentSize));

    if (QFontDatabase().isSmoothlyScalable(currentFamily, currentStyle) && (selFont.pointSize() == floor(currentSize)))

#endif

    {
        selFont.setPointSizeF(currentSize);
    }

    Q_EMIT q->fontSelected(selFont);

    if (!style.isEmpty())
    {
        selectedStyle = currentStyle;
    }

    signalsAllowed = true;
}

void DFontProperties::Private::_d_size_chosen_slot(const QString& size)
{
    if (!signalsAllowed)
    {
        return;
    }

    signalsAllowed = false;

    qreal currentSize;

    if (size.isEmpty())
    {
        currentSize = QLocale::system().toDouble(sizeListBox->currentItem()->text());
    }
    else
    {
        currentSize = QLocale::system().toDouble(size);
    }

    // Reset the customized size slot in the list if not needed.

    if ((customSizeRow >= 0) && (selFont.pointSizeF() != currentSize))
    {
        sizeListBox->item(customSizeRow)->setText(standardSizeAtCustom);
        customSizeRow = -1;
    }

    sizeOfFont->setValue(currentSize);
    selFont.setPointSizeF(currentSize);

    Q_EMIT q->fontSelected(selFont);

    if (!size.isEmpty())
    {
        selectedSize = currentSize;
    }

    signalsAllowed = true;
}

void DFontProperties::Private::_d_size_value_slot(double dval)
{
    if (!signalsAllowed)
    {
        return;
    }

    signalsAllowed = false;

    // We compare with qreal, so convert for platforms where qreal != double.

    qreal val      = qreal(dval);
    QString family = qtFamilies[familyListBox->currentItem()->text()];
    QString style  = qtStyles[styleListBox->currentItem()->text()];

    // Reset current size slot in list if it was customized.

    if ((customSizeRow >= 0) && (sizeListBox->currentRow() == customSizeRow))
    {
        sizeListBox->item(customSizeRow)->setText(standardSizeAtCustom);
        customSizeRow = -1;
    }

    bool canCustomize = true;

    // For Qt-bad-sizes workaround: skip this block unconditionally.

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    if (!QFontDatabase::isSmoothlyScalable(family, style))

#else

    if (!QFontDatabase().isSmoothlyScalable(family, style))

#endif

    {
        // Bitmap font, allow only discrete sizes.
        // Determine the nearest in the direction of change.

        canCustomize = false;
        int nrows    = sizeListBox->count();
        int row      = sizeListBox->currentRow();
        int nrow;

        if (val - selFont.pointSizeF() > 0)
        {
            for (nrow = row + 1 ; nrow < nrows ; ++nrow)
            {
                if (QLocale::system().toDouble(sizeListBox->item(nrow)->text()) >= val)
                {
                    break;
                }
            }
        }
        else
        {
            for (nrow = row - 1 ; nrow >= 0 ; --nrow)
            {
                if (QLocale::system().toDouble(sizeListBox->item(nrow)->text()) <= val)
                {
                    break;
                }
            }
        }

        // Make sure the new row is not out of bounds.

        nrow = (nrow < 0) ? 0
                          : (nrow >= nrows) ? nrows - 1
                                            : nrow;

        // Get the size from the new row and set the spinbox to that size.

        val  = QLocale::system().toDouble(sizeListBox->item(nrow)->text());
        sizeOfFont->setValue(val);
    }

    // Set the current size in the size listbox.

    int row        = nearestSizeRow(val, canCustomize);
    sizeListBox->setCurrentRow(row);

    selectedSize   = val;
    selFont.setPointSizeF(val);

    Q_EMIT q->fontSelected(selFont);

    signalsAllowed = true;
}

void DFontProperties::Private::_d_displaySample(const QFont& font)
{
    sampleEdit->setFont(font);
}

int DFontProperties::Private::nearestSizeRow(qreal val, bool customize)
{
    qreal diff = 1000;
    int row    = 0;

    for (int r = 0 ; r < sizeListBox->count() ; ++r)
    {
        qreal cval = QLocale::system().toDouble(sizeListBox->item(r)->text());

        if (qAbs(cval - val) < diff)
        {
            diff = qAbs(cval - val);
            row  = r;
        }
    }

    // For Qt-bad-sizes workaround: ignore value of customize, use true.

    if (customize && diff > 0)
    {
        customSizeRow        = row;
        standardSizeAtCustom = sizeListBox->item(row)->text();
        sizeListBox->item(row)->setText(formatFontSize(val));
    }

    return row;
}

qreal DFontProperties::Private::fillSizeList(const QList<qreal>& sizes_)
{
    if (!sizeListBox)
    {
        return 0;
    }

    QList<qreal> sizes = sizes_;
    bool canCustomize  = false;

    if (sizes.count() == 0)
    {
        static const int c[] =
        {
            4,  5,  6,  7,
            8,  9,  10, 11,
            12, 13, 14, 15,
            16, 17, 18, 19,
            20, 22, 24, 26,
            28, 32, 48, 64,
            72, 80, 96, 128,
            0
        };

        for (int i = 0 ; c[i] ; ++i)
        {
            sizes.append(c[i]);
        }

        // Since sizes were not supplied, this is a vector font,
        // and size slot customization is allowed.

        canCustomize = true;
    }

    // Insert sizes into the listbox.

    sizeListBox->clear();

    std::sort(sizes.begin(), sizes.end());

    for (qreal size : std::as_const(sizes))
    {
        sizeListBox->addItem(formatFontSize(size));
    }

    // Return the nearest to selected size.
    // If the font is vector, the nearest size is always same as selected,
    // thus size slot customization is allowed.
    // If the font is bitmap, the nearest size need not be same as selected,
    // thus size slot customization is not allowed.

    customSizeRow = -1;
    int row       = nearestSizeRow(selectedSize, canCustomize);

    return QLocale::system().toDouble(sizeListBox->item(row)->text());
}

qreal DFontProperties::Private::setupSizeListBox(const QString& family, const QString& style)
{
    QList<qreal>  sizes;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    if (QFontDatabase::isSmoothlyScalable(family, style))

#else

    if (QFontDatabase().isSmoothlyScalable(family, style))

#endif

    {
        // A vector font.
        //sampleEdit->setPaletteBackgroundPixmap( VectorPixmap ); // TODO
    }
    else
    {
        // A bitmap font.
        //sampleEdit->setPaletteBackgroundPixmap( BitmapPixmap ); // TODO

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        QList<int> smoothSizes = QFontDatabase::smoothSizes(family, style);

#else

        QList<int> smoothSizes = QFontDatabase().smoothSizes(family, style);

#endif

        for (int size : std::as_const(smoothSizes))
        {
            sizes.append(size);
        }
    }

    // Fill the listbox (uses default list of sizes if the given is empty).
    // Collect the best fitting size to selected size, to use if not smooth.

    qreal bestFitSize = fillSizeList(sizes);

    // Set the best fit size as current in the listbox if available.

    const QList<QListWidgetItem*> selectedSizeList = sizeListBox->findItems(formatFontSize(bestFitSize),
                                                                            Qt::MatchExactly);
    if (!selectedSizeList.isEmpty())
    {
        sizeListBox->setCurrentItem(selectedSizeList.first());
    }

    return bestFitSize;
}

void DFontProperties::Private::setupDisplay()
{
    QString family  = selFont.family().toLower();
    QString styleID = styleIdentifier(selFont);
    qreal size      = selFont.pointSizeF();

    if (size == -1)
    {
        size = QFontInfo(selFont).pointSizeF();
    }

    int numEntries, i;

    // Direct family match.

    numEntries = familyListBox->count();

    for (i = 0 ; i < numEntries ; ++i)
    {
        if (family == qtFamilies[familyListBox->item(i)->text()].toLower())
        {
            familyListBox->setCurrentRow(i);
            break;
        }
    }

    // 1st family fallback.

    if (i == numEntries)
    {
        if (family.contains(QLatin1Char('[')))
        {
            family = family.left(family.indexOf(QLatin1Char('['))).trimmed();

            for (i = 0 ; i < numEntries ; ++i)
            {
                if (family == qtFamilies[familyListBox->item(i)->text()].toLower())
                {
                    familyListBox->setCurrentRow(i);
                    break;
                }
            }
        }
    }

    // 2nd family fallback.

    if (i == numEntries)
    {
        QString fallback = family + QLatin1String(" [");

        for (i = 0 ; i < numEntries ; ++i)
        {
            if (qtFamilies[familyListBox->item(i)->text()].toLower().startsWith(fallback))
            {
                familyListBox->setCurrentRow(i);
                break;
            }
        }
    }

    // 3rd family fallback.

    if (i == numEntries)
    {
        for (i = 0 ; i < numEntries ; ++i)
        {
            if (qtFamilies[familyListBox->item(i)->text()].toLower().startsWith(family))
            {
                familyListBox->setCurrentRow(i);
                break;
            }
        }
    }

    // Family fallback in case nothing matched. Otherwise, diff doesn't work

    if (i == numEntries)
    {
        familyListBox->setCurrentRow(0);
    }

    // By setting the current item in the family box, the available
    // styles and sizes for that family have been collected.
    // Try now to set the current items in the style and size boxes.

    // Set current style in the listbox.

    numEntries = styleListBox->count();

    for (i = 0 ; i < numEntries ; ++i)
    {
        if (styleID == styleIDs[styleListBox->item(i)->text()])
        {
            styleListBox->setCurrentRow(i);
            break;
        }
    }

    if (i == numEntries)
    {
        // Style not found, fallback.

        styleListBox->setCurrentRow(0);
    }

    // Set current size in the listbox.
    // If smoothly scalable, allow customizing one of the standard size slots,
    // Otherwise just select the nearest available size.

    QString currentFamily = qtFamilies[familyListBox->currentItem()->text()];
    QString currentStyle  = qtStyles[styleListBox->currentItem()->text()];

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    bool canCustomize     = QFontDatabase::isSmoothlyScalable(currentFamily, currentStyle);

#else

    bool canCustomize     = QFontDatabase().isSmoothlyScalable(currentFamily, currentStyle);

#endif

    sizeListBox->setCurrentRow(nearestSizeRow(size, canCustomize));

    // Set current size in the spinbox.

    sizeOfFont->setValue(QLocale::system().toDouble(sizeListBox->currentItem()->text()));
}

void DFontProperties::getFontList(QStringList& list, uint fontListCriteria)
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    QStringList lstSys(QFontDatabase::families());

#else

    QStringList lstSys(QFontDatabase().families());

#endif

    // If we have criteria; then check fonts before adding.

    if (fontListCriteria)
    {
        QStringList lstFonts;

        for (QStringList::const_iterator it = lstSys.constBegin() ; it != lstSys.constEnd() ; ++it)
        {
            if (
                ((fontListCriteria & FixedWidthFonts) > 0) &&

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                !QFontDatabase::isFixedPitch(*it)

#else

                !QFontDatabase().isFixedPitch(*it)

#endif

               )
            {
                continue;
            }

            if (
                ((fontListCriteria & (SmoothScalableFonts | ScalableFonts)) == ScalableFonts) &&

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                !QFontDatabase::isBitmapScalable(*it)

#else

                !QFontDatabase().isBitmapScalable(*it)

#endif

               )
            {
                continue;
            }

            if (
                ((fontListCriteria & SmoothScalableFonts) > 0) &&

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                !QFontDatabase::isSmoothlyScalable(*it)

#else

                !QFontDatabase().isSmoothlyScalable(*it)

#endif

               )
            {
                continue;
            }

            lstFonts.append(*it);
        }

        if ((fontListCriteria & FixedWidthFonts) > 0)
        {
            // Fallback. if there are no fixed fonts found, it's probably a
            // bug in the font server or Qt.  In this case, just use 'fixed'.

            if (lstFonts.count() == 0)
            {
                lstFonts.append(QLatin1String("fixed"));
            }
        }

        lstSys = lstFonts;
    }

    lstSys.sort();

    list = lstSys;
}

void DFontProperties::Private::setFamilyBoxItems(const QStringList& fonts)
{
    signalsAllowed      = false;
    QStringList trfonts = translateFontNameList(fonts, &qtFamilies);
    familyListBox->clear();
    familyListBox->addItems(trfonts);

    signalsAllowed      = true;
}

void DFontProperties::Private::fillFamilyListBox(bool onlyFixedFonts)
{
    QStringList fontList;
    getFontList(fontList, onlyFixedFonts ? FixedWidthFonts : 0);
    setFamilyBoxItems(fontList);
}

/**
 * Human-readable style identifiers returned by QFontDatabase::styleString()
 * do not always survive round trip of QFont serialization/deserialization,
 * causing wrong style in the style box to be highlighted when
 * the chooser dialog is opened. This will cause the style to be changed
 * when the dialog is closed and the user did not touch the style box.
 * Hence, construct custom style identifiers sufficient for the purpose.
 */
QString DFontProperties::Private::styleIdentifier(const QFont& font)
{
    const QChar comma(QLatin1Char(','));

    return (
            QString::number(font.weight())     + comma +
            QString::number((int)font.style()) + comma +
            QString::number(font.stretch())
           );
}

void DFontProperties::Private::splitFontString(const QString& name, QString* family, QString* foundry)
{
    int p1 = name.indexOf(QLatin1Char('['));

    if (p1 < 0)
    {
        if (family)
        {
            *family = name.trimmed();
        }

        if (foundry)
        {
            foundry->clear();
        }
    }
    else
    {
        int p2 = name.indexOf(QLatin1Char(']'), p1);
        p2     = (p2 > p1) ? p2 : name.length();

        if (family)
        {
            *family = name.left(p1).trimmed();
        }

        if (foundry)
        {
            *foundry = name.mid(p1 + 1, p2 - p1 - 1).trimmed();
        }
    }
}

QString DFontProperties::Private::translateFontName(const QString& name)
{
    QString family, foundry;
    splitFontString(name, &family, &foundry);

    // Obtain any regular translations for the family and foundry.

    QString trFamily  = QCoreApplication::translate("FontHelpers",
                                                    family.toUtf8().constData(),
                                                    "@item Font name");
    QString trFoundry = foundry;

    if (!foundry.isEmpty())
    {
        trFoundry = QCoreApplication::translate("FontHelpers",
                                                foundry.toUtf8().constData(),
                                                "@item Font foundry");
    }

    // Assemble full translation.

    QString trfont;

    if (foundry.isEmpty())
    {
        // i18n: Filter by which the translators can translate, or otherwise
        // operate on the font names not put up for regular translation.

        trfont = QCoreApplication::translate("FontHelpers", "%1", "@item Font name").arg(trFamily);
    }
    else
    {
        // i18n: Filter by which the translators can translate, or otherwise
        // operate on the font names not put up for regular translation.

        trfont = QCoreApplication::translate("FontHelpers", "%1 [%2]", "@item Font name [foundry]")
                 .arg(trFamily).arg(trFoundry);
    }

    return trfont;
}

QStringList DFontProperties::Private::translateFontNameList(const QStringList& names,
                                                            QHash<QString, QString>* trToRawNames)
{
    // Generic fonts, in the inverse of desired order.

    QStringList genericNames;
    genericNames.append(QLatin1String("Monospace"));
    genericNames.append(QLatin1String("Serif"));
    genericNames.append(QLatin1String("Sans Serif"));

    // Translate fonts, but do not add generics to the list right away.

    QStringList             trNames;
    QHash<QString, QString> trMap;

    for (const QString& name : std::as_const(names))
    {
        QString trName = translateFontName(name);

        if (!genericNames.contains(name))
        {
            trNames.append(trName);
        }

        trMap.insert(trName, name);
    }

    // Sort real fonts alphabetically.

    std::sort(trNames.begin(), trNames.end(), localeLessThan);

    // Prepend generic fonts, in the predefined order.

    for (const QString& genericName : std::as_const(genericNames))
    {
        QString trGenericName = translateFontName(genericName);

        if (trMap.contains(trGenericName))
        {
            trNames.prepend(trGenericName);
        }
    }

    if (trToRawNames)
    {
        *trToRawNames = trMap;
    }

    return trNames;
}

} // namespace Digikam

#include "moc_dfontproperties.cpp"
