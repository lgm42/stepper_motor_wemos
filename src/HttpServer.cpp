// 
// 
// 
#include <time.h>
#include <ArduinoJson.h>
#include <FS.h>

#include "ParameterProvider.h"
#include "StepperManager.h"

#include "HttpServer.h"

// Timezone
#define UTC_OFFSET + 1

// change for different ntp (time servers)
#define NTP_SERVERS "0.fr.pool.ntp.org", "time.nist.gov", "pool.ntp.org"

HttpServer::HttpServer(ParameterProvider & params, StepperManager & stepper)
	: _webServer(80), _paramProvider(params), _httpUpdater(true), _stepper(stepper)
{
}

HttpServer::~HttpServer()
{
}


void HttpServer::setup(void)
{
	MDNS.begin(_paramProvider.systemParams().hostname.c_str()); 
	MDNS.addService("http", "tcp", 80);

	_ftpServer.begin(_paramProvider.systemParams().ftpLogin, _paramProvider.systemParams().ftpPassword);
	MDNS.addService("ftp", "tcp", 21);

    _httpUpdater.setup(&_webServer, "/update");
	_webServer.begin();

	updateNTP();

    _webServer.on("/reboot", [&]() {
        _webServer.send(200, "text/plain", "ESP reboot now !");
        delay(200);
        ESP.restart();
    });
	
    _webServer.on("/startRotate", [&]() {
        Serial.println("REST: " + _webServer.uri());
		if (_webServer.arg("absoluteAngle") != "")
		{
			String angle(_webServer.arg("absoluteAngle"));
        	_stepper.startRotateAbsolute(angle.toFloat());
			sendOk();
		}
		else if (_webServer.arg("relativeAngle") != "")
		{
			String angle(_webServer.arg("relativeAngle"));
        	_stepper.startRotateRelative(angle.toFloat());
			sendOk();
		}
		else
		{
			sendKo("Query must have parameter absoluteAngle or relativeAngle");
		}
        
    });

    _webServer.on("/position", [&]() {
		Serial.println("REST: " + _webServer.uri());
		String data = "{ \"position\" : " + String(_paramProvider.position(), 2) + "}";
        sendOkAnswerWithParams(data);
        sendOk();
    });

	_webServer.on("/parameters", HTTP_GET, [&]() {
		Serial.println("REST: " + _webServer.uri());
		String data = "{ \"parameters\" : " + _paramProvider.toJson() + "}";
        sendOkAnswerWithParams(data);
        sendOk();
    });

	_webServer.on("/parameters",  HTTP_POST, [&]() {
		DynamicJsonDocument doc(1024);

		DeserializationError error = deserializeJson(doc, (String)_webServer.arg("plain"));
		if (error)
		{
			sendKo("Unable to parse body");
			return;
		}
		
		if (doc.containsKey("positive-angle-amplitude"))
			_paramProvider.params().clockWizeAngleAmplitude = doc["positive-angle-amplitude"];
		if (doc.containsKey("negative-angle-amplitude"))
			_paramProvider.params().counterClockWizeAngleAmplitude = doc["negative-angle-amplitude"];
		if (doc.containsKey("motor-step-number"))
			_paramProvider.params().motorStepNumber = doc["motor-step-number"];
		if (doc.containsKey("origin-angle"))
			_paramProvider.params().originAngle = doc["origin-angle"];
		if (doc.containsKey("reduction-rate"))
			_paramProvider.params().reductionRate = doc["reduction-rate"];

        sendOk();
    });

	//called when the url is not defined here
	//use it to load content from SPIFFS
	_webServer.onNotFound([&]() {
		if (!handleFileRead(_webServer.uri()))
			_webServer.send(404, "text/plain", "FileNotFound");
	});
}

String HttpServer::getContentType(String filename)
{
	if (_webServer.hasArg("download")) return "application/octet-stream";
	else if (filename.endsWith(".htm")) return "text/html";
	else if (filename.endsWith(".html")) return "text/html";
	else if (filename.endsWith(".css")) return "text/css";
	else if (filename.endsWith(".js")) return "application/javascript";
	else if (filename.endsWith(".png")) return "image/png";
	else if (filename.endsWith(".gif")) return "image/gif";
	else if (filename.endsWith(".jpg")) return "image/jpeg";
	else if (filename.endsWith(".ico")) return "image/x-icon";
	else if (filename.endsWith(".xml")) return "text/xml";
	else if (filename.endsWith(".pdf")) return "application/x-pdf";
	else if (filename.endsWith(".zip")) return "application/x-zip";
	else if (filename.endsWith(".gz")) return "application/x-gzip";
	return "text/plain";
}

bool HttpServer::handleFileRead(String path)
{
	//_webServer.sendHeader("Access-Control-Allow-Origin", "*");
	if (_webServer.method() == HTTP_OPTIONS)
    {
        _webServer.sendHeader("Access-Control-Allow-Origin", "*");
        _webServer.sendHeader("Access-Control-Max-Age", "10000");
        _webServer.sendHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
        _webServer.sendHeader("Access-Control-Allow-Headers", "*");
        _webServer.send(204);
		return true;
	}

    if (path.endsWith("/")) 
        path += "index.html";

    String contentType = getContentType(path);
    String pathWithGz = path + ".gz";
    if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
        if (SPIFFS.exists(pathWithGz))
            path += ".gz";
        File file = SPIFFS.open(path, "r");
        _webServer.streamFile(file, contentType);
        file.close();
        return true;
    }

	return false;
}

void HttpServer::sendOk()
{
	_webServer.sendHeader("Access-Control-Allow-Origin", "*");
	_webServer.send(200, "application/json", "{\"result\":true}");
}

void HttpServer::sendOkAnswerWithParams(const String & params)
{
	String data("{\"result\":true, \"data\":");
	data += params;
	data += "}";
	_webServer.sendHeader("Access-Control-Allow-Origin", "*");
	_webServer.send(200, "application/json", data);
}

void HttpServer::sendKo(const String & message)
{
	String data("{\"result\":false, \"message\":\"");
	data += message;
	data += "\"}";
	_webServer.sendHeader("Access-Control-Allow-Origin", "*");
	_webServer.send(400, "application/json", data);
}

void HttpServer::updateNTP() 
{
  configTime(UTC_OFFSET * 3600, 0, NTP_SERVERS);
  delay(500);
  while (!time(nullptr)) {
    Serial.print("#");
    delay(1000);
  }
  Serial.println("Update NTP");
}

void HttpServer::handle(void)
{
	_webServer.handleClient();
	_ftpServer.handleFTP();
    MDNS.update();
}

ESP8266WebServer & HttpServer::webServer() 
{
	return _webServer;
}