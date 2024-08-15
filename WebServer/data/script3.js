var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

// When websocket is established, call the getReadings() function
function onOpen(event) {
    console.log('Connection opened');
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    console.log(event.data);
    if(event.data == "RELAY1_ON" || event.data == "RELAY1_OFF") {
        var state = (event.data == "RELAY1_ON") ? "Đang hoạt động" : "Đang tắt";
        document.getElementById("state1").innerHTML = state;
        document.getElementById("state1").style.color = (state == "Đang hoạt động") ? "blue" : "red";
    }
    else if(event.data == "RELAY2_ON" || event.data == "RELAY2_OFF") {
        var state = (event.data == "RELAY2_ON")? "Đang hoạt động" : "Đang tắt";
        document.getElementById("state2").innerHTML = state;
        document.getElementById("state2").style.color = (state == "Đang hoạt động") ? "blue" : "red";
    }
    else if(event.data == "RELAY3_ON" || event.data == "RELAY3_OFF") {
        var state = (event.data == "RELAY3_ON")? "Đang hoạt động" : "Đang tắt";
        document.getElementById("state3").innerHTML = state;
        document.getElementById("state3").style.color = (state == "Đang hoạt động") ? "blue" : "red";
    }
    else if(event.data == "RELAY4_ON" || event.data == "RELAY4_OFF") {
        var state = (event.data == "RELAY4_ON")? "Đang hoạt động" : "Đang tắt";
        document.getElementById("state4").innerHTML = state;
        document.getElementById("state4").style.color = (state == "Đang hoạt động") ? "blue" : "red";
    }
    else if(event.data == "RELAY5_ON" || event.data == "RELAY5_OFF") {
        var state = (event.data == "RELAY5_ON")? "Đang hoạt động" : "Đang tắt";
        document.getElementById("state5").innerHTML = state;
        document.getElementById("state5").style.color = (state == "Đang hoạt động") ? "blue" : "red";
    }
    else if(event.data == "RELAY6_ON" || event.data == "RELAY6_OFF") {
        var state = (event.data == "RELAY6_ON")? "Đang hoạt động" : "Đang tắt";
        document.getElementById("state6").innerHTML = state;
        document.getElementById("state6").style.color = (state == "Đang hoạt động") ? "blue" : "red";
    }
    else{
        var message = JSON.parse(event.data);
        if (message.relayTimes) {
            for (var i = 0; i < message.relayTimes.length; i++) {
                var elapsed = message.relayTimes[i];
                var hours = Math.round(elapsed / 3600000);
                var minutes = Math.round((elapsed % 3600000) / 60000);
                document.getElementById("uptime" + (i + 1)).innerHTML = hours + " giờ " + minutes + " phút";
            }
        }
    }
}