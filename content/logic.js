//server_ip = "http://192.168.3.162"
server_ip = ""

function init() {
    createSegments();
    setInterval(function () {
        getData();
    }, 500);
}

function getData() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            const data = JSON.parse(this.responseText);
            setDisplayValue(displayActVoltage, data.voltage);
            setDisplayValue(displayActCurrent, data.current);
            setDisplayValue(displayActPower, data.power);
            document.getElementById("on-button").classList.remove("my-active-button");
            document.getElementById("off-button").classList.remove("my-active-button");
            if (data.output) {
                document.getElementById("on-button").classList.add("my-active-button");
            } else {
                document.getElementById("off-button").classList.add("my-active-button");
            }
            setDisplayValue(displayTargetVoltage, data.tvoltage);
            setDisplayValue(displayTargetCurrent, data.tcurrent);
            setDisplayValue(displayTargetPower, data.tpower);
            setDisplayValue(displayInputVoltage, data.ivoltage);
            item = document.querySelector("[id='connection-state']");
            if (data.connected) {
                item.style.display = "inline";
            } else {
                item.style.display = "none";
            }

        }
    };
    xhttp.open("GET", server_ip + "/control", true);
    xhttp.send();
}


function getConfig() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            console.log(this.responseText);
            const cfg = JSON.parse(this.responseText);
            console.log(cfg);
            inp = document.querySelector("[name='ssid']");
            inp.value = cfg.ssid;

            inp = document.querySelector("[name='use-static-ip']");
            inp.checked = cfg["use-static-ip"];
            inp = document.querySelector("[name='static-ip']");
            inp.value = cfg["static-ip"];
            inp = document.querySelector("[name='subnet']");
            inp.value = cfg["subnet"];
            inp = document.querySelector("[name='gateway']");
            inp.value = cfg["gateway"];

            inp = document.querySelector("[name='mqtt-server']");
            inp.value = cfg["mqtt-server"];
            inp = document.querySelector("[name='mqtt-port']");
            inp.value = cfg["mqtt-port"];
            inp = document.querySelector("[name='mqtt-user']");
            inp.value = cfg["mqtt-user"];
            inp = document.querySelector("[name='mqtt-id']");
            inp.value = cfg["mqtt-id"];

            inp = document.querySelector("[name='zero-feed-in']");
            inp.checked = cfg["zero-feed-in"];
            inp = document.querySelector("[name='smi-topic']");
            inp.value = cfg["smi-topic"];
            inp = document.querySelector("[name='sm-name']");
            inp.value = cfg["sm-name"];

            inp = document.querySelector("[name='enable-input-limits']");
            inp.checked = cfg["enable-input-limits"];
            inp = document.querySelector("[name='switch-off-voltage']");
            inp.value = cfg["switch-off-voltage"];
            inp = document.querySelector("[name='switch-on-voltage']");
            inp.value = cfg["switch-on-voltage"];
        }
    };
    xhttp.open("GET", server_ip + "/config", true);
    xhttp.send();
}

function applySettings(reset) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            if (this.responseText != "OK") {
                alert("Applying settings failed!");
            }
        }
    };
    xhttp.open("POST", server_ip + "/config", true);
    cfg = {};
    inp = document.querySelector("[name='ssid']");
    cfg["ssid"] = inp.value;
    inp = document.querySelector("[name='wifi-pass']");
    cfg["wifi-password"] = inp.value;
    inp = document.querySelector("[name='use-static-ip']");
    cfg["use-static-ip"] = inp.checked;
    inp = document.querySelector("[name='static-ip']");
    cfg["static-ip"] = inp.value;
    inp = document.querySelector("[name='subnet']");
    cfg["subnet"] = inp.value;
    inp = document.querySelector("[name='gateway']");
    cfg["gateway"] = inp.value;

    inp = document.querySelector("[name='mqtt-server']");
    cfg["mqtt-server"] = inp.value;
    inp = document.querySelector("[name='mqtt-port']");
    cfg["mqtt-port"] = parseInt(inp.value);
    inp = document.querySelector("[name='mqtt-user']");
    cfg["mqtt-user"] = inp.value;
    inp = document.querySelector("[name='mqtt-pass']");
    cfg["mqtt-pass"] = inp.value;
    inp = document.querySelector("[name='mqtt-id']");
    cfg["mqtt-id"] = inp.value;

    inp = document.querySelector("[name='zero-feed-in']");
    cfg["zero-feed-in"] = inp.checked;
    inp = document.querySelector("[name='smi-topic']");
    cfg["smi-topic"] = inp.value;
    inp = document.querySelector("[name='sm-name']");
    cfg["sm-name"] = inp.value;

    inp = document.querySelector("[name='enable-input-limits']");
    cfg["enable-input-limits"] = inp.checked;
    inp = document.querySelector("[name='switch-off-voltage']");
    cfg["switch-off-voltage"] = inp.value;
    inp = document.querySelector("[name='switch-on-voltage']");
    cfg["switch-on-voltage"] = inp.value;

    data = JSON.stringify(cfg);
    console.log(data);
    xhttp.send(data);
}

function goBack() {
    window.location.href = 'index.html';
}

function goToSettings() {
    window.location.href = 'settings.html';
}

function resetEsp() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("POST", server_ip + "/control?reset", true);
    xhttp.send();
    goBack();
}

function setOutput(state) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            if (this.responseText != "OK") {
                alert("Set output failed!");
            }
        }
    };
    xhttp.open("POST", server_ip + "/control?output=" + state, true);
    xhttp.send();
}

function setTargetValue(id) {
    value = prompt("Please enter target value...");
    if (value != null && parseFloat(value) != NaN) {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                if (this.responseText != "OK") {
                    alert("Set parameter failed!");
                }
            }
        };
        var param = "";
        if (id == "set-voltage-button") {
            param = "voltage";
        } else if (id == "set-current-button") {
            param = "current";
        } if (id == "set-power-button") {
            param = "max-power";
        }
        value = parseFloat(value);
        xhttp.open("POST", server_ip + "/control?" + param + "=" + value, true);
        xhttp.send();
    }
}

function createSegment(display) {
    display.pattern = "###.##";
    display.displayAngle = 1.5;
    display.digitHeight = 21;
    display.digitWidth = 14;
    display.digitDistance = 3.1;
    display.segmentWidth = 2.9;
    display.segmentDistance = 0.9;
    display.segmentCount = 7;
    display.cornerType = 3;
    display.colorOn = "#f0f0f0";
    display.colorOff = "#3b3b3b";
    display.draw();
    display.setValue('  0.00');
}

function createSegments() {
    //actual
    displayActVoltage = new SegmentDisplay("actVoltage");
    createSegment(displayActVoltage);
    displayActVoltage.colorOn = "#a0a0ff";

    displayActCurrent = new SegmentDisplay("actCurrent");
    createSegment(displayActCurrent);
    displayActCurrent.colorOn = "#ffa0a0";

    displayActPower = new SegmentDisplay("actPower");
    createSegment(displayActPower);
    displayActPower.colorOn = "#a0ffa0";
    displayActPower.pattern = "####.#";
    displayActPower.setValue('   0.0');

    displayInputVoltage = new SegmentDisplay("inputVoltage");
    createSegment(displayInputVoltage);
    displayInputVoltage.colorOn = "#ffffa0";
    setDisplayValue(displayInputVoltage, 0);

    //target
    displayTargetVoltage = new SegmentDisplay("targetVoltage");
    createSegment(displayTargetVoltage);
    displayTargetCurrent = new SegmentDisplay("targetCurrent");
    createSegment(displayTargetCurrent);
    displayTargetPower = new SegmentDisplay("targetPower");
    createSegment(displayTargetPower);
    displayTargetPower.pattern = "####.#";
    displayTargetPower.setValue('   0.0');
}

function setDisplayValue(display, value) {
    var pattern_words = display.pattern.split('.');
    var total_len = display.pattern.length;
    var post_len = pattern_words[1].length;
    var value_words = String(value).split('.');
    var post_word = '';
    if (value_words.length == 2) {
        post_word = value_words[1];
    }
    post_word = post_word.padEnd(post_len, '0');
    display.setValue(value_words[0].padStart(total_len - post_len - 1) + '.' + post_word);
}

