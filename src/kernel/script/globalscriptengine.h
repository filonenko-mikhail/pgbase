#ifndef GLOBALSCRIPTENGINE_H
#define GLOBALSCRIPTENGINE_H

#include <QtCore/QObject>

#include <QtScript/QScriptValue>

class QScriptContext;
class QScriptEngine;
class QScriptEngineDebugger;

/*!
	\ingroup script
	Объект для инициализации оболочки qt сцеанариев. С небольшими изменениями скопированно с qt script bindings.
* @class GlobalScriptEngine globalscriptengine.h "src/kernel/script/globalscriptengine.h"
*/
class GlobalScriptEngine
{
	static QScriptEngine * staticEngine;
	static QScriptEngineDebugger * staticDebugger;
public:

	GlobalScriptEngine(){};
	virtual ~GlobalScriptEngine(){};

	/*!
		Загрузить оболочку сценариев.
	* @return QScriptEngine
	*/
	static int main();
	/*!
		Удалить оболочку qt сценариев.
	*/
	static void down();
	/*!
		Перезагрузить оболочку qt сценариев
	* @return bool
	*/
	static int reload();
	static QScriptValue reloadWrapper(QScriptContext* context, QScriptEngine* engine);
	static bool isUpped();

	static QScriptEngine* engine();
	static QScriptEngineDebugger* debugger();
	/*!
		Выполнить qt script файл.
	* @param fileName
	* @return bool
	*/
	static bool evaluateFile(const QString& fileName);
};

#endif // GLOBALSCRIPTENGINE_H
