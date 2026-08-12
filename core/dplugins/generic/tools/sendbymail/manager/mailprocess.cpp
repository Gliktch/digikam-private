/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-02-25
 * Description : a tool to e-mailing images
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Andi Clemens <andi dot clemens at googlemail dot com>
 * SPDX-FileCopyrightText: 2006      by Tom Albers <tomalbers at kde dot nl>
 * SPDX-FileCopyrightText: 2006      by Michael Hoechstetter <michael dot hoechstetter at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mailprocess.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QPointer>
#include <QStringList>
#include <QTextStream>
#include <QApplication>
#include <QMessageBox>
#include <QTemporaryDir>

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#   include <QTextCodec>
#endif

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "imageresizethread.h"

namespace DigikamGenericSendByMailPlugin
{

class Q_DECL_HIDDEN MailProcess::Private
{
public:

    Private() = default;

    bool               cancel               = false;

    QList<QUrl>        attachementFiles;
    QList<QUrl>        failedResizedImages;

    MailSettings*      settings             = nullptr;
    DInfoInterface*    iface                = nullptr;

    ImageResizeThread* threadImgResize      = nullptr;
};

MailProcess::MailProcess(MailSettings* const settings,
                        DInfoInterface* const iface,
                        QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
    d->settings        = settings;
    d->iface           = iface;
    d->threadImgResize = new ImageResizeThread(this);

    connect(d->threadImgResize, SIGNAL(startingResize(QUrl)),
            this, SLOT(slotStartingResize(QUrl)));

    connect(d->threadImgResize, SIGNAL(finishedResize(QUrl,QUrl,int)),
            this, SLOT(slotFinishedResize(QUrl,QUrl,int)));

    connect(d->threadImgResize, SIGNAL(failedResize(QUrl,QString,int)),
            this, SLOT(slotFailedResize(QUrl,QString,int)));

    connect(d->threadImgResize, SIGNAL(finished()),
            this, SLOT(slotCompleteResize()));
}

MailProcess::~MailProcess()
{
    delete d;
}

void MailProcess::firstStage()
{
    d->cancel = false;

    if (!d->threadImgResize->isRunning())
    {
        d->threadImgResize->cancel();
        d->threadImgResize->wait();
    }

    QTemporaryDir tempPath;
    tempPath.setAutoRemove(false);

    if (!tempPath.isValid())
    {
        Q_EMIT signalMessage(i18n("Cannot create a temporary directory"), true);

        slotCancel();

        Q_EMIT signalComplete(false);

        return;
    }

    d->settings->tempPath = tempPath.path() + QLatin1Char('/');

    d->attachementFiles.clear();
    d->failedResizedImages.clear();

    if (d->settings->imagesChangeProp)
    {
        // Resize all images if necessary in a separate threadImgResize.
        // Attachments list is updated by slotFinishedResize().

        d->threadImgResize->resize(d->settings);
        d->threadImgResize->start();
    }
    else
    {
        // Add all original files to the attachments list.

        for (QMap<QUrl, QUrl>::const_iterator it = d->settings->itemsList.constBegin() ;
             it != d->settings->itemsList.constEnd() ; ++it)
        {
            d->attachementFiles.append(it.key());
            d->settings->setMailUrl(it.key(), it.key());
        }

        Q_EMIT signalProgress(50);

        secondStage();
    }
}

void MailProcess::slotCancel()
{
    d->cancel = true;

    if (!d->threadImgResize->isRunning())
    {
        d->threadImgResize->cancel();
        d->threadImgResize->wait();
    }

    Q_EMIT signalProgress(0);

    slotCleanUp();
}

void MailProcess::slotStartingResize(const QUrl& orgUrl)
{
    if (d->cancel)
    {
        return;
    }

    QString text = i18n("Resizing %1", orgUrl.fileName());

    Q_EMIT signalMessage(text, false);
}

void MailProcess::slotFinishedResize(const QUrl& orgUrl, const QUrl& emailUrl, int percent)
{
    if (d->cancel)
    {
        return;
    }

    Q_EMIT signalProgress((int)(80.0*(percent/100.0)));

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << emailUrl;
    d->attachementFiles.append(emailUrl);
    d->settings->setMailUrl(orgUrl, emailUrl);

    QString text = i18n("%1 resized successfully", orgUrl.fileName());

    Q_EMIT signalMessage(text, false);
}

void MailProcess::slotFailedResize(const QUrl& orgUrl, const QString& error, int percent)
{
    if (d->cancel)
    {
        return;
    }

    Q_EMIT signalProgress((int)(80.0*(percent/100.0)));

    QString text = i18n("Failed to resize %1: %2", orgUrl.fileName(), error);

    Q_EMIT signalMessage(text, true);

    d->failedResizedImages.append(orgUrl);
}

void MailProcess::slotCompleteResize()
{
    if (d->cancel)
    {
        return;
    }

    if (!showFailedResizedImages())
    {
        slotCancel();

        return;
    }

    secondStage();
}

void MailProcess::secondStage()
{
    if (d->cancel)
    {
        return;
    }

    // If the initial list of files contained only unsupported file formats,
    // and the user chose not to attach them without resizing, then there are
    // no files approved for sending.

    if (d->attachementFiles.isEmpty())
    {
        Q_EMIT signalMessage(i18n("There are no files to send"), false);
        Q_EMIT signalProgress(0);

        return;
    }

    buildPropertiesFile();

    Q_EMIT signalProgress(90);

    invokeMailAgent();

    Q_EMIT signalProgress(100);
}

void MailProcess::buildPropertiesFile()
{
    if (d->cancel)
    {
        return;
    }

    if (d->iface && d->settings->addFileProperties)
    {
        Q_EMIT signalMessage(i18n("Build images properties file"), false);

        QString propertiesText;

        for (QMap<QUrl, QUrl>::const_iterator it = d->settings->itemsList.constBegin() ;
             it != d->settings->itemsList.constEnd() ; ++it)
        {
            DItemInfo info(d->iface->itemInfo(it.key()));
            QString comments  = info.comment();
            QString tags      = info.keywords().join(QLatin1String(", "));
            QString rating    = QString::number(info.rating());
            QString orgFile   = it.key().fileName();
            QString emailFile = it.value().fileName();

            if (comments.isEmpty())
            {
                comments = i18n("no caption");
            }

            if (tags.isEmpty())
            {
                tags = i18n("no keywords");
            }

            propertiesText.append(i18n("file \"%1\":\nOriginal images: %2\n", emailFile, orgFile));

            propertiesText.append(i18n("Comments: %1\n", comments));

            propertiesText.append(i18n("Tags: %1\n", tags));

            propertiesText.append(i18n("Rating: %1\n", rating));

            propertiesText.append(QLatin1Char('\n'));
        }

        QFile propertiesFile(d->settings->tempPath + i18n("properties.txt"));
        QTextStream stream(&propertiesFile);

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)

        // In Qt5 only. Qt6 uses UTF-8 by default.

        stream.setCodec(QTextCodec::codecForName("UTF-8"));

#endif

        stream.setAutoDetectUnicode(true);

        if (!propertiesFile.open(QIODevice::WriteOnly))
        {
            Q_EMIT signalMessage(i18n("Image properties file cannot be opened"), true);

            qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "File open error:" << propertiesFile.fileName();
            return;
        }

        stream << propertiesText << QLatin1Char('\n');
        propertiesFile.close();
        d->attachementFiles << QUrl::fromLocalFile(propertiesFile.fileName());

        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Image properties file done" << propertiesFile.fileName();

        Q_EMIT signalMessage(i18n("Image properties file done"), false);
    }
}

bool MailProcess::showFailedResizedImages() const
{
    bool ret = true;

    if (!d->failedResizedImages.isEmpty())
    {
        QStringList list;

        for (QList<QUrl>::const_iterator it = d->failedResizedImages.constBegin() ;
            it != d->failedResizedImages.constEnd() ; ++it)
        {
            list.append((*it).fileName());
        }

        QPointer<QMessageBox> mbox = new QMessageBox(QApplication::activeWindow());
        mbox->setIcon(QMessageBox::Warning);
        mbox->setWindowTitle(i18nc("@title:window", "Processing Failed"));
        mbox->setText(i18n("Some images cannot be resized.\n"
                           "Do you want them to be added as attachments without resizing?"));
        mbox->setStandardButtons(QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel));
        mbox->setDefaultButton(QMessageBox::No);
        mbox->setDetailedText(list.join(QLatin1Char('\n')));

        int valRet = mbox->exec();

        switch (valRet)
        {
            case QMessageBox::Yes:
            {
                // Added source image files instead resized images...

                for (QList<QUrl>::const_iterator it = d->failedResizedImages.constBegin() ;
                    it != d->failedResizedImages.constEnd() ; ++it)
                {
                    d->attachementFiles.append(*it);
                    d->settings->setMailUrl(*it, *it);
                }

                break;
            }

            case QMessageBox::No:
            {
                // Do nothing...

                break;
            }

            case QMessageBox::Cancel:
            {
                // Stop process...

                ret = false;
                break;
            }
        }

        delete mbox;
    }

    return ret;
}

QList<QUrl> MailProcess::divideEmails()
{
    qint64 myListSize = 0;

    QList<QUrl> processedNow;            // List which can be processed now.
    QList<QUrl> todoAttachement;         // Still todo list

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Attachment limit: " << d->settings->attachementLimit();

    for (QList<QUrl>::const_iterator it = d->attachementFiles.constBegin() ;
        it != d->attachementFiles.constEnd() ; ++it)
    {
        QFile file((*it).toLocalFile());
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "File: " << file.fileName() << " Size: " << file.size();

        if ((myListSize + file.size()) <= d->settings->attachementLimit())
        {
            myListSize += file.size();
            processedNow.append(*it);
            qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Current list size: " << myListSize;
        }
        else
        {
            if ((file.size()) >= d->settings->attachementLimit())
            {
                qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "File \"" << file.fileName()
                                             << "\" is out of attachment limit!";
                QString mess = i18n("The file \"%1\" is too big to be sent, "
                                    "please reduce its size or change your settings",
                                    file.fileName());

                Q_EMIT signalMessage(mess, true);
            }
            else
            {
                todoAttachement.append(*it);
            }
        }
    }

    d->attachementFiles = todoAttachement;

    return processedNow;
}

bool MailProcess::invokeMailAgent()
{
    if (d->cancel) return false;

    bool        agentInvoked = false;
    QList<QUrl> fileList;

    do
    {
        fileList = divideEmails();

        if (!fileList.isEmpty())
        {
            QString prog = QDir::toNativeSeparators(d->settings->binPaths[d->settings->mailProgram]);

            if (prog.isEmpty())
            {
                qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Mail binary path is empty."
                                             << "Cannot start Mail client program!";
                return false;
            }

            switch ((int)d->settings->mailProgram)
            {
                case MailSettings::BALSA:
                {
                    QStringList args;

                    args.append(QLatin1String("-m"));
                    args.append(QLatin1String("mailto:"));

                    for (QList<QUrl>::ConstIterator it = fileList.constBegin() ; it != fileList.constEnd() ; ++it)
                    {
                        args.append(QLatin1String("-a"));
                        args.append(QDir::toNativeSeparators((*it).toLocalFile()));
                    }

                    QProcess process;
                    process.setProcessEnvironment(adjustedEnvironmentForAppImage());

                    if (!process.startDetached(prog, args))
                    {
                        invokeMailAgentError(prog, args);
                    }
                    else
                    {
                        invokeMailAgentDone(prog, args);
                        agentInvoked = true;
                    }

                    break;
                }

                case MailSettings::CLAWSMAIL:
                case MailSettings::SYLPHEED:
                {
                    QStringList args;

                    args.append(QLatin1String("--compose"));
                    args.append(QLatin1String("--attach"));

                    for (QList<QUrl>::ConstIterator it = fileList.constBegin() ;
                         it != fileList.constEnd() ; ++it)
                    {
                        args.append(QDir::toNativeSeparators((*it).toLocalFile()));
                    }

                    QProcess process;
                    process.setProcessEnvironment(adjustedEnvironmentForAppImage());

                    if (!process.startDetached(prog, args))
                    {
                        invokeMailAgentError(prog, args);
                    }
                    else
                    {
                        invokeMailAgentDone(prog, args);
                        agentInvoked = true;
                    }

                    break;
                }

                case MailSettings::EVOLUTION:
                {
                    QStringList args;

                    QString tmp = QLatin1String("mailto:?subject=");

                    for (QList<QUrl>::ConstIterator it = fileList.constBegin() ; it != fileList.constEnd() ; ++it)
                    {
                        tmp.append(QLatin1String("&attach="));
                        tmp.append(QDir::toNativeSeparators((*it).toLocalFile()));
                    }

                    args.append(tmp);

                    QProcess process;
                    process.setProcessEnvironment(adjustedEnvironmentForAppImage());

                    if (!process.startDetached(prog, args))
                    {
                        invokeMailAgentError(prog, args);
                    }
                    else
                    {
                        invokeMailAgentDone(prog, args);
                        agentInvoked = true;
                    }

                    break;
                }

                case MailSettings::KMAIL:
                {
                    QStringList args;

                    for (QList<QUrl>::ConstIterator it = fileList.constBegin() ;
                         it != fileList.constEnd() ; ++it)
                    {
                        args.append(QLatin1String("--attach"));
                        args.append(QDir::toNativeSeparators((*it).toLocalFile()));
                    }

                    QProcess process;
                    process.setProcessEnvironment(adjustedEnvironmentForAppImage());

                    if (!process.startDetached(prog, args))
                    {
                        invokeMailAgentError(prog, args);
                    }
                    else
                    {
                        invokeMailAgentDone(prog, args);
                        agentInvoked = true;
                    }

                    break;
                }

                // More info about command lines options with Mozilla & co:
                // www.mozilla.org/docs/command-line-args.html#Syntax_Rules

                case MailSettings::NETSCAPE:
                case MailSettings::THUNDERBIRD:
                {
                    QStringList args;

                    args.append(QLatin1String("-compose"));
                    QString tmp = QLatin1String("attachment='");

                    for (QList<QUrl>::ConstIterator it = fileList.constBegin() ;
                         it != fileList.constEnd() ; ++it)
                    {
                        tmp.append(QLatin1String("file://"));
                        tmp.append(QDir::toNativeSeparators((*it).toLocalFile()));
                        tmp.append(QLatin1String(","));
                    }

                    tmp.remove(tmp.length()-1, 1);
                    tmp.append(QLatin1String("'"));

                    args.append(tmp);

                    QProcess process;
                    process.setProcessEnvironment(adjustedEnvironmentForAppImage());

                    if (!process.startDetached(prog, args))
                    {
                        invokeMailAgentError(prog, args);
                    }
                    else
                    {
                        invokeMailAgentDone(prog, args);
                        agentInvoked = true;
                    }

                    break;
                }
                case MailSettings::OUTLOOK:
                {
                    QStringList args;

                    args.append(QLatin1String("/a"));
                    args.append(QDir::toNativeSeparators(fileList.first().toLocalFile()));

                    QProcess process;
                    process.setProcessEnvironment(adjustedEnvironmentForAppImage());

                    if (!process.startDetached(prog, args))
                    {
                        invokeMailAgentError(prog, args);
                    }
                    else
                    {
                        invokeMailAgentDone(prog, args);
                        agentInvoked = true;
                    }

                    break;
                }
            }
        }
    }
    while (!fileList.isEmpty());

    return agentInvoked;
}

void MailProcess::invokeMailAgentError(const QString& prog, const QStringList& args)
{
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Command Line: " << prog << args;
    QString text = i18n("Failed to start \"%1\" program. Check your system.", prog);

    Q_EMIT signalMessage(text, true);

    slotCleanUp();

    Q_EMIT signalComplete(false);
}

void MailProcess::invokeMailAgentDone(const QString& prog, const QStringList& args)
{
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Command Line: " << prog << args;
    QString text = i18n("Starting \"%1\" program...", prog);

    Q_EMIT signalMessage(text, false);

    Q_EMIT signalMessage(i18n("After having sent your images by email..."), false);
    Q_EMIT signalMessage(i18n("Press 'Finish' button to clean up temporary files"), false);
    Q_EMIT signalComplete(true);
}

void MailProcess::slotCleanUp()
{
    if (QDir().exists(d->settings->tempPath))
    {
        QDir(d->settings->tempPath).removeRecursively();
    }
}

} // namespace DigikamGenericSendByMailPlugin

#include "moc_mailprocess.cpp"
