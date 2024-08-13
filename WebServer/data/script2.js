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

function toggleRelay(checkbox) {
    var relayId = checkbox.id;
    if(relayId == 1){
        websocket.send("RELAY1");
    }
    else if(relayId == 2){
        websocket.send("RELAY2");
    }
    else if(relayId == 3){
        websocket.send("RELAY3");
    }
    else if(relayId == 4){
        websocket.send("RELAY4");
    }
    else if(relayId == 5){
        websocket.send("RELAY5");
    }
    else if(relayId == 6){
        websocket.send("RELAY6");
    }
}

function onMessage(event) {
    console.log(event.data);
    if(event.data == "RELAY1_ON" || event.data == "RELAY1_OFF") {
        var state = (event.data == "RELAY1_ON") ? "ON" : "OFF";
        document.getElementById("state1").innerHTML = state;
        document.getElementById("state1").style.color = (state == "ON") ? "blue" : "red";
        document.getElementById("1").checked = (state == "ON") ? true : false;
    }
    else if(event.data == "RELAY2_ON" || event.data == "RELAY2_OFF") {
        var state = (event.data == "RELAY2_ON")? "ON" : "OFF";
        document.getElementById("state2").innerHTML = state;
        document.getElementById("state2").style.color = (state == "ON") ? "blue" : "red";
        document.getElementById("2").checked = (state == "ON") ? true : false;
    }
    else if(event.data == "RELAY3_ON" || event.data == "RELAY3_OFF") {
        var state = (event.data == "RELAY3_ON")? "ON" : "OFF";
        document.getElementById("state3").innerHTML = state;
        document.getElementById("state3").style.color = (state == "ON") ? "blue" : "red";
        document.getElementById("3").checked = (state == "ON") ? true : false;
    }
    else if(event.data == "RELAY4_ON" || event.data == "RELAY4_OFF") {
        var state = (event.data == "RELAY4_ON")? "ON" : "OFF";
        document.getElementById("state4").innerHTML = state;
        document.getElementById("state4").style.color = (state == "ON") ? "blue" : "red";
        document.getElementById("4").checked = (state == "ON") ? true : false;
    }
    else if(event.data == "RELAY5_ON" || event.data == "RELAY5_OFF") {
        var state = (event.data == "RELAY5_ON")? "ON" : "OFF";
        document.getElementById("state5").innerHTML = state;
        document.getElementById("state5").style.color = (state == "ON") ? "blue" : "red";
        document.getElementById("5").checked = (state == "ON") ? true : false;
    }
    else if(event.data == "RELAY6_ON" || event.data == "RELAY6_OFF") {
        var state = (event.data == "RELAY6_ON")? "ON" : "OFF";
        document.getElementById("state6").innerHTML = state;
        document.getElementById("state6").style.color = (state == "ON") ? "blue" : "red";
        document.getElementById("6").checked = (state == "ON") ? true : false;
    }
}