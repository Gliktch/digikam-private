/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-11-03
 * Description : Automatic retrieving, saving and resetting skeleton based settings in a dialog.
 *
 * SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2003      by Benjamin C Meyer <ben plus kdelibs at meyerhome dot net>
 * SPDX-FileCopyrightText: 2003      by Waldo Bastian <bastian at kde dot org>
 * SPDX-FileCopyrightText: 2017      by Friedrich W. H. Kossebau <kossebau at kde dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dconfigdlgmngr.h"

// Qt includes

#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QMetaObject>
#include <QMetaProperty>
#include <QLayout>
#include <QTimer>
#include <QAbstractButton>

// KDE includes

#include <kconfigskeleton.h>

// Local includes

#include "digikam_debug.h"

// clazy:skip

namespace Digikam
{

typedef QHash<QString, QByteArray> MyHash;

Q_GLOBAL_STATIC(MyHash, s_propertyMap)
Q_GLOBAL_STATIC(MyHash, s_changedMap)

class Q_DECL_HIDDEN DConfigDlgMngr::Private
{
public:

    explicit Private(DConfigDlgMngr* const qq)
        : q(qq)
    {
    }

public:

    DConfigDlgMngr* const    q              = nullptr;

    /**
     * Skeleton object used to store settings
     */
    KConfigSkeleton*         conf           = nullptr;

    /**
     * Dialog being managed
     */
    QWidget*                 dialog         = nullptr;

    QHash<QString, QWidget*> knownWidget;
    QHash<QString, QWidget*> buddyWidget;
    QSet<QWidget*>           allExclusiveGroupBoxes;
    bool                     insideGroupBox = false;
    bool                     trackChanges   = false;
};

DConfigDlgMngr::DConfigDlgMngr(QWidget* const parent, KConfigSkeleton* const conf)
    : QObject(parent),
      d      (new Private(this))
{
    d->conf   = conf;
    d->dialog = parent;
    this->init(true);
}

DConfigDlgMngr::~DConfigDlgMngr()
{
    delete d;
}

void DConfigDlgMngr::initMaps()
{
    if (s_changedMap()->isEmpty())
    {
        s_changedMap()->insert(QLatin1String("QCheckBox"),      SIGNAL(stateChanged(int)));
        s_changedMap()->insert(QLatin1String("QPushButton"),    SIGNAL(clicked(bool)));
        s_changedMap()->insert(QLatin1String("QRadioButton"),   SIGNAL(toggled(bool)));
        s_changedMap()->insert(QLatin1String("QGroupBox"),      SIGNAL(toggled(bool)));
        s_changedMap()->insert(QLatin1String("QComboBox"),      SIGNAL(activated(int)));
        s_changedMap()->insert(QLatin1String("QDateEdit"),      SIGNAL(dateChanged(QDate)));
        s_changedMap()->insert(QLatin1String("QTimeEdit"),      SIGNAL(timeChanged(QTime)));
        s_changedMap()->insert(QLatin1String("QDateTimeEdit"),  SIGNAL(dateTimeChanged(QDateTime)));
        s_changedMap()->insert(QLatin1String("QDial"),          SIGNAL(valueChanged(int)));
        s_changedMap()->insert(QLatin1String("QDoubleSpinBox"), SIGNAL(valueChanged(double)));
        s_changedMap()->insert(QLatin1String("QLineEdit"),      SIGNAL(textChanged(QString)));
        s_changedMap()->insert(QLatin1String("QSlider"),        SIGNAL(valueChanged(int)));
        s_changedMap()->insert(QLatin1String("QSpinBox"),       SIGNAL(valueChanged(int)));
        s_changedMap()->insert(QLatin1String("QTextEdit"),      SIGNAL(textChanged()));
        s_changedMap()->insert(QLatin1String("QTextBrowser"),   SIGNAL(sourceChanged(QString)));
        s_changedMap()->insert(QLatin1String("QPlainTextEdit"), SIGNAL(textChanged()));
        s_changedMap()->insert(QLatin1String("QTabWidget"),     SIGNAL(currentChanged(int)));
    }
}

QHash<QString, QByteArray>* DConfigDlgMngr::propertyMap()
{
    initMaps();

    return s_propertyMap();
}

QHash<QString, QByteArray>* DConfigDlgMngr::changedMap()
{
    initMaps();

    return s_changedMap();
}

void DConfigDlgMngr::init(bool trackChanges)
{
    initMaps();
    d->trackChanges = trackChanges;

    // Go through all of the children of the widgets and find all known widgets

    (void)parseChildren(d->dialog, trackChanges);
}

void DConfigDlgMngr::addWidget(QWidget* const widget)
{
    (void)parseChildren(widget, true);
}

void DConfigDlgMngr::setupWidget(QWidget* widget, KConfigSkeletonItem* item)
{
    QVariant minValue = item->minValue();

    if (minValue.isValid())
    {
        if (widget->metaObject()->indexOfProperty("minValue") != -1)
        {
            widget->setProperty("minValue", minValue);
        }

        if (widget->metaObject()->indexOfProperty("minimum") != -1)
        {
            widget->setProperty("minimum", minValue);
        }
    }

    QVariant maxValue = item->maxValue();

    if (maxValue.isValid())
    {
        if (widget->metaObject()->indexOfProperty("maxValue") != -1)
        {
            widget->setProperty("maxValue", maxValue);
        }

        if (widget->metaObject()->indexOfProperty("maximum") != -1)
        {
            widget->setProperty("maximum", maxValue);
        }
    }

    if (widget->whatsThis().isEmpty())
    {
        QString whatsThis = item->whatsThis();

        if (!whatsThis.isEmpty())
        {
            widget->setWhatsThis(whatsThis);
        }
    }

    if (widget->toolTip().isEmpty())
    {
        QString toolTip = item->toolTip();

        if (!toolTip.isEmpty())
        {
            widget->setToolTip(toolTip);
        }
    }

    // If it is a QGroupBox with only autoExclusive buttons
    // and has no custom property and the config item type
    // is an integer, assume we want to save the index
    // instead of if it is checked or not

    QGroupBox* const gb = qobject_cast<QGroupBox*>(widget);

    if (gb && getCustomProperty(gb).isEmpty())
    {
        const KConfigSkeletonItem* const sitem = d->conf->findItem(widget->objectName().mid(5));

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        if (sitem->property().typeId() == QMetaType::Int)

#else

        if (sitem->property().type() == QVariant::Int)

#endif

        {
            QObjectList children                  = gb->children();
            children.removeAll(gb->layout());
            const QList<QAbstractButton*> buttons = gb->findChildren<QAbstractButton*>();
            bool allAutoExclusiveDirectChildren   = true;

            for (QAbstractButton* const button : buttons)
            {
                // cppcheck-suppress useStlAlgorithm
                allAutoExclusiveDirectChildren = (
                                                  allAutoExclusiveDirectChildren &&
                                                  button->autoExclusive()        &&
                                                  (button->parent() == gb)
                                                 );
            }

            // cppcheck-suppress knownConditionTrueFalse
            if (allAutoExclusiveDirectChildren)
            {
                d->allExclusiveGroupBoxes << widget;
            }
        }
    }

    if (!item->isEqual(property(widget)))
    {
        setProperty(widget, item->property());
    }
}

bool DConfigDlgMngr::parseChildren(const QWidget* widget, bool trackChanges)
{
    bool valueChanged                    = false;
    const QList<QObject*> listOfChildren = widget->children();

    if (listOfChildren.isEmpty())
    {
        return valueChanged;
    }

    const QMetaMethod widgetModifiedSignal = metaObject()->method(metaObject()->indexOfSignal("widgetModified()"));

    Q_ASSERT(widgetModifiedSignal.isValid() && (metaObject()->indexOfSignal("widgetModified()") >= 0));

    for (QObject* const object : listOfChildren)
    {
        if (!object->isWidgetType())
        {
            // Skip non-widgets

            continue;
        }

        QWidget* const childWidget = static_cast<QWidget*>(object);
        QString widgetName         = childWidget->objectName();
        bool bParseChildren        = true;
        bool bSaveInsideGroupBox   = d->insideGroupBox;

        if      (widgetName.startsWith(QLatin1String("kcfg_")))
        {
            // This is one of our widgets!

            QString configId                = widgetName.mid(5);
            KConfigSkeletonItem* const item = d->conf->findItem(configId);

            if (item)
            {
                d->knownWidget.insert(configId, childWidget);

                setupWidget(childWidget, item);

                if (trackChanges)
                {
                    bool changeSignalFound = false;

                    if (d->allExclusiveGroupBoxes.contains(childWidget))
                    {
                        const QList<QAbstractButton*> buttons = childWidget->findChildren<QAbstractButton*>();

                        for (QAbstractButton* const button : buttons)
                        {
                            connect(button, &QAbstractButton::toggled,
                                    this, &DConfigDlgMngr::widgetModified);
                        }
                    }

                    QByteArray propertyChangeSignal = getCustomPropertyChangedSignal(childWidget);

                    if (propertyChangeSignal.isEmpty())
                    {
                        propertyChangeSignal = getUserPropertyChangedSignal(childWidget);
                    }

                    if (propertyChangeSignal.isEmpty())
                    {
                        // get the change signal from the meta object

                        const QMetaObject* const metaobj = childWidget->metaObject();
                        QByteArray userproperty          = getCustomProperty(childWidget);

                        if (userproperty.isEmpty())
                        {
                            userproperty = getUserProperty(childWidget);
                        }

                        if (!userproperty.isEmpty())
                        {
                            const int indexOfProperty = metaobj->indexOfProperty(userproperty.constData());

                            if (indexOfProperty != -1)
                            {
                                const QMetaProperty prop       = metaobj->property(indexOfProperty);
                                const QMetaMethod notifySignal = prop.notifySignal();

                                if (notifySignal.isValid())
                                {
                                    connect(childWidget, notifySignal,
                                            this, widgetModifiedSignal);       // clazy:exclude=connect-non-signal

                                    changeSignalFound = true;
                                }
                            }
                        }
                        else
                        {
                           qCWarning(DIGIKAM_GENERAL_LOG) << "Do not know how to monitor widget '"
                                                          << childWidget->metaObject()->className()
                                                          << "' for changes!";
                        }
                    }
                    else
                    {
                        connect(childWidget, propertyChangeSignal.constData(),
                                this, SIGNAL(widgetModified()));

                        changeSignalFound = true;
                    }

                    if (changeSignalFound)
                    {
                        QComboBox* const cb = qobject_cast<QComboBox*>(childWidget);

                        if (cb && cb->isEditable())
                        {
                            connect(cb, &QComboBox::editTextChanged,
                                    this, &DConfigDlgMngr::widgetModified);
                        }
                    }
                }

                const QGroupBox* const gb = qobject_cast<QGroupBox*>(childWidget);

                if (!gb)
                {
                    bParseChildren = false;
                }
                else
                {
                    d->insideGroupBox = true;
                }
            }
            else
            {
               qCWarning(DIGIKAM_GENERAL_LOG) << "A widget named '" << widgetName
                                              << "' was found but there is no setting named '"
                                              << configId << "'";
            }
        }
        else if (QLabel* const label = qobject_cast<QLabel*>(childWidget))
        {
            QWidget* const buddy = label->buddy();

            if (!buddy)
            {
                continue;
            }

            QString buddyName = buddy->objectName();

            if (buddyName.startsWith(QLatin1String("kcfg_")))
            {
                // This is one of our widgets!

                QString configId = buddyName.mid(5);
                d->buddyWidget.insert(configId, childWidget);
            }
        }

        if (bParseChildren)
        {
            // this widget is not known as something we can store.
            // Maybe we can store one of its children.

            valueChanged |= parseChildren(childWidget, trackChanges);
        }

        d->insideGroupBox = bSaveInsideGroupBox;
    }

    return valueChanged;
}

void DConfigDlgMngr::updateWidgets()
{
    bool changed         = false;
    bool bSignalsBlocked = signalsBlocked();
    blockSignals(true);

    QWidget* widget      = nullptr;
    QHashIterator<QString, QWidget*> it(d->knownWidget);

    while (it.hasNext())
    {
        it.next();
        widget = it.value();

        KConfigSkeletonItem* const item = d->conf->findItem(it.key());

        if (!item)
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "The setting '" << it.key() << "' has disappeared!";
            continue;
        }

        if (!item->isEqual(property(widget)))
        {
            setProperty(widget, item->property());
            changed = true;
        }

        if (item->isImmutable())
        {
            widget->setEnabled(false);
            QWidget* const buddy = d->buddyWidget.value(it.key(), nullptr);

            if (buddy)
            {
                buddy->setEnabled(false);
            }
        }
    }

    blockSignals(bSignalsBlocked);

    if (changed)
    {
        QTimer::singleShot(0, this, &DConfigDlgMngr::widgetModified);
    }
}

void DConfigDlgMngr::updateWidgetsDefault()
{
    bool bUseDefaults = d->conf->useDefaults(true);
    updateWidgets();
    d->conf->useDefaults(bUseDefaults);
}

void DConfigDlgMngr::updateSettings()
{
    bool changed    = false;
    QWidget* widget = nullptr;
    QHashIterator<QString, QWidget*> it(d->knownWidget);

    while (it.hasNext())
    {
        it.next();
        widget                          = it.value();
        KConfigSkeletonItem* const item = d->conf->findItem(it.key());

        if (!item)
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "The setting '" << it.key() << "' has disappeared!";
            continue;
        }

        QVariant fromWidget = property(widget);

        if (!item->isEqual(fromWidget))
        {
            item->setProperty(fromWidget);
            changed = true;
        }
    }

    if (changed)
    {
        d->conf->save();

        Q_EMIT settingsChanged();
    }
}

QByteArray DConfigDlgMngr::getUserProperty(const QWidget* widget) const
{
    if (!s_propertyMap()->contains(QString::fromUtf8(widget->metaObject()->className())))
    {
        const QMetaObject* const metaobj = widget->metaObject();
        const QMetaProperty user         = metaobj->userProperty();

        if (user.isValid())
        {
            s_propertyMap()->insert(QString::fromUtf8(widget->metaObject()->className()), user.name());
        }
        else
        {
            return QByteArray(); // no USER property
        }
    }

    const QComboBox* const cb = qobject_cast<const QComboBox*>(widget);

    if (cb)
    {
        const char* const qcomboUserPropertyName = cb->QComboBox::metaObject()->userProperty().name();
        const int qcomboUserPropertyIndex        = qcomboUserPropertyName ? cb->QComboBox::metaObject()->indexOfProperty(qcomboUserPropertyName) : -1;
        const char* const widgetUserPropertyName = widget->metaObject()->userProperty().name();
        const int widgetUserPropertyIndex        = widgetUserPropertyName ? cb->metaObject()->indexOfProperty(widgetUserPropertyName)            : -1;

        // no custom user property set on subclass of QComboBox?

        if (qcomboUserPropertyIndex == widgetUserPropertyIndex)
        {
            return QByteArray(); // use the QCombobox special code
        }
    }

    return s_propertyMap()->value(QString::fromUtf8(widget->metaObject()->className()));
}

QByteArray DConfigDlgMngr::getCustomProperty(const QWidget* widget) const
{
    QVariant prop(widget->property("kcfg_property"));

    if (prop.isValid())
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        if (!prop.canConvert(QMetaType(QMetaType::QByteArray)))

#else

        if (!prop.canConvert(QVariant::ByteArray))

#endif

        {
           qCWarning(DIGIKAM_GENERAL_LOG) << "Property on"
                                          << widget->metaObject()->className()
                                          << "is not of type ByteArray";
        }
        else
        {
            return prop.toByteArray();
        }
    }

    return QByteArray();
}

QByteArray DConfigDlgMngr::getUserPropertyChangedSignal(const QWidget* widget) const
{
    QHash<QString, QByteArray>::const_iterator changedIt = s_changedMap()->constFind(QString::fromUtf8(widget->metaObject()->className()));

    if (changedIt == s_changedMap()->constEnd())
    {
        // If the class name of the widget wasn't in the monitored widgets map, then look for
        // it again using the super class name.

        if (widget->metaObject()->superClass())
        {
            changedIt = s_changedMap()->constFind(QString::fromUtf8(widget->metaObject()->superClass()->className()));
        }
    }

    return ((changedIt == s_changedMap()->constEnd()) ? QByteArray() : *changedIt);
}

QByteArray DConfigDlgMngr::getCustomPropertyChangedSignal(const QWidget *widget) const
{
    QVariant prop(widget->property("kcfg_propertyNotify"));

    if (prop.isValid())
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        if (!prop.canConvert(QMetaType(QMetaType::QByteArray)))

#else

        if (!prop.canConvert(QVariant::ByteArray))

#endif

        {
           qCWarning(DIGIKAM_GENERAL_LOG) << "PropertyNotify on"
                                          << widget->metaObject()->className()
                                          << "is not of type ByteArray";
        }
        else
        {
            return prop.toByteArray();
        }
    }

    return QByteArray();
}

void DConfigDlgMngr::setProperty(QWidget* w, const QVariant& v)
{
    if (d->allExclusiveGroupBoxes.contains(w))
    {
        const QList<QAbstractButton*> buttons = w->findChildren<QAbstractButton*>();

        if (v.toInt() < buttons.count())
        {
            buttons[v.toInt()]->setChecked(true);
        }

        return;
    }

    QByteArray userproperty = getCustomProperty(w);

    if (userproperty.isEmpty())
    {
        userproperty = getUserProperty(w);
    }

    if (userproperty.isEmpty())
    {
        QComboBox* const cb = qobject_cast<QComboBox*>(w);

        if (cb)
        {
            if (cb->isEditable())
            {
                int i = cb->findText(v.toString());

                if (i != -1)
                {
                    cb->setCurrentIndex(i);
                }
                else
                {
                    cb->setEditText(v.toString());
                }
            }
            else
            {
                cb->setCurrentIndex(v.toInt());
            }

            return;
        }

        qCWarning(DIGIKAM_GENERAL_LOG) << w->metaObject()->className() << "widget not handled!";
        return;
    }

    w->setProperty(userproperty.constData(), v);
}

QVariant DConfigDlgMngr::property(QWidget* w) const
{
    if (d->allExclusiveGroupBoxes.contains(w))
    {
        const QList<QAbstractButton*> buttons = w->findChildren<QAbstractButton*>();

        for (int i = 0 ; i < buttons.count() ; ++i)
        {
            if (buttons[i]->isChecked())
            {
                return i;
            }
        }

        return -1;
    }

    QByteArray userproperty = getCustomProperty(w);

    if (userproperty.isEmpty())
    {
        userproperty = getUserProperty(w);
    }

    if (userproperty.isEmpty())
    {
        QComboBox* const cb = qobject_cast<QComboBox*>(w);

        if (cb)
        {
            if (cb->isEditable())
            {
                return QVariant(cb->currentText());
            }
            else
            {
                return QVariant(cb->currentIndex());
            }
        }

        qCWarning(DIGIKAM_GENERAL_LOG) << w->metaObject()->className() << "widget not handled!";
        return QVariant();
    }

    return w->property(userproperty.constData());
}

bool DConfigDlgMngr::hasChanged() const
{
    QWidget* widget = nullptr;
    QHashIterator<QString, QWidget*> it(d->knownWidget);

    while (it.hasNext())
    {
        it.next();
        widget                          = it.value();
        KConfigSkeletonItem* const item = d->conf->findItem(it.key());

        if (!item)
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "The setting '" << it.key() << "' has disappeared!";
            continue;
        }

        if (!item->isEqual(property(widget)))
        {
            return true;
        }
    }

    return false;
}

bool DConfigDlgMngr::isDefault() const
{
    bool bUseDefaults = d->conf->useDefaults(true);
    bool result       = !hasChanged();
    d->conf->useDefaults(bUseDefaults);

    return result;
}

} // namespace Digikam

#include "moc_dconfigdlgmngr.cpp"
