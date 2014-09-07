
var GPSPosition = {'lat':411234569, 'lon':-81234568, 'fix':0, 'nSat':0},
battery_percentage = 100,
stopped_by_battery = false;

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
  console.log("Recebi: " + msg);

  // External sensing system asks for GPS location
  if (msg.search("\\*GPS!") != -1)
    processGPS(true);

  else if (msg.search("\\*GPS_ACK!") != -1)
    processGPS(false);
});

// com = True -> received *GPS! // com = False -> received *GPS_ACK!
function processGPS(com)
{
  if (com && !processGPS.sending)
  {
    //console.log("Processei! vai começar")
    processGPS.sending = true;
    processGPS.timerID = setInterval(sendGPSPosition, 500);
  }

  else if (!com) // received ACK_GPS!
  {
    //console.log("Recebi o ACK");
    processGPS.sending = false;
    clearInterval(processGPS.timerID);
  }

  //else // if received *GPS! and is sending, not do anything
    //console.log("Recebi GPS e ja tava a mandar");
}

function sendGPSPosition()
{ 
  console.log("Mandei " + "/" + GPSPosition.lat*Math.pow(10, -7) + "," + GPSPosition.lon*Math.pow(10, -7) + "!");
  console.log("Satelites: " + GPSPosition.nSat);
  sp.write("/" + GPSPosition.lat*Math.pow(10, -7) + "," + GPSPosition.lon*Math.pow(10, -7) + "!\n");
}

// ******************************************************************
// ************************* MAVLINK Lib ****************************
// ******************************************************************
var mavlink = require('mavlink'),
myMAV = new mavlink(1,190),
dgram = require("dgram"),
server = dgram.createSocket("udp4");

// Pause/Continue command/argument values
var MAV_CMD_OVERRIDE_GOTO = 252,
    MAV_GOTO_DO_HOLD = 0,
    MAV_GOTO_DO_CONTINUE = 1,
    MAV_GOTO_HOLD_AT_CURRENT_POSITION = 2;

var MAV_CMD_NAV_LAND = 21,
    MAV_CMD_NAV_TAKEOFF = 22;

var MAV_CMD_CONDITION_DELAY = 112;

var MAV_PARAM_TYPE_REAL32 = 9;

// Component IDs
var MAV_COMP_ID_ALL = 0;

function launchMission()
{ sendCommand(MAV_CMD_NAV_TAKEOFF, MAV_COMP_ID_ALL, 0, 0, 0, 0, 0, 0, 0); }

function cancelMission()
{ sendCommand(MAV_CMD_NAV_LAND, MAV_COMP_ID_ALL, 0, 0, 0, 0, 0, 0, 0); }

function limitSpeed(speed)
{
    myMAV.createMessage('PARAM_SET', 
  {
    'target_system' : 1,
    'target_component' : MAV_COMP_ID_ALL,
    'param_id' : "SPEED-M-S",
    'param_value' : speed,
    'param_type' : MAV_PARAM_TYPE_REAL32
  },
  function(msg) {
   server.send(msg.buffer, 0, msg.buffer.length, 14551, '192.168.1.1', function(err, bytes) { console.log("Sent speed limit to " + speed + "m/s"); });
 });
}

function sendCommand(command, component, param1, param2, param3, param4, param5, param6, param7)
{
  myMAV.createMessage('COMMAND_LONG', 
  {
    'target_system' : 1,
    'target_component' : component,
    'command' : command,
    'confirmation' : 1,
    'param1' : param1,
    'param2' : param2,
    'param3' : param3,
    'param4' : param4,
    'param5' : param5,
    'param6' : param6,
    'param7' : param7
  },
  function(msg) {
   server.send(msg.buffer, 0, msg.buffer.length, 14551, '192.168.1.1', function(err, bytes) { console.log("Sent command nº" + command); });
 });
}

myMAV.on("ready", function() {

  server.bind(14550);

  server.on("message", function (msg, rinfo) {
    myMAV.parse(msg);
  });

  myMAV.on('GPS_RAW_INT', function(msg, attr) {
    GPSPosition.lat = attr.lat;
    GPSPosition.lon = attr.lon;
    GPSPosition.nSat = attr.satellites_visible;
  });

  myMAV.on('SYS_STATUS', function(msg, attr) {

    battery_percentage = attr.battery_remaining;

    if (battery_percentage < 15 && !stopped_by_battery)
    {
      console.log("Low battery - cancelling mission!");
      stopped_by_battery = true;
      cancelMission();
    }
  });

  myMAV.on('PARAM_VALUE ', function(msg, attr) {
    console.log("PARA_VALUE:");
    console.log(attr);
  });

  myMAV.on('COMMAND_ACK', function(msg, attr) {

    console.log("COMMAND_ACK received from command " + attr.command);

    if (attr.result == 0)
      console.log("Command Accepted");
    else if (attr.result == 1)
     console.log("Command Temporarily Rejected");
   else if (attr.result == 2)
     console.log("Command Denied");
   else if (attr.result == 3)
     console.log("Command Unsupported");
   else
     console.log("Command Failed");
 });

  limitSpeed(0.25);
});

// ******************************************************************
// ************************ STDIN COMMAND ***************************
// ******************************************************************
var stdin = process.stdin;
stdin.setRawMode(true);
stdin.resume();

stdin.on('data', function(key)
{
  key = key.toString();
  console.log(key);

  if ((key === 'i' || key === 'I') && !stopped_by_battery)
  {
    console.log("Initiating....\n");
    launchMission();
  }

  else if (key === 'o' || key === 'O')
  {
    console.log("Closing mission....\n");
    cancelMission();
  }

  else if (key === 'q' || key === 'Q')
    process.exit();
});

/*
setInterval(function() { 
  var datetime = new Date();
  console.log(datetime);
}, 5000);
*/

setInterval(global.gc, 500);
console.log("\n** File interpreted **\n");
console.log("Press (I) to initiate the flight, (O) to land and (Q) to quit");