#ifndef IMPORTOBJECTS_H
#define IMPORTOBJECTS_H

class QScriptEngine;
/*!
	\ingroup script
	Функции для испортирования C++ объектов приложения в оболочку сценариев.
* @class ImportObjects importobjects.h "src/kernel/script/importobjects.h"
*/
class ImportObjects
{
public:
	ImportObjects(){};

	/*!
		Создает QMetaObject оболочки в QScriptEngine для последующего использования C++ объектов в скрипте. Для раширения приложения необходимо в этой функции создавать оболочки для новых c++ объектов.
	* @param engine
	* @return int
	*/
	static int import(QScriptEngine* engine);
};

#endif // IMPORTOBJECTS_H
