// Extensions
importExtension("qt.core");
importExtension("qt.gui");
importExtension("qt.sql");
importExtension("qt.uitools");

importExtension("com.asvil");
importExtension("org.log4qt");

// Includes
include("application.js");
include("initialization.js");
include("saverestoreutils.js");
include("qsmainwindow.js");
include("getopt.js");
include("sqlutils.js");

include("widgets/formwidget.js");

//templates
//include("template/tableobjecttemplate.js");
// 3rd party scripts
include("json2/json2.js");


function print(message)
{
  if (typeof message != "undefined")
    Log4Qt_Logger.logger("Script").info(message);
}

// function alert([title,] message)
function alert()
{
  var title = qsTr("Script alert");
  var message;
  if (arguments.length = 1) {
      message = arguments[0];
  } else if (arguments.length = 2) {
    title = arguments[0];
    message = arguments[1];
  }
  QMessageBox.information(QApplication.activeWindow(), title, message);
  print(title + ". " + message);
}

// Function main called with one parameter, which contains string list of parameters
function main (argv) {
  // If argv contains --debug-script-engine option interrupt execution
  for (var i = 0; i < argv.length; ++i)
    if (argv[i] === "--debug-script-engine") {
    // Leave this option, that it placed in opts variable
    debugger;
    break;
  }

  // First of all load script translation file
  var trFileDelimiter = "_";
  var localeName = QLocale.system().name();
  var trFileName = "script" + trFileDelimiter + localeName;
  var tsDirInfo = new QDir("js:/translations");
  print("Load translation file for script framework...");
  var scriptTranslator = new QTranslator(application);
  if (!scriptTranslator.load(trFileName, tsDirInfo.absolutePath()))
    print("Not loaded");
  else
    print("Loaded");

  QCoreApplication.installTranslator(scriptTranslator);

  // Getting command line options
  opts = new JsGetOpt();

  opts["connection-name"] = { value : application.applicationName + "_sql_connection",
                              help : qsTr("Qt Sql database connection name")
                              , type : "<string>"};
  opts["driver-name"] = { value : "QPSQL"
                          , help : qsTr("Qt Sql driver name. Available drivers:") + QSqlDatabase.drivers()
                          , type : "<string>"};
  opts["h"] = { value : system.env.PGHOST?system.env.PGHOST:"localhost",
                help : qsTr("Host name")
                , type : "<string>"};
  opts["host"] = opts["h"];
  opts["p"] = { value : system.env.PGPORT?system.env.PGPORT:5432,
                help: qsTr("Port")
                  , type : "<int>"};
  opts["port"] = opts["p"];
  opts["d"] = { value : system.env.PGDATABASE?system.env.PGDATABASE:application.applicationName,
                help : qsTr("Database name")
                , type : "<string>"};
  opts["database"] = opts["d"];
  opts["s"] = { value : system.env.PGDATABASE?system.env.PGDATABASE:application.applicationName,
                help : qsTr("Database schema")
                , type : "<string>"};
  opts["schema"] = opts["s"];
  opts["U"] = { value : system.env.PGUSER?system.env.PGUSER:getLogin(),
                help: qsTr("Database user name")
                  , type : "<string>"};
  opts["username"] = opts["U"];
  opts["w"] = { value : false,
                help: qsTr("Never promt for password"),
                flag: true};
  opts["no-password"] = opts["w"];
  opts["W"] = { value : true,
                help: qsTr("Force password promt"),
                flag: true};
  opts["password"] = opts["W"];
  opts["G"] = { value : "",
                help: qsTr("User name")
                  , type : "<string>"};
  opts["app-username"] = opts["G"];
  opts["help"] = { flag : true, value : false};
  opts["debug-script-engine"] = { flag : true, value : false,
                                  help : qsTr("Interrupt on start script engine")};


  //QMessageBox.information(0, "", getLogin());
  if (!opts.parse(argv)) {
    print(qsTr("Error while parse command line. Exit."));
    print(qsTr("Usage:"));
    opts.printUsage();
    return 1;
  }
  if (opts["help"].value) {
    print(qsTr("Usage:"));
    opts.printUsage();
    return 1;
  }

  if (opts.w.value)
    opts.W.value = false;

  if (opts["G"].value.length == 0) {
    opts["G"].value = opts["U"].value;
  }
  var password = "";
  if (opts.W.value) {
    var ok = false;
    password = QInputDialog.getText(0, qsTr("Enter password")
                                    , qsTr("Password:")
                                    , QLineEdit.Password
                                    , "", ok);
  }

  print(qsTr("Connecting to database (host: %1 user: %2 database: %3)...")
        .arg(opts.host.value).arg(opts.U.value).arg(opts.d.value));

  database = QSqlDatabase.addDatabase(opts["driver-name"].value, opts["connection-name"].value);
  database.setHostName(opts["host"].value);
  database.setDatabaseName(opts["database"].value);
  database.setPort(opts["port"].value);
  if (!database.open(opts["username"].value, password)) {
    print(qsTr("Could not connect"));
    print(database.lastError().text());
    QMessageBox.critical.call(this, null, qsTranslate("main","Error")
                              , qsTranslate("main", "Could not connect to database\nServer answer:\n")
                              + database.lastError().text());
    return 1;
  }
  print(qsTr("Successfully connected"));

  database.settings = new QSettings(QSettings.defaultFormat, QSettings.UserScope, application.organizationName
                                    , database.connectionName(), application);

  if (!PQsetNoticeReceiver(database.connectionName(), noticeReceiver))
      print(qsTr("Unable to set postgresql notice receiver"));

  var query = new QSqlQuery("SET search_path=" + "\"$user\"," + opts["schema"].value + ",public", database);
  if (query.lastError().isValid())
    QMessageBox.warning(0, "Error", query.lastError().text());

  window = new QsMainWindow;
  window.show();

  // ---------------------------------------------------------
  if (opts["schema"].value == "jockey")
    window.windowIcon = new QIcon("images:32x32/application/red_elephant.ico");

  // Load configuration
  configuration = include("js:configuration.js");
  configuration.up();

  include("js:users/%1.js".arg(opts["app-username"].value));

  application.aboutToQuit.connect(applicationAboutToQuit);

  //!\retval 0 It indicate that main was succesfully executed, and QApplication::exec() required.
  return 0;
}

noticeReceiver = function(severity, primary, detail, hint) {
  message = primary;
  if (detail != "")
    message += "\n" + detail;
  if (hint != "")
    message += "\n" + detail;
  if (severity == "WARNING") {
    Log4Qt_Logger.logger("Script").warn(message);
  } else if (severity == "NOTICE") {
    Log4Qt_Logger.logger("Script").info(message);
  } else if (severity == "INFO") {
    Log4Qt_Logger.logger("Script").info(message);
  } else if (severity == "LOG") {
    Log4Qt_Logger.logger("Script").info(message);
  }
};

function applicationAboutToQuit()
{
  this.configuration.down();
  // it's a really problem to remove database
  // QSqlDatabasePrivate::removeDatabase: connection 'pgBase_sql_connection' is still in use, all queries will cease to work.
  database.close();
  QSqlDatabase.removeDatabase(database);
}
