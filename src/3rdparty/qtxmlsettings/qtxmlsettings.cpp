#include "qtxmlsettings.h"

#include <QtXml/QDomDocument>

#include <QtCore/QCoreApplication>

#include <QtCore/QDebug>

const QSettings::Format QtXmlSettings::xml_format = QSettings::registerFormat("xml", readXmlFile, writeXmlFile);

bool QtXmlSettings::readXmlFile(QIODevice &device, QSettings::SettingsMap &map)
{
	QDomDocument doc("");
	if ( !doc.setContent( &device ) ) return false;

	QDomElement root = doc.documentElement();

	processReadKey( "", map, root );

	return true;
}

bool QtXmlSettings::writeXmlFile(QIODevice &device, const QSettings::SettingsMap &map)
{
	QDomDocument doc("");
	QDomElement root = doc.createElement("root");
	doc.appendChild(root);

	QMapIterator<QString, QVariant> i(map);
	while ( i.hasNext() )
	{
		i.next();

		QString  sKey = i.key();

		processWriteKey( doc, root, sKey, i.value() );
	};

	QDomNode xmlNode = doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
	doc.insertBefore(xmlNode, doc.firstChild());

	QTextStream out( &device );
	doc.save(out, 4);

	return true;
}

void QtXmlSettings::processWriteKey( QDomDocument& doc, QDomElement& domElement
																	 , QString key
																	 , const QVariant& value )
{
	int slashPos = key.indexOf( '/' );

	// переданный ключ является параметром
	if ( slashPos < 0 )
	{
		// не пишем в конфиг параметр size (является ограничением - нельзя исп. пар-тр с таким именем)
		if ( key == "size" ) return;
		//domElement.setAttribute( key, value.toString() );

		// Пишем не в атрибут, а в дочерний элемент.

		QDomElement key_element = doc.createElement( key );
		QDomText value_text = doc.createTextNode( value.toString() );
		key_element.appendChild( value_text );
		domElement.appendChild( key_element );
		return;
	};

	// получение имени группы соответствующей xml ноде
	QString groupName = key.left( slashPos );
	// если в качестве имени использован числовой параметр - это табличная строка, преобразуем ее в row_?
	if ( groupName.toInt() )
	{
		groupName = "row_" + groupName;
		domElement.toElement().setAttribute("table", "1");
	};

	// поиск/создание ноды соответствующей ключу
	QDomElement groupElement;
	QDomNodeList findedNodes = domElement.elementsByTagName( groupName );

	if ( !findedNodes.count() ) // Заданный элемент-нода не найден - создаем его
	{
		groupElement = doc.createElement( groupName );
		domElement.appendChild( groupElement );
	}
	else
	{
		Q_ASSERT( findedNodes.count() == 1 ); // в конфигах не должно быть несколько элементов с одинаковым именем
		groupElement = findedNodes.item(0).toElement();
	};

	// готовим обрезанную часть ключа
	key = key.right( key.size() - slashPos - 1 );

	// продолжение обработки (создание/поиск групп) - пока не найдется параметр
	processWriteKey( doc, groupElement, key, value );
}

void QtXmlSettings::processReadKey(QString key, QSettings::SettingsMap &map, QDomElement& domElement )
{

	/*
	 *QDomNamedNodeMap namedNodeMap = domElement.attributes();
	 *
	 * //Добавление всех атрибутов элемента в качестве значений
	 *for (int i = 0; i < namedNodeMap.count(); ++i)
	 *{
	 *    QString name = namedNodeMap.item( i ).toAttr().name();
	 *    QString value = namedNodeMap.item( i ).toAttr().value();
	 *    map.insert( key + name, value );
	 *};
	 */

	//static int deep = 0;
	//qDebug() << deep++ << " " << key;

	QDomNodeList nlChild = domElement.childNodes();

	// если узел является таблицей - то все дети строки
	bool isTable = domElement.attribute("table", "0").toInt();

	// создаем доп. элемент size равный числу детей (необходим для QSettings - beginArray)
	if ( isTable )
		map.insert( key + "size", nlChild.count() );

	// проход по всем детям
	for (int i = 0; i < nlChild.count(); ++i)
	{
		QString childName = nlChild.item(i).toElement().tagName();
		// Если тег содержит в начале строку row_, тогда это индекс массива
		if ( childName.contains("row_") )
		{
			childName = childName.right( childName.size() - 4 );
		// Если элемент является текстом, это данные добавляем их, и переходим к следующей итерации цикла
		} else if (nlChild.item(i).isText()) {
			QString value = nlChild.item(i).toText().data();
			QString work_key = key;
			work_key.chop(1);
			// Убираем слеш в конце
			map.insert( work_key, value );
			//qDebug() << "inserted " << work_key << " = " << value;
			continue ;
		}

		QString subKey = key + childName + "/";
		QDomElement subElement = nlChild.item(i).toElement();

		processReadKey( subKey, map, subElement );
	};

	//--deep;
}
