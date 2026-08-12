/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database settings widget
 *
 * SPDX-FileCopyrightText: 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dbsettingswidget_p.h"

namespace Digikam
{

DatabaseSettingsWidget::DatabaseSettingsWidget(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    setupMainArea();
}

DatabaseSettingsWidget::~DatabaseSettingsWidget()
{
    delete d;
}

void DatabaseSettingsWidget::setupMainArea()
{
    QVBoxLayout* const layout       = new QVBoxLayout();
    setLayout(layout);

    // --------------------------------------------------------

    const int spacing = layoutSpacing();

    QGroupBox* const dbConfigBox    = new QGroupBox(i18n("Database Configuration"), this);
    QVBoxLayout* const vlay         = new QVBoxLayout(dbConfigBox);

    DHBox* const typeHbox           = new DHBox();
    QLabel* const databaseTypeLabel = new QLabel(typeHbox);
    d->dbType                       = new QComboBox(typeHbox);
    QWidget* const spacer           = new QWidget(typeHbox);
    databaseTypeLabel->setText(i18n("Type: "));
    typeHbox->setStretchFactor(spacer, 10);

    // --------- fill with default values ---------------------

    int dbTypeIdx               = 0;
    d->dbType->addItem(i18n("SQLite"),         SQlite);
    d->dbTypeMap[SQlite]        = dbTypeIdx++;

#ifdef HAVE_MYSQLSUPPORT

#   ifdef HAVE_INTERNALMYSQL

    d->dbType->addItem(i18n("Mysql Internal"), MysqlInternal);
    d->dbTypeMap[MysqlInternal] = dbTypeIdx++;

#   endif

    d->dbType->addItem(i18n("MySQL Server"),   MysqlServer);
    d->dbTypeMap[MysqlServer]   = dbTypeIdx++;

#endif

    QString tip = i18n("<p>Select here the type of database backend.</p>"
                       "<p><b>SQlite</b> backend is for local database storage with a small or medium collection sizes. "
                       "It is the default and recommended backend for collections with less than 100K items.</p>");


#ifdef HAVE_MYSQLSUPPORT

#   ifdef HAVE_INTERNALMYSQL

    tip.append(i18n("<p><b>MySQL Internal</b> backend is for local database storage with huge collection sizes. "
                    "This backend is recommend for local collections with more than 100K items.</p>"));

#   endif

    tip.append(i18n("<p><b>MySQL Server</b> backend is a more robust solution especially for remote and shared database storage. "
                    "It is also more efficient to manage huge collection sizes with more than 100K items.</p>"));

#endif

    d->dbTypeBtn           = new QPushButton(typeHbox);
    d->dbTypeBtn->setIcon(QIcon::fromTheme(QLatin1String("dialog-information")));
    d->dbTypeBtn->setToolTip(i18nc("@info:tooltip", "Get information about <b>Database Type</b>"));
    d->dbTypeBtn->setWhatsThis(tip);

    connect(d->dbTypeBtn, &QPushButton::clicked,
            this, [this]()
        {
            qApp->postEvent(d->dbTypeBtn, new QHelpEvent(QEvent::WhatsThis, QPoint(0, 0), QCursor::pos()));
        }
    );

    // --------------------------------------------------------

    DHBox* const dbPathBox = new DHBox(dbConfigBox);

    d->dbPathLbl           = new QLabel(i18nc("@label:database path", "Path: "), dbPathBox);

    d->dbPathEdit          = new DFileSelector(dbPathBox);
    d->dbPathEdit->setFileDlgMode(QFileDialog::Directory);
    d->dbPathEdit->setFileDlgOptions(QFileDialog::ShowDirsOnly);

    d->dbPathBtn           = new QPushButton(dbPathBox);
    d->dbPathBtn->setIcon(QIcon::fromTheme(QLatin1String("dialog-information")));
    d->dbPathBtn->setToolTip(i18nc("@info:tooltip", "Get information about <b>Database Path</b>"));
    d->dbPathBtn->setWhatsThis(i18nc("@info:tooltip",
                                     "<p>Set here the location where the database files will be stored on your system. "
                                     "There are four databases: "
                                     "one for all collections properties, "
                                     "one to store compressed thumbnails, "
                                     "one to store faces recognition metadata, "
                                     "and one to store similarity fingerprints.<br>"
                                     "Write access is required to be able to edit image properties.</p>"
                                     "<p>Databases are digiKam core engines. Take care to use a place hosted by fast "
                                     "hardware (eg. SSD or NVMe) with enough free space especially for thumbnails database.</p>"
                                     "<p>Note: a remote file system such as NFS, cannot be used here. "
                                     "For performance reasons, it is also recommended not to use network storage media.</p>"));

    connect(d->dbPathBtn, &QPushButton::clicked,
            this, [this]()
        {
            qApp->postEvent(d->dbPathBtn, new QHelpEvent(QEvent::WhatsThis, QPoint(0, 0), QCursor::pos()));
        }
    );


    // --------------------------------------------------------

    DHBox* const walModeBox = new DHBox(dbConfigBox);

    d->walModeCheck         = new QCheckBox(i18n("Enable WAL mode for the databases"), walModeBox);
    d->walModeCheck->setToolTip(i18n("The WAL (Write-Ahead Log) mode is significantly "
                                     "faster in most scenarios on supported systems."));

    QWidget* const spacer2  = new QWidget(walModeBox);
    walModeBox->setStretchFactor(spacer2, 10);
    d->walModeBtn           = new QPushButton(walModeBox);
    d->walModeBtn->setIcon(QIcon::fromTheme(QLatin1String("dialog-information")));
    d->walModeBtn->setToolTip(i18nc("@info:tooltip", "Get information about <b>WAL Mode</b>"));
    d->walModeBtn->setWhatsThis(i18n("Write-Ahead Log is a mode to use a roll-forward journal that records transactions "
                                     "that have been committed but not yet applied to the databases. It uses an auxiliary "
                                     "journalized file to host structures for recovery transactions during a crash. The changes "
                                     "are first recorded in the log, before the changes are written to the database. This made "
                                     "database requests atomic and robust in extensive and critical use cases."));

    connect(d->walModeBtn, &QPushButton::clicked,
            this, [this]()
        {
            qApp->postEvent(d->walModeBtn, new QHelpEvent(QEvent::WhatsThis, QPoint(0, 0), QCursor::pos()));
        }
    );

    // --------------------------------------------------------

    d->mysqlCmdBox = new DVBox(dbConfigBox);
    d->mysqlCmdBox->layout()->setContentsMargins(QMargins());

    new DLineWidget(Qt::Horizontal, d->mysqlCmdBox);
    QLabel* const mysqlBinariesLabel  = new QLabel(i18n("<p>Here you can configure locations where MySQL binary tools are located. "
                                                        "digiKam will try to find these binaries automatically if they are "
                                                        "already installed on your computer.</p>"),
                                                   d->mysqlCmdBox);
    mysqlBinariesLabel->setWordWrap(true);

    QGroupBox* const binaryBox        = new QGroupBox(d->mysqlCmdBox);
    QGridLayout* const binaryLayout   = new QGridLayout;
    binaryBox->setLayout(binaryLayout);
    binaryBox->setTitle(i18nc("@title:group", "MySQL Binaries"));
    d->dbBinariesWidget               = new DBinarySearch(binaryBox);
    d->dbBinariesWidget->header()->setSectionHidden(2, true);

    d->dbBinariesWidget->addBinary(d->mysqlServerBin);
    d->dbBinariesWidget->addBinary(d->mysqlAdminBin);
    d->dbBinariesWidget->addBinary(d->mysqlUpgradeBin);
    d->dbBinariesWidget->addBinary(d->mysqlInitBin);

#ifdef Q_OS_LINUX

    d->dbBinariesWidget->addDirectory(QLatin1String("/usr/bin"));
    d->dbBinariesWidget->addDirectory(QLatin1String("/usr/sbin"));

#endif

#ifdef Q_OS_MACOS

    // Std Macports install

    d->dbBinariesWidget->addDirectory(QLatin1String("/opt/local/bin"));
    d->dbBinariesWidget->addDirectory(QLatin1String("/opt/local/sbin"));
    d->dbBinariesWidget->addDirectory(QLatin1String("/opt/local/lib/mariadb/bin"));

    // digiKam Bundle PKG install

    d->dbBinariesWidget->addDirectory(macOSBundlePrefix() + QLatin1String("lib/mariadb/bin"));

#endif

#ifdef Q_OS_WIN

    d->dbBinariesWidget->addDirectory(QLatin1String("C:/Program Files/MariaDB 10.5/bin"));
    d->dbBinariesWidget->addDirectory(QLatin1String("C:/Program Files/MariaDB 11.4/bin"));

#endif

    d->dbBinariesWidget->allBinariesFound();

    // --------------------------------------------------------

    d->tab                                           = new QTabWidget(this);

    QLabel* const hostNameLabel                      = new QLabel(i18n("Host Name:"));
    d->hostName                                      = new QLineEdit();
    d->hostName->setPlaceholderText(i18n("Set the host computer name"));
    d->hostName->setToolTip(i18n("This is the computer name running MySQL server.\n"
                                 "This can be \"localhost\" for a local server, "
                                 "or the network computer\n"
                                 "name (or IP address) in case of remote computer."));

    QLabel* const connectOptsLabel                   = new QLabel(i18n("<a href=\"https://doc.qt.io/qt-6/"
                                                                       "sql-driver.html#qmysql\">Connect options:</a>"));
    connectOptsLabel->setOpenExternalLinks(true);
    d->connectOpts                                   = new QLineEdit();
    d->connectOpts->setPlaceholderText(i18n("Set the database connection options"));
    d->connectOpts->setToolTip(i18n("Set the MySQL server connection options.\n"
                                    "For advanced users only."));

    QLabel* const userNameLabel                      = new QLabel(i18n("User:"));
    d->userName                                      = new QLineEdit();
    d->userName->setPlaceholderText(i18n("Set the database account name"));
    d->userName->setToolTip(i18n("Set the MySQL server account name used\n"
                                 "by digiKam to be connected to the server.\n"
                                 "This account must be available on the remote\n"
                                 "MySQL server when database have been created."));

    QLabel* const passwordLabel                      = new QLabel(i18n("Password:"));
    d->password                                      = new QLineEdit();
    d->password->setToolTip(i18n("Set the MySQL server account password used\n"
                                 "by digiKam to be connected to the server.\n"
                                 "You can left this field empty to use an account set without password."));
    d->password->setEchoMode(QLineEdit::Password);

    DHBox* const phbox                               = new DHBox();
    QLabel* const hostPortLabel                      = new QLabel(i18n("Host Port:"));
    d->hostPort                                      = new QSpinBox(phbox);
    d->hostPort->setToolTip(i18n("Set the host computer port.\nUsually, MySQL server use port number 3306 by default"));
    d->hostPort->setMaximum(65535);
    d->hostPort->setValue(3306);
    QWidget* const space                             = new QWidget(phbox);
    phbox->setStretchFactor(space, 10);
    QPushButton* const checkDBConnectBtn             = new QPushButton(i18n("Check Connection"), phbox);
    checkDBConnectBtn->setToolTip(i18n("Run a basic database connection to see if current MySQL server settings is suitable."));

    // Only accept printable Ascii char for database names.

    QRegularExpression asciiRx(QLatin1String("[\x20-\x7F]+$"));
    QValidator* const asciiValidator = new QRegularExpressionValidator(asciiRx, this);

    QLabel* const dbNameCoreLabel                    = new QLabel(i18n("Core Db Name:"));
    d->dbNameCore                                    = new QLineEdit();
    d->dbNameCore->setPlaceholderText(i18n("Set the core database name"));
    d->dbNameCore->setToolTip(i18n("The core database is lead digiKam container used to store\n"
                                   "albums, items, and searches metadata."));
    d->dbNameCore->setValidator(asciiValidator);

    d->dbThumbsLabel                                 = new QLabel(i18n("Thumbs Db Name:"));
    d->dbNameThumbs                                  = new DFileSelector();
    d->dbNameThumbs->setFileDlgMode(QFileDialog::Directory);
    d->dbNameThumbs->setFileDlgOptions(QFileDialog::ShowDirsOnly);
    d->dbNameThumbs->lineEdit()->setPlaceholderText(i18n("Set the thumbnails database name or folder"));
    d->dbNameThumbs->setToolTip(i18n("The thumbnails database is used by digiKam to host\n"
                                     "image thumbs with wavelets compression images.\n"
                                     "This one can use quickly a lots of space,\n"
                                     "especially if you have huge collections.\n"
                                     "Choose a local folder to use a SQLite database."));

    QLabel* const dbNameFaceLabel                    = new QLabel(i18n("Face Db Name:"));
    d->dbNameFace                                    = new QLineEdit();
    d->dbNameFace->setPlaceholderText(i18n("Set the face database name"));
    d->dbNameFace->setToolTip(i18n("The face database is used by digiKam to host image histograms\n"
                                   "dedicated to faces recognition process.\n"
                                   "This one can use quickly a lots of space, especially\n"
                                   "if you a lots of image with people faces detected and tagged."));
    d->dbNameFace->setValidator(asciiValidator);

    QLabel* const dbNameSimilarityLabel              = new QLabel(i18n("Similarity Db Name:"));
    d->dbNameSimilarity                              = new QLineEdit();
    d->dbNameSimilarity->setPlaceholderText(i18n("Set the similarity database name"));
    d->dbNameSimilarity->setToolTip(i18n("The similarity database is used by digiKam to host\n"
                                         "image Haar matrix data for the similarity search."));
    d->dbNameSimilarity->setValidator(asciiValidator);

    QPushButton* const defaultValuesBtn              = new QPushButton(i18n("Default Settings"));
    defaultValuesBtn->setToolTip(i18n("Reset database names settings to common default values."));

    d->expertSettings                                = new QGroupBox();
    d->expertSettings->setFlat(true);
    QFormLayout* const expertSettinglayout           = new QFormLayout();
    d->expertSettings->setLayout(expertSettinglayout);

    expertSettinglayout->addRow(hostNameLabel,         d->hostName);
    expertSettinglayout->addRow(userNameLabel,         d->userName);
    expertSettinglayout->addRow(passwordLabel,         d->password);
    expertSettinglayout->addRow(connectOptsLabel,      d->connectOpts);
    expertSettinglayout->addRow(hostPortLabel,         phbox);
    expertSettinglayout->addRow(new DLineWidget(Qt::Horizontal, d->expertSettings));
    expertSettinglayout->addRow(dbNameCoreLabel,       d->dbNameCore);
    expertSettinglayout->addRow(d->dbThumbsLabel,      d->dbNameThumbs);
    expertSettinglayout->addRow(dbNameFaceLabel,       d->dbNameFace);
    expertSettinglayout->addRow(dbNameSimilarityLabel, d->dbNameSimilarity);
    expertSettinglayout->addRow(new QWidget(),         defaultValuesBtn);

    d->tab->addTab(d->expertSettings, i18n("Remote Server Settings"));

    // --------------------------------------------------------

    d->dbNoticeBox           = new QGroupBox(i18n("Database Server Instructions"), this);
    QVBoxLayout* const vlay2 = new QVBoxLayout(d->dbNoticeBox);
    QLabel* const notice     = new QLabel(i18n("<p>digiKam expects that database is already created with a dedicated user account. "
                                               "This user name <i>digikam</i> will require full access to the database.<br>"
                                               "If your database is not already set up, you can use the following SQL commands "
                                               "(after replacing the <b><i>password</i></b> with the correct one).</p>"),
                                          d->dbNoticeBox);
    notice->setWordWrap(true);

    d->sqlInit                  = new QTextBrowser(d->dbNoticeBox);
    d->sqlInit->setOpenExternalLinks(false);
    d->sqlInit->setOpenLinks(false);
    d->sqlInit->setReadOnly(false);

    QTextBrowser* const notice2 = new QTextBrowser(this);
    notice2->setText(i18n("<p>Note: with a Linux server, a database can be initialized following the commands below:</p>"
                          "<p># su</p>"
                          "<p># systemctl restart mysqld</p>"
                          "<p># mysql -u root</p>"
                          "<p>...</p>"
                          "<p>Enter SQL code to Mysql prompt in order to init digiKam databases with grant privileges (see behind)</p>"
                          "<p>...</p>"
                          "<p>quit</p>"
                          "<p>NOTE: If you have problems with a MySQL server on Ubuntu based Linux system, "
                          "use the addition command in the mysql prompt to be able to create MySQL triggers.<br>"
                          "SET GLOBAL log_bin_trust_function_creators=1;</p>"
                          "<p>NOTE: If you have an enormous collection, you should start MySQL server with "
                          "mysql --max_allowed_packet=128M OR in my.ini or ~/.my.cnf, change the settings</p>"));

    notice2->setOpenExternalLinks(false);
    notice2->setOpenLinks(false);
    notice2->setReadOnly(true);

    vlay2->addWidget(notice,     0);
    vlay2->addWidget(d->sqlInit, 10);
    vlay2->addWidget(notice2,    20);
    vlay2->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay2->setSpacing(spacing);

    d->tab->addTab(d->dbNoticeBox, i18n("Requirements"));

    // --------------------------------------------------------

    d->dbDetailsBox          = new QGroupBox(i18n("Database Server Technical Details"), this);
    QVBoxLayout* const vlay3 = new QVBoxLayout(d->dbDetailsBox);
    QLabel* const details    = new QLabel(i18n("<p>Use this configuration view to set all information "
                                               "to be connected to a remote "
                                               "<a href=\"https://en.wikipedia.org/wiki/MySQL\">Mysql database server</a> "
                                               "(or <a href=\"https://en.wikipedia.org/wiki/MariaDB\">MariaDB</a>) "
                                               "through a network. "
                                               "As with Sqlite or MySQL internal server, 3 databases will be stored "
                                               "on the remote server: one for all collections properties, "
                                               "one to store compressed thumbnails, and one to store faces "
                                               "recognition metadata.</p>"
                                               "<p>Unlike Sqlite or MySQL internal server, you can customize the "
                                               "database names to simplify your backups.</p>"
                                               "<p>Databases are digiKam core engines. To prevent performance issues, "
                                               "take a care to use a fast network link between the client and the server "
                                               "computers. It is also recommended to host database files on "
                                               "fast hardware (as <a href=\"https://en.wikipedia.org/wiki/Solid-state_drive\">SSD</a>) "
                                               "with enough free space, especially for thumbnails database, even if data are compressed using wavelets image format <a href=\"https://en.wikipedia.org/wiki/Progressive_Graphics_File\">"
                                               "PGF</a>.</p>"
                                               "<p>The databases must be created previously on the remote server by the administrator. "
                                               "Look in <b>Requirements</b> tab for details.</p>"),
                                          d->dbDetailsBox);
    details->setWordWrap(true);

    vlay3->addWidget(details);
    vlay3->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay3->setSpacing(spacing);

    d->tab->addTab(d->dbDetailsBox, i18n("Documentation"));

    // --------------------------------------------------------

    vlay->addWidget(typeHbox);
    vlay->addWidget(new DLineWidget(Qt::Horizontal));
    vlay->addWidget(dbPathBox);
    vlay->addWidget(new DLineWidget(Qt::Horizontal));
    vlay->addWidget(d->mysqlCmdBox);
    vlay->addWidget(d->tab);
    vlay->addWidget(walModeBox);
    vlay->addStretch(10);
    vlay->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay->setSpacing(spacing);

    // --------------------------------------------------------

    layout->setContentsMargins(QMargins());
    layout->setSpacing(spacing);
    layout->addWidget(dbConfigBox);

    // --------------------------------------------------------

    connect(d->dbType, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotHandleDBTypeIndexChanged(int)));

    connect(checkDBConnectBtn, SIGNAL(clicked()),
            this, SLOT(slotCheckMysqlServerConnection()));

    connect(defaultValuesBtn, SIGNAL(clicked()),
            this, SLOT(slotResetMysqlServerDBNames()));

    connect(d->dbNameCore, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    connect(d->dbNameThumbs->lineEdit(), SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    connect(d->dbNameFace, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    connect(d->dbNameSimilarity, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    connect(d->userName, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    slotHandleDBTypeIndexChanged(d->dbType->currentIndex());
}

void DatabaseSettingsWidget::slotUpdateSqlInit()
{
    QString sql = QString::fromLatin1("CREATE USER \'%1\'@\'%2\' IDENTIFIED BY \'<b>password</b>\';<br>")
                                      .arg(d->userName->text())
                                      .arg(d->hostName->text());

    sql += QString::fromLatin1("GRANT ALL ON *.* TO \'%1\'@\'%2\' IDENTIFIED BY \'<b>password</b>\';<br>")
                               .arg(d->userName->text())
                               .arg(d->hostName->text());

    sql += QString::fromLatin1("CREATE DATABASE `%1`;<br>"
                               "GRANT ALL PRIVILEGES ON `%2`.* TO \'%3\'@\'%4\';<br>")
                               .arg(d->dbNameCore->text())
                               .arg(d->dbNameCore->text())
                               .arg(d->userName->text())
                               .arg(d->hostName->text());

    if (isNotEqualToThumbName(d->dbNameCore->text()))
    {
        sql += QString::fromLatin1("CREATE DATABASE `%1`;<br>"
                                   "GRANT ALL PRIVILEGES ON `%2`.* TO \'%3\'@\'%4\';<br>")
                                   .arg(d->dbNameThumbs->fileDlgPath())
                                   .arg(d->dbNameThumbs->fileDlgPath())
                                   .arg(d->userName->text())
                                   .arg(d->hostName->text());
    }

    if (
        isNotEqualToThumbName(d->dbNameFace->text())   &&
        (d->dbNameFace->text() != d->dbNameCore->text())
       )
    {
        sql += QString::fromLatin1("CREATE DATABASE `%1`;<br>"
                                   "GRANT ALL PRIVILEGES ON `%2`.* TO \'%3\'@\'%4\';<br>")
                                   .arg(d->dbNameFace->text())
                                   .arg(d->dbNameFace->text())
                                   .arg(d->userName->text())
                                   .arg(d->hostName->text());
    }

    if (
        isNotEqualToThumbName(d->dbNameSimilarity->text())     &&
        (d->dbNameSimilarity->text() != d->dbNameCore->text()) &&
        (d->dbNameSimilarity->text() != d->dbNameFace->text())
       )
    {
        sql += QString::fromLatin1("CREATE DATABASE `%1`;<br>"
                                   "GRANT ALL PRIVILEGES ON `%2`.* TO \'%3\'@\'%4\';<br>")
                                   .arg(d->dbNameSimilarity->text())
                                   .arg(d->dbNameSimilarity->text())
                                   .arg(d->userName->text())
                                   .arg(d->hostName->text());
    }

    sql += QLatin1String("FLUSH PRIVILEGES;<br>");

    d->sqlInit->setText(sql);

    QFileInfo thumbDB(d->dbNameThumbs->fileDlgPath());

    if (thumbDB.exists() && thumbDB.isDir())
    {
        d->dbThumbsLabel->setText(i18n("Thumbs Db Folder:"));
    }
    else
    {
        d->dbThumbsLabel->setText(i18n("Thumbs Db Name:"));
    }
}

} // namespace Digikam

#include "moc_dbsettingswidget.cpp"
