configuration.forms.mainform.showForms = ["waybills", "groups", "planning_groups",
                                                  , "searchcursant"
                                                  , "masters", "theor_waybills"
                                                  , "cursant_statistics"];

configuration.forms.mainform.showTables = ["catalog_cars", "catalog_violations", "catalog_masters", "journal_currency_exchange"
                                           , "catalog_group_forms", "catalog_exersizes", "journal_fuel_costs"];

// -------------------------------------------
// Создаем главное меню
configuration.activateObject(configuration.forms.mainform);

configuration.activateObject(configuration.forms.groups);
configuration.activateObject(configuration.forms.waybills);
