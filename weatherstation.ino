#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <WiFi.h>
#include <Wire.h>
#include <Deneyap_BasincOlcer.h>
#include <Deneyap_SicaklikNemBasincOlcer.h>
#include <HTTPClient.h>

AtmosphericPressure Pressure;
SHT4x TempHum;

float pres;
float temp;
float hum;

const char* ssid = "YOUR-WIFI-NAME";
const char* password = "YOUR-WIFI-PASSWORD";

const char* telegramBotToken = "YOUR-TELEGRAMBOT-TOKEN";
const int telegramChatIds[] = {YOUR-CHATID, YOUR-FRIENDS-CHATID"(NOT NECESSARY)"};            
const int numChatIds = sizeof(telegramChatIds) / sizeof(telegramChatIds[0]); 

WiFiServer server(80);

// Variables to store HTTP requests and response timings
String header;
unsigned long currentTime = millis();
unsigned long previousHttpResponseTime = 0;
unsigned long previousTelegramMessageTime = 0;
const long timeoutTime = 2000;
const long telegramMessageInterval = 10000;

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  
  // Initialize sensors (pressure and temperature/humidity)
  Pressure.begin(0x76);
  if (!TempHum.begin(0X44)) {
    Serial.println("I²C connection failed ");
    while (1);
  }

  // Connect to Wi-Fi network
  Serial.print("Connecting to Wi-Fi network: ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connection successful.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  // Start the server to listen for incoming HTTP requests
  server.begin();
}

void loop() {
  // Wait for a client to connect
  WiFiClient client = server.available();
  
  // Measure temperature, humidity, and pressure from sensors
  TempHum.measure();
  temp = TempHum.TtoDegC();
  hum = TempHum.RHtoPercent();
  pres = Pressure.getPressure();

  // If a client connects, handle the HTTP request and response
  if (client) {
    currentTime = millis();
    previousHttpResponseTime = currentTime;
    Serial.println("New Client.");
    String currentLine = "";
    
    // Read the client's HTTP request
    while (client.connected() && currentTime - previousHttpResponseTime <= timeoutTime) {
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        
        // Check for the end of the client's request
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // Send the HTTP response with sensor data to the client
            sendHttpResponse(client);
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  // Send data to Telegram at defined intervals
  if (currentTime - previousTelegramMessageTime >= telegramMessageInterval) {
    sendTelegramMessage();
    previousTelegramMessageTime = currentTime;
  }
}

void sendHttpResponse(WiFiClient client) {
  // Send standard HTTP headers
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println("Refresh: 5");
  client.println();
  
  // Build the HTML response to display sensor data on a webpage
  client.println("<!DOCTYPE html>");
  client.println("<html lang=\"en\">");
  client.println("<head>");
  client.println("    <meta charset=\"UTF-8\">");
  client.println("    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
  client.println("    <title>Weather Station</title>");
  client.println("    <link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-QWTKZyjpPEjISv5WaRU9OFeRpok6YctnYmDr5pNlyT2bRjXh0JMhjY6hW+ALEwIH\" crossorigin=\"anonymous\">");
  client.println("    <link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.2/css/all.min.css\" integrity=\"sha512-SnH5WK+bZxgPHs44uWIX+LLJAJ9/2PkPKZ5QiAj6Ta86w+fsb2TkcmfRyVX3pBnMFcV7oQPJkl9QevSCWr3W6A==\" crossorigin=\"anonymous\" referrerpolicy=\"no-referrer\" />");
  client.println("    <style>");
  client.println("        body {");
  client.println("            margin: 0;");
  client.println("            box-sizing: border-box;");
  client.println("            min-height: 100vh;");
  client.println("            display: flex;");
  client.println("            justify-content: center;");
  client.println("            align-items: center; ");
  client.println("            background: linear-gradient(245.59deg,#34e89e 0% ,#0f3443 28.53% , #000000 75.52%);");
  client.println("        }");
  client.println("        .card {");
  client.println("            background: transparent;");
  client.println("            border: 2px solid;");
  client.println("            border-image: linear-gradient(45deg, #ff8a00, #e52e71) 1;");
  client.println("            border-radius: 8px;");
  client.println("            width: 18rem;");
  client.println("        }");
  client.println("        i {");
  client.println("            font-size: 70px;");
  client.println("        }");
  client.println("        .card-text, i, .card-title, h1, h2, span {");
  client.println("            background: linear-gradient(45deg, #ff8a00, #e52e71);");
  client.println("            -webkit-background-clip: text;");
  client.println("            color: transparent;");
  client.println("        }");
  client.println("    </style>");
  client.println("</head>");
  client.println("<body>");
  
  // Display temperature (Celsius and Fahrenheit), pressure, and humidity
  client.println("    <div class=\"container\">");
  client.println("        <div class=\"row\">");
  client.println("            <h1 class=\"text-center mb-2 p-3\">WEATHER STATION</h1>");
  client.println("        </div>");
  client.println("        <div class=\"row text-center justify-content-center\">");
  client.println("            <div class=\"card col-md-3 col-sm-6 mx-3 my-2 d-flex justify-content-center\" >");
  client.println("                <i class=\"fa-solid fa-temperature-low d-flex justify-content-center align-items-center p-4\"></i>");
  client.println("                <div class=\"card-body\">");
  client.println("                    <h4 class=\"card-title text-center p-3\">TEMPERATURE</h4>");
  client.println("                    <h2 class=\"card-text p-4 text-center\">" + String(temp) + " C°</h2>");
  client.println("                </div>");
  client.println("            </div>");
  client.println("            <div class=\"card col-md-3 col-sm-6 mx-3 my-2\">");
  client.println("                <i class=\"fa-solid fa-temperature-quarter d-flex justify-content-center align-items-center p-4\"><span style=\"font-size: 15px; position: absolute;top: 10%; left: 60%;\">F°</span></i>");
  client.println("                <div class=\"card-body\">");
  client.println("                    <h4 class=\"card-title text-center p-3\">TEMPERATURE</h4>");
  client.println("                    <h2 class=\"card-text p-4 text-center\">" + String((temp * 1.8) + 32) + " F°</h2>");
  client.println("                </div>");
  client.println("            </div>");
  client.println("            <div class=\"card col-md-3 col-sm-6 mx-3 my-2\">");
  client.println("                <i class=\"fa-solid fa-gauge d-flex justify-content-center align-items-center p-4\"></i>");
  client.println("                <div class=\"card-body\">");
  client.println("                    <h4 class=\"card-title text-center p-3\">AIR PRESSURE</h4>");
  client.println("                    <h2 class=\"card-text p-4 text-center\">" + String(pres) + " hPa</h2>");
  client.println("                </div>");
  client.println("            </div>");
  client.println("            <div class=\"card col-md-3 col-sm-6 mx-3 my-2\">");
  client.println("                <i class=\"fa-solid fa-droplet d-flex justify-content-center align-items-center p-4\"></i>");
  client.println("                <div class=\"card-body\">");
  client.println("                    <h4 class=\"card-title text-center p-3\">HUMIDITY</h4>");
  client.println("                    <h2 class=\"card-text p-4 text-center\">" + String(hum) + " %</h2>");
  client.println("                </div>");
  client.println("            </div>");
  client.println("        </div>");
  client.println("    </div> ");
  client.println("</body>");
  client.println("</html>");
}

void sendTelegramMessage() {
  // Send the sensor data to all defined Telegram chat IDs
  for (int i = 0; i < numChatIds; i++) {
    DynamicJsonDocument json(200);
    
    // Fill the JSON object with the sensor data and chat ID
    json["chat_id"] = telegramChatIds[i];
    json["text"] = "Temperature (C°): " + String(temp) + " C°\n" +
                   "Temperature (F°): " + String((temp * 1.8) + 32) + " F°\n" +
                   "Humidity: " + String(hum) + " %\n" +
                   "Air Pressure: " + String(pres) + " hPa";

    // Serialize the JSON object to a string
    String jsonString;
    serializeJson(json, jsonString);

    // Send an HTTP POST request to the Telegram API
    HTTPClient http;
    http.begin("https://api.telegram.org/bot" + String(telegramBotToken) + "/sendMessage");
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonString);
    if (httpResponseCode > 0) {
      Serial.print("Telegram message sent successfully to chat id ");
      Serial.print(telegramChatIds[i]);
      Serial.print(". Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error sending Telegram message to chat id ");
      Serial.print(telegramChatIds[i]);
      Serial.print(". HTTP error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
}
