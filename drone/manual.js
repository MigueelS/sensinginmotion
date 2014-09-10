// ******************************************************************
// ********************** ArDrone Node Lib **************************
// ******************************************************************
var arDrone = require('ar-drone'),
arDroneConstants = require('ar-drone/lib/constants'),
client  = arDrone.createClient();

client.disableEmergency();

var GPSPosition = {'lat':41.1234569, 'lon':-8.1234568, 'nSat':0},
battery_percentage = '?',
stopped_by_battery = false,
last_update = -1,
system_init_time = -1;

function navdata_option_mask(c) { return 1 << c; }

var navdata_options = (
	navdata_option_mask(arDroneConstants.options.DEMO)
	| navdata_option_mask(arDroneConstants.options.ZIMMU_3000)
	);

// Connect and configure the drone
client.config('general:navdata_demo', true);
client.config('general:navdata_options', navdata_options);

client.on('navdata', function(navdata)
{
	last_update = new Date().getTime();

	if (navdata.gps)
	{
		GPSPosition.lat = navdata.gps.latitude;
		GPSPosition.lon = navdata.gps.longitude;
		GPSPosition.nSat = navdata.gps.nbSatellites;
	}

	if (navdata.demo)
		battery_percentage = navdata.demo.batteryPercentage;
});

// ******************************************************************
// *************** SERIAL COMMUNICATION *****************************
// ******************************************************************
var serialport = require('node-serialport');

var sp = new serialport.SerialPort("/dev/ttyO3", {
	parser: serialport.parsers.readline('\n'),
	baud: 9600
});

sp.on('data', function(msg) {

	msg = msg.toString();
	msg = msg.substr(0, msg.length - 1);
	console.log("Received: " + msg);

  // External sensing system asks for GPS location
  if (msg.search("\\*GPS!") != -1)
  	processGPS(true);

  else if (msg.search("\\*GPS_ACK!") != -1)
  	processGPS(false);
});

// Processes the income of a GPS command
// com = True -> received *GPS! // com = False -> received *GPS_ACK!
function processGPS(com)
{
	if (com && !processGPS.sending)
	{
		processGPS.sending = true;
		processGPS.timerID = setInterval(sendGPSPosition, 500);
	}

  else if (!com) // received ACK_GPS!
  {
  	processGPS.sending = false;
  	clearInterval(processGPS.timerID);
  }
}

// Sends the GPS Position by serial com
function sendGPSPosition()
{ 
	console.log("Sent " + "/" + GPSPosition.lat + "," + GPSPosition.lon + "!");
	sp.write("/" + GPSPosition.lat + "," + GPSPosition.lon + "!\n");
}

// ******************************************************************
// ************************* Stardard Input *************************
// ******************************************************************
var speed_go = 0.1,
speed_turn = 0.5;

var stdin = process.stdin;
stdin.setRawMode(true);
stdin.resume();

stdin.on('data', function(key)
{
	key = key.toString();

	if (key === 't' || key === 'T')
		client.takeoff();
	else if (key === 'r' || key === 'R')
		client.land();
	else if (key === 'q' || key === 'Q')
		process.exit();
	else if (key === 'w' || key === 'W')
		client.front(speed_go);
	else if (key === 's' || key === 'S')
		client.back(speed_go);
	else if (key === 'd' || key === 'D')
		client.right(speed_go);
	else if (key === 'a' || key === 'A')
		client.left(speed_go);
	else if (key === 'i' || key === 'I')
		client.up(speed_go);
	else if (key === 'k' || key === 'K')
		client.down(speed_go);
	else if (key === 'l' || key === 'L')
		client.clockwise(speed_turn);
	else if (key === 'j' || key === 'J')
		client.counterClockwise(speed_turn);
	else if (key === 'c' || key === 'C')
		client.calibrate(0);
	else if (key === ' ')
		client.stop();
	else if (key === '+')
	{
		if (speed_go < 0.9)
		{
			speed_go += 0.1;
			client.stop();
		}
	}

	else if (key === '-')
	{
		if (speed_go > 0.1)
		{
			speed_go -= 0.1;
			client.stop();
		}
	}	
});

// Shows the current state on the screen
setInterval(function() {

	console.log("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");

	var current_time = new Date().getTime();

	if (current_time - last_update > 2000)
		console.log("\n!! No connection to the drone !!\n");

	if (stopped_by_battery)
		console.log("\n!! Reached less than 15% battery! Automatic land !!");
	else if (battery_percentage !== '?' && battery_percentage < 15)
	{
		client.stop();
		client.land();
		stopped_by_battery = true;
	}

	var running_time = current_time - system_init_time;

	console.log("System running for %dmins, %dsecs", Math.floor(running_time/(1000*60)), Math.floor(running_time/1000) % 60);
	console.log("Speed: %d", speed_go);
	//console.log("***** GPS: ******")
	//console.log(navdaty.gps)
	console.log("Battery (%): " + battery_percentage)
	console.log("W-front / S-back / D-right / A-left\nI-up / K-down / L-clockwise / J-counterclockwise\nC-calibrate / SPACE-stop / T-takeoff / R-land / Q-EXIT\n(+) SPEED+ (-) SPEED-");
	console.log("******************************************************\n");

}, 1000); 

last_update = new Date().getTime();
system_init_time = last_update;

setInterval(global.gc, 500);
console.log("\n** File interpreted **\n");