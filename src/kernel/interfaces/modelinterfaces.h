#ifndef MODELINTERFACES_H
#define MODELINTERFACES_H

#include <QtCore/QObject>

#include <QtGui/QUndoStack>

class IRefreshModel
{
public :
		virtual void refresh() = 0;
};

Q_DECLARE_INTERFACE(IRefreshModel, "com.asvil.IRefreshModel/1.0")

class IUndoRedoModel
{
public:
  // can not be null
  virtual QUndoStack * undoStack() const = 0;
};

Q_DECLARE_INTERFACE(IUndoRedoModel, "com.asvil.IUndoRedoModel/1.0")

#endif // #ifndef MODELINTERFACES_H
