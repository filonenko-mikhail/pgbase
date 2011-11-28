#include "globalscriptengine.h"
#include "qtscriptsignalhandler.h"

#include "importobjects.h"

#include <QtCore/QDateTime>
#include <QtCore/QTextStream>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QMetaEnum>
#include <QtCore/QSettings>
#include <QtCore/QDebug>

#include <QtGui/QApplication>
#include <QtGui/QMainWindow>
#include <QtGui/QAction>

#include <QtScript/QScriptEngine>
#include <QtScriptTools/QScriptEngineDebugger>

bool loadFile(const QString& fileName, QScriptEngine *engine, QScriptValue& retValue)
{
  QVariantMap loadedFiles = engine->property("_pg_loaded_files").value<QVariantMap>();

  QString localFileName(fileName);
  QFileInfo fileInfo(localFileName);

  QString absoluteFileName = fileInfo.absoluteFilePath();
  QString absolutePath = fileInfo.absolutePath();
  QString canonicalFileName = fileInfo.canonicalFilePath();
  if (loadedFiles.contains(canonicalFileName))
    if (loadedFiles.value(canonicalFileName).toDateTime() == fileInfo.lastModified()) {
      return true;
    }

  loadedFiles.insert(canonicalFileName, fileInfo.lastModified());
  engine->setProperty("_pg_loaded_files", loadedFiles);

  //QString path = fileInfo.path();

  // load the file
  QFile file(fileInfo.absoluteFilePath());
  if (file.open(QFile::ReadOnly)) {
    QTextStream stream(&file);
    QString contents = stream.readAll();
    file.close();

    int endlineIndex = contents.indexOf('\n');
    QString line = contents.left(endlineIndex);
    int lineNumber = 1;

    // strip off #!/usr/bin/env qscript line
    if (line.startsWith("#!")) {
      contents.remove(0, endlineIndex+1);
      ++lineNumber;
    }

    // set qt.script.absoluteFilePath
    QScriptValue script = engine->globalObject().property("script");
    QScriptValue oldFilePathValue = script.property("absoluteFilePath");
    QScriptValue oldPathValue = script.property("absolutePath");
    script.setProperty("absoluteFilePath", engine->toScriptValue(absoluteFileName));
    script.setProperty("absolutePath", engine->toScriptValue(absolutePath));

    retValue = engine->evaluate(contents, fileInfo.absoluteFilePath(), lineNumber);
    if (engine->hasUncaughtException()) {
      QStringList backtrace = engine->uncaughtExceptionBacktrace();
      qWarning() << QString("    %1\n%2\n\n").arg(retValue.toString()).arg(backtrace.join("\n"));
      return true;
    }

    script.setProperty("absoluteFilePath", oldFilePathValue); // if we come from includeScript(), or whereever
    script.setProperty("absolutePath", oldPathValue); // if we come from includeScript(), or whereever
  } else {
    qWarning() << QObject::tr("File %1 not found").arg(fileName);
    return false;
  }
  return true;
}

QScriptValue includeScript(QScriptContext *context, QScriptEngine *engine)
{
  QString currentFileName = engine->globalObject().property("script").property("absoluteFilePath").toString();
  QFileInfo currentFileInfo(currentFileName);

  QString path = currentFileInfo.path();
  QString importFile = context->argument(0).toString();
  QFileInfo importInfo(importFile);
  if (importInfo.isRelative()) {
    importFile =  path + "/" + importInfo.filePath();
  }
  QScriptValue retValue;
  if (!loadFile(importFile, engine, retValue))
    return context->throwError(QObject::tr("Failed to resolve include: %1").arg(importFile));

  return retValue;
}

QScriptValue importedExtensions(QScriptContext */*context*/, QScriptEngine* engine)
{
  return engine->toScriptValue(engine->importedExtensions());
}

QScriptValue availableExtensions(QScriptContext */*context*/, QScriptEngine* engine)
{
  return engine->toScriptValue(engine->availableExtensions());
}

QScriptValue importExtension(QScriptContext *context, QScriptEngine *engine)
{
  return engine->importExtension(context->argument(0).toString());
}

QScriptEngine* GlobalScriptEngine::staticEngine = 0;
QScriptEngineDebugger* GlobalScriptEngine::staticDebugger = 0;


int GlobalScriptEngine::main()
{
  //if (!staticEngine) {
    staticEngine = new QScriptEngine;
    staticDebugger = new QScriptEngineDebugger(staticEngine);
    staticDebugger->attachTo(staticEngine);
    QMainWindow *debugWindow = staticDebugger->standardWindow();
    debugWindow->resize(1024, 640);

    staticEngine->installTranslatorFunctions();

    installSignalHandler(staticEngine);

    QScriptValue global = staticEngine->globalObject();
    // add a 'system' object
    QScriptValue system = staticEngine->newObject();
    global.setProperty("system", system);
    QScriptValue script = staticEngine->newObject();
    global.setProperty("script", script);


    // add os information to qt.system.os
#ifdef Q_OS_WIN32
    QScriptValue osName = staticEngine->toScriptValue(QString("windows"));
#elif defined(Q_OS_LINUX)
    QScriptValue osName = staticEngine->toScriptValue(QString("linux"));
#elif defined(Q_OS_MAC)
    QScriptValue osName = staticEngine->toScriptValue(QString("mac"));
#elif defined(Q_OS_UNIX)
    QScriptValue osName = staticEngine->toScriptValue(QString("unix"));
#endif
    system.setProperty("os", osName);

    // add environment variables to qt.system.env
    QMap<QString,QVariant> envMap;
    QStringList envList = QProcess::systemEnvironment();
    foreach (const QString &entry, envList) {
      QStringList keyVal = entry.split('=');
      if (keyVal.size() == 1)
        envMap.insert(keyVal.at(0), QString());
      else
        envMap.insert(keyVal.at(0), keyVal.at(1));
    }
    system.setProperty("env", staticEngine->toScriptValue(envMap));

    // add the include functionality to qt.script.include
    global.setProperty("include", staticEngine->newFunction(includeScript));
    // add the importExtension functionality to qt.script.importExtension
    global.setProperty("importExtension", staticEngine->newFunction(importExtension));
    global.setProperty("availableExtensions", staticEngine->newFunction(availableExtensions));
    global.setProperty("importedExtensions", staticEngine->newFunction(importedExtensions));
    global.setProperty("reloadScriptEngine", staticEngine->newFunction(reloadWrapper));

    ImportObjects::import(staticEngine);

    QStringList arguments = qApp->arguments();
    QString mainJsFileName = "js:main.js";
    if (!evaluateFile(mainJsFileName))
      return -1;

    QScriptString mainFunctionString = staticEngine->toStringHandle("main");
    QScriptValue globalObject = staticEngine->globalObject();
    QScriptValue mainFunctionValue = globalObject.property(mainFunctionString);
    QScriptValueList mainFunctionArguments;
    mainFunctionArguments << staticEngine->toScriptValue(arguments);
    QScriptValue resultOfMainFunction;
    if (mainFunctionValue.isFunction()) {
      resultOfMainFunction = mainFunctionValue.call(globalObject, mainFunctionArguments);
    } else {
      qWarning() << QObject::tr("Function with name 'main' not founded in %1").arg(mainJsFileName);
    }

    if (staticEngine->hasUncaughtException())
      return -1;

    return resultOfMainFunction.toInteger();
}

void GlobalScriptEngine::down()
{
  if (staticEngine) {
    if (staticDebugger) {
//      switch(staticDebugger->state()) {
//      case QScriptEngineDebugger::RunningState:
//        break;
//      case QScriptEngineDebugger::SuspendedState:
//        staticDebugger->action(QScriptEngineDebugger::ContinueAction)->trigger();
//        break;
//      }

      staticDebugger->standardWindow()->close();
      staticDebugger->detach();
      staticDebugger->deleteLater();;
      staticDebugger = 0;
    }
    staticEngine->deleteLater();;
    staticEngine = 0;
  }
}

int GlobalScriptEngine::reload()
{
  down();
  return main();
}

QScriptEngine* GlobalScriptEngine::engine()
{
  return staticEngine;
}

QScriptEngineDebugger* GlobalScriptEngine::debugger()
{
  return staticDebugger;
}

bool GlobalScriptEngine::evaluateFile(const QString& fileName)
{
  QScriptValue retValue;
  return loadFile(fileName, staticEngine, retValue);
}

QScriptValue GlobalScriptEngine::reloadWrapper(QScriptContext */*context*/, QScriptEngine */*engine*/)
{
  return GlobalScriptEngine::reload();
}
