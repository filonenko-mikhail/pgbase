#ifndef QTSCRIPTSIGNALHANDLER_H
#define QTSCRIPTSIGNALHANDLER_H

#include <QtScript/QScriptValue>

#include <QtCore/QtGlobal>

#if defined(QTSCRIPTSIGNALHANDLER_EXPORTS)
//#  warning("QtScriptSignalHandler exporting symbols")
#  define QTSCRIPTSIGNALHANDLER_EXPORT Q_DECL_EXPORT
#elif defined(QTSCRIPTSIGNALHANDLER_IMPORTS)
//#  warning("QtScriptSignalHandler compiling")
#  define QTSCRIPTSIGNALHANDLER_EXPORT Q_DECL_IMPORT   /**/
#else
//#  warning("QtScriptSignalHandler importing symbols")
#  define QTSCRIPTSIGNALHANDLER_EXPORT    /**/
#endif

class QScriptContext;
class QScriptEngine;
// MINI HACK
// Макрос для вычисления сигнатуры слота после макроса SLOT();
#ifndef QT_NO_DEBUG
# define DESLOT(a)    (a + 1)
#else
//# define SLOT(a)      "1"#a
/*!
  Макрос для получения сигнатуры слота после макроса SLOT
  */
# define DESLOT(a)    (a + 1)
#endif

/*!
  \page build_install "Build and install"

  cmake .
  make
  make install
  */

/*!
  \page qtscriptsignalhandler "Создание сигналов в qt script"

  После инсталляции qt содержит две дполонительные библиотеки:
  \li ${QT_LIBRARY_DIR}/qtscriptsignalhandler, (e.g. /usr/lib/qt4/lib/qtscriptsignalhandler)

  Вторая библиотека содержит более полный набор функций. С помощью нее можно создавать соединения script-cpp.

  Для подключения второй библиотеки к проекту необходимо:

  QMAKE

  In *.pro:

  CONFIG += qtscriptsignalhandler

  CMAKE

  In CMakeLists.txt:

  include_directories(${QT_INCLUDE_DIR}/QtScriptTools)

  find_library(QT_QTSCRIPT_SIGNAL_HANDLER NAMES qtscriptsignalhandler PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH )

  target_link_libraries(main ${QT_QTSCRIPT_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTSCRIPT_SIGNAL_HANDLER})

  Для инициализации необходимо вызвать installSignalHandler(). Первый параметр используемая система сценариев. Второй параметр необязателен, в нем можно задать имя функции, используемой в дальнейшем для создания сигналов.
  \warning Совпадение списка и типов параметров между сигналом и слотом лежит на ответственности программиста.

  \section begin Определение сигнала в qt script
  Пример создания qtscript-сигнала в функции-конструкторе объекта:
\code
//  ПРАВИЛЬНО (RIGHT)
SignalBaseClass = function() {
  this.test = "signalBaseClass";
  this.baseSignal = signal();
  this.intSignal = signal();
};
SignalBaseClass.prototype.emitBaseSignal = function() {
  this.baseSignal.emit("emitBaseSignal from " + this.test);
};
SignalBaseClass.prototype.emitBaseSignal2 = function() {
  this.baseSignal.emit("emitBaseSignal2 from " + this.test);
};
SignalBaseClass.prototype.emitBaseIntSignal = function() {
  this.intSignal.emit(123);
};

//  НЕПРАВИЛЬНО (WRONG)
SignalBaseClass.prototype.baseSignal = signal();
\endcode

  Теперь свойство baseSignal любого экземпляра класса SignalBaseClass в свою очередь имеет три свойства:
  \li connect([object,] slot)
    Функция для подключения слота к данному сигналу.
    Если параметр object опущен, при генерации сигнала slot будет выполняться в контексте глобального объекта (QScriptEngine::globalObject()), иначе - в контексте object.
    Параметр slot может быть объектом-функцией или строкой.
  \li disconnect([[object,], slot])
    Функция для отключения слота от данного сигнала.
    Вызов без параметров отключает от сигнала все слоты.
    Вызов только с объектом, отключает все слоты данного объекта от сигнала.
    Вызов с объектом и функцией отключает только данный слот.
    Параметр slot может быть объектом-функцией или строкой.
  \li emit([param1, param2, ...])
    Функция для генерации данного сигнала с заданными параметрами. Создана для удобства разделения обчных функций и сигналов, потому что сигнал можно генерировать его вызовом.

  Пример наследования сигналов:
\code
SignalClass = function() {
// Вызываем родительский конструктор
  SignalBaseClass.call(this);
  this.newSignal = signal();
  this.test = "signalClass";
};

SignalClass.prototype = new SignalBaseClass();

SignalClass.prototype.emitBaseSignal = function() {
  this.baseSignal.emit("emitBaseSignal from " + this.test);
};
SignalClass.prototype.emitBaseIntSignal = function() {
  this.intSignal.emit(456);
};
SignalClass.prototype.emitNewSignal = function() {
  this.newSignal("emitNewSignal from " + this.test, "param2", 3);
};
\endcode

 \section scripttoscript Подключение сигнала script/script
  Пример подключения в qt сценарии:
\code
testSlot = function() {
  print("testSlot arguments count: " + arguments.length);
  for (var i = 0; i < arguments.length; ++i)
    print("testSlot first argument: " + arguments[i]);
};

SlotClass = function() {
  this.name = "slotClass";
};

SlotClass.prototype.slot = function() {
  print(this.name + " slot arguments count: " + arguments.length);
  for (var i = 0; i < arguments.length; ++i)
    print(this.name + " argument(" + i + ") " +  arguments[i]);
};
baseObject = new SignalBaseClass();
slotObject = new SlotClass();
baseObject.intSignal.connect(testSlot);
baseObject.emitBaseIntSignal();

signalObject = new SignalClass();
signalObject.baseSignal.connect(testSlot);
signalObject.newSignal.connect(slotObject, slotObject.slot);
signalObject.emitNewSignal();
  \endcode

  Вариации подключения слота
\code
  signalObject.newSignal.connect('testSlot');
  signalObject.newSignal.connect(slotObject, 'slot');
\endcode

  Пример отключения от сигнала в qt сценарии:
  \code
baseObject.intSignal.disconnect(testSlot);
baseObject.emitBaseIntSignal();
signalObject.newSignal.disconnect(slotObject, slotObject.slot);
signalObject.emitNewSignal();
signalObject.emitBaseSignal();
signalObject.baseSignal.disconnect();
signalObject.emitBaseSignal();
  \endcode

  Вариации отключения слота:
\code
// Отключить все слоты
  signalObject.newSignal.disconnect();
// Отключить все слоты данного объекта
  signalObject.newSignal.disconnect(slotObject);
// Отключить данный слот
  signalObject.newSignal.disconnect(slotObject, slotObject.slot);
\endcode

  \section scripttocpp Подключение сигнала script/с++
Для (под|от)ключения с++ слота к|от qtscript-сигнал(у|а) определены две функции:

\li bool qScriptConnect(QScriptValue sender, QScriptValue signal
                    , QObject* receiver, const char* slot);
Первый параметр объект генерирующий сигнал
Второй параметр строка имя сигнала или свойство объекта sender, которое является сигналом
Функция возвращает:
  \li true соединение прошло успешно
  \li false возникли ошибки, вывод в qDebug()

\li bool qScriptDisconnect(QScriptValue sender, QScriptValue signal
                       , QObject* receiver, const char* slot);
Первый параметр объект генерирующий сигнал
Второй параметр строка имя сигнала или свойство объекта sender, которое является сигналом
  если receiver == 0 от сигнала отключаются все слоты, даже те которые были определены и подсоединены в qt script
  если slot == 0 от сигнала отключаются все слоты объекта receiver
Функция возвращает:
  \li true отсоединение прошло успешно
  \li false возникли ошибки, вывод в qDebug()

  Пример (под|от)ключения c++ слота:
\code
  QLineEdit lineEdit4(0);
  lineEdit4.setWindowTitle("4C++ LineEdit");
  lineEdit4.show();

  qScriptConnect(engine.globalObject().property("baseObject")
                 , "baseSignal"
                 , &lineEdit4, SLOT(setText(QString)));

  engine.evaluate("baseObject.emitBaseSignal();");

  qScriptDisconnect(engine.globalObject().property("baseObject")
                 , "baseSignal"
                 , &lineEdit4, SLOT(setText(QString)));

  engine.evaluate("baseObject.emitBaseSignal2();");
\endcode
*/

/*!
  Обертка для функции emit скрипт-сигнала.
  */
QScriptValue signalEmitWrapper(QScriptContext* context, QScriptEngine* engine);

/*!
  Обертка для скрипт-сигнала.
  */
QScriptValue signalFunctionWrapper(QScriptContext* context, QScriptEngine* engine);

/*!
  Обертка для функции disconnect сигнала.
  \return QScriptValue
  \retval undefinedValue success
  otherwise, it will throw a script exception
  */
QScriptValue signalDisconnectWrapper(QScriptContext* context, QScriptEngine* engine);

/*!
  Обертка для функции connect сигнала.
  \return QScriptValue
  \retval undefinedValue success
  otherwise, it will throw a script exception
  */
QScriptValue signalConnectWrapper(QScriptContext* context, QScriptEngine* engine);

/*!
  Обертка функции signal(), создающей новый сигнал.
  \return QScriptValue
  \retval undefinedValue success
  otherwise, it will throw a script exception
  */
QScriptValue signalWrapper(QScriptContext* context, QScriptEngine* engine);

/*!
  c++ функция для отсоединения c++ слота от qt script сигнала.
  \return bool
  \retval true отсоединение прошло успешно
  \retval false возникли ошибки, вывод в qDebug()
  */
QTSCRIPTSIGNALHANDLER_EXPORT
    bool qScriptDisconnect(QScriptValue sender, QScriptValue signal
                       , QObject* receiver, const char* slot);

/*!
  с++ функция для присоединения c++ слота к qt script сигналу.
  \return bool
  \retval true присоединение прошло успешно
  \retval false возникли ошибки, вывод в qDebug()
  */
QTSCRIPTSIGNALHANDLER_EXPORT
    bool qScriptConnect(QScriptValue sender, QScriptValue signal
                    , QObject* receiver, const char* slot);

/*!
  Функция создающая систему qt script сигналов.
  \param engine
  \param functionName имя функции для создания сигналов.
  */
QTSCRIPTSIGNALHANDLER_EXPORT
    void installSignalHandler(QScriptEngine* engine, const QString& functionName = "signal");

#endif // QTSCRIPTSIGNALHANDLER_H
