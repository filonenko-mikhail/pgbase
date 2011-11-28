#ifndef STANDARDTREEMODEL_H
#define STANDARDTREEMODEL_H

#include <QtGui/QStandardItemModel>

//class QStandardItem;
class StandardTreeModel : public QStandardItemModel
{
	Q_OBJECT
public:
	StandardTreeModel(int rows, int columns, QObject *parent = 0);
	StandardTreeModel(QObject *parent = 0);

	/**
	* @brief Возвращает QStandardItem для заданного пути. Все элементы, встречающиеся в пути создаются, если их не было.
	*
	* @param path Задает путь к элементу иерархической модели. Элементы в пути разделяются слешем '/'
	* @param sectionPos Не использовать. Параметр для рекурсии. Указывает на конкретный элемент в пути.
	* @param parentItem Не использовать. Параметр для рекурсии. Указывает на родительский элемент.
	* @return QStandardItem Созданный или найденный элемент.
	*/
	QStandardItem* itemForPath(const QString& path, int sectionPos = 0, QStandardItem* parentItem = 0);

	/**
	* @brief Удаляет элемент модели заданный путем.
	* @return bool
	\retval true Элемент был удален
	\retval false Элемент не найден
	*/
	bool removeItemWithPath(const QString& path, int sectionPos = 0, QStandardItem* parentItem = 0);

};

#endif //#ifndef STANDARDTREEMODEL_H
