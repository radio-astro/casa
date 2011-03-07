var repopulate = function() {
    unitswitch(document.getElementById('cunit').value);
    baselineswitch();
    checkbuttons("cdoppler",["doppler0","doppler1"]);
    checkbuttons("cframe",["frame0","frame1"]);
    selectFiles();
    selectIFs();
    selectSources();
    var sumtext = document.getElementById('sum').value;
    if (sumtext == "") {
        document.getElementById('summary').style.display = 'none';
    } else {
        document.getElementById('summary').style.display = 'inline';
    }
    var csource = document.getElementById('csource').value;
    if (csource == "") {
        document.getElementById('sourcelbl').style.display = 'none';
    } else {
        document.getElementById('sourcelbl').style.display = 'inline';
    }
}

function summaryPopUp() {
    var sumtext = document.getElementById('sum').value;
    if (sumtext == "") {return false;}
    TheNewWin = window.open('','name','height=500,width=600,toolbar=no,directories=no,status=no,menubar=no');
    TheNewWin.document.write('<!DOCTYPE html PUBLIC"-//W3C//DTD XHTML 1.0 Transitional//EN" "http:\/\/www.w3.org\/TR\/xhtml1\/DTD\/xhtml1-transitional.dtd"><html xmlns="http:\/\/www.w3.org\/1999\/xhtml">');
    TheNewWin.document.write('<head><title>Popup<\/title><\/head><body style="overflow:hidden" bgcolor="#ffffff"><pre>');
    TheNewWin.document.write(sumtext);
    TheNewWin.document.write('</pre><hr \/> <p align="right"><a href="#" onclick="self.close();return false;">Close');
    TheNewWin.document.write(' Window<\/a><\/p> <\/body><\/html>');
    return false;
}

function checkbuttons(hiddenfields, fields){
    var theval = document.getElementById(hiddenfields).value;
    for (i=0; i<fields.length;++i ){
        var elem = document.getElementById(fields[i]);
        if ( elem.value == theval ) {
            elem.checked = 'checked';
        }
    }
}

function selectSources() {
    var sl = document.getElementById("sourcelist");
    var cs = document.getElementById("csource").value;
    var opts = sl.options;
    for (k=0; k<opts.length;++k) {
        if (opts[k].value == cs ) {
            opts[k].selected = 'selected';
        }
    }
    if (cs) {
        var sourcediv = document.getElementById("sourcelbl");
        sourcediv.style.display = "inline";
    }
}

function selectFiles() {
    var indeces = eval(document.getElementById("cfilesel").value);
    var fl = document.getElementById("filelist");
    var opts = fl.options;
    for (k=0; k<indeces.length;++k) {
        opts[indeces[k]].selected = 'selected';
    }
}

function selectIFs() {
    nif = eval(document.getElementById("nif").value);
    for (i=0; i<nif.length;++i ) {
        p0 = "cif"+nif[i]
        p1 = "rest"+nif[i]
        var ifsel = eval(document.getElementById(p0).value);
        var f = document.getElementById(p1);
        f.options[ifsel].selected = 'selected';
    }
}

function baselineswitch() {
    var fields = ["polyorder","baselinerange"];
    var elem = document.getElementById("baseline");
    for (i=0; i<fields.length; ++i) {
        var cont = document.getElementById(fields[i]);
        if (elem.checked) {
            elem.value = 1;
            cont.style.display = "block";
        } else {
            cont.style.display = "none";
            elem.value = 0;
        }
    }
}

function clearunit() {
    var fields = ["plotwindow","brangewindow"];
    for (i=0; i<fields.length; ++i) {
        var cont = document.getElementById(fields[i]);
        cont.value = "";
    }
}

function unitswitch(unitval) {
    var fields = ["velframe","veldoppler","velrest"];
    for (i=0; i<fields.length; ++i) {
        var cont = document.getElementById(fields[i]);
        if (unitval == "channel") {
            cont.style.display = "none";
        } else {
            if (unitval == "GHz") {
             if (fields[i] == "velrest" || fields[i] == "veldoppler") cont.style.display = "none";
             else cont.style.display = "block";
            } else {
                cont.style.display = "block";
            }
        }
    }
    var ulblcont = document.getElementById("prangeunit");
    ulblcont.innerHTML = "";
    ulblcont.innerHTML = unitval;
    var ulblcont = document.getElementById("brangeunit");
    ulblcont.innerHTML = "";
    ulblcont.innerHTML = unitval;
    if ( unitval != document.getElementById('cunit').value ) {
        clearunit();
    }
}


function listFiles(name, serviceURL) {
        var callback = (arguments.length == 3 ? arguments[2] : null);
        var request = new Ajax.Request(serviceURL, {
                parameters: "path=" + name,
                onComplete: function(transport) {
                        var resolutions = processResolutions(transport.responseXML.documentElement);
                        if (callback != null) {
                                callback.process(resolutions);
                        } else {
                                return resolutions;
                        }
                }
        });
}

function processResolutions(result) {
    //var results = result.getElementsByTagName("Listing");

    var fnames = result.getElementsByTagName("File");
    var files = [];
    if (fnames.length > 0 ) {
        for (i=0;i<fnames.length;++i) {
            files.push(fnames[i].firstChild.nodeValue)
        }
    }
    return files;
}


function insertFields() {
    var opts = document.getElementById("directory");
    var path = opts.selectedIndex;
    scripturl = new String(window.document.location);
    scripturl = scripturl.replace(/\w+\.py.*$/gi,"filelist.py");
    listFiles(path,scripturl, callbackHandler);
    return;
}


function invalidateSources() {
    var opts = document.getElementById("sourcelist");
    var fileopt = opts.options;
    //clear
    for (i=0;i<fileopt.length;++i) {
        fileopt[i] = null;
    }
    fileopt.length = 0;
    var sourcediv = document.getElementById("sourcelbl");
    sourcediv.style.display = "none";
}

function invalidateFiles() {
    insertFields();
    invalidateSources();
}

var callbackHandler = {
    process: function(parm) {
        var opts = document.getElementById("filelist");
        var fileopt = opts.options;
        //clear
        for (i=0;i<fileopt.length;++i) {
            fileopt[i] = null;
        }
        fileopt.length = 0;
        for (i=0;i<parm.length;++i) {
            var opt = document.createElement("option");
            opt.text = parm[i];
            opt.value = i;
            fileopt.add(opt);
        }
        // last file is selected
        if (fileopt.length > 0)
            opt.selected = fileopt.length-1;
    }
}
