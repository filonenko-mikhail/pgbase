
#include "qtsqlsettings.h"
#include "private/qtsqlsettings_p.h"

#include <QtCore/QDebug>

#include <QtCore/QMutexLocker>

#ifndef QT_NO_GEOM_VARIANT
#include <QtCore/QRect>
#endif  //#ifndef QT_NO_GEOM_VARIANT

#include <QtSql/QSqlDriver>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>
#include <QtSql/QSqlError>


QtSqlSettings::QtSqlSettings(QtSqlSettingsPrivate &dd, QObject* parent)
	:QObject(parent)
	, d_ptr(&dd)
{
	d_ptr->q_ptr = this;
	d_ptr->connection = QSqlDatabase::defaultConnection;
}

QtSqlSettings::QtSqlSettings(QObject *parent)
	:QObject(parent)
	, d_ptr(new QtSqlSettingsPrivate)
{
	d_ptr->q_ptr = this;
	d_ptr->connection = QSqlDatabase::defaultConnection;
}

QtSqlSettings::QtSqlSettings(const QString &connection, QObject *parent)
	:QObject(parent)
	, d_ptr(new QtSqlSettingsPrivate)
{
	d_ptr->q_ptr = this;
	Q_D(QtSqlSettings);
	d->connection = connection;
}

QtSqlSettings::~QtSqlSettings()
{
	//save();
	delete d_ptr;
}

QStringList QtSqlSettings::allKeys() const
{
	Q_D(const QtSqlSettings);

	return d->children(d->groupPrefix, QtSqlSettingsPrivate::AllKeys);
}

void QtSqlSettings::beginGroup(const QString &prefix)
{
	Q_D(QtSqlSettings);

	d->beginGroupOrArray(QSettingsGroup(d->normalizedKey(prefix)));
}

int QtSqlSettings::beginReadArray(const QString &prefix)
{
	Q_D(QtSqlSettings);
	d->beginGroupOrArray(QSettingsGroup(d->normalizedKey(prefix), false));
	return value(QLatin1String("size")).toInt();
}

void QtSqlSettings::beginWriteArray(const QString &prefix, int size)
{
	Q_D(QtSqlSettings);
	d->beginGroupOrArray(QSettingsGroup(d->normalizedKey(prefix), size < 0));

	if (size < 0)
			remove(QLatin1String("size"));
	else
			setValue(QLatin1String("size"), size);
}

QStringList QtSqlSettings::childGroups() const
{
	Q_D(const QtSqlSettings);

	return d->children(d->groupPrefix, QtSqlSettingsPrivate::ChildGroups);
}

QStringList QtSqlSettings::childKeys() const
{
	Q_D(const QtSqlSettings);
	return d->children(d->groupPrefix, QtSqlSettingsPrivate::ChildKeys);

}

void QtSqlSettings::clear()
{
	Q_D(QtSqlSettings);
	d->clear();
}

bool QtSqlSettings::contains(const QString &key) const
{
	Q_D(const QtSqlSettings);
	QString k = d->actualKey(key);
	return d->get(k, 0);

}

void QtSqlSettings::endArray()
{
	Q_D(QtSqlSettings);
	if (d->groupStack.isEmpty()) {
			qWarning("QSettings::endArray: No matching beginArray()");
			return;
	}

	QSettingsGroup group = d->groupStack.top();
	int len = group.toString().size();
	d->groupStack.pop();
	if (len > 0)
			d->groupPrefix.truncate(d->groupPrefix.size() - (len + 1));

	if (group.arraySizeGuess() != -1)
			setValue(group.name() + QLatin1String("/size"), group.arraySizeGuess());

	if (!group.isArray())
			qWarning("QSettings::endArray: Expected endGroup() instead");
}

void QtSqlSettings::endGroup()
{
	Q_D(QtSqlSettings);
	if (d->groupStack.isEmpty()) {
			qWarning("QSettings::endGroup: No matching beginGroup()");
			return;
	}

	QSettingsGroup group = d->groupStack.pop();
	int len = group.toString().size();
	if (len > 0)
			d->groupPrefix.truncate(d->groupPrefix.size() - (len + 1));

	if (group.isArray())
			qWarning("QSettings::endGroup: Expected endArray() instead");
}

QString QtSqlSettings::group() const
{
	Q_D(const QtSqlSettings);

	return d->groupPrefix.left(d->groupPrefix.size() - 1);
}

void QtSqlSettings::remove(const QString &key)
{
	Q_D(QtSqlSettings);
	/*
			We cannot use actualKey(), because remove() supports empty
			keys. The code is also tricky because of slash handling.
	*/
	QString theKey = d->normalizedKey(key);
	if (theKey.isEmpty())
			theKey = group();
	else
			theKey.prepend(d->groupPrefix);

	if (theKey.isEmpty()) {
			d->clear();
	} else {
			d->remove(theKey);
	}
}

void QtSqlSettings::setArrayIndex(int i)
{
	Q_D(QtSqlSettings);
	if (d->groupStack.isEmpty() || !d->groupStack.top().isArray()) {
			qWarning("QSettings::setArrayIndex: Missing beginArray()");
			return;
	}

	QSettingsGroup &top = d->groupStack.top();
	int len = top.toString().size();
	top.setArrayIndex(qMax(i, 0));
	d->groupPrefix.replace(d->groupPrefix.size() - len - 1, len, top.toString());
}

void QtSqlSettings::setValue(const QString &key, const QVariant &value)
{
	Q_D(QtSqlSettings);
	QString k = d->actualKey(key);
	d->set(k, value);

}

QVariant QtSqlSettings::value(const QString &key, const QVariant &defaultValue) const
{
	Q_D(const QtSqlSettings);
	QVariant result = defaultValue;
	QString k = d->actualKey(key);
	d->get(k, &result);
	return result;
}

QString QtSqlSettings::table() const
{
	Q_D(const QtSqlSettings);
	return d->table;
}

QString QtSqlSettings::connection() const
{
	Q_D(const QtSqlSettings);
	return d->connection;
}

void QtSqlSettings::setTable(const QString &table)
{
	Q_D(QtSqlSettings);
	d->table = table;
}

void QtSqlSettings::setConnection(const QString& connection)
{
	Q_D(QtSqlSettings);
	d->connection = connection;
}

QString QtSqlSettings::keyColumn() const
{
	Q_D(const QtSqlSettings);
	return d->keyColumn;
}

QString QtSqlSettings::valueColumn() const
{
	Q_D(const QtSqlSettings);
	return d->valueColumn;
}

void QtSqlSettings::setKeyColumn(const QString &columnName)
{
	Q_D(QtSqlSettings);
	d->keyColumn = columnName;
}

void QtSqlSettings::setValueColumn(const QString &columnName)
{
	Q_D(QtSqlSettings);
	d->valueColumn = columnName;
}

bool QtSqlSettings::load(const QString& table, const QString &keyColumn, const QString &valueColumn)
{
	setTable(table);
	setKeyColumn(keyColumn);
	setValueColumn(valueColumn);
	return load();
}

bool QtSqlSettings::load()
{
	Q_D(QtSqlSettings);

	// clear old cached settings map;
	d->map.clear();

	bool result(true);
	QSqlDatabase db = QSqlDatabase::database(d->connection);
	QSqlQuery sqlQuery(db);

	QString selectStatment = QString("SELECT %1, %2 FROM %3 ")
													.arg(db.driver()->escapeIdentifier(d->keyColumn, QSqlDriver::FieldName)
															 , db.driver()->escapeIdentifier(d->valueColumn, QSqlDriver::FieldName)
															 , db.driver()->escapeIdentifier(d->table, QSqlDriver::TableName));

	if (sqlQuery.exec(selectStatment)) {
		while (sqlQuery.next())
			d->map[sqlQuery.value(0).toString()] = d->stringToVariant(sqlQuery.value(1).toString());
	} else {
		result = false;
		qWarning() << Q_FUNC_INFO;
		qWarning() << sqlQuery.lastError().text();
	}

	return result;
}

bool QtSqlSettings::save(const QString& table, const QString& keyColumn, const QString& valueColumn)
{
	setTable(table);
	setKeyColumn(keyColumn);
	setValueColumn(valueColumn);
	return save();
}

bool QtSqlSettings::save()
{
	Q_D(QtSqlSettings);
	// clear old cached settings map

	bool result;
	QSqlDatabase db = QSqlDatabase::database(d->connection);
	QSqlQuery sqlQuery(db);

	//QString truncateStatment = QString("TRUNCATE TABLE %1")
	//												.arg(db.driver()->escapeIdentifier(d->table, QSqlDriver::TableName));

	QString truncateStatment = QString("DELETE FROM %1")
													.arg(db.driver()->escapeIdentifier(d->table, QSqlDriver::TableName));


	if ((result = sqlQuery.exec(truncateStatment))) {

		QString insertStatment = QString("INSERT INTO %1(%2, %3) VALUES (?, ?)")
														 .arg(db.driver()->escapeIdentifier(d->table, QSqlDriver::TableName)
																	, db.driver()->escapeIdentifier(d->keyColumn, QSqlDriver::FieldName)
																	, db.driver()->escapeIdentifier(d->valueColumn, QSqlDriver::FieldName));

		sqlQuery.prepare(insertStatment);

		QVariant keys = QStringList(d->map.keys());
		QVariant values = d->variantListToStringList(d->map.values());

		sqlQuery.addBindValue(keys);
		sqlQuery.addBindValue(values);

		if (!(result = sqlQuery.execBatch())) {
			qWarning() << Q_FUNC_INFO;
			qWarning() << sqlQuery.lastError().text();
		}
	} else {
		result = false;
		qWarning() << Q_FUNC_INFO;
		qWarning() << sqlQuery.lastError().text();
	}

	return result;
}

QString QtSqlSettingsPrivate::actualKey(const QString &key) const
{
		QString n = normalizedKey(key);
		Q_ASSERT_X(!n.isEmpty(), "QSettings", "empty key");
		n.prepend(groupPrefix);
		return n;
}

/*
		Returns a string that never starts nor ends with a slash (or an
		empty string). Examples:

						"foo"            becomes   "foo"
						"/foo//bar///"   becomes   "foo/bar"
						"///"            becomes   ""

		This function is optimized to avoid a QString deep copy in the
		common case where the key is already normalized.
*/
QString QtSqlSettingsPrivate::normalizedKey(const QString &key)
{
		QString result = key;

		int i = 0;
		while (i < result.size()) {
				while (result.at(i) == QLatin1Char('/')) {
						result.remove(i, 1);
						if (i == result.size())
								goto after_loop;
				}
				while (result.at(i) != QLatin1Char('/')) {
						++i;
						if (i == result.size())
								return result;
				}
				++i; // leave the slash alone
		}

after_loop:
		if (!result.isEmpty())
				result.truncate(i - 1); // remove the trailing slash
		return result;
}

void QtSqlSettingsPrivate::processChild(QString key, ChildSpec spec, QMap<QString, QString> &result)
{
		if (spec != AllKeys) {
				int slashPos = key.indexOf(QLatin1Char('/'));
				if (slashPos == -1) {
						if (spec != ChildKeys)
								return;
				} else {
						if (spec != ChildGroups)
								return;
						key.truncate(slashPos);
				}
		}
		result.insert(key, QString());
}

void QtSqlSettingsPrivate::beginGroupOrArray(const QSettingsGroup &group)
{
		groupStack.push(group);
		if (!group.name().isEmpty()) {
				groupPrefix += group.name();
				groupPrefix += QLatin1Char('/');
		}
}

QStringList QtSqlSettingsPrivate::variantListToStringList(const QVariantList &l)
{
		QStringList result;
		QVariantList::const_iterator it = l.constBegin();
		for (; it != l.constEnd(); ++it)
				result.append(variantToString(*it));
		return result;
}

QVariant QtSqlSettingsPrivate::stringListToVariantList(const QStringList &l)
{
		QStringList outStringList = l;
		for (int i = 0; i < outStringList.count(); ++i) {
				const QString &str = outStringList.at(i);

				if (str.startsWith(QLatin1Char('@'))) {
						if (str.length() >= 2 && str.at(1) == QLatin1Char('@')) {
								outStringList[i].remove(0, 1);
						} else {
								QVariantList variantList;
								for (int j = 0; j < l.count(); ++j)
										variantList.append(stringToVariant(l.at(j)));
								return variantList;
						}
				}
		}
		return outStringList;
}

QString QtSqlSettingsPrivate::variantToString(const QVariant &v)
{
		QString result;

		switch (v.type()) {
				case QVariant::Invalid:
						result = QLatin1String("@Invalid()");
						break;

				case QVariant::ByteArray: {
						QByteArray a = v.toByteArray();
						result = QLatin1String("@ByteArray(");
						//result += QString::fromLatin1(a.constData(), a.size());
						// added
						result += a.toHex();
						result += QLatin1Char(')');
						break;
				}

				case QVariant::String:
				case QVariant::LongLong:
				case QVariant::ULongLong:
				case QVariant::Int:
				case QVariant::UInt:
				case QVariant::Bool:
				case QVariant::Double:
				case QVariant::KeySequence: {
						result = v.toString();
						if (result.startsWith(QLatin1Char('@')))
								result.prepend(QLatin1Char('@'));
						break;
				}
#ifndef QT_NO_GEOM_VARIANT
				case QVariant::Rect: {
						QRect r = qvariant_cast<QRect>(v);
						result += QLatin1String("@Rect(");
						result += QString::number(r.x());
						result += QLatin1Char(' ');
						result += QString::number(r.y());
						result += QLatin1Char(' ');
						result += QString::number(r.width());
						result += QLatin1Char(' ');
						result += QString::number(r.height());
						result += QLatin1Char(')');
						break;
				}
				case QVariant::Size: {
						QSize s = qvariant_cast<QSize>(v);
						result += QLatin1String("@Size(");
						result += QString::number(s.width());
						result += QLatin1Char(' ');
						result += QString::number(s.height());
						result += QLatin1Char(')');
						break;
				}
				case QVariant::Point: {
						QPoint p = qvariant_cast<QPoint>(v);
						result += QLatin1String("@Point(");
						result += QString::number(p.x());
						result += QLatin1Char(' ');
						result += QString::number(p.y());
						result += QLatin1Char(')');
						break;
				}
#endif // !QT_NO_GEOM_VARIANT

				default: {
#ifndef QT_NO_DATASTREAM
						QByteArray a;
						{
								QDataStream s(&a, QIODevice::WriteOnly);
								s.setVersion(QDataStream::Qt_4_0);
								s << v;
						}

						result = QLatin1String("@Variant(");
						//result += QString::fromLatin1(a.constData(), a.size());
						// added
						result += a.toHex();
						result += QLatin1Char(')');
#else
						Q_ASSERT(!"QSettings: Cannot save custom types without QDataStream support");
#endif
						break;
				}
		}

		return result;
}

QVariant QtSqlSettingsPrivate::stringToVariant(const QString &s)
{
		if (s.startsWith(QLatin1Char('@'))) {
				if (s.endsWith(QLatin1Char(')'))) {
						if (s.startsWith(QLatin1String("@ByteArray("))) {
							QByteArray a(s.toLatin1().mid(11, s.size() - 12));
							// added
							return QVariant(QByteArray::fromHex(a));
							//return QVariant(s.toLatin1().mid(11, s.size() - 12));
						} else if (s.startsWith(QLatin1String("@Variant("))) {
#ifndef QT_NO_DATASTREAM
							QByteArray a(s.toLatin1().mid(9, s.size() - 10));
							// added
							a = QByteArray::fromHex(a);
							QDataStream stream(&a, QIODevice::ReadOnly);
							stream.setVersion(QDataStream::Qt_4_0);
							QVariant result;
							stream >> result;
							return result;
							//return QByteArray::fromHex(a);
#else
								Q_ASSERT(!"QSettings: Cannot load custom types without QDataStream support");
#endif
#ifndef QT_NO_GEOM_VARIANT
						} else if (s.startsWith(QLatin1String("@Rect("))) {
								QStringList args = QtSqlSettingsPrivate::splitArgs(s, 5);
								if (args.size() == 4)
										return QVariant(QRect(args[0].toInt(), args[1].toInt(), args[2].toInt(), args[3].toInt()));
						} else if (s.startsWith(QLatin1String("@Size("))) {
								QStringList args = QtSqlSettingsPrivate::splitArgs(s, 5);
								if (args.size() == 2)
										return QVariant(QSize(args[0].toInt(), args[1].toInt()));
						} else if (s.startsWith(QLatin1String("@Point("))) {
								QStringList args = QtSqlSettingsPrivate::splitArgs(s, 6);
								if (args.size() == 2)
										return QVariant(QPoint(args[0].toInt(), args[1].toInt()));
#endif
						} else if (s == QLatin1String("@Invalid()")) {
								return QVariant();
						}

				}
				if (s.startsWith(QLatin1String("@@")))
						return QVariant(s.mid(1));
		}

		return QVariant(s);
}

QStringList QtSqlSettingsPrivate::splitArgs(const QString &s, int idx)
{
		int l = s.length();
		Q_ASSERT(l > 0);
		Q_ASSERT(s.at(idx) == QLatin1Char('('));
		Q_ASSERT(s.at(l - 1) == QLatin1Char(')'));

		QStringList result;
		QString item;

		for (++idx; idx < l; ++idx) {
				QChar c = s.at(idx);
				if (c == QLatin1Char(')')) {
						Q_ASSERT(idx == l - 1);
						result.append(item);
				} else if (c == QLatin1Char(' ')) {
						result.append(item);
						item.clear();
				} else {
						item.append(c);
				}
		}

		return result;
}

void QtSqlSettingsPrivate::remove(const QString &key)
{
	QMutexLocker locker(&mutex);

	QString prefix(key + QLatin1Char('/'));

	QtSqlSettings::SettingsMap::Iterator j = map.lowerBound(prefix);
	QtSqlSettings::SettingsMap::Iterator end = map.end();

	while (j != end && j.key().startsWith(prefix))
		j = map.erase(j);

	map.remove(key);
}

void QtSqlSettingsPrivate::set(const QString &key, const QVariant &value)
{
	QMutexLocker locker(&mutex);

	map.insert(key, value);
}


bool QtSqlSettingsPrivate::get(const QString &key, QVariant *value) const
{
	QMutexLocker locker(&mutex);

	QtSqlSettings::SettingsMap::ConstIterator j = map.constFind(key);
	if (j != map.constEnd()) {
		if (value)
			*value = j.value();
		return true;
	}

	return false;
}

QStringList QtSqlSettingsPrivate::children(const QString &prefix, ChildSpec spec) const
{
	QMutexLocker locker(&mutex);

	QMap<QString, QString> result;

	QtSqlSettings::SettingsMap::ConstIterator j;
	QtSqlSettings::SettingsMap::ConstIterator end = map.constEnd();

	int startPos = prefix.size();

	j = map.lowerBound(prefix);

	while (j != end && j.key().startsWith(prefix)) {
		processChild(j.key().mid(startPos), spec, result);
		++j;
	}

	return result.keys();
}

void QtSqlSettingsPrivate::clear()
{
	QMutexLocker locker(&mutex);
	map.clear();
}
