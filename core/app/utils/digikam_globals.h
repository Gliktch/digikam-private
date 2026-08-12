/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-08
 * Description : global macros, variables and flags
 *
 * SPDX-FileCopyrightText: 2009-2010 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDateTime>
#include <QIODevice>
#include <QStringList>
#include <QProcessEnvironment>

// Local includes

#include "digikam_export.h"

class QDate;
class QWidget;
class QObject;
class QDialog;
class QShortcut;
class QKeySequence;
class QApplication;
class QAction;

/**
 * Macros for image filters.
 */
#define CLAMP0255(a)   qBound(0,a,255)
#define CLAMP065535(a) qBound(0,a,65535)
#define CLAMP(x,l,u)   qBound(l,x,u)
#define MAX3(a, b, c)  (qMax(qMax(a,b),c))
#define MIN3(a, b, c)  (qMin(qMin(a,b),c))

/**
 * Macro to use right return type with qHash(), changed with new Qt6 API.
 */
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#   define QT_HASH_TYPE /* clazy:exclude=qt6-qhash-signature */ uint
#else
#   define QT_HASH_TYPE size_t
#endif

namespace Digikam
{

/**
 * Degrees to radian conversion coeff (PI/180). To optimize computation.
 */
static const double DEG2RAD         = 0.017453292519943;

/**
 * Field value limits for all digiKam-specific fields (not EXIF/IPTC fields)
 */
static const int RatingMin          = 0;
static const int RatingMax          = 5;
static const int NoRating           = -1;

// --------------------------------------------------------

/**
 * Segments for histograms and curves
 */
static const int NUM_SEGMENTS_16BIT = 65536;
static const int NUM_SEGMENTS_8BIT  = 256;
static const int MAX_SEGMENT_16BIT  = NUM_SEGMENTS_16BIT - 1;
static const int MAX_SEGMENT_8BIT   = NUM_SEGMENTS_8BIT  - 1;

// --------------------------------------------------------

/**
 * Delay in milliseconds to automatically expands album tree-view with D&D
 * See bug #286263 for details.
 */
static const int AUTOEXPANDDELAY    = 800;

// --------------------------------------------------------

/**
 * Color label in enum order as text
 */
static const QStringList ColorLabelAsText({QLatin1String("NoColor"),
                                           QLatin1String("Red"),
                                           QLatin1String("Orange"),
                                           QLatin1String("Yellow"),
                                           QLatin1String("Green"),
                                           QLatin1String("Blue"),
                                           QLatin1String("Purple"),
                                           QLatin1String("Gray"),
                                           QLatin1String("Black"),
                                           QLatin1String("White")}
                                         );

// --------------------------------------------------------

/**
 * String for the date/time format used in the application
 */

DIGIKAM_EXPORT extern QString DATETIMEFORMATSTRING;

// --------------------------------------------------------

enum ColorLabel
{
    NoColorLabel        = 0,
    RedLabel,
    OrangeLabel,
    YellowLabel,
    GreenLabel,
    BlueLabel,
    MagentaLabel,
    GrayLabel,
    BlackLabel,
    WhiteLabel,
    FirstColorLabel     = NoColorLabel,
    LastColorLabel      = WhiteLabel,
    NumberOfColorLabels = LastColorLabel + 1
};

// --------------------------------------------------------

enum PickLabel
{
    NoPickLabel        = 0,
    RejectedLabel,
    PendingLabel,
    AcceptedLabel,
    FirstPickLabel     = NoPickLabel,
    LastPickLabel      = AcceptedLabel,
    NumberOfPickLabels = LastPickLabel + 1
};

// --------------------------------------------------------

enum HistogramBoxType
{
    RGB = 0,
    RGBA,
    LRGB,
    LRGBA,
    LRGBC,
    LRGBAC
};

enum HistogramScale
{
    LinScaleHistogram = 0,      ///< Linear scale
    LogScaleHistogram           ///< Logarithmic scale
};

enum HistogramRenderingType
{
    FullImageHistogram = 0,     ///< Full image histogram rendering.
    ImageSelectionHistogram     ///< Image selection histogram rendering.
};

// --------------------------------------------------------

enum ChannelType
{
    LuminosityChannel = 0,
    RedChannel,
    GreenChannel,
    BlueChannel,
    AlphaChannel,
    ColorChannels
};

/**
 * Types of operations for ApplicationSettings.
 * Originally introduced for grouping to configure whether an operation
 * should be done on all group members or only it's head.
 */
enum OperationType
{
    MetadataOps = 0,
    ImportExportOps,
    BQMOps,
    LightTableOps,
    SlideshowOps,
    RenameOps,
    ToolsOps,
    UnspecifiedOps ///< This element must always come last
};

// --------------------------------------------------------

/**
 * Map of [name, action instance] from the managed by the parent window
 * to be hosted the toolbar or the context menu from the preview view (image or video).
 * See ItemPreviewView and ItemPreviewVideo classes for the usage.
 *
 * Know action names:
 *
 *  "FullScreen"  : Switch the window in fullscreen mode.
 *  "ShowMenu"    : Turn on/off the window main menu visibility.
 *  "ColorManaged": Turn on/off Color Managed view to render the content.
  */
typedef QMap<QString, QAction*> HostActionsMap;

// --------------------------------------------------------

/**
 * Convenience method for creating keyboard shortcuts.
 */
DIGIKAM_EXPORT QShortcut* defineShortcut(QWidget* const w, const QKeySequence& key, const QObject* receiver, const char* slot);

/**
 * Return list of supported image formats by Qt for reading or writing operations if suitable
 * container used by QFileDialog.
 * For simple container of type mime, use 'allTypes' string.
 * Supported modes are QIODevice::ReadOnly, QIODevice::WriteOnly, and QIODevice::ReadWrite.
 */
DIGIKAM_EXPORT QStringList supportedImageMimeTypes(QIODevice::OpenModeFlag mode, QString& allTypes);

/**
 * Return true if filePath is an image readable by application for thumbnail, preview, or edit.
 */
DIGIKAM_EXPORT bool isReadableImageFile(const QString& filePath);

/**
 * Show a dialog with all RAW camera supported by digiKam, through libraw.
 */
DIGIKAM_EXPORT void showRawCameraList();

/**
 * Style sheet for transparent QToolButtons over image and video preview.
 */
DIGIKAM_EXPORT QString toolButtonStyleSheet();

/**
 * Default spacing to use in layout.
 */
DIGIKAM_EXPORT int layoutSpacing();

/**
 * Default margin to use in layout.
 */
DIGIKAM_EXPORT int layoutMargin();

/**
 * Show a QDialog with show() and wait for the result.
 */
DIGIKAM_EXPORT int dialogExec(QDialog* const dlg);

// --- Static functions for the bundles ---

/**
 * Return true if application run in AppImage bundle.
 */
DIGIKAM_EXPORT bool isRunningInAppImageBundle();

/**
 * Return true if application run on native KDE desktop.
 */
DIGIKAM_EXPORT bool isRunningOnNativeKDE();

/**
 * If digiKam run into AppImage, return a cleaned environment for QProcess to execute a
 * program outside the bundle without broken run-time dependencies.
 * Use case : system based Hugin CLI tools called by Panorama wizard.
 * If digiKam do not run as AppImage bundle, this method return a QProcessEnvironment instance
 * based on system environment.
 */
DIGIKAM_EXPORT QProcessEnvironment adjustedEnvironmentForAppImage();

/**
 * Prefix of macOS Bundle to access to internal Unix hierarchy.
 */
DIGIKAM_EXPORT QString macOSBundlePrefix();

/**
 * For bundles only, unload all Qt translation files at run-time in application instance.
 */
DIGIKAM_EXPORT void unloadQtTranslationFiles(QApplication& app);

/**
 * For bundles only, load standard Qt translation files at run-time in application instance.
 */
DIGIKAM_EXPORT void loadStdQtTranslationFiles(QApplication& app);

/**
 * For bundles only, load ECM Qt translation files at run-time in application instance.
 */
DIGIKAM_EXPORT void loadEcmQtTranslationFiles(QApplication& app);

/**
 * For bundles only, main function to manage all Qt translation files at run-time in application instance.
 */
DIGIKAM_EXPORT void installQtTranslationFiles(QApplication& app);

/**
 * For MacOS bundles only, set necessary MacOS environment variables
 */
DIGIKAM_EXPORT void setMacOSEnvironment();

/**
 * For Windows only, set necessary Windows environment variables
 */
DIGIKAM_EXPORT void setWindowsEnvironment(QApplication& app);

/**
 * For Linux only, prefer the PulseAudio backend of Qt Multimedia if the user
 * has not selected an audio backend explicitly with QT_AUDIO_BACKEND.
 */
DIGIKAM_EXPORT void setAudioBackendEnvironment();

/**
 * For OpenCV framework, set necessary environment variables to use OpenCL features.
 */
DIGIKAM_EXPORT void setOpenCLEnvironment(bool b);

/**
 * This method returns QDateTime from with date set to parameter date and time set to start of the day.
 */
DIGIKAM_EXPORT QDateTime startOfDay(const QDate& date);

/**
 * This method returns QDateTime with UTC timespec.
 */
DIGIKAM_EXPORT QDateTime asDateTimeUTC(const QDateTime& dt);

/**
 * This method returns QDateTime with Local timespec.
 */
DIGIKAM_EXPORT QDateTime asDateTimeLocal(const QDateTime& dt);

/**
 * This method returns QString as user QDateTime.
 */
DIGIKAM_EXPORT QString asUserDateTime(const QDateTime& dt);

/**
 * This method returns QString as user date format.
 */
DIGIKAM_EXPORT QString getUserDateFormatString();

/**
 * This method returns QString as link to the QDateTime doc.
 */
DIGIKAM_EXPORT QString getDateFormatLinkText();

/**
 * Open online handbook at the section/chapter/reference page.
 *
 * if section and chapter and reference are empty, front page is open.  (https://en.wikipedia.org/wiki/Matrix_(protocol)#Bridges)
 * if only chapter and reference are empty, section page is open.       (as: https://docs.digikam.org/en/main_window.html)
 * if only reference is empty, chapter from section page is open.       (as: https://docs.digikam.org/en/main_window/people_view.html)
 * else reference at chapter from section page is open.                 (as: https://docs.digikam.org/en/main_window/people_view.html#face-recognition)
 */
DIGIKAM_EXPORT void openOnlineDocumentation(const QString& section = QString(), const QString& chapter = QString(), const QString& reference = QString());

/**
 * @brief function to wait attachment from a remote debugger before continuing execution.
 * This method must be called just after the QApplication instance.
 * This method is only active if the DIGIKAM_REMOTE_DEBUG env.var is declared to ON before to start the program.
 */
DIGIKAM_EXPORT void delayForRemoteDebuging(int delaySecs = 30);

/**
 * @return true if the KDE desktop (eg. Plasma) is running.
 */
DIGIKAM_EXPORT bool detectKDEDesktopIsRunning();

} // namespace Digikam
