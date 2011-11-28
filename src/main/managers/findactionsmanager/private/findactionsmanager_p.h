#ifndef FINDACTIONSMANAGER_P_H
#define FINDACTIONSMANAGER_P_H

#include "findactionsmanager.h"

#include <QtCore/QSet>

class FindActionsManagerHelper;
class FindActionsManagerPrivate
{
public:
  FindActionsManagerPrivate();
  virtual ~FindActionsManagerPrivate();

  QWidget *parent;

  SearchWidget *searchWidget;

  QWidget *searchableFocusWidget;
  FindActionsManagerHelper *currentHelper;

  QSet<FindActionsManagerHelper*> helpers;

};

#endif // FINDACTIONSMANAGER_P_H
