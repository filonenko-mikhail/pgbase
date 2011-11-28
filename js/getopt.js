//You "specify" your options by declaring an properties of JsOptGet object.
JsGetOpt = function() {
  this.argv = [];
};

JsGetOpt.prototype.parse = function(argv) {
  var argument;
  var value;

  var option;
  var splittedArgument;

  var waitFor = 0;
  var waitForArgument = 0;
  var waitForOptionArgument = 1;

  for (var i = 0; i < argv.length; ++i) {
    argument = argv[i];
    // short or long option
    if (argument[0] === "-") {
      // Long option
      if (argument[1] === "-") {
        splittedArgument = argument.split("=", 2);
        option = splittedArgument[0].slice(2);
        if (splittedArgument.length === 1) {
          if (!this[option])
            this[option] = {value:undefined};
          this[option].value = true;
        } else if (splittedArgument.length === 2) {
          if (!this[option])
            this[option] = {value:undefined};
          this[option].value = splittedArgument[1];
        }
      // Short option
      } else {
        option = argument.slice(1,2);
        // option with argument e.g. -cValue
        if (argument.length > 2) {
          if (!this[option])
            this[option] = {value:undefined};
          this[option].value = argument.slice(2);
          // e.g. -c or -c value
        } else {
          if (!this[option]) {
            this[option] = {value:true};
          } else if (this[option].flag !== true) {
            waitFor = waitForOptionArgument;
          } else {
            this[option].value = true;
          }
        }
      }
      // argument, may be for option
    } else {
      switch (waitFor) {
        case waitForOptionArgument:
          this[option].value = argument;
          waitFor = waitForArgument;
        break;
        case waitForArgument:
        default:
          this.argv.push(argument);
        break;
      }
    }
  }
  return true;
};

JsGetOpt.prototype.printUsage = function() {
  var fileInfo = new QFileInfo(QCoreApplication.applicationFilePath());
  var commandLineTemplate = fileInfo.fileName();
  var messagesList = [];
  var optionPrefix;
  var optionType;
  var optionHelp;
  for (var option in this) if (this.hasOwnProperty(option)) {
    if (option === "argv")
      continue;
    optionPrefix = (option.length === 1) ? "-": "--";
    optionType = (!this[option].type)? "<string>" : this[option].type;
    optionHelp = optionPrefix + option;

    if (this[option].required === true) {
      commandLineTemplate += " " + optionPrefix + option;
    } else {
      if (this[option].flag === true) {
        commandLineTemplate += " [," + optionPrefix + option + "]";
      } else {
        optionHelp += " " + optionType;
        commandLineTemplate += " [," + optionPrefix + option + " " + optionType + "]";
      }
    }

    if (this[option].help !== undefined) {
      optionHelp += "\t-\t" + this[option].help;
    }

    if (this[option].value !== undefined) {
      optionHelp += " " + qsTr("Default:") + " " + this[option].value;
    }

    // option have order
    if (this[option].order === undefined)
      messagesList.push(optionHelp);
    // option dont have order
    else
      messagesList[this[option].order] = optionHelp;
  }
  print(commandLineTemplate);
  messagesList.reverse();
  while(messagesList.length) {
    print(messagesList.pop());
    print("\n");
  }
};
