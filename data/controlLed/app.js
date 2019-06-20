var connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);

var btnRed = document.getElementById('tggRed');
var btnGreen = document.getElementById('tggGreen');
var btnBlue = document.getElementById('tggBlue');
var btnEffect = document.getElementById('tggEffect');

btnRed.onchange = () => {
    connection.send(`CL:LR:${btnRed.checked ? 1 : 0}`);
};

btnGreen.onchange = () => {
    connection.send(`CL:LG:${btnGreen.checked ? 1 : 0}`);
};

btnBlue.onchange = () => {
    connection.send(`CL:LB:${btnBlue.checked ? 1 : 0}`);
};

btnEffect.onchange = () => {
    btnRed.checked = !btnEffect.checked;
    connection.send(`CL:LR:${!btnEffect.checked ? 1 : 0}`);

    btnGreen.checked = !btnEffect.checked;
    connection.send(`CL:LG:${!btnEffect.checked ? 1 : 0}`);

    btnBlue.checked = !btnEffect.checked;
    connection.send(`CL:LB:${!btnEffect.checked ? 1 : 0}`);

    btnRed.disabled = !btnEffect.checked;
    btnGreen.disabled = !btnEffect.checked;
    btnBlue.disabled = !btnEffect.checked;

    connection.send(`CL:LE:${btnEffect.checked ? 1 : 0}`);
};