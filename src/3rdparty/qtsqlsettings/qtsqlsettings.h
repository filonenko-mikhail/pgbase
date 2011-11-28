#ifndef QTSQLSETTING_H
#define QTSQLSETTING_H

#if defined(QTSQLSETTINGS_EXPORTS)
#  define QTSQLSETTINGS_EXPORT Q_DECL_EXPORT
#elif defined(QTSQLSETTINGS_IMPORTS)
#  define QTSQLSETTINGS_EXPORT Q_DECL_IMPORT   /**/
#else
#  define QTSQLSETTINGS_EXPORT   /**/
#endif

#include <QtCore/QSettings>


/*!
	The QtSqlSettings class provides persistent platform-independent application settings. Settings are stored in database.
	\warning This class is not a descendant of QSettings, but it provides an identical interface.
	\note All functions in this class are reentrant.
	\warning Method save not called in destructor.

	Using in your project:
	\list
	\li Create qt sql connection, for e.g:
	\code
		QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
		db.setDatabaseName("database");
		if (db.open("admin", "admin")) {

			if (!db.tables().contains("app_settings")) {
				QSqlQuery q = db.exec("CREATE TABLE app_settings "
															"( "
															"	key_column text NOT NULL, "
															"	value_column text, "
															"	CONSTRAINT app_settings_pk PRIMARY KEY (key_column) "
															")");
				if (q.lastError().isValid())
					qDebug() << q.lastError().text();
			}
		}
	\endcode
	\li Create QtSqlSettings object with opened connection name and load settings from table, for e.g:
	\code
		#include <QtSql/QtSqlSettings>
	\endcode
	\code
			QtSqlSettings *settings = new QtSqlSettings(db.connectionName());
			settings->load("app_settings","key_column", "value_column");
	\endcode
	\li Read|write settings as if the object is QSettings, for e.g:
	\code
			settings->setValue("New Value", QStringList() << "First line" << "SecondLine");

			settings->setValue("group1/value1", QVariantList() << 1 << 3);

			settings->setValue("bytearray", QByteArray("bytearray", 9));
	\endcode
	\li Settings saved using method save(), for e.g.:
	\code
	 settings->save();
	\endcode
	\endlist

	\attention Missing methods:
	\li bool	fallbacksEnabled () const
	\li QString	fileName () const
	\li Format	format () const
	\li QTextCodec *	iniCodec () const
	\li bool	isWritable () const
	\li QString	organizationName () const
	\li Scope	scope () const
	\li void	setFallbacksEnabled ( bool b )
	\li void	setIniCodec ( QTextCodec * codec )
	\li void	setIniCodec ( const char * codecName )
	\li Status	status () const
	\li void	sync ()
	\li Format	defaultFormat ()
	\li Format	registerFormat ( const QString & extension, ReadFunc readFunc, WriteFunc writeFunc, Qt::CaseSensitivity, caseSensitivity = Qt::CaseSensitive )
	\li void	setDefaultFormat ( Format format )
	\li void	setPath ( Format format, Scope scope, const QString & path )\li

* @class QtSqlSettings qtsqlsettings.h
*/
class QtSqlSettingsPrivate;
class QTSQLSETTINGS_EXPORT QtSqlSettings : public QObject
{
	Q_OBJECT

	Q_DECLARE_PRIVATE(QtSqlSettings)

	/*!
	\property table
	\sa table(), setTable()
	*/
	Q_PROPERTY(QString table READ table WRITE setTable)
	/*!
	\property connection
	\sa connection(), setConnection()
	*/
	Q_PROPERTY(QString connection READ connection WRITE setConnection)

	/*!
	\property keyColumn
	\sa keyColumn(), setKeyColumn()
	*/
	Q_PROPERTY(QString keyColumn READ keyColumn WRITE setKeyColumn)
	/*!
	\property valueColumn
	\sa valueColumn(), setValueColumn()
	*/
	Q_PROPERTY(QString valueColumn READ valueColumn WRITE setValueColumn)

protected:

	QtSqlSettings(QtSqlSettingsPrivate &dd, QObject* parent);
	QtSqlSettingsPrivate* const d_ptr;

public:

	typedef QMap<QString, QVariant> SettingsMap;

	QtSqlSettings(QObject* parent = 0);
	QtSqlSettings(const QString& connection
		, QObject* parent = 0);
	virtual ~QtSqlSettings();

	/*!
		Table name using to load|save settings
	* @return QString Name of table
	*/
	QString table() const;
	/*!
		Name of qt sql connection using for load|sabe settings
	* @return QString
	*/
	QString connection() const;

	/*!
		Column name used for store setting keys
	* @return QString
	*/
	QString keyColumn() const;
	/*!
		Column name used for store setting values
	* @return QString
	*/
	QString valueColumn() const;

	/*!
		Load settings from table, used keyColumn name for load setting keys and valueColumn for setting values.
	* @param table
	* @param keyColumn
	* @param valueColumn
	* @return bool
	*/
	bool load(const QString& table, const QString& keyColumn, const QString& valueColumn);

	/*!
		Loads the settings using the previously assigned properties
	* @return bool
	*/
	bool load();

	/*!
		Save settings to table, used keyColumn name for save setting keys and valueColumn for setting values.
	* @param table
	* @param keyColumn
	* @param valueColumn
	* @return bool
	*/
	bool save(const QString& table, const QString& keyColumn, const QString& valueColumn);
	/*!
		Save the settings using the previously assigned properties.
	* @return bool
	*/
	bool save();

	QStringList allKeys() const;

	void  beginGroup(const QString& prefix);
	int beginReadArray(const QString& prefix);
	void  beginWriteArray(const QString& prefix, int size = -1);
	QStringList childGroups() const;
	QStringList childKeys() const;
	void  clear();
	bool  contains(const QString& key) const;
	void  endArray();
	void  endGroup();
	QString group() const;
	void  remove(const QString& key);
	void  setArrayIndex(int i);
	void  setValue(const QString& key, const QVariant& value);
	QVariant  value(const QString& key, const QVariant& defaultValue = QVariant()) const;

public slots:
	void setTable(const QString& table);
	void setConnection(const QString& connection);

	void setKeyColumn(const QString& columnName);
	void setValueColumn(const QString& columnName);

private:
};

#endif // QTSQLSETTING_H
