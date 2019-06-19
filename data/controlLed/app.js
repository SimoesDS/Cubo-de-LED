var connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);

var btnRed = document.getElementById('tggRed');
var btnGreen = document.getElementById('tggGreen');
var btnBlue = document.getElementById('tggBlue');

btnRed.onchange = () => {
    connection.send(`CL:LR:${btnRed.checked ? 1 : 0}`);
};

btnGreen.onchange = () => {
    connection.send(`CL:LR:${btnGreen.checked ? 1 : 0}`);
};

btnBlue.onchange = () => {
    connection.send(`CL:LR:${btnBlue.checked ? 1 : 0}`);
};