/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-10-24
 * Description : XMP workflow status properties settings page.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "xmpproperties.h"

// Qt includes

#include <QCheckBox>
#include <QPushButton>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QComboBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dlayoutbox.h"
#include "squeezedcombobox.h"
#include "digikam_debug.h"
#include "altlangstringedit.h"
#include "metadatacheckbox.h"
#include "multivaluesedit.h"
#include "objectattributesedit.h"
#include "dexpanderbox.h"
#include "dtextedit.h"

namespace DigikamGenericMetadataEditPlugin
{

class Q_DECL_HIDDEN XMPProperties::Private
{
public:

    Private()
    {
        sceneCodeMap.insert( QLatin1String("010100"), i18nc("@item: scene code", "Headshot") );
        sceneCodeMap.insert( QLatin1String("010200"), i18nc("@item: scene code", "Half-length") );
        sceneCodeMap.insert( QLatin1String("010300"), i18nc("@item: scene code", "Full-length") );
        sceneCodeMap.insert( QLatin1String("010400"), i18nc("@item: side view of a person", "Profile") );
        sceneCodeMap.insert( QLatin1String("010500"), i18nc("@item: scene code", "Rear view") );
        sceneCodeMap.insert( QLatin1String("010600"), i18nc("@item: scene code", "Single") );
        sceneCodeMap.insert( QLatin1String("010700"), i18nc("@item: scene code", "Couple") );
        sceneCodeMap.insert( QLatin1String("010800"), i18nc("@item: scene code", "Two") );
        sceneCodeMap.insert( QLatin1String("010900"), i18nc("@item: group of people", "Group") );
        sceneCodeMap.insert( QLatin1String("011000"), i18nc("@item: scene code", "General view") );
        sceneCodeMap.insert( QLatin1String("011100"), i18nc("@item: scene code", "Panoramic view") );
        sceneCodeMap.insert( QLatin1String("011200"), i18nc("@item: scene code", "Aerial view") );
        sceneCodeMap.insert( QLatin1String("011300"), i18nc("@item: scene code", "Under-water") );
        sceneCodeMap.insert( QLatin1String("011400"), i18nc("@item: scene code", "Night scene") );
        sceneCodeMap.insert( QLatin1String("011500"), i18nc("@item: scene code", "Satellite") );
        sceneCodeMap.insert( QLatin1String("011600"), i18nc("@item: scene code", "Exterior view") );
        sceneCodeMap.insert( QLatin1String("011700"), i18nc("@item: scene code", "Interior view") );
        sceneCodeMap.insert( QLatin1String("011800"), i18nc("@item: scene code", "Close-up") );
        sceneCodeMap.insert( QLatin1String("011900"), i18nc("@item: scene code", "Action") );
        sceneCodeMap.insert( QLatin1String("012000"), i18nc("@item: scene code", "Performing") );
        sceneCodeMap.insert( QLatin1String("012100"), i18nc("@item: scene code", "Posing") );
        sceneCodeMap.insert( QLatin1String("012200"), i18nc("@item: scene code", "Symbolic") );
        sceneCodeMap.insert( QLatin1String("012300"), i18nc("@item: scene code", "Off-beat") );
        sceneCodeMap.insert( QLatin1String("012400"), i18nc("@item: scene code", "Movie scene") );

        typeCodeMap.insert( QLatin1String("Advisory"),           i18nc("@item: type code", "Advisory") );
        typeCodeMap.insert( QLatin1String("Alert"),              i18nc("@item: type code", "Alert") );
        typeCodeMap.insert( QLatin1String("Catalog"),            i18nc("@item: type code", "Catalog") );
        typeCodeMap.insert( QLatin1String("Data"),               i18nc("@item: type code", "Data") );
        typeCodeMap.insert( QLatin1String("Document"),           i18nc("@item: type is a document", "Document") );
        typeCodeMap.insert( QLatin1String("DTD"),                i18nc("@item: type code", "DTD") );
        typeCodeMap.insert( QLatin1String("Maintenance"),        i18nc("@item: type code", "Maintenance") );
        typeCodeMap.insert( QLatin1String("News"),               i18nc("@item: type code", "News") );
        typeCodeMap.insert( QLatin1String("NewsManagementMode"), i18nc("@item: type code", "News Management Mode") );
        typeCodeMap.insert( QLatin1String("Package"),            i18nc("@item: type code", "Package") );
        typeCodeMap.insert( QLatin1String("Schema"),             i18nc("@item: type code", "Schema") );
        typeCodeMap.insert( QLatin1String("Topic"),              i18nc("@item: type code", "Topic") );
        typeCodeMap.insert( QLatin1String("TopicSet"),           i18nc("@item: type code", "Topic Set") );

        DMetadata::CountryCodeMap map = DMetadata::countryCodeMap();

        for (DMetadata::CountryCodeMap::Iterator it = map.begin() ; it != map.end() ; ++it)
        {
            languageCodeMap.insert(it.key(), it.value());
        }
    }

public:

    typedef QMap<QString, QString>  SceneCodeMap;
    typedef QMap<QString, QString>  TypeCodeMap;
    typedef QMap<QString, QString>  LanguageCodeMap;

    SceneCodeMap                    sceneCodeMap;
    TypeCodeMap                     typeCodeMap;
    LanguageCodeMap                 languageCodeMap;

    QCheckBox*                      originalTransCheck      = nullptr;

    QComboBox*                      priorityCB              = nullptr;
    QComboBox*                      objectTypeCB            = nullptr;

    DTextEdit*                      objectAttributeEdit     = nullptr;
    DTextEdit*                      originalTransEdit       = nullptr;

    MetadataCheckBox*               priorityCheck           = nullptr;
    MetadataCheckBox*               objectAttributeCheck    = nullptr;

    MultiValuesEdit*                sceneEdit               = nullptr;
    MultiValuesEdit*                objectTypeEdit          = nullptr;
    MultiValuesEdit*                languageEdit            = nullptr;

    SqueezedComboBox*               objectAttributeCB       = nullptr;
};

XMPProperties::XMPProperties(QWidget* const parent)
    : MetadataEditPage(parent),
      d               (new Private)
{
    QGridLayout* const grid = new QGridLayout(widget());

    // --------------------------------------------------------

    d->languageEdit = new MultiValuesEdit(this, i18nc("@option", "Language:"),
                          i18nc("@info", "Select here the language of content."));

    QStringList list;

    for (Private::LanguageCodeMap::Iterator it = d->languageCodeMap.begin() ;
         it != d->languageCodeMap.end() ; ++it)
    {
        list.append(QString::fromUtf8("%1 - %2").arg(it.key()).arg(it.value()));
    }

    d->languageEdit->setData(list);

    // --------------------------------------------------------

    d->priorityCheck = new MetadataCheckBox(i18nc("@option", "Priority:"), this);
    d->priorityCB    = new QComboBox(this);
    d->priorityCB->insertItem(0, i18nc("@item: editorial urgency of content", "0: None"));
    d->priorityCB->insertItem(1, i18nc("@item: editorial urgency of content", "1: High"));
    d->priorityCB->insertItem(2, QLatin1String("2"));
    d->priorityCB->insertItem(3, QLatin1String("3"));
    d->priorityCB->insertItem(4, QLatin1String("4"));
    d->priorityCB->insertItem(5, i18nc("@item: editorial urgency of content", "5: Normal"));
    d->priorityCB->insertItem(6, QLatin1String("6"));
    d->priorityCB->insertItem(7, QLatin1String("7"));
    d->priorityCB->insertItem(8, i18nc("@item: editorial urgency of content", "8: Low"));
    d->priorityCB->insertItem(9, i18nc("@item: editorial urgency of content", "9: User-defined"));
    d->priorityCB->setWhatsThis(i18nc("@info", "@info: Select here the editorial urgency of content."));

    // --------------------------------------------------------

    d->sceneEdit = new MultiValuesEdit(this, i18nc("@option", "Scene:"),
                       i18nc("@info", "Select here the scene type of the content."));

    QStringList list2;

    for (Private::SceneCodeMap::Iterator it = d->sceneCodeMap.begin() ;
         it != d->sceneCodeMap.end() ; ++it)
    {
        list2.append(QString::fromUtf8("%1 - %2").arg(it.key()).arg(it.value()));
    }

    d->sceneEdit->setData(list2);

    // --------------------------------------------------------

    d->objectTypeEdit = new MultiValuesEdit(this, i18nc("@option", "Type:"),
                            i18nc("@info", "Select here the editorial type of the content."));

    QStringList list3;

    for (Private::TypeCodeMap::Iterator it = d->typeCodeMap.begin() ;
         it != d->typeCodeMap.end() ; ++it)
    {
        list3.append(it.value());
    }

    d->objectTypeEdit->setData(list3);

    // --------------------------------------------------------

    DHBox* const objectBox  = new DHBox(this);
    d->objectAttributeCheck = new MetadataCheckBox(i18nc("@option", "Attribute:"), this);
    d->objectAttributeCB    = new SqueezedComboBox(objectBox);
    d->objectAttributeCB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->objectAttributeEdit  = new DTextEdit(objectBox);
    d->objectAttributeEdit->setLinesVisible(1);
    d->objectAttributeEdit->setPlaceholderText(i18nc("@info", "Set here the editorial attribute description"));
    d->objectAttributeEdit->setWhatsThis(i18nc("@info", "Set here the editorial attribute description of the content."));

    d->objectAttributeCB->setWhatsThis(i18nc("@info", "Select here the editorial attribute of the content."));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("001 - ") + i18nc("@item: editorial content attribute", "Current"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("002 - ") + i18nc("@item: editorial content attribute", "Analysis"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("003 - ") + i18nc("@item: editorial content attribute", "Archive material"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("004 - ") + i18nc("@item: editorial content attribute", "Background"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("005 - ") + i18nc("@item: editorial content attribute", "Feature"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("006 - ") + i18nc("@item: editorial content attribute", "Forecast"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("007 - ") + i18nc("@item: editorial content attribute", "History"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("008 - ") + i18nc("@item: editorial content attribute", "Obituary"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("009 - ") + i18nc("@item: editorial content attribute", "Opinion"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("010 - ") + i18nc("@item: editorial content attribute", "Polls & Surveys"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("011 - ") + i18nc("@item: editorial content attribute", "Profile"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("012 - ") + i18nc("@item: editorial content attribute", "Results Listings & Table"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("013 - ") + i18nc("@item: editorial content attribute", "Side bar & Supporting information"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("014 - ") + i18nc("@item: editorial content attribute", "Summary"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("015 - ") + i18nc("@item: editorial content attribute", "Transcript & Verbatim"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("016 - ") + i18nc("@item: editorial content attribute", "Interview"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("017 - ") + i18nc("@item: editorial content attribute", "From the Scene"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("018 - ") + i18nc("@item: editorial content attribute", "Retrospective"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("019 - ") + i18nc("@item: editorial content attribute", "Statistics"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("020 - ") + i18nc("@item: editorial content attribute", "Update"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("021 - ") + i18nc("@item: editorial content attribute", "Wrap-up"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("022 - ") + i18nc("@item: editorial content attribute", "Press Release"));

    // --------------------------------------------------------

    d->originalTransCheck = new QCheckBox(i18nc("@option", "Reference:"), this);
    d->originalTransEdit  = new DTextEdit(this);
    d->originalTransEdit->setLinesVisible(1);
    d->originalTransEdit->setPlaceholderText(i18nc("@info", "Set here the content reference"));
    d->originalTransEdit->setWhatsThis(i18nc("@info", "Set here the original content transmission reference."));

    // --------------------------------------------------------

    grid->addWidget(d->languageEdit,                        0, 0, 1, 5);
    grid->addWidget(d->priorityCheck,                       1, 0, 1, 1);
    grid->addWidget(d->priorityCB,                          1, 1, 1, 1);
    grid->addWidget(d->sceneEdit,                           2, 0, 1, 5);
    grid->addWidget(d->objectTypeEdit,                      3, 0, 1, 5);
    grid->addWidget(new DLineWidget(Qt::Horizontal, this),  4, 0, 1, 5);
    grid->addWidget(d->objectAttributeCheck,                5, 0, 1, 1);
    grid->addWidget(objectBox,                              5, 1, 1, 4);
    grid->addWidget(new DLineWidget(Qt::Horizontal, this),  6, 0, 1, 5);
    grid->addWidget(d->originalTransCheck,                  7, 0, 1, 1);
    grid->addWidget(d->originalTransEdit,                   7, 1, 1, 4);
    grid->setRowStretch(8, 10);
    grid->setColumnStretch(4, 10);

    int spacing = layoutSpacing();

    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->priorityCheck, SIGNAL(toggled(bool)),
            d->priorityCB, SLOT(setEnabled(bool)));

    connect(d->objectAttributeCheck, SIGNAL(toggled(bool)),
            d->objectAttributeCB, SLOT(setEnabled(bool)));

    connect(d->objectAttributeCheck, SIGNAL(toggled(bool)),
            d->objectAttributeEdit, SLOT(setEnabled(bool)));

    connect(d->originalTransCheck, SIGNAL(toggled(bool)),
            d->originalTransEdit, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    connect(d->languageEdit, SIGNAL(signalModified()),
            this, SIGNAL(signalModified()));

    connect(d->priorityCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->sceneEdit, SIGNAL(signalModified()),
            this, SIGNAL(signalModified()));

    connect(d->objectTypeEdit, SIGNAL(signalModified()),
            this, SIGNAL(signalModified()));

    connect(d->objectAttributeCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->originalTransCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->priorityCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->objectAttributeCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->objectAttributeEdit, SIGNAL(textChanged()),
            this, SIGNAL(signalModified()));

    connect(d->originalTransEdit, SIGNAL(textChanged()),
            this, SIGNAL(signalModified()));
}

XMPProperties::~XMPProperties()
{
    delete d;
}

void XMPProperties::readMetadata(const DMetadata& meta)
{
    blockSignals(true);

    int         val;
    QString     data;
    QStringList code, list, list2;

    // ---------------------------------------------------------------

    code = meta.getXmpTagStringBag("Xmp.dc.language", false);

    for (QStringList::Iterator it = code.begin() ; it != code.end() ; ++it)
    {
        QStringList lst1 = d->languageEdit->getData();
        QStringList::Iterator it2;

        for (it2 = lst1.begin() ; it2 != lst1.end() ; ++it2)
        {
            if ((*it2).left(2) == (*it))
            {
                list.append(*it2);
                break;
            }
        }

        if (it2 == lst1.end())
        {
            d->languageEdit->setValid(false);
        }
    }

    d->languageEdit->setValues(list);

    // ---------------------------------------------------------------

    d->priorityCB->setCurrentIndex(0);
    d->priorityCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.photoshop.Urgency", false);

    if (!data.isNull())
    {
        val = data.toInt();

        if ((val >= 0) && (val <= 9))
        {
            d->priorityCB->setCurrentIndex(val);
            d->priorityCheck->setChecked(true);
        }
        else
        {
            d->priorityCheck->setValid(false);
        }
    }

    d->priorityCB->setEnabled(d->priorityCheck->isChecked());

    // ---------------------------------------------------------------

    code = meta.getXmpTagStringBag("Xmp.iptc.Scene", false);

    for (QStringList::Iterator it = code.begin() ; it != code.end() ; ++it)
    {
        QStringList lst2 = d->sceneEdit->getData();
        QStringList::Iterator it2;

        for (it2 = lst2.begin() ; it2 != lst2.end() ; ++it2)
        {
            if ((*it2).left(6) == (*it))
            {
                list.append(*it2);
                break;
            }
        }

        if (it2 == lst2.end())
        {
            d->sceneEdit->setValid(false);
        }
    }

    d->sceneEdit->setValues(list);

    // ---------------------------------------------------------------

    code = meta.getXmpTagStringBag("Xmp.dc.type", false);

    for (QStringList::Iterator it3 = code.begin() ; it3 != code.end() ; ++it3)
    {
        QStringList lst3 = d->objectTypeEdit->getData();
        QStringList::Iterator it4;

        for (it4 = lst3.begin() ; it4 != lst3.end() ; ++it4)
        {
            if ((*it4) == (*it3))
            {
                list2.append(*it4);
                break;
            }
        }

        if (it4 == lst3.end())
        {
            d->objectTypeEdit->setValid(false);
        }
    }

    d->objectTypeEdit->setValues(list2);

    // ---------------------------------------------------------------

    d->objectAttributeCB->setCurrentIndex(0);
    d->objectAttributeEdit->clear();
    d->objectAttributeCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.iptc.IntellectualGenre", false);

    if (!data.isNull())
    {
        QString attrSec = data.section(QLatin1Char(':'), 0, 0);

        if (!attrSec.isEmpty())
        {
            int attr = attrSec.toInt()-1;

            if ((attr >= 0) && (attr < 23))
            {
                d->objectAttributeCB->setCurrentIndex(attr);
                d->objectAttributeEdit->setText(data.section(QLatin1Char(':'), -1));
                d->objectAttributeCheck->setChecked(true);
            }
            else
            {
                d->objectAttributeCheck->setValid(false);
            }
        }
    }

    d->objectAttributeCB->setEnabled(d->objectAttributeCheck->isChecked());
    d->objectAttributeEdit->setEnabled(d->objectAttributeCheck->isChecked());

    // ---------------------------------------------------------------

    d->originalTransEdit->clear();
    d->originalTransCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.photoshop.TransmissionReference", false);

    if (!data.isNull())
    {
        d->originalTransEdit->setText(data);
        d->originalTransCheck->setChecked(true);
    }

    d->originalTransEdit->setEnabled(d->originalTransCheck->isChecked());

    // ---------------------------------------------------------------

    blockSignals(false);
}

void XMPProperties::applyMetadata(const DMetadata& meta)
{
    QStringList oldList, newList;

    // ---------------------------------------------------------------

    if (d->languageEdit->getValues(oldList, newList))
    {
        QStringList newCode;

        for (QStringList::Iterator it2 = newList.begin() ; it2 != newList.end() ; ++it2)
        {
            newCode.append((*it2).left(2));
        }

        meta.setXmpTagStringBag("Xmp.dc.language", newCode);
    }
    else
    {
        meta.removeXmpTag("Xmp.dc.language");
    }

    // ---------------------------------------------------------------

    if      (d->priorityCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.photoshop.Urgency", QString::number(d->priorityCB->currentIndex()));
    }
    else if (d->priorityCheck->isValid())
    {
        meta.removeXmpTag("Xmp.photoshop.Urgency");
    }

    // ---------------------------------------------------------------

    if (d->sceneEdit->getValues(oldList, newList))
    {
        QStringList newCode;

        for (QStringList::Iterator it2 = newList.begin() ; it2 != newList.end() ; ++it2)
        {
            newCode.append((*it2).left(6));
        }

        meta.setXmpTagStringBag("Xmp.iptc.Scene", newCode);
    }
    else
    {
        meta.removeXmpTag("Xmp.iptc.Scene");
    }

    // ---------------------------------------------------------------

    if (d->objectTypeEdit->getValues(oldList, newList))
    {
        meta.setXmpTagStringBag("Xmp.dc.type", newList);
    }
    else
    {
        meta.removeXmpTag("Xmp.dc.type");
    }

    // ---------------------------------------------------------------

    if      (d->objectAttributeCheck->isChecked())
    {
        QString objectAttribute;
        objectAttribute = QString().asprintf("%3d", d->objectAttributeCB->currentIndex()+1);
        objectAttribute.append(QLatin1Char(':'));

        if (!d->objectAttributeEdit->text().isEmpty())
        {
            objectAttribute.append(d->objectAttributeEdit->text());
        }
        else
        {
            objectAttribute.append(d->objectAttributeCB->itemHighlighted()
                                   .section(QLatin1String(" - "), -1));
        }

        meta.setXmpTagString("Xmp.iptc.IntellectualGenre", objectAttribute);
    }
    else if (d->objectAttributeCheck->isValid())
    {
        meta.removeXmpTag("Xmp.iptc.IntellectualGenre");
    }

    // ---------------------------------------------------------------

    if (d->originalTransCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.photoshop.TransmissionReference", d->originalTransEdit->text());
    }
    else
    {
        meta.removeXmpTag("Xmp.photoshop.TransmissionReference");
    }
}

} // namespace DigikamGenericMetadataEditPlugin

#include "moc_xmpproperties.cpp"
