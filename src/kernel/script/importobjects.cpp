#include "importobjects.h"

#include <QtDebug>

#include <QtCore/QMetaEnum>
#include <QtCore/QDateTime>

#include <QtGui/QMessageBox>
#include <QtGui/QAbstractItemDelegate>
#include <QtGui/QApplication>

#include <QtSql/QSqlDriver>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlField>

#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>

// MINI HACK
#include "plugin.cpp"

// MINI HACK
//#ifdef PGSQL
// Подключаем qt sql psql драйвер
#include <QtSql/QSqlDatabase>
#include <QtSql/qsql_psql.h>
//#include "libpq-fe.h"

// Переменные обозначающие части сообщений сервера
#define PG_DIAG_SEVERITY		'S'
#define PG_DIAG_SQLSTATE		'C'
#define PG_DIAG_MESSAGE_PRIMARY 'M'
#define PG_DIAG_MESSAGE_DETAIL	'D'
#define PG_DIAG_MESSAGE_HINT	'H'
#define PG_DIAG_STATEMENT_POSITION 'P'
#define PG_DIAG_INTERNAL_POSITION 'p'
#define PG_DIAG_INTERNAL_QUERY	'q'
#define PG_DIAG_CONTEXT			'W'
#define PG_DIAG_SOURCE_FILE		'F'
#define PG_DIAG_SOURCE_LINE		'L'
#define PG_DIAG_SOURCE_FUNCTION 'R'

// Функция-подписчик на PostgreSQL нотисы
typedef void (*PQnoticeReceiver) (void *arg, const PGresult *res);

// Функция устанавливающая подписчика на PostgreSQL нотисы
typedef PQnoticeReceiver (*PQsetNoticeReceiver)(PGconn *conn,
                                                PQnoticeReceiver proc,
                                                void *arg);

// Функция возвращающая определенные части сообщения
typedef char * (*PQresultErrorField)(const PGresult *res, int fieldcode);

// Переменные хранящие указатели на функции
PQsetNoticeReceiver pqSetNoticeReceiver;
PQresultErrorField pqResultErrorField;

#include <QtCore/QLibrary>

/*!
  Загрузка libpq
  \return bool
  \retval true успешно
  \retval false не успешно
*/
bool initPostgresqlLibrary()
{
  QLibrary library("libpq");
  if (library.load()) {
    pqSetNoticeReceiver = (PQsetNoticeReceiver)(library.resolve("PQsetNoticeReceiver"));
    pqResultErrorField = (PQresultErrorField)(library.resolve("PQresultErrorField"));
    return pqSetNoticeReceiver && pqResultErrorField;
  }
  return false;
}

// Переменная хранящая функцию в qt script, обрабатывающая нотисы
QScriptValue scriptNoticeReceiver = QScriptValue();

// MINI HACK
// Вызывает qt script функцию с 4-мя параметрами severity, primary, detail, hint
// severity - уровень сообщения
void PostgreSQLnoticeReceiver(void *arg ,const PGresult *res)
{
  QString severity = QString::fromUtf8(pqResultErrorField(res, PG_DIAG_SEVERITY));
  QString primary = QString::fromUtf8(pqResultErrorField(res, PG_DIAG_MESSAGE_PRIMARY));
  QString detail = QString::fromUtf8(pqResultErrorField(res, PG_DIAG_MESSAGE_DETAIL));
  QString hint = QString::fromUtf8(pqResultErrorField(res, PG_DIAG_MESSAGE_HINT));

  if (scriptNoticeReceiver.isFunction()) {
    QScriptValueList arguments;
    arguments << severity << primary << detail << hint;
    scriptNoticeReceiver.call(scriptNoticeReceiver.engine()->globalObject(), arguments);
  }
}
// MINI HACK
// subscribe to pgsql notification

// PQsetNoticeProcessor(sqlConnectionName, function)
QScriptValue PQsetNoticeReceiverWrapper(QScriptContext* context, QScriptEngine* /*engine*/)
{
  if (context->argumentCount() == 2) {
    QString connectionName = context->argument(0).toString();
    if (QSqlDatabase::contains(connectionName) && context->argument(1).isFunction()) {
      QVariant driverHandle = QSqlDatabase::database(connectionName).driver()->handle();
      if (!QString::compare(driverHandle.typeName(),"PGconn*")) {
        PGconn *handle = *static_cast<PGconn **>(driverHandle.data());
        if (handle != 0) {
          scriptNoticeReceiver = context->argument(1);
          if (initPostgresqlLibrary()) {
            pqSetNoticeReceiver(handle, PostgreSQLnoticeReceiver, 0);
            return true;
          }
        }
      }
    }
  }
  return false;
}
//#endif //PGSQL

//#ifdef LOG4QT

#include <log4qt/databaseappender.h>
#include <log4qt/databaselayout.h>
#include <log4qt/logmanager.h>
#include <log4qt/level.h>



// MINI HACK
// Crosplatform getUserName function
#ifdef Q_WS_WIN
#include <windows.h>
#include <lmcons.h>
#elif defined Q_WS_X11
#include <pwd.h>
#endif //#ifdef Q_WS_WIN

QScriptValue getLoginWrapper(QScriptContext* context, QScriptEngine* engine)
{
  QString userName;
#ifdef Q_WS_WIN
  WCHAR* lpszSystemInfo; // pointer to system information
  DWORD cchBuff = 256; // size of user name
  WCHAR tchBuffer[UNLEN + 1]; // buffer for expanded string

  lpszSystemInfo = tchBuffer;

  // Get and display the user name.
  GetUserNameW(lpszSystemInfo, &cchBuff);

  //Unicode string needs to be converted
  userName = QString::fromWCharArray(lpszSystemInfo);
#elif defined Q_WS_X11
  register struct passwd *pw;
  pw = getpwuid(getuid());
  if (pw)
    userName = pw->pw_name;
#endif //#ifdef Q_WS_WIN

  return userName;
}

QScriptValue importMainStaticExtensionWrapper(QScriptContext* context, QScriptEngine* engine)
{
  com_asvil_pgbase_ScriptPlugin *plugin = new com_asvil_pgbase_ScriptPlugin();
  QStringList keys = plugin->keys();
  for (int i = 0; i < keys.size(); ++i)
    plugin->initialize(keys.at(i), engine);
  return QScriptValue();
}

template <typename T>
    QScriptValue pointerToScript(QScriptEngine *engine, const T& value)
{
  return engine->newVariant(*value);
}

template <typename T>
    void pointerFromScript(const QScriptValue& object, T& value)
{
  *value = object.toVariant();
}

template <typename T>
    QScriptValue flagsToScriptValue(QScriptEngine *engine, const T& value)
{
  return engine->newVariant(static_cast<int>(value));
}

template <typename T>
    void flagsFromScriptValue(const QScriptValue& object, T& value)
{
  int preValue = object.toInteger();

  value = static_cast<T>(preValue);
}

template <typename T>
    QScriptValue qobjectToScriptValue(QScriptEngine *engine, const T& value)
{
  return engine->newQObject(value);
}

template <typename T>
    void qobjectFromScriptValue(const QScriptValue& object, T& value)
{
  value = static_cast<T>(object.toQObject());
}

template<typename T>
int qScriptRegisterMetaTypeQO(
    QScriptEngine *engine,
    const QScriptValue &prototype = QScriptValue())
{
  return qScriptRegisterMetaType(engine, qobjectToScriptValue<T>, qobjectFromScriptValue<T>, prototype);
}

template<typename T>
QScriptValue qScriptCreateConstructor(QScriptEngine *engine
                                      , const QScriptValue::PropertyFlags& flags = QScriptValue::KeepExistingFlags)
{
  QScriptValue ctor = qScriptValueFromQMetaObject<T>(engine);

  QString className = T::staticMetaObject.className();
  QScriptValue value = engine->globalObject();
  QScriptValue prevValue = value;
  int start = 0;
  QString prevSection;
  QString section = className.section("::", start, start);
  while (!section.isEmpty()) {
    if (!value.property(section).isValid()) {
      value.setProperty(section, engine->newObject());
    }
    prevValue = value;
    value = value.property(section);
    ++start;
    prevSection = section;
    section = className.section("::", start, start);
  }
  prevValue.setProperty(prevSection, ctor, flags);

  return ctor;
}

#include <log4qt/layout.h>
#include <log4qt/helpers/datetime.h>
#include <log4qt/signalappender.h>
namespace Log4Qt
{
  class ColorHtmlLayout : public Layout
  {
  public:
    ColorHtmlLayout(QObject* parent = 0);
    virtual QString format(const LoggingEvent &rEvent);
    virtual QDebug debug(QDebug &rDebug) const;
  };

  ColorHtmlLayout::ColorHtmlLayout(QObject *parent)
    :Layout(parent){}
  QString ColorHtmlLayout::format(const LoggingEvent &rEvent)
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
      formattedMessage.append("<font color=#FFCC33><b>%1</b></font>");
      break;
    case Level::INFO_INT:
      formattedMessage.append("<font color=#3366FF><b>%1</b></font>");
      break;
    case Level::WARN_INT:
      formattedMessage.append("<font color=#CC0000><b>%1</b></font>");
      break;
    case Level::ERROR_INT:
      formattedMessage.append("<font color=#CC0000><b>%1</b></font>");
      break;
    case Level::FATAL_INT:
      formattedMessage.append("<font color=#CC0000><b>%1</b></font>");
      break;
    }
    formattedMessage.append(endOfLine());
    return formattedMessage.arg(rEvent.message());
  }
  QDebug ColorHtmlLayout::debug(QDebug &rDebug) const
  {
    rDebug.nospace() << metaObject()->className() << "("
        << "name:" << objectName()
        << ")";
    return rDebug.space();
  }
}

#include <QtScript/QScriptValueIterator>

QScriptValue arrayContainsPrototype(QScriptContext* context, QScriptEngine* engine)
{
  QScriptValue array = context->thisObject();
  if (array.isArray() && context->argumentCount()) {
    QScriptValueIterator it(array);
    while (it.hasNext()) {
      it.next();
      if (it.flags() & QScriptValue::SkipInEnumeration)
        continue;
      it.name();
      if (it.value().equals(context->argument(0)))
        return true;
    }
  }
  return false;
}

int ImportObjects::import(QScriptEngine* engine)
{
  bool result = false;
  if (engine) {
    engine->globalObject().setProperty("PQsetNoticeReceiver", engine->newFunction(PQsetNoticeReceiverWrapper));
    engine->globalObject().setProperty("importMainStaticExtension"
                                       , engine->newFunction(importMainStaticExtensionWrapper));
    engine->globalObject().setProperty("getLogin", engine->newFunction(getLoginWrapper));

    engine->globalObject().property("Array").prototype().setProperty("contains", engine->newFunction(arrayContainsPrototype), QScriptValue::SkipInEnumeration);

    result = true;

    using namespace Log4Qt;
    ColorHtmlLayout *colorHtmlLayout;
    SignalAppender *signalAppender;

    colorHtmlLayout = new ColorHtmlLayout;
    signalAppender = new SignalAppender;
    signalAppender->setObjectName("signalAppender");
    signalAppender->setLayout(colorHtmlLayout);
    LogManager::rootLogger()->addAppender(signalAppender);

    engine->globalObject().setProperty("signalLogger", engine->newQObject(signalAppender));
  }

  return result;
}
