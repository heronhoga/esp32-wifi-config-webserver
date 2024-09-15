#include <WiFi.h>
#include <Preferences.h> 

Preferences preferences;
WiFiServer server(80);

//WIFI CRED
String ssid = "default_ssid";
String password = "default_password";

//GET VALUE
String getValue(String data, String key) {
  int start = data.indexOf(key + "=");
  if (start == -1) return "";
  start += key.length() + 1;
  int end = data.indexOf('&', start);
  if (end == -1) end = data.indexOf(' ', start);
  return data.substring(start, end);
}

void setup() {
  Serial.begin(115200);

  preferences.begin("wifi_creds", true);
  ssid = preferences.getString("ssid", ssid);
  password = preferences.getString("password", password);
  preferences.end();

  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("New Client Connected");
    String request = client.readStringUntil('\r');
    Serial.println(request);
    client.flush();

    if (request.indexOf("ssid=") != -1 && request.indexOf("password=") != -1) {
      String new_ssid = getValue(request, "ssid");
      String new_password = getValue(request, "password");

      preferences.begin("wifi_creds", false);
      preferences.putString("ssid", new_ssid);
      preferences.putString("password", new_password);
      preferences.end();

      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/html");
      client.println();
      client.println("<html><body><h1>WiFi settings updated. Restarting...</h1></body></html>");
      client.println();
      client.stop();

      delay(2000);
      ESP.restart();
    } else {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/html");
      client.println();
      client.println("<html><body>");
      client.println("<h1>Change WiFi Credentials</h1>");
      client.println("<form action=\"/\" method=\"GET\">");
      client.println("SSID: <input type=\"text\" name=\"ssid\"><br>");
      client.println("Password: <input type=\"text\" name=\"password\"><br>");
      client.println("<input type=\"submit\" value=\"Submit\">");
      client.println("</form>");
      client.println("</body></html>");
      client.println();
    }

    client.stop();
    Serial.println("Client Disconnected");
  }
}
