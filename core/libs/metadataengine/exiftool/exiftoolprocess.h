/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-02-18
 * Description : Qt5 and Qt6 interface for exiftool.
 *               Based on ZExifTool Qt interface published at 18 Feb 2021
 *               https://github.com/philvl/ZExifTool
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021 by Philippe Vianney Liaud <philvl dot dev at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QProcess>
#include <QPointer>

// Local includes

#include "digikam_export.h"

namespace Digikam
{
class ExifToolParser;

class DIGIKAM_EXPORT ExifToolProcess : public QProcess
{
    Q_OBJECT

public:

    /**
     * ExifTool actions to process.
     */
    enum Action
    {
        LOAD_METADATA       = 0,                    ///< Load all metadata from a file with ExifTool.
        LOAD_CHUNKS,                                ///< Load Exif, Iptc, and Xmp chunks from a file as byte-array for MetaEngine.
        APPLY_CHANGES,                              ///< Apply tag changes in a file with ExifTool.
        APPLY_CHANGES_EXV,                          ///< Apply tag changes in a file with ExifTool using an EXV container.
        APPLY_METADATA_FILE,                        ///< Apply a metadata file to a file with ExifTool.
        CHANGE_TIMESTAMPS,                          ///< Change all timestamps in a file with ExifTool.
        READ_FORMATS,                               ///< Return the list of readable ExifTool file formats.
        WRITE_FORMATS,                              ///< Return the list of writable ExifTool file formats.
        TRANSLATIONS_LIST,                          ///< List of ExifTool languages available for translations.
        TAGS_DATABASE,                              ///< List of ExifTool tags from database.
        VERSION_STRING,                             ///< Return the ExifTool version as string.
        COPY_TAGS,                                  ///< Copy tags from one file to another one. See CopyTagsSource enum for details.
        TRANS_TAGS,                                 ///< Translate tags in file. See TranslateTagsOps enum for details.
        EXTRACT_MOTION_PHOTO_VIDEO,                 ///< Extract embedded video from a Motion Photo as byte-array.
        NO_ACTION                                   ///< Last value from this list. Do nothing.
    };

    /**
     * Possible copying tags operations to OR combine with COPY_TAGS action.
     */
    enum CopyTagsSource
    {
        COPY_EXIF           = 0x01,                 ///< Copy all Exif Tags from source file.
        COPY_MAKERNOTES     = 0x02,                 ///< Copy all Makernotes tags from source file.
        RESTORE_PREVIEW     = 0x04,                 ///< Restore preview image from source file.
        COPY_IPTC           = 0x08,                 ///< Copy all Iptc tags from source file.
        COPY_XMP            = 0x10,                 ///< Copy all Xmp tags from source file.
        COPY_ICC            = 0x20,                 ///< Copy ICC profile from source file.
        COPY_ALL            = 0x40,                 ///< Copy all tags from source file.
        COPY_NONE           = 0x80                  ///< No copy operation.
    };

    /**
     * Possible writing tags mode to OR combine with COPY_TAGS action.
     */
    enum WritingTagsMode
    {
        WRITE_EXISTING_TAGS = 0x01,                 ///< Overwrite existing tags.
        CREATE_NEW_TAGS     = 0x02,                 ///< Create new tags.
        CREATE_NEW_GROUPS   = 0x04,                 ///< Create new groups if necessary.
        ALL_MODES           = WRITE_EXISTING_TAGS |
                              CREATE_NEW_TAGS     |
                              CREATE_NEW_GROUPS
    };

    /**
     * Possible translating tags operations to OR combine with COPY_TAGS action.
     */
    enum TranslateTagsOps
    {
        TRANS_ALL_XMP       = 0x01,                 ///< Translate all existing Tags from source file to Xmp.
        TRANS_ALL_IPTC      = 0x02,                 ///< Translate all existing Tags from source file to Iptc.
        TRANS_ALL_EXIF      = 0x04                  ///< Translate all existing Tags from source file to Exif.
    };

    /**
     * Command result state.
     */
    enum ResultStatus
    {
        COMMAND_RESULT      = 0,
        FINISH_RESULT,
        ERROR_RESULT
    };

public:

    class Result
    {
    public:

        Result() = default;

        bool       waitError    = false;
        int        cmdStatus    = ExifToolProcess::COMMAND_RESULT;
        int        cmdAction    = ExifToolProcess::NO_ACTION;
        int        cmdNumber    = 0;
        int        elapsed      = 0;
        QByteArray output;
    };

public:

    /**
     * Constructs a ExifToolProcess.
     */
    ExifToolProcess();

    /**
     * Destructs the ExifToolProcess object, i.e., killing the process.
     * Note that this function will not return until the process is terminated.
     */
    ~ExifToolProcess();

    /**
     * @brief internalPtr - singleton implementation
     */
    static QPointer<ExifToolProcess> internalPtr;

    static bool                      isCreated();
    static ExifToolProcess*          instance();

    /**
     * Setup connections, apply Settings and start ExifTool process.
     * This function cannot be called from another thread.
     */
    void initExifTool();

    /**
     * Attempts to shut down the ExifTool process.
     * This function cannot be called from another thread.
     */
    void shutDownExifTool();

public:

    /**
     * Change the ExifTool path configuration.
     * This function can be called from another thread.
     */
    void setExifToolProgram(const QString& etExePath);

    QString getExifToolProgram()                             const;

public:

    /**
     * Returns true if ExifToolProcess is available (process state == Running)
     */
    bool                    exifToolAvailable()              const;

    /**
     * Returns true if a command is running.
     */
    bool                    exifToolIsBusy()                 const;

    /**
     * Returns the type of error that occurred last.
     */
    QProcess::ProcessError  exifToolError()                  const;

    /**
     * Returns an error message.
     */
    QString                 exifToolErrorString()            const;

    /**
     * Returns the ExifToolProcess result.
     */
    ExifToolProcess::Result getExifToolResult(int cmdId)     const;

    /**
     * WatCondition for the ExifToolParser class.
     * Returns the ExifToolProcess result.
     */
    ExifToolProcess::Result waitForExifToolResult(int cmdId) const;

    /**
     * Send a command to exiftool process.
     * This function can be called from another thread.
     * Return 0: ExitTool not running, write channel is closed or args is empty.
     */
    int command(const QByteArrayList& args, Action ac);

Q_SIGNALS:

    void signalExifToolResult(int cmdId);

private:

    /**
     * Starts exiftool in a new process.
     */
    bool startExifTool();

    /**
     * Restart exiftool in a new process.
     */
    void restartExifTool();

    /**
     * Kills the current process, causing it to exit immediately.
     * On Windows, kill() uses TerminateProcess, and on Unix and macOS,
     * the SIGKILL signal is sent to the process.
     */
    void killExifTool();

    QString exifToolBin()                                    const;

    bool checkExifToolProgram()                              const;

    void changeExifToolProgram(const QString& etExePath);

Q_SIGNALS:

    void signalExecNextCmd();
    void signalChangeProgram(const QString& etExePath);

private Q_SLOTS:

    void slotStarted();
    void slotApplySettingsAndStart();
    void slotReadyReadStandardError();
    void slotReadyReadStandardOutput();
    void slotChangeProgram(const QString& etExePath);
    void slotErrorOccurred(QProcess::ProcessError error);
    void slotFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:

    /// @note disabled
    explicit ExifToolProcess(QObject*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
