// 
// 
// 
#include <time.h>
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
	_paramProvider.loadSystemParameters();
	
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
		String angle(_webServer.arg("angle"));
        _stepper.startRotate(angle.toFloat());
        sendOk();
    });

    _webServer.on("/position", [&]() {
		Serial.println("REST: " + _webServer.uri());
		String data = "{ \"position\" : " + String(_stepper.position(), 2) + "}";
        sendOkAnswerWithParams(data);
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
	_webServer.sendHeader("Access-Control-Allow-Origin", "*");
	
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
	_webServer.send(200, "application/json", "{\"result\":true}");
}

void HttpServer::sendOkAnswerWithParams(const String & params)
{
	String data("{\"result\":true, \"data\":");
	data += params;
	data += "}";
	_webServer.send(200, "application/json", data);
}

void HttpServer::sendKo(const String & message)
{
	String data("{\"result\":false, \"message\":\"");
	data += message;
	data += "\"}";
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
	//_ftpServer.handleFTP();
    MDNS.update();
}

ESP8266WebServer & HttpServer::webServer() 
{
	return _webServer;
}