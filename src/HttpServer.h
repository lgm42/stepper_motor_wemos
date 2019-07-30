// HttpServer.h

#pragma once

#include "Arduino.h"

#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include <ESP8266FtpServer.h>
#include <ESP8266HTTPUpdateServer.h>

#include "ParameterProvider.h"
#include "StepperManager.h"

class HttpServer
{
  public:
	  HttpServer(ParameterProvider & params, StepperManager & stepper);
	  virtual ~HttpServer();

	virtual void setup(void);
	virtual void handle(void);

	String getContentType(String filename);

	ESP8266WebServer & webServer();
private:
	ESP8266WebServer _webServer;
	FtpServer _ftpServer;
	WiFiUDP _ntpUDP;
	ParameterProvider & _paramProvider;
    ESP8266HTTPUpdateServer _httpUpdater;
	StepperManager & _stepper;

	void updateNTP();
	bool handleFileRead(String path);

	void sendOk();
	void sendKo(const String & message);
	void sendOkAnswerWithParams(const String & params);

};