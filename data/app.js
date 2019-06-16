var connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);
var formNetwork = document.getElementById('formNetwork');

formNetwork.addEventListener('submit', function(e){
    e.preventDefault(); // impede o envio do form

    let inputNameNetwork = document.getElementById('inputNameNetwork');
    let inputPassNetwork = document.getElementById('inputPassNetwork');

    connection.send(`CN:NN:${inputNameNetwork.value},PN:${inputPassNetwork.value}`);
})