/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-04-07
 * Description : Solid hardware list dialog
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "solidhardwaredlg.h"

// Qt includes

#include <QStringList>
#include <QDateTime>
#include <QString>
#include <QLabel>
#include <QLayout>
#include <QGridLayout>
#include <QTreeWidget>
#include <QHeaderView>
#include <QMimeData>
#include <QClipboard>
#include <QApplication>
#include <QPushButton>
#include <QDialogButtonBox>

// KDE includes

#include <klocalizedstring.h>

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wnonportable-include-path"
#endif

#include <solid_version.h>
#include <solid/solidnamespace.h>
#include <solid/camera.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/devicenotifier.h>
#include <solid/genericinterface.h>

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Local includes

#include "digikam_config.h"

using namespace std;

namespace Digikam
{

class Q_DECL_HIDDEN SolidHardwareDlg::Private
{
public:

    Private() = default;

    QLabel*        header       = nullptr;
    SearchTextBar* searchBar    = nullptr;
    QTreeWidget*   eventsList   = nullptr;
};

SolidHardwareDlg::SolidHardwareDlg(QWidget* const parent)
    : InfoDlg(parent),
      d      (new Private)
{
    setWindowTitle(i18nc("@title:window", "List of Detected Hardware"));

    d->header     = new QLabel(this);
    d->header->setText(i18nc("@label", "%1 uses Solid framework version %2\n"
                            "to detect and manage devices from your computer.\n"
                            "Press \"Refresh\" to update list if you plug a removable device.",
                       QApplication::applicationName(),
                       QLatin1String(SOLID_VERSION_STRING)));
    d->searchBar  = new SearchTextBar(this, QLatin1String("SolidHardwareDlgSearchBar"));

    listView()->setHeaderLabels(QStringList() << QLatin1String("Properties") << QLatin1String("Value")); // Hidden header -> no i18n
    listView()->setSortingEnabled(true);
    listView()->setRootIsDecorated(true);
    listView()->setSelectionMode(QAbstractItemView::SingleSelection);
    listView()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    listView()->setAllColumnsShowFocus(true);
    listView()->setColumnCount(2);
    listView()->header()->setSectionResizeMode(QHeaderView::Stretch);
    listView()->header()->hide();

    // --------------------------------------------------------

    d->eventsList = new QTreeWidget(this);
    d->eventsList->setSortingEnabled(true);
    d->eventsList->setRootIsDecorated(true);
    d->eventsList->setSelectionMode(QAbstractItemView::SingleSelection);
    d->eventsList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->eventsList->setAllColumnsShowFocus(true);
    d->eventsList->setColumnCount(2);
    d->eventsList->sortItems(0, Qt::AscendingOrder);
    d->eventsList->header()->setSectionResizeMode(QHeaderView::Stretch);
    d->eventsList->header()->hide();

    tabView()->addTab(d->eventsList, i18nc("@title", "Hotplug Device Events"));
    tabView()->setTabText(0, i18nc("@title", "Devices List"));

    Solid::DeviceNotifier* const notifier = Solid::DeviceNotifier::instance();

    connect(notifier, SIGNAL(deviceAdded(QString)),
            this, SLOT(slotDeviceAdded(QString)));

    connect(notifier, SIGNAL(deviceRemoved(QString)),
            this, SLOT(slotDeviceRemoved(QString)));

    // --------------------------------------------------------

    buttonBox()->addButton(QDialogButtonBox::Reset);
    buttonBox()->button(QDialogButtonBox::Reset)->setText(i18nc("@action", "Refresh"));

    // --------------------------------------------------------

    QGridLayout* const  grid = dynamic_cast<QGridLayout*>(mainWidget()->layout());
    grid->addWidget(d->header,     1, 0, 1, -1);
    grid->addWidget(d->searchBar,  3, 0, 1, -1);

    // --------------------------------------------------------

    connect(d->searchBar, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            this, SLOT(slotSearchTextChanged(SearchTextSettings)));

    connect(buttonBox()->button(QDialogButtonBox::Reset), SIGNAL(clicked()),
            this, SLOT(slotPopulateDevices()));

    // --------------------------------------------------------

    slotPopulateDevices();
}

SolidHardwareDlg::~SolidHardwareDlg()
{
    delete d;
}

void SolidHardwareDlg::slotPopulateDevices()
{
    listView()->clear();

    const QList<Solid::Device> all = Solid::Device::allDevices();

    for (const Solid::Device& device : all)
    {
        QString typeStr;
        QString typeDesc;

        if      (device.isDeviceInterface(Solid::DeviceInterface::StorageDrive))
        {
            typeStr  = Solid::DeviceInterface::typeToString(Solid::DeviceInterface::StorageDrive);
            typeDesc = Solid::DeviceInterface::typeDescription(Solid::DeviceInterface::StorageDrive);
        }
        else if (device.isDeviceInterface(Solid::DeviceInterface::StorageAccess))
        {
            typeStr  = Solid::DeviceInterface::typeToString(Solid::DeviceInterface::StorageAccess);
            typeDesc = Solid::DeviceInterface::typeDescription(Solid::DeviceInterface::StorageAccess);
        }
        else if (device.isDeviceInterface(Solid::DeviceInterface::StorageVolume))
        {
            typeStr  = Solid::DeviceInterface::typeToString(Solid::DeviceInterface::StorageVolume);
            typeDesc = Solid::DeviceInterface::typeDescription(Solid::DeviceInterface::StorageVolume);
        }
        else if (device.isDeviceInterface(Solid::DeviceInterface::OpticalDrive))
        {
            typeStr  = Solid::DeviceInterface::typeToString(Solid::DeviceInterface::OpticalDrive);
            typeDesc = Solid::DeviceInterface::typeDescription(Solid::DeviceInterface::OpticalDrive);
        }
        else if (device.isDeviceInterface(Solid::DeviceInterface::OpticalDisc))
        {
            typeStr  = Solid::DeviceInterface::typeToString(Solid::DeviceInterface::OpticalDisc);
            typeDesc = Solid::DeviceInterface::typeDescription(Solid::DeviceInterface::OpticalDisc);
        }
        else if (device.isDeviceInterface(Solid::DeviceInterface::Camera))
        {
            typeStr  = Solid::DeviceInterface::typeToString(Solid::DeviceInterface::Camera);
            typeDesc = Solid::DeviceInterface::typeDescription(Solid::DeviceInterface::Camera);
        }
        else if (device.isDeviceInterface(Solid::DeviceInterface::Processor))
        {
            typeStr  = Solid::DeviceInterface::typeToString(Solid::DeviceInterface::Processor);
            typeDesc = Solid::DeviceInterface::typeDescription(Solid::DeviceInterface::Processor);
        }
        else if (device.isDeviceInterface(Solid::DeviceInterface::Block))
        {
            typeStr  = Solid::DeviceInterface::typeToString(Solid::DeviceInterface::Block);
            typeDesc = Solid::DeviceInterface::typeDescription(Solid::DeviceInterface::Block);
        }
        else if (device.isDeviceInterface(Solid::DeviceInterface::PortableMediaPlayer))
        {
            typeStr  = Solid::DeviceInterface::typeToString(Solid::DeviceInterface::PortableMediaPlayer);
            typeDesc = Solid::DeviceInterface::typeDescription(Solid::DeviceInterface::PortableMediaPlayer);
        }
        else if (device.isDeviceInterface(Solid::DeviceInterface::NetworkShare))
        {
            typeStr  = Solid::DeviceInterface::typeToString(Solid::DeviceInterface::NetworkShare);
            typeDesc = Solid::DeviceInterface::typeDescription(Solid::DeviceInterface::NetworkShare);
        }
        else if (device.isDeviceInterface(Solid::DeviceInterface::Unknown))
        {
            typeStr  = Solid::DeviceInterface::typeToString(Solid::DeviceInterface::Unknown);
            typeDesc = Solid::DeviceInterface::typeDescription(Solid::DeviceInterface::Unknown);
        }

        if (!typeStr.isEmpty())
        {
            // NOTE: Data UserRole in column 0 is used to store index level of tree widget item.
            //       This value is used later in copy to clipboard to render hierarchy as text.

            QList<QTreeWidgetItem*> lst = listView()->findItems(typeStr, Qt::MatchExactly);
            QTreeWidgetItem* hitem      = nullptr;

            if (!lst.isEmpty())
            {
                hitem = lst[0];
            }
            else
            {
                hitem = new QTreeWidgetItem(listView(), QStringList() << typeStr << typeDesc);
                hitem->setData(0, Qt::UserRole, 0);
                listView()->addTopLevelItem(hitem);
            }

            QTreeWidgetItem* vitem       = nullptr;
            QString title;

#if (SOLID_VERSION >= QT_VERSION_CHECK(5, 71, 0))

            title                        = device.displayName();

#endif

            // cppcheck-suppress knownConditionTrueFalse
            if (title.isEmpty())
            {
                if (!device.vendor().isEmpty())
                {
                    title = device.vendor();
                }

                if (!device.product().isEmpty())
                {
                    title += QLatin1String(" - ") + device.product();
                }
            }

            if (title.isEmpty())
            {
                title = device.udi().section(QLatin1Char('/'), -1);
            }

            QTreeWidgetItem* const titem = new QTreeWidgetItem(hitem, QStringList() << title);
            titem->setData(0, Qt::UserRole, 1);

            vitem = new QTreeWidgetItem(titem, QStringList() << i18nc("@item: universal device ID", "Udi")         << (device.udi().isEmpty()         ? i18nc("@item: device property", "empty") : device.udi()));
            vitem->setData(0, Qt::UserRole, 2);

            vitem = new QTreeWidgetItem(titem, QStringList() << i18nc("@item: parent device ID",    "Parent Udi")  << (device.parentUdi().isEmpty()   ? i18nc("@item: device property", "none")  : device.parentUdi()));
            vitem->setData(0, Qt::UserRole, 2);

            vitem = new QTreeWidgetItem(titem, QStringList() << i18nc("@item; device constructor",  "Vendor")      << (device.vendor().isEmpty()      ? i18nc("@item: device property", "empty") : device.vendor()));
            vitem->setData(0, Qt::UserRole, 2);

            vitem = new QTreeWidgetItem(titem, QStringList() << i18nc("@item: device name",         "Product")     << (device.product().isEmpty()     ? i18nc("@item: device property", "empty") : device.product()));
            vitem->setData(0, Qt::UserRole, 2);

            vitem = new QTreeWidgetItem(titem, QStringList() << i18nc("@item: device description",  "Description") << (device.description().isEmpty() ? i18nc("@item: device property", "empty") : device.description()));
            vitem->setData(0, Qt::UserRole, 2);

            vitem = new QTreeWidgetItem(titem, QStringList() << i18nc("@item: device status",       "States")      << (device.emblems().isEmpty()     ? i18nc("@item: device property", "none")  : device.emblems().join(QLatin1String(", "))));
            vitem->setData(0, Qt::UserRole, 2);

#ifndef Q_OS_WIN

            if (device.is<Solid::GenericInterface>())
            {
                QTreeWidgetItem* const vitem2 = new QTreeWidgetItem(titem, QStringList() << i18nc("@item", "Properties") << i18nc("@item", "Non-portable info"));
                vitem2->setData(0, Qt::UserRole, 2);

                QMap<QString, QVariant> properties = device.as<Solid::GenericInterface>()->allProperties();

                for (auto it = properties.constBegin() ; it != properties.constEnd() ; ++it)
                {
                    QTreeWidgetItem* const pitem = new QTreeWidgetItem(vitem2, QStringList() << it.key() << it.value().toString());
                    pitem->setData(0, Qt::UserRole, 3);
                }
            }

#endif

        }
    }
}

QTreeWidget* SolidHardwareDlg::currentTreeView() const
{
    if (tabView()->currentIndex() == 0)
    {
        return (listView());
    }

    return (d->eventsList);
}

void SolidHardwareDlg::slotSearchTextChanged(const SearchTextSettings& settings)
{
    bool query     = false;
    int  results   = 0;
    Q_UNUSED(results);
    QString search = settings.text.toLower();

    QTreeWidgetItemIterator it(currentTreeView());

    while (*it)
    {
        QTreeWidgetItem* const item  = *it;

        if (
            item->text(0).toLower().contains(search, settings.caseSensitive) ||
            item->text(1).toLower().contains(search, settings.caseSensitive)
           )
        {
            ++results;
            query = true;
            item->setHidden(false);
        }
        else
        {
            item->setHidden(true);
        }

        ++it;
    }

    d->searchBar->slotSearchResult(query);
}

void SolidHardwareDlg::slotCopy2ClipBoard()
{
    QString textInfo;

    textInfo.append(QApplication::applicationName());
    textInfo.append(QLatin1String(" version "));
    textInfo.append(QApplication::applicationVersion());
    textInfo.append(QLatin1Char('\n'));
    textInfo.append(QLatin1String("Solid framework version "));
    textInfo.append(QLatin1String(SOLID_VERSION_STRING));
    textInfo.append(QLatin1Char('\n'));

    QTreeWidgetItemIterator it(currentTreeView());

    while (*it)
    {
        int id = (*it)->data(0, Qt::UserRole).toInt();
        textInfo.append(QString().fill(QLatin1Char(' '), id*3));
        textInfo.append((*it)->text(0));
        textInfo.append(QLatin1String(": "));
        textInfo.append((*it)->text(1));
        textInfo.append(QLatin1Char('\n'));
        ++it;
    }

    QMimeData* const mimeData = new QMimeData();
    mimeData->setText(textInfo);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

void SolidHardwareDlg::slotDeviceAdded(const QString& udi)
{
    QTreeWidgetItem* const titem = new QTreeWidgetItem(d->eventsList, QStringList()
                                           << QDateTime::currentDateTime().toString(Qt::ISODate)
                                           << i18nc("@item", "New Device Added"));
    titem->setData(0, Qt::UserRole, 0);

    const Solid::Device device(udi);

    QTreeWidgetItem* vitem = nullptr;

    vitem = new QTreeWidgetItem(titem, QStringList() << i18nc("@item: universal device ID", "Udi")         << udi);
    vitem->setData(0, Qt::UserRole, 1);

    vitem = new QTreeWidgetItem(titem, QStringList() << i18nc("@item: parent device ID",    "Parent Udi")  << (device.parentUdi().isEmpty()   ? i18nc("@info: device property", "none")  : device.parentUdi()));
    vitem->setData(0, Qt::UserRole, 1);

    vitem = new QTreeWidgetItem(titem, QStringList() << i18nc("@item: device constructor",  "Vendor")      << (device.vendor().isEmpty()      ? i18nc("@info: device property", "empty") : device.vendor()));
    vitem->setData(0, Qt::UserRole, 1);

    vitem = new QTreeWidgetItem(titem, QStringList() << i18nc("@item: device name",         "Product")     << (device.product().isEmpty()     ? i18nc("@info: device property", "empty") : device.product()));
    vitem->setData(0, Qt::UserRole, 1);

    vitem = new QTreeWidgetItem(titem, QStringList() << i18nc("@item: device description",  "Description") << (device.description().isEmpty() ? i18nc("@info: device property", "empty") : device.description()));
    vitem->setData(0, Qt::UserRole, 1);

    vitem = new QTreeWidgetItem(titem, QStringList() << i18nc("@item: device status",       "States")      << (device.emblems().isEmpty()     ? i18nc("@info: device property", "none")  : device.emblems().join(QLatin1String(", "))));
    vitem->setData(0, Qt::UserRole, 1);

    titem->setExpanded(true);
}

void SolidHardwareDlg::slotDeviceRemoved(const QString& udi)
{
    QTreeWidgetItem* const titem = new QTreeWidgetItem(d->eventsList, QStringList()
                                           << QDateTime::currentDateTime().toString(Qt::ISODate)
                                           << i18nc("@item", "Device Removed"));
    titem->setData(0, Qt::UserRole, 0);

    QTreeWidgetItem* const vitem = new QTreeWidgetItem(titem, QStringList() << i18nc("@item", "Udi") << udi);
    vitem->setData(0, Qt::UserRole, 1);

    titem->setExpanded(true);
}

} // namespace Digikam

#include "moc_solidhardwaredlg.cpp"
