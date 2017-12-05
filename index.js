require('console-stamp')(console, 'yyyy.mm.dd HH:MM:ss.l');

let path = require('path');

let config = require(path.join(__dirname, 'config', 'config.js'));
let Math = require('mathjs');
let mysql = require('mysql');
let SerialPort = require('serialport');

let cts = [
	'ct1',
	'ct2',
	'ct3',
	'ct4',
	];

let datafields = [
	'realPower',
	'Irms',
	'Vrms',
];

let connection = mysql.createConnection({
	host: config.db.host,
	user: config.db.username,
	password: config.db.password,
	database: config.db.database,
	port: config.db.port,
});

connection.connect((err) => {
	if (err) {
		console.error('error connecting: ' + err.stack);
	} else {
		console.log('connected as id ' + connection.threadId);
	}
});

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

function initCts() {
	config.ct.forEach((ct) => {
		port.write('|' + ct + '|T', (err) => {
			if (err) {
				console.log('Error: ', err.message);
			}
		});
	});
}

function parseMessages() {
	let jsonStack = 0;
	let start = -1;

	for (let i = 0; i < data.length; ++i) {
		if (data[i] == '{') {
			++jsonStack;
			if (start == -1) {
				start = i;
			}
		} else if (data[i] == '}') {
			if (jsonStack < 0) {
				data = data.substr(i + 1);
				i = data.length;
			} else {
				--jsonStack;
				if (jsonStack === 0) {
					try {
						let message = JSON.parse(data.substr(start, i + 1));
						messages.push(message);
					} catch (e) {
						console.error(e);
					}
					data = data.substr(i + 1);
					i = data.length;
				}
			}
		}
	}
}

function processMessages() {
	let message = messages.shift();

	if (message != null) {
		if (message['disableInstructions'] !== undefined) {
			initCts();
		} else {
			processCt(message, 'ct1');
			processCt(message, 'ct2');
			processCt(message, 'ct3');
			processCt(message, 'ct4');
		}
	}
}

let datapoints = {};

cts.forEach((ct) => {
	datapoints[ct] = {};

	datafields.forEach((datafield) => {
		datapoints[ct][datafield] = [];
	});
});

function processCt(message, ctKey) {
	if (message[ctKey] !== undefined) {
		datafields.forEach((datafield) => {
			datapoints[ctKey][datafield].push(message[ctKey][datafield]);
		});
	}
}

function uploadCtData() {
	cts.forEach((ct) => {
		let P = datapoints[ct][datafields[0]].slice();
		let I = datapoints[ct][datafields[1]].slice();

		let count = P.length;

		datapoints[ct][datafields[0]] = [];
		datapoints[ct][datafields[1]] = [];

		if (P.length == 0) {
			initCts();
			return;
		}

		P = getMedianAndAverage(P);
		I = getMedianAndAverage(I);

		let sql = 'INSERT INTO readings (ct, averagePower, medianPower, averageCurrent, medianCurrent, readings) VALUES (?, ?, ?, ?, ?, ?);';

		connection.query(sql,
			[ct, P.average, P.median, I.average, I.median, count],
			(error, results, fields) => {
			if (error) {
				console.log(error);
			}
		});
	});
}

function getMedianAndAverage(ray, compareFunction) {
	let average = 0;

	ray.forEach((val) => {
		average += val;
	});

	average /= ray.length;

	return {
		average: Math.abs(average),
		median: Math.abs(Math.median(ray)),
	};
}

setInterval(() => {
	parseMessages();
	processMessages();
}, 10);

setInterval(uploadCtData, 10000);
