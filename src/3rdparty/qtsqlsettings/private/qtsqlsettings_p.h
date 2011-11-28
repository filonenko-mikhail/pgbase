#ifndef QTSQLSETTING_P_H
#define QTSQLSETTING_P_H

#include "qtsqlsettings.h"

#include <QtCore/QMutex>

#include <QtCore/QStringList>
#include <QtCore/QStack>

#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>

class QSettingsGroup
{
public:
		inline QSettingsGroup()
				: num(-1), maxNum(-1) {}
		inline QSettingsGroup(const QString &s)
				: str(s), num(-1), maxNum(-1) {}
		inline QSettingsGroup(const QString &s, bool guessArraySize)
				: str(s), num(0), maxNum(guessArraySize ? 0 : -1) {}

		inline QString name() const { return str; }
		inline QString toString() const;
		inline bool isArray() const { return num != -1; }
		inline int arraySizeGuess() const { return maxNum; }
		inline void setArrayIndex(int i)
		{ num = i + 1; if (maxNum != -1 && num > maxNum) maxNum = num; }

		QString str;
		int num;
		int maxNum;
};

inline QString QSettingsGroup::toString() const
{
		QString result;
		result = str;
		if (num > 0) {
				result += QLatin1Char('/');
				result += QString::number(num);
		}
		return result;
}

class QtSqlSettingsPrivate
{
	Q_DECLARE_PUBLIC(QtSqlSettings)
public:
	QtSqlSettingsPrivate()
	{}
	virtual ~QtSqlSettingsPrivate(){};

	QtSqlSettings::SettingsMap map;

	QString table;
	QString connection;

	QString keyColumn;
	QString valueColumn;

	virtual void remove(const QString &key);
	virtual void set(const QString &key, const QVariant &value);
	virtual bool get(const QString &key, QVariant *value) const;
	enum ChildSpec { AllKeys, ChildKeys, ChildGroups };
	virtual QStringList children(const QString &prefix, ChildSpec spec) const;

	virtual void clear();

	QString actualKey(const QString &key) const;
	void beginGroupOrArray(const QSettingsGroup &group);

	static QString normalizedKey(const QString &key);

	static void processChild(QString key, ChildSpec spec, QMap<QString, QString> &result);

	static QStringList variantListToStringList(const QVariantList &l);
	static QVariant stringListToVariantList(const QStringList &l);

	// parser functions
	static QString variantToString(const QVariant &v);
	static QVariant stringToVariant(const QString &s);
	static QStringList splitArgs(const QString &s, int idx);

	QtSqlSettings * q_ptr;

protected:
		QStack<QSettingsGroup> groupStack;
		QString groupPrefix;

		mutable QMutex mutex;
};

#endif //#ifndef QTSQLSETTING_P_H
