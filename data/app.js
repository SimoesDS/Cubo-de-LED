var connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);
var formNetwork = document.getElementById('formNetwork');

formNetwork.addEventListener('submit', function(e){
    e.preventDefault(); // impede o envio do form

    let inputNameNetwork = document.getElementById('inputNameNetwork');
    let inputPassNetwork = document.getElementById('inputPassNetwork');

    sendNameNetwork(inputNameNetwork.value);
    sendPassNetwork(inputPassNetwork.value);
})

function sendNameNetwork(msg) { connection.send('NNW:' + msg) }

function sendPassNetwork(msg) { connection.send('PNW:' + msg) }


/*
var connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);

var statesAzul = ['ligar_azul', 'desligar_azul'];
var stsBtnAzul = statesAzul[0];

connection.onopen = function () {
  connection.send('Connect ' + new Date());
};
connection.onerror = function (error) {
  console.log('WebSocket Error ', error);
};
connection.onmessage = function (e) {
  console.log('Server: ', e.data);
};
connection.onclose = function () {
  console.log('WebSocket connection closed');
};

function btnBlue() { connection.send('B'); }

function btnRed() { connection.send('R'); }

function btnGreen() { connection.send('G'); }

function btnTest() { connection.send('T'); }
*/