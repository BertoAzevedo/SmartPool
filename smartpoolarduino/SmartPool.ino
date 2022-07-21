#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TM1637Display.h>
#include <math.h>

// Network config
#ifndef STASSID
#define STASSID "<YOURNETWORK>"
#define STAPSK "<YOURNETWORKPASSWORD>"
#endif

// Display Pins
#define CLK 25
#define DIO 27

// Sensor Pins
#define poolSensorPin 17
#define panelSensorPin 16

// Button Pins
#define automationButtonPin 15
#define lightsButtonPin 12

// Replay Pins
#define poolRelayPin 14
#define lightsRelayPin 13

// System Variables
int automationButtonValue = 0;
int lightsButtonValue = 0;

float panelTemperature = 0;
float poolTemperature = 0;
float temperatureDifference = 0;

bool automationStatus = false;
bool lightsStatus = false;

// Configs
int defaultMotorDuration = 120; // in seconds
int defaultTemperatureThreshold = 5;

int temperatureThreshold = defaultTemperatureThreshold;
unsigned long milliseconds = 1000L;
unsigned long duration = defaultMotorDuration;
unsigned long motorDuration = milliseconds * duration;

OneWire oneWirePin_painel(panelSensorPin);
DallasTemperature panelSensor(&oneWirePin_painel);

OneWire oneWirePin_piscina(poolSensorPin);
DallasTemperature poolSensor(&oneWirePin_piscina);

TM1637Display display(CLK, DIO);

const char *ssid = STASSID;
const char *password = STAPSK;

WebServer server(80);

const int led = LED_BUILTIN;

const String postForms = "<html>\
    <h1>SmartPool</h1>\
    <h2>Routes:</h2>\
    <h3>- Get /data to get all the sensor data</h3><br>\
    <h3>- Get /lights?status=1 to activate exterior lights or 0 to deactivate</h3><br>\
    <h3>- Get /motor?status=1 to activate motor</h3><br>\
    <h3>- Get /config to get current system configuration</h3><br>\
    <h3>- Get /config/update?duration=120&threshold=5 to change system configuration. Duration of motor activation in seconds and threshold in celsius</h3><br>\
    <h3>- Get /config/reset to reset system configs to default (duration = 2 min and threshold = 5 degrees)</h3><br>\
    <h3>- Get /state to get the current system state</h3><br>\
</html>";

void handleRoot()
{
	digitalWrite(led, 1);
	server.send(200, "text/html", postForms);
	digitalWrite(led, 0);
}

void handleData()
{
	digitalWrite(led, 1);
	// martelo
	panelTemperature = random(25, 45);
	poolTemperature = random(15, 30);

	String data = "{\"panel_temp\": ";
	data += panelTemperature;
	data += ", \"pool_temp\": ";
	data += poolTemperature;
	data += "}";
	server.send(200, "text/plain", data);
	digitalWrite(led, 0);
}

void handleLights()
{
	digitalWrite(led, 1);
	String data;

	if (server.arg("status") == "")
	{
		data = "Lights status was not changed: missing query bool param 'status'. Send ?status=1 or 0 to change exterior light status";
	}
	else
	{
		String statusArg = server.arg("status");

		if (lightsStatus == true)
		{
			if (statusArg == "1")
			{
				changeLightsStatus(true);
				data = "Lights activated successfully";
			}
			else if (statusArg == "0")
			{
				changeLightsStatus(false);
				data = "Lights deactivated successfully";
			}
			else
			{
				data = "Invalid status param. Please use 0 or 1";
			}
		}
		else
		{
			changeLightsStatus(false);
			data = "Lights cannot be activated (Lights Button is disabled)";
		}
	}

	server.send(200, "text/plain", data);
	digitalWrite(led, 0);
}

void handleMotor()
{
	digitalWrite(led, 1);
	String data;

	if (server.arg("status") == "")
	{
		data = "Motor status was not changed: missing query bool param 'status'. Send ?status=1 activate motor";
	}
	else
	{
		String statusArg = server.arg("status");

		if (automationStatus == true)
		{
			if (statusArg == "1")
			{
				data = "Motor activated successfully";
				activate_motor();
			}
			else
			{
				data = "Invalid status param. Please use status=1 to activate motor";
			}
		}
		else
		{
			data = "Motor cannot be activated (Automation Button is disabled)";
		}
	}

	server.send(200, "text/plain", data);
	digitalWrite(led, 0);
}

void handleConfig()
{
	digitalWrite(led, 1);
	String data = "{\"duration\": ";
	data += duration;
	data += ", \"threshold\": ";
	data += temperatureThreshold;
	data += "}";
	server.send(200, "text/plain", data);
	digitalWrite(led, 0);
}

bool isValidNumber(String number)
{
	for (auto x : number)
	{
		if (!isDigit(x))
		{
			return false;
		}
	}
	return true;
}

void handleChangeConfig()
{
	digitalWrite(led, 1);
	String data;

	if (server.arg("duration") != "")
	{
		String durationArg = server.arg("duration");

		if (isValidNumber(durationArg))
		{
			int newDuration = durationArg.toInt();
			duration = newDuration;
			motorDuration = milliseconds * duration;
			data += "Duration config updated. ";
		}
		else
		{
			data += "Invalid duration param. Param is not a valid digit. ";
		}
	}

	if (server.arg("threshold") != "")
	{
		String thresholdArg = server.arg("threshold");

		if (isValidNumber(thresholdArg))
		{
			int newThreshold = thresholdArg.toInt();
			temperatureThreshold = newThreshold;
			data += "Temperature threshold config updated. ";
		}
		else
		{
			data += "Invalid threshold param. Param is not a valid digit. ";
		}
	}

	server.send(200, "text/plain", data);
	digitalWrite(led, 0);
}

void handleResetConfig()
{
	digitalWrite(led, 1);
	duration = defaultMotorDuration;
	motorDuration = milliseconds * duration;
	temperatureThreshold = defaultTemperatureThreshold;
	String data = "System configs resetted";
	server.send(200, "text/plain", data);
	digitalWrite(led, 0);
}

void handleState()
{
	digitalWrite(led, 1);
	String data = "{\"automationState\": ";
	data += automationStatus;
	data += ", \"lightsState\": ";
	data += lightsStatus;
	data += "}";
	server.send(200, "text/plain", data);
	digitalWrite(led, 0);
}

void handleNotFound()
{
	digitalWrite(led, 1);
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";
	for (uint8_t i = 0; i < server.args(); i++)
	{
		message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
	}
	server.send(404, "text/plain", message);
	digitalWrite(led, 0);
}

// função para ativar o motor
void activate_motor()
{
	Serial.println("Motor on..");
	digitalWrite(poolRelayPin, LOW);
	delay(motorDuration);
	digitalWrite(poolRelayPin, HIGH);
	Serial.println("Motor off..");
}

// Function to change exterior light status
// True = On
// False = Off
void changeLightsStatus(bool status)
{
	if (status == true)
	{
		digitalWrite(lightsRelayPin, LOW);
	}
	else
	{
		digitalWrite(lightsRelayPin, HIGH);
	}
}

// verificar se o botão foi premido
void check_buttons()
{
	Serial.print("\n---------------------------------------------------------\n");
	Serial.print("\nAutomatic State: ");
	Serial.print(automationStatus);
	Serial.print("\n");

	Serial.print("\Lights State: ");
	Serial.print(lightsStatus);
	Serial.print("\n");

	automationButtonValue = digitalRead(automationButtonPin); // read the push button value
	lightsButtonValue = digitalRead(lightsButtonPin);		  // read lights button

	Serial.print("Automatic Button Value: ");
	Serial.print(automationButtonValue);
	Serial.print("\n");

	Serial.print("Lights Button Value: ");
	Serial.print(lightsButtonValue);
	Serial.print("\n");
	Serial.print("\n---------------------------------------------------------\n");

	automationStatus = automationButtonValue == 1 ? true : false;
	lightsStatus = lightsButtonValue == 1 ? true : false;

	delay(500);
}

// receber as temperaturas
void check_temperatures()
{
	Serial.print("\nRequesting Temperatures from sensors...");
	panelSensor.requestTemperatures();
	poolSensor.requestTemperatures();
	Serial.println("\nDONE\n");

	Serial.print("\n---------------------------------------------------------\n");
	panelTemperature = panelSensor.getTempCByIndex(0);
	Serial.print("Panel Temp: ");
	Serial.print(panelTemperature);

	poolTemperature = poolSensor.getTempCByIndex(0);
	Serial.print("  ||  Pool temp: ");
	Serial.print(poolTemperature);

	Serial.print("\n");
	temperatureDifference = panelTemperature - poolTemperature;
	Serial.print("\nTemperature difference: ");
	Serial.print(temperatureDifference);
	Serial.print("\n");

	display_values();

	if (automationStatus == true)
	{
		Serial.print("\nAutomation is Enabled!\n");
		if (poolTemperature != -127.00 && panelTemperature != -127.00)
		{
			if (poolTemperature > 0 && panelTemperature > 0)
			{
				if (temperatureDifference >= temperatureThreshold)
				{
					activate_motor();
				}
			}
		}
	}
	else
	{
		Serial.print("\nAutomation is Disabled!\n");
	}
	Serial.print("\n---------------------------------------------------------\n");
}

void display_values()
{
	int panelTemp = 0;
	int poolTemp = 0;

	if (panelTemperature > 0)
	{
		panelTemp = round(panelTemperature);
	}
	if (poolTemperature > 0)
	{
		poolTemp = round(poolTemperature);
	}

	display.showNumberDec(panelTemp, false, 2, 0);
	display.showNumberDec(poolTemp, false, 2, 2);
}

void setup(void)
{
	// setup pins
	pinMode(led, OUTPUT);
	pinMode(automationButtonPin, INPUT);
	pinMode(lightsButtonPin, INPUT);
	pinMode(poolRelayPin, OUTPUT);
	pinMode(lightsRelayPin, OUTPUT);

	digitalWrite(poolRelayPin, HIGH);
	digitalWrite(lightsRelayPin, HIGH);

	// setup display
	display.setBrightness(0x0f);
	uint8_t data[] = {0xff, 0xff, 0xff, 0xff};
	display.setSegments(data);

	digitalWrite(led, 0);
	Serial.begin(115200);
	// Serial.begin(9600);

	// initialize sensors
	panelSensor.begin();
	poolSensor.begin();

	WiFi.begin(ssid, password);
	Serial.println("");

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.print("Connected to ");
	Serial.println(ssid);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	if (MDNS.begin("esp8266"))
	{
		Serial.println("MDNS responder started");
	}

	server.sendHeader("Access-Control-Allow-Origin", "*");
	server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
	server.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, PATCH, OPTIONS");

	server.on("/", handleRoot);
	server.on("/data", handleData);
	server.on("/lights", handleLights);
	server.on("/motor", handleMotor);
	server.on("/config", handleConfig);
	server.on("/config/update", handleChangeConfig);
	server.on("/config/reset", handleResetConfig);
	server.on("/state", handleState);
	server.onNotFound(handleNotFound);

	server.begin();
	Serial.println("HTTP server started");
}

void loop(void)
{
	server.handleClient();
	check_buttons();
	check_temperatures();
}
