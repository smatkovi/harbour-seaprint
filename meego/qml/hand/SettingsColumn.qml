import QtQuick 1.1
import seaprint.ippprinter 1.0
import seaprint.ippmsg 1.0

// The column of print settings, and the job attributes they build up.
//
// The one change from upstream is how jobParams is written. QtQuick 1.1 has no
// `property var`: a `property variant` converts what is put into it and hands
// out a copy on every read, so upstream's jobParams[name] = ... would modify a
// throwaway object and the setting would be lost. Every write therefore takes
// a copy, changes it, and assigns it back -- which is also why the writes are
// gathered in these functions instead of being spread over the settings.
Column {
    id: settingsColumn

    property variant printer
    property variant jobParams: ({})
    property string selectedFile
    property string selectedFileType: Mimer.get_type(selectedFile)

    function isValid(name) {
        return printer.attrs.hasOwnProperty(name+"-supported");
    }

    function setInitialChoice(setting) {
        if(setting.valid)
        {
            var params = jobParams;
            if(setting.subkey == "")
            {
                if(params.hasOwnProperty(setting.name))
                {
                    setting.choice = params[setting.name].value;
                }
            }
            else
            {
                if(params.hasOwnProperty(setting.subkey) && params[setting.subkey].value.hasOwnProperty(setting.name))
                {
                    setting.choice = params[setting.subkey].value[setting.name].value;
                }
            }
        }
        else
        { // Clear jobParams of invalid settings
            var params = jobParams;
            delete params[setting.name];
            jobParams = params;
        }
    }

    function getChoices(name) {
        return isValid(name) ? printer.attrs[name+"-supported"].value : [];
    }

    function getDefaultChoice(name) {
        return printer.attrs.hasOwnProperty(name+"-default") ? printer.attrs[name+"-default"].value : undefined;
    }

    function choiceMade(setting)
    {
        var params = jobParams;

        if(setting.subkey == "")
        {
            if(setting.choice != undefined)
            {
                params[setting.name] = {tag: setting.tag, value: setting.choice};
            }
            else
            {
                delete params[setting.name];
            }
        }
        else
        {
            var tmpObj = Object();
            if(params.hasOwnProperty(setting.subkey))
            {
                tmpObj = params[setting.subkey].value;
            }

            if(setting.choice != undefined)
            {
                tmpObj[setting.name] = {tag: setting.tag, value: setting.choice};
            }
            else
            {
                delete tmpObj[setting.name];
            }

            if(Object.keys(tmpObj).length != 0)
            {
                params[setting.subkey] = {tag: IppMsg.BeginCollection, value: tmpObj};
            }
            else
            {
                delete params[setting.subkey];
            }
        }

        jobParams = params;
    }
}
