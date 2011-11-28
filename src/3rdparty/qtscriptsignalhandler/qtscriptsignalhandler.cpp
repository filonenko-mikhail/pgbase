
#include "qtscriptsignalhandler.h"

#include <QtCore/QDebug>
#include <QtCore/QMetaMethod>

#include <QtScript/QScriptContext>
#include <QtScript/QScriptEngine>

Q_DECLARE_METATYPE(QObjectList);

QScriptValue signalEmitWrapper(QScriptContext* context, QScriptEngine* engine)
{
	QScriptValue thisObject = context->thisObject();
	if (thisObject.isFunction())
		thisObject.call(thisObject, context->argumentsObject());
	return engine->undefinedValue();
	//return context->throwError(QObject::tr("Error emit script signal"));
}


QScriptValue signalFunctionWrapper(QScriptContext* context, QScriptEngine* engine)
{
	QScriptValue thisObject = context->callee();
	QScriptValue binded = thisObject.data();
	QScriptString lengthHandle = engine->toStringHandle("length");
	if (binded.isValid()) {
		QScriptValue scriptObjects = binded.property("objects");
		QScriptValue scriptSlots = binded.property("slots");
		for (int i = 0; i < scriptObjects.property(lengthHandle).toInt32(); ++i)
			scriptSlots.property(i).call(scriptObjects.property(i), context->argumentsObject());
	}
	return engine->undefinedValue();
	//return context->throwError(QObject::tr("Error emit script signal"));
}


QScriptValue signalDisconnectWrapper(QScriptContext* context, QScriptEngine* engine)
{
	QScriptValue thisObject = context->thisObject();
	QScriptValue binded = thisObject.data();
	QScriptValue result = engine->undefinedValue();
	if (!binded.isValid())
		return context->throwError(QObject::tr("This function is not signal"));

	QScriptValue scriptObjects = binded.property("objects");
	QScriptValue scriptSlots = binded.property("slots");
	// Если данные свойства существуют
	if (!scriptObjects.isArray() || !scriptSlots.isArray())
		return context->throwError(QObject::tr("This function is not signal"));

	QScriptValue bindedObject;
	QScriptValue bindedSlot;
	QScriptString lengthHandle = engine->toStringHandle("length");
	switch (context->argumentCount()) {
			// не было агрументов удаляем все объекты/слоты
		case 0:
			binded.setProperty("objects", engine->newArray());
			binded.setProperty("slots", engine->newArray());
			break;
		case 1:
			// один агрумент,
			bindedObject = context->argument(0);
			// Если первый агрумент строка, ищем данное свойство у глобального объекта
			if (bindedObject.isString())
				bindedObject = engine->globalObject().property(bindedObject.toString());
			// аргумент функция, значит объект глобальный
			if (bindedObject.isFunction()) {
				bindedSlot = bindedObject;
				bindedObject = engine->globalObject();
				// аргумент объект, будем удалять все соединения с этим оъектом
			} else if (!bindedObject.isObject()) {
				result = context->throwError(QObject::tr("Argument 1 is not function"));
			}
			break;
		case 2:
			// два аргумента, объект, слот
			bindedObject = context->argument(0);
			bindedSlot = context->argument(1);
			// первый агрумент объект
			if (bindedObject.isObject()) {
				if (bindedSlot.isString())
					bindedSlot = bindedObject.property(bindedSlot.toString());
				// второй агрумент функция
				if (!bindedSlot.isFunction())
					result = context->throwError(QObject::tr("Argument 2 is not function"));
			} else
				result = context->throwError(QObject::tr("Argument 1 is not object"));
			break;
		default:
			return context->throwError(QObject::tr("Too many arguments"));
			break;
	}
	// агрументы прошли все проверки
	if (result.isUndefined()) {
		// задан слот для отсоединения
		if (bindedSlot.isValid()) {
			for (int i = 0; i < scriptObjects.property(lengthHandle).toInt32(); ++i) {
				if (scriptObjects.property(i).strictlyEquals(bindedObject)
						&& scriptSlots.property(i).strictlyEquals(bindedSlot)) {
					// не искушаем судьбу, удаляем объект и слот из списка объектов/слотов с помощью свойств array прототипа
					scriptObjects.property("splice").call(scriptObjects, QScriptValueList() << i << 1);
					scriptSlots.property("splice").call(scriptSlots, QScriptValueList() << i << 1);
				}
			}
			// слот не задан
		} else {
			// удаляем все вхождения объекта в списке объектов/слотов
			for (int i = 0; i < scriptObjects.property(lengthHandle).toInt32(); ++i) {
				if (scriptObjects.property(i).strictlyEquals(bindedObject)) {
					// не искушаем судьбу, удаляем объект и слот из списка объектов/слотов с помощью свойств array прототипа
					scriptObjects.property("splice").call(scriptObjects, QScriptValueList() << i << 1);
					scriptSlots.property("splice").call(scriptSlots, QScriptValueList() << i << 1);
				}
			}
		}
	}
	return result;
}

QScriptValue signalConnectWrapper(QScriptContext* context, QScriptEngine* engine)
{
	QScriptValue thisObject = context->thisObject();
	QScriptValue binded = thisObject.data();
	QScriptValue result = engine->undefinedValue();

	if (!binded.isValid())
		return context->throwError(QObject::tr("This function is not signal"));

	QScriptValue scriptObjects = binded.property("objects");
	QScriptValue scriptSlots = binded.property("slots");

	// Если данные свойства существуют
	if (!scriptObjects.isArray() || !scriptSlots.isArray())
		return context->throwError(QObject::tr("This function is not signal"));

	QScriptValue bindObject;
	QScriptValue bindSlot;
	int objectsCount = scriptObjects.property("length").toInt32();
	switch (context->argumentCount()) {
		case 1:
			// один агрумент - значит функция свойство глобального объекта
			bindSlot = context->argument(0);
			// Если агрумент строка - ищем данное свойство у глобального объекта
			if (bindSlot.isString())
				bindSlot = engine->globalObject().property(bindSlot.toString());
			if (bindSlot.isFunction()) {
				bindObject = engine->globalObject();
				scriptObjects.setProperty(objectsCount, bindObject);
				scriptSlots.setProperty(objectsCount, bindSlot);
			} else {
				result = context->throwError(QObject::tr("Argument 1 is not function"));
			}
			break;
		case 2:
			// первый аргумент объект, второй функция объекта
			bindObject = context->argument(0);
			bindSlot = context->argument(1);
			if (bindObject.isObject()) {
				if (bindSlot.isString())
					bindSlot = bindObject.property(bindSlot.toString());
				if (bindSlot.isFunction()) {
					scriptObjects.setProperty(objectsCount, bindObject);
					scriptSlots.setProperty(objectsCount, bindSlot);
				} else {
					result = context->throwError(QObject::tr("Argument 2 must be function"));
				}
			} else {
				result = context->throwError(QObject::tr("Argument 1 must be object"));
			}
			break;
		default:
			return context->throwError(QObject::tr("'connect' property of signal need 1 or 2 arguments (function|(object, function))"));
			break;
	}
	return result;
}

QScriptValue signalWrapper(QScriptContext* context, QScriptEngine* engine)
{
	if (!context->isCalledAsConstructor()) {
		QScriptValue thisObject = engine->newFunction(signalFunctionWrapper);
		QScriptValue binded = engine->newObject();
		binded.setProperty("objects", engine->newArray());
		binded.setProperty("slots", engine->newArray());
		thisObject.setData(binded);
		thisObject.setProperty("emit", engine->newFunction(signalEmitWrapper));
		thisObject.setProperty("connect", engine->newFunction(signalConnectWrapper));
		thisObject.setProperty("disconnect", engine->newFunction(signalDisconnectWrapper));
		return thisObject;
	} else {
		return context->throwError(QObject::tr("Signal function can't be called as constructor"));
	}

	return engine->undefinedValue();
}

const char* deslot(const QObject* object, const char* slot)
{
	int methodIndex = object->metaObject()->indexOfMethod(QMetaObject::normalizedSignature(slot + 1));
	return (methodIndex != -1) ? object->metaObject()->method(methodIndex).signature() : 0;
}

bool qScriptDisconnect(QScriptValue sender, QScriptValue signal
											 , QObject* receiver, const char* slot)
{
	if (!sender.isObject()) {
		qWarning() << Q_FUNC_INFO;
		qWarning() << "Sender script value must be object";
		return false;
	}

	if (signal.isString())
		signal = sender.property(signal.toString());
	if (!signal.isFunction()) {
		qWarning() << Q_FUNC_INFO;
		qWarning() << "Script value is not signal";
		return false;
	}

	QScriptValue binded = signal.data();
	QScriptValue objects = binded.property("objects");
	QScriptValue _slots = binded.property("slots");
	if (!objects.isValid()) {
		qWarning() << Q_FUNC_INFO;
		qWarning() << "Script value is not signal";
	}


	// если получатель равен 0 - удалить все соединения
	if (!receiver) {
		binded.setProperty("objects", sender.engine()->newArray());
		binded.setProperty("slots", sender.engine()->newArray());
	} else {
		QScriptString lengthHandle = sender.engine()->toStringHandle("length");
		// список объектов конвертируем в c++, потому что qscriptvalue receivera нам не известно
		QObjectList qtObjects = qScriptValueToValue<QObjectList>(objects);
		const char *desloted = deslot(receiver, slot);
		// удалить конкретное подключение
		if (desloted) {
			for (int i = 0; i < objects.property(lengthHandle).toInt32(); ++i)
				if ((qtObjects.at(i) == receiver)
						&& (_slots.property(i).strictlyEquals(objects.property(i).property(desloted)))) {
					objects.property("splice").call(objects, QScriptValueList() << i << 1);
					_slots.property("splice").call(_slots, QScriptValueList() << i << 1);
				}
			// удалить все подключения для данного объекта
		} else {
			for (int i = 0; i < objects.property(lengthHandle).toInt32(); ++i)
				if (qtObjects.at(i) == receiver) {
					objects.property("splice").call(objects, QScriptValueList() << i << 1);
					_slots.property("splice").call(_slots, QScriptValueList() << i << 1);
				}
		}
	}
	return true;
}

bool qScriptConnect(QScriptValue sender, QScriptValue signal
										, QObject* receiver, const char* slot)
{
	if (!sender.isObject()) {
		qWarning() << Q_FUNC_INFO;
		qWarning() << QObject::tr("Sender script value must be object");
		return false;
	}
	if (!receiver) {
		qWarning() << Q_FUNC_INFO;
		qWarning() << QObject::tr("Receiver object must be valid");
		return false;
	}
	if (!slot) {
		qWarning() << Q_FUNC_INFO;
		qWarning() << QObject::tr("Empty slot for object:") << receiver;
		return false;
	}

	const char *desloted = deslot(receiver, slot);
	if (desloted) {
		if (receiver->metaObject()->indexOfSlot(deslot(receiver, slot)) == -1) {
			qWarning() << Q_FUNC_INFO;
			qWarning() << QObject::tr("Slot %1 for object %2 not found").arg(slot, receiver->objectName());
			return false;
		}
		// если скрипт

		if (signal.isString())
			signal = sender.property(signal.toString());
		if (!signal.isFunction()) {
			qWarning() << Q_FUNC_INFO;
			qWarning() << QObject::tr("Script value is not signal");
			return false;
		}

		QScriptValue binded = signal.data();
		// если переданное значение не является сигналом
		if (!binded.isValid() || !signal.property("connect").isValid()) {
			qWarning() << Q_FUNC_INFO;
			qWarning() << QObject::tr("Script value is not signal");
			return false;
		}

		// прошли многочисленные проверки, вызывает функцию-свойство script сигнала connect
		QScriptValueList args;
		args << sender.engine()->newQObject(receiver);
		args << desloted;
		return signal.property("connect").call(signal, args).isUndefined();
	} else {
		qWarning() << Q_FUNC_INFO;
		qWarning() << QObject::tr("Slot %1 for object %2 not found").arg(slot, receiver->objectName());
		return false;
	}
}

void installSignalHandler(QScriptEngine* engine, const QString& functionName)
{
	engine->globalObject().setProperty(functionName, engine->newFunction(signalWrapper));
}
