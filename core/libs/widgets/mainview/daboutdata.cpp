/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-07-30
 * Description : digiKam about data.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "daboutdata.h"

// Qt includes

#include <QIcon>
#include <QAction>

// KDE includes

#include <klocalizedstring.h>
#include <kaboutdata.h>

// Local includes

#include "dxmlguiwindow.h"

namespace Digikam
{

DAboutData::DAboutData(DXmlGuiWindow* const parent)
    : QObject(parent)
{
}

const QString DAboutData::digiKamSloganFormated()
{
    return i18nc("This is the slogan formatted string displayed in splashscreen. "
                 "Please translate using short words else the slogan can be truncated.",
                 "<qt><font color=\"white\">"
                 "<b>Professional</b> Photo <b>Management</b><br>with the Power of <b>Open Source</b>"
                 "</font></qt>"
                );
}

const QString DAboutData::digiKamSlogan()
{
    return i18n("Professional Photo Management with the Power of Open Source");
}

const QString DAboutData::digiKamFamily()
{
    return i18n("A KDE Family Project");
}

const QString DAboutData::copyright()
{
    return i18n("(c) 2001-2026, digiKam developers team");
}

const QUrl DAboutData::webProjectUrl()
{
    return QUrl(QString::fromUtf8("https://www.digikam.org"));
}

void DAboutData::authorsRegistration(KAboutData& aboutData)
{
    // -- Core team --------------------------------------------------------------

    aboutData.addAuthor ( QString::fromUtf8("Caulier Gilles"),
                          i18n("Coordinator, Developer, and Mentoring"),
                          QString::fromUtf8("caulier dot gilles at gmail dot com"),
                          QString::fromUtf8("https://www.linkedin.com/in/gilles-caulier/")
                        );

    aboutData.addAuthor ( QString::fromUtf8("Maik Qualmann"),
                          i18n("Developer and Mentoring"),
                          QString::fromUtf8("metzpinguin at gmail dot com"),
                          QString::fromUtf8("https://www.linkedin.com/in/maik-qualmann-2b266717b/")
                        );

    aboutData.addAuthor ( QString::fromUtf8("Michael Miller"),
                          i18n("SFace and YuNet DNN models integration\n"
                               "in face workflow.\n"
                               "Face Management and auto-tag AI improvements.\n"
                               "AI red eyes detection"),
                          QString::fromUtf8("michael underscore miller at msn dot com"),
                          QString::fromUtf8("https://www.linkedin.com/in/michaelamiller01/")
                        );

    aboutData.addAuthor ( QString::fromUtf8("Mohamed Anwer"),                                       // krazy:exclude=spelling
                          i18n("Developer and Mentoring"),
                          QString::fromUtf8("mohammed dot ahmed dot anwer at gmail dot com"),       // krazy:exclude=spelling
                          QString::fromUtf8("https://www.linkedin.com/in/mohamedanwer/")
                        );

    aboutData.addAuthor ( QString::fromUtf8("Michael G. Hansen"),
                          i18n("Developer and Mentoring"),
                          QString::fromUtf8("mike at mghansen dot de"),
                          QString::fromUtf8("http://www.mghansen.de")                               // krazy:exclude=insecurenet
                        );

    aboutData.addAuthor ( QString::fromUtf8("Teemu Rytilahti"),
                          i18n("Developer"),
                          QString::fromUtf8("tpr at iki dot fi"),
                          QString::fromUtf8("https://www.linkedin.com/in/teemurytilahti/")
                        );

    aboutData.addAuthor ( QString::fromUtf8("Thanh Trung Dinh"),
                          i18n("Mentoring, Port web-service tools to OAuth,\n"
                               "factoring web service tools,\n"
                               "and port faces recognition engine\n"
                               "to OpenCV neural network"),
                          QString::fromUtf8("dinhthanhtrung1996 at gmail dot com"),
                          QString::fromUtf8("https://www.linkedin.com/in/thanhtrungdinh/")
                        );


    // -- Contributors -----------------------------------------------------------

    aboutData.addAuthor ( QString::fromUtf8("Matthias Welwarsky"),
                          i18n("Developer"),
                          QString::fromUtf8("matze at welwarsky dot de"),
                          QString::fromUtf8("https://www.linkedin.com/in/matthiaswelwarsky/")
                        );

    aboutData.addAuthor ( QString::fromUtf8("Julien Narboux"),
                          i18n("Developer"),
                          QString::fromUtf8("Julien at narboux dot fr"),
                          QString::fromUtf8("https://www.linkedin.com/in/julien-narboux-17566610/")
                        );

    aboutData.addAuthor ( QString::fromUtf8("Mario Frank"),
                          i18n("Advanced Searches Tool Improvements"),
                          QString::fromUtf8("mario.frank@uni-potsdam.de")
                        );

    aboutData.addAuthor ( QString::fromUtf8("Nicolas Lécureuil"),
                          i18n("Releases Manager"),
                          QString::fromUtf8("neoclust dot kde at gmail dot com"),
                          QString::fromUtf8("https://www.linkedin.com/in/nicolaslecureuil/")
                        );

    // -- Contributors -----------------------------------------------------------

    aboutData.addCredit ( QString::fromUtf8("André Molkentin"),
                          i18n("Quality Test Engineer"),
                          QString::fromUtf8("zunar at gmx dot de")
                        );

    // -- Students ---------------------------------------------------------------

    aboutData.addCredit ( QString::fromUtf8("Anjani Kumar"),
                          i18n("Port to Qt6"),
                          QString::fromUtf8("anjanik012 at gmail dot com"),
                          QString::fromUtf8("https://www.linkedin.com/in/anjanik012/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Phuoc Khanh LE"),
                          i18n("Rewrite Image Quality Scanner algorithms\n"
                               "and Photo Focus Information Extraction.\n"
                               "Aesthetic Detection to classify images\n"
                               "using deep learning model.\n"
                               "Slideshow and Presentation tools improvement"),
                          QString::fromUtf8("phuockhanhnk94 at gmail dot com "),
                          QString::fromUtf8("https://www.linkedin.com/in/phuoc-khanh-le-476448169/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Quốc Hưng Trần"),
                          i18n("MJPEGStream plugin,\n"
                               "OCR text Converter plugin,\n"
                               "auto-tags assignment engine"),
                          QString::fromUtf8("quochungtran1999 at gmail dot com "),
                          QString::fromUtf8("https://www.linkedin.com/in/qu%E1%BB%91c-h%C6%B0ng-tr%E1%BA%A7n-92a504163/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Minh Nghĩa Duong"),
                          i18n("Face workflow Clustering support,\n"
                               "improvement for Face Engine,\n"
                               "Port Slideshow tool to plugins interface"),
                          QString::fromUtf8("minhnghiaduong997 at gmail dot com"),
                          QString::fromUtf8("https://www.linkedin.com/in/minh-nghia-duong-2b5bbb15a/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Kartik Ramesh"),
                          i18n("Face management workflow improvements"),
                          QString::fromUtf8("kartikx2000 at gmail dot com"),
                          QString::fromUtf8("https://www.linkedin.com/in/kartikxramesh/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Ahmed Fathi"),
                          i18n("UPNP/DLNA export tool\nand Healing clone tool for image editor"),
                          QString::fromUtf8("ahmed dot fathi dot abdelmageed at gmail dot com"),
                          QString::fromUtf8("https://ahmedfathishabanblog.wordpress.com/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Veaceslav Munteanu"),
                          i18n("Tags Manager"),
                          QString::fromUtf8("veaceslav dot munteanu90 at gmail dot com"),
                          QString::fromUtf8("https://www.linkedin.com/in/veaceslav-munteanu-4370a063/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Tarek Talaat"),
                          i18n("New OneDrive, Pinterrest, and Box export tools"),
                          QString::fromUtf8("tarektalaat93 at gmail dot com"),
                          QString::fromUtf8("https://www.linkedin.com/in/tarek-talaat-9bb5b1a6/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Yingjie Liu"),
                          i18n("Face-engine improvements\nand manual icon-view sort"),
                          QString::fromUtf8("yingjiewudi at gmail dot com"),
                          QString::fromUtf8("https://yjwudi.github.io")
                        );

    aboutData.addCredit ( QString::fromUtf8("Yiou Wang"),
                          i18n("Model/View Port of Image Editor Canvas"),
                          QString::fromUtf8("geow812 at gmail dot com"),
                          QString::fromUtf8("https://www.linkedin.com/in/yiouwang/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Gowtham Ashok"),
                          i18n("Image Quality Scanner"),
                          QString::fromUtf8("gwty93 at gmail dot com"),
                          QString::fromUtf8("https://www.linkedin.com/in/gowtham-ashok/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Aditya Bhatt"),
                          i18n("Face Detection"),
                          QString::fromUtf8("aditya at bhatts dot org"),
                          QString::fromUtf8("https://www.linkedin.com/in/adityabhatt/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Martin Klapetek"),
                          i18n("Non-destructive image editing"),
                          QString::fromUtf8("martin dot klapetek at gmail dot com"),
                          QString::fromUtf8("https://www.linkedin.com/in/mck182/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Gabriel Voicu"),
                          i18n("Reverse Geo-Coding"),
                          QString::fromUtf8("ping dot gabi at gmail dot com"),
                          QString::fromUtf8("https://www.linkedin.com/in/gabriel-voicu-958b8327/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Mahesh Hegde"),
                          i18n("Face Recognition"),
                          QString::fromUtf8("maheshmhegade at gmail dot com"),
                          QString::fromUtf8("https://www.linkedin.com/in/maheshmhegade/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Pankaj Kumar"),
                          i18n("Multi-core Support in Batch Queue Manager and Mentoring"),
                          QString::fromUtf8("me at panks dot me"),
                          QString::fromUtf8("https://www.linkedin.com/in/panks42/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Smit Mehta"),
                          i18n("UPnP/DLNA Export tool and Mentoring"),
                          QString::fromUtf8("smit dot tmeh at gmail dot com"),
                          QString::fromUtf8("https://www.linkedin.com/in/smit-mehta-45b82640/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Islam Wazery"),
                          i18n("Model/View port of Import Tool and Mentoring"),
                          QString::fromUtf8("wazery at ubuntu dot com"),
                          QString::fromUtf8("https://www.linkedin.com/in/wazery/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Abhinav Badola"),
                          i18n("Video Metadata Support and Mentoring"),
                          QString::fromUtf8("mail dot abu dot to at gmail dot com"),
                          QString::fromUtf8("https://www.linkedin.com/in/abhinav-badola-86085424/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Benjamin Girault"),
                          i18n("Panorama Tool and Mentoring"),
                          QString::fromUtf8("benjamin dot girault at gmail dot com"),
                          QString::fromUtf8("https://www.linkedin.com/in/benjamingirault/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Victor Dodon"),
                          i18n("XML based GUI port of tools"),
                          QString::fromUtf8("dodonvictor at gmail dot com"),
                          QString::fromUtf8("https://www.linkedin.com/in/dodonvictor/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Sayantan Datta"),
                          i18n("Auto Noise Reduction"),
                          QString::fromUtf8("sayantan dot knz at gmail dot com"),
                          QString::fromUtf8("https://www.linkedin.com/in/stndta/")
                        );

    // -- Former contributors ----------------------------------------------------

    aboutData.addAuthor ( QString::fromUtf8("Ananta Palani"),
                          i18n("Windows Port and Release Manager"),
                          QString::fromUtf8("anantapalani at gmail dot com"),
                          QString::fromUtf8("https://www.linkedin.com/in/anantapalani/")
                        );

    aboutData.addAuthor ( QString::fromUtf8("Andi Clemens"),
                          i18n("Developer"),
                          QString::fromUtf8("andi dot clemens at gmail dot com")
                        );

    aboutData.addAuthor ( QString::fromUtf8("Patrick Spendrin"),
                          i18n("Developer and Windows port"),
                          QString::fromUtf8("patrick_spendrin at gmx dot de"),
                          QString::fromUtf8("https://www.linkedin.com/in/patrickspendrin/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Francesco Riosa"),
                          i18n("LCMS2 library port"),
                          QString::fromUtf8("francesco plus kde at pnpitalia dot it"),
                          QString::fromUtf8("https://www.linkedin.com/in/vivogentoo/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Johannes Wienke"),
                          i18n("Developer"),
                          QString::fromUtf8("languitar at semipol dot de"),
                          QString::fromUtf8("https://www.facebook.com/languitar")
                        );

    aboutData.addAuthor ( QString::fromUtf8("Julien Pontabry"),
                          i18n("Developer"),
                          QString::fromUtf8("julien dot pontabry at ulp dot u-strasbg dot fr"),
                          QString::fromUtf8("https://www.linkedin.com/in/julien-pontabry-b784a247/")
                        );

    aboutData.addAuthor ( QString::fromUtf8("Arnd Baecker"),
                          i18n("Developer"),
                          QString::fromUtf8("arnd dot baecker at web dot de")
                        );

    aboutData.addAuthor ( QString::fromUtf8("Francisco J. Cruz"),
                          i18n("Color Management"),
                          QString::fromUtf8("fj dot cruz at supercable dot es")
                        );

    aboutData.addCredit ( QString::fromUtf8("Pieter Edelman"),
                          i18n("Developer"),
                          QString::fromUtf8("p dot edelman at gmx dot net"),
                          QString::fromUtf8("https://www.facebook.com/pieter.edelman")
                        );

    aboutData.addCredit ( QString::fromUtf8("Holger Foerster"),
                          i18n("MySQL interface"),
                          QString::fromUtf8("hamsi2k at freenet dot de")
                        );

    aboutData.addCredit ( QString::fromUtf8("Risto Saukonpaa"),
                          i18n("Design, icons, logo, banner, mockup, beta tester"),
                          QString::fromUtf8("paristo at gmail dot com")
                        );

    aboutData.addCredit ( QString::fromUtf8("Mikolaj Machowski"),
                          i18n("Bug reports and patches"),
                          QString::fromUtf8("mikmach at wp dot pl"),
                          QString::fromUtf8("https://www.facebook.com/mikolaj.machowski")
                        );

    aboutData.addCredit ( QString::fromUtf8("Achim Bohnet"),
                          i18n("Bug reports and patches"),
                          QString::fromUtf8("ach at mpe dot mpg dot de"),
                          QString::fromUtf8("https://www.facebook.com/achim.bohnet")
                        );

    aboutData.addCredit ( QString::fromUtf8("Luka Renko"),
                          i18n("Developer"),
                          QString::fromUtf8("lure at kubuntu dot org"),
                          QString::fromUtf8("https://www.facebook.com/luka.renko")
                        );

    aboutData.addCredit ( QString::fromUtf8("Angelo Naselli"),
                          i18n("Developer"),
                          QString::fromUtf8("a dot naselli at libero dot it"),
                          QString::fromUtf8("https://www.linkedin.com/in/angelo-naselli-11199028/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Fabien Salvi"),
                          i18n("Webmaster"),
                          QString::fromUtf8("fabien dot ubuntu at gmail dot com")
                        );

    aboutData.addCredit ( QString::fromUtf8("Todd Shoemaker"),
                          i18n("Developer"),
                          QString::fromUtf8("todd at theshoemakers dot net")
                        );

    aboutData.addCredit ( QString::fromUtf8("Gerhard Kulzer"),
                          i18n("Handbook writer, alpha tester, webmaster"),
                          QString::fromUtf8("gerhard at kulzer dot net"),
                          QString::fromUtf8("https://www.linkedin.com/in/gerhard-kulzer-8931301/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Oliver Doerr"),
                          i18n("Beta tester"),
                          QString::fromUtf8("oliver at doerr-privat dot de")
                        );

    aboutData.addCredit ( QString::fromUtf8("Charles Bouveyron"),
                          i18n("Beta tester"),
                          QString::fromUtf8("c dot bouveyron at tuxfamily dot org")
                        );

    aboutData.addCredit ( QString::fromUtf8("Richard Taylor"),
                          i18n("Feedback and patches. Handbook writer"),
                          QString::fromUtf8("rjt-digicam at thegrindstone dot me dot uk")
                        );

    aboutData.addCredit ( QString::fromUtf8("Hans Karlsson"),
                          i18n("digiKam website banner and application icons"),
                          QString::fromUtf8("karlsson dot h at home dot se")
                        );

    aboutData.addCredit ( QString::fromUtf8("Aaron Seigo"),
                          i18n("Various usability fixes\nand general application polishing"),
                          QString::fromUtf8("aseigo at kde dot org"),
                          QString::fromUtf8("https://www.linkedin.com/in/aaronseigo/")
                        );

    aboutData.addCredit ( QString::fromUtf8("Yves Chaufour"),
                          i18n("digiKam website, Feedback"),
                          QString::fromUtf8("yves dot chaufour at wanadoo dot fr")
                        );

    aboutData.addCredit ( QString::fromUtf8("Tung Nguyen"),
                          i18n("Bug reports, feedback and icons"),
                          QString::fromUtf8("ntung at free dot fr")
                        );

    // -- Former Members ---------------------------------------------------------

    aboutData.addAuthor ( QString::fromUtf8("Renchi Raju"),
                          i18n("Developer (2001-2005)"),
                          QString::fromUtf8("renchi dot raju at gmail dot com"),
                          QString::fromUtf8("https://www.facebook.com/renchi.raju")
                        );

    aboutData.addAuthor ( QString::fromUtf8("Joern Ahrens"),
                          i18n("Developer (2004-2005)"),
                          QString::fromUtf8("kde at jokele dot de"),
                          QString::fromUtf8("http://www.jokele.de/")        // krazy:exclude=insecurenet
                        );

    aboutData.addAuthor ( QString::fromUtf8("Tom Albers"),
                          i18n("Developer (2004-2005)"),
                          QString::fromUtf8("tomalbers at kde dot nl"),
                          QString::fromUtf8("https://www.linkedin.com/in/tom-a-676a58157/")
                        );

    aboutData.addAuthor ( QString::fromUtf8("Ralf Holzer"),
                          i18n("Developer (2004)"),
                          QString::fromUtf8("kde at ralfhoelzer dot com")
                        );

    aboutData.addAuthor ( QString::fromUtf8("Marcel Wiesweg"),
                          i18n("Developer and Mentoring (2005-2012)"),
                          QString::fromUtf8("marcel dot wiesweg at gmx dot de"),
                          QString::fromUtf8("https://www.facebook.com/marcel.wiesweg")
                        );
}

} // namespace Digikam

#include "moc_daboutdata.cpp"
