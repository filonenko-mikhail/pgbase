#include "globalscriptengine.h"

#include <QtCore/QVariant>
#include <QtCore/QDebug>
#include <QtCore/QLibraryInfo>
#include <QtCore/QTextCodec>
#include <QtCore/QDir>
#include <QtCore/QTranslator>
#include <QtCore/QLibrary>
#include <QtCore/QSettings>

#include <QtGui/QApplication>
#include <QtGui/QIcon>

// Log4Qt
#include <log4qt/logger.h>
#include <log4qt/logmanager.h>
#include <log4qt/patternlayout.h>
#include <log4qt/helpers/datetime.h>
#include <log4qt/colorconsoleappender.h>
#include <log4qt/rollingfileappender.h>
#include <log4qt/loggingevent.h>

namespace Log4Qt
{
  class ColorConsoleLayout : public Layout
  {
  public:
    ColorConsoleLayout(QObject* parent = 0);
    virtual QString format(const LoggingEvent &rEvent);
    virtual QDebug debug(QDebug &rDebug) const;
  };

  ColorConsoleLayout::ColorConsoleLayout(QObject *parent)
    :Layout(parent){}
  QString ColorConsoleLayout::format(const LoggingEvent &rEvent)
  {
    QString formattedMessage = DateTime::fromMilliSeconds(rEvent.timeStamp()).toString("dd.MM.yyyy hh:mm");
    formattedMessage.append(" [");
    formattedMessage.append(rEvent.threadName());
    formattedMessage.append("] ");
    formattedMessage.append(rEvent.level().toString());
    formattedMessage.append(" ");
    formattedMessage.append(rEvent.loggerName());
    formattedMessage.append(" ");
    switch (rEvent.level().toInt()) {
    case Level::DEBUG_INT:
      formattedMessage.append("[33m%1[0m");
      break;
    case Level::INFO_INT:
      formattedMessage.append("[1;34m%1[0m");
      break;
    case Level::WARN_INT:
      formattedMessage.append("\e[1;31m%1\e[0m");
      break;
    case Level::ERROR_INT:
      formattedMessage.append("\e[1;31m%1\e[0m");
      break;
    case Level::FATAL_INT:
      formattedMessage.append("\e[1;31m%1\e[0m");
      break;
    }
    formattedMessage.append(endOfLine());
    return formattedMessage.arg(rEvent.message());
  }
  QDebug ColorConsoleLayout::debug(QDebug &rDebug) const
  {
    rDebug.nospace() << metaObject()->className() << "("
        << "name:" << objectName()
        << ")";
    return rDebug.space();
  }

}

int main(int argc, char *argv[])
{
  using namespace Log4Qt;

  // UTF8 codecs for all possible strings in application
  QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
  QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
  QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

  // Create application object
  QApplication application(argc, argv);

  QDir currentDir = QDir(QCoreApplication::applicationDirPath());

  QString binAppPath = currentDir.absolutePath();
  currentDir.cdUp();
  QString rootAppPath = currentDir.absolutePath();

  QString pluginsAppPath;
  QString translationsAppPath;
  QString jsAppPath;
  QString shareAppPath;
  QString imagesAppPath;

  if (currentDir.exists("plugins"))
    pluginsAppPath = currentDir.filePath("plugins");

  if (currentDir.exists("translations"))
    translationsAppPath = currentDir.filePath("translations");

  if (currentDir.exists("js"))
    jsAppPath = currentDir.filePath("js");

  if (currentDir.exists("share"))
    shareAppPath = currentDir.filePath("share");
  if (currentDir.exists("share/images"))
    imagesAppPath = currentDir.filePath("share/images");

  //QDir::addSearchPath("");
  QDir::addSearchPath("bin", binAppPath);
  QDir::addSearchPath("approot", rootAppPath);
  QDir::addSearchPath("plugins", pluginsAppPath);
  QDir::addSearchPath("js", jsAppPath);
  QDir::addSearchPath("translations", translationsAppPath);
  QDir::addSearchPath("share", shareAppPath);
  QDir::addSearchPath("images", imagesAppPath);

  QCoreApplication::addLibraryPath(pluginsAppPath);

  QCoreApplication::setApplicationName("pgBase");
  QCoreApplication::setApplicationVersion("0.1.0");
  QCoreApplication::setOrganizationName("asvil");
  QCoreApplication::setOrganizationDomain("asvil.org");

  QApplication::setWindowIcon(QIcon("images:32x32/application/elephant.ico"));

  QSettings *settings = new QSettings(&application);
  settings->setObjectName("settings");

  // Log engine objects
  PatternLayout *patternLayout;
  ColorConsoleLayout *colorConsoleLayout;
  ColorConsoleAppender *colorConsoleAppender;

  //RollingFileAppender *fileAppender;

  // Create logger
  LogManager::rootLogger();
  // Hook qDebug(), etc.
  LogManager::setHandleQtMessages(true);

  // Format log message "date time [thread] logger level - message \newline"
  patternLayout = new PatternLayout("%d{dd.MM.yyyy hh:mm} [%t] %p %c %x - %m%n", LogManager::rootLogger());
  patternLayout->activateOptions();

  colorConsoleLayout = new ColorConsoleLayout(LogManager::rootLogger());
  colorConsoleLayout->setName("colorConsoleLayout");
  colorConsoleLayout->activateOptions();

  // Create an appender to console with possible color formatiing
  colorConsoleAppender = new ColorConsoleAppender(colorConsoleLayout, ColorConsoleAppender::STDOUT_TARGET
                                                  , LogManager::rootLogger());
  colorConsoleAppender->setName(QLatin1String("colorConsoleAppender"));
  colorConsoleAppender->activateOptions();

  LogManager::rootLogger()->addAppender(colorConsoleAppender);

  // Translation suffix
  QString localeName = QLocale::system().name();
  QString trFileDelimiter = "_";

  Logger *appLogger = LogManager::logger("Application");

  // First of all load qt translation file
  QString qtTranslationFileName = "qt" + trFileDelimiter + localeName;
  appLogger->info("Load translation file for Qt framework...");
  QTranslator *qtTranslator = new QTranslator(&application);
  if (!qtTranslator->load(qtTranslationFileName, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
    appLogger->warn("Not loaded");
  else
    appLogger->info("Loaded");

  QCoreApplication::installTranslator(qtTranslator);

  // Load application translation file
  QString appTranslationFileName = application.applicationName().toLower() + trFileDelimiter + localeName;
  appLogger->info("Load translation file for application...");
  QTranslator *appTranslator = new QTranslator(&application);
  if (!appTranslator->load(appTranslationFileName, translationsAppPath))
    appLogger->warn("Not loaded");
  else
    appLogger->info("Loaded");
  QCoreApplication::installTranslator(appTranslator);

  if (!GlobalScriptEngine::main()) {
    return application.exec();
  }
  return 1;
}
