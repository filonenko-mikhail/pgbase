#ifndef FINDACTIONSMANAGERHELPER_H
#define FINDACTIONSMANAGERHELPER_H

#include <QtCore/QObject>

class FindActionsManagerHelper : public QObject
{
  Q_OBJECT
public:
  FindActionsManagerHelper(QObject* parent = 0);
  virtual ~FindActionsManagerHelper();

  virtual unsigned char approach(QWidget* widget) const {return 0;};

  virtual void activateFor(QWidget* widget) = 0;
  virtual void deactivate() = 0;

  virtual bool canFind() const {return false;};
  virtual bool canReplace() const {return false;};

  virtual QString initialFindString() const {return QString::null;} ;

  virtual QString initialFindHintString() const {return QString::null;};

signals:
  void canFindChanged(bool enable);
  void canReplaceChanged(bool enable);

public slots:
  virtual void findNext(const QVariant& value, Qt::MatchFlags flags){};
  virtual void findPrevious(const QVariant& value, Qt::MatchFlags flags){};

  virtual void replaceNext(const QVariant& find, const QString& replace, Qt::MatchFlags flags){};
  virtual void replacePrevious(const QVariant& find, const QString& replace, Qt::MatchFlags flags){};

  virtual void replaceAll(const QVariant& find, const QString& replace, Qt::MatchFlags flags){};

  virtual void findEdited(const QVariant& value, Qt::MatchFlags flags){};
};

class QAbstractItemView;

class ItemViewFindActionsManagerHelper : public FindActionsManagerHelper
{
  Q_OBJECT
public:
  ItemViewFindActionsManagerHelper(QObject* parent = 0);

  virtual unsigned char approach(QWidget* widget) const;

  virtual void activateFor(QWidget* widget);
  virtual void deactivate();

  virtual bool canFind() const;
  virtual bool canReplace() const;

  virtual QString initialFindString() const;

  virtual QString initialFindHintString() const;

signals:
  void canFindChanged(bool enable);
  void canReplaceChanged(bool enable);

public slots:
  virtual void findNext(const QVariant& value, Qt::MatchFlags flags);
  virtual void findPrevious(const QVariant& value, Qt::MatchFlags flags);

  virtual void replaceNext(const QVariant& find, const QString& replace, Qt::MatchFlags flags);
  virtual void replacePrevious(const QVariant& find, const QString& replace, Qt::MatchFlags flags);

  virtual void replaceAll(const QVariant& find, const QString& replace, Qt::MatchFlags flags);

  virtual void findEdited(const QVariant& value, Qt::MatchFlags flags);
private:
  QAbstractItemView *itemView;
};

#endif //#ifndef FINDACTIONSMANAGERHELPER_H
