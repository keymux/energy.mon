let path = require('path');
let config = require(path.join(__dirname, 'config', 'config.js'));
let SerialPort = require('serialport');

let messages = [];

let port = new SerialPort('/dev/ttyACM0', {
	baudRate: 9600
});

port.on('error', (err) => {
	console.log('Error: ', err.message);
});

let data = '';

port.on('data', (chunk) => {
	data += chunk;
});

function parseMessages() {
	let jsonStack = 0;

	for (let i = 0; i < data.length; ++i) {
		if (data[i] == '{') {
			++jsonStack;
		} else if (data[i] == '}') {
			--jsonStack;
			if (jsonStack === 0) {
				messages.push(JSON.parse(data.substr(0, i + 1)), null, 2);
				data = data.substr(i + 1);
				i = data.length;
			}
		}
	}
}

function processMessages() {
	let message = messages.shift();

	if (message != null) {
		if (message['disableInstructions'] !== undefined) {

		}
	}
}

setInterval(() => {
	parseMessages();
	processMessages();
}, 100);
