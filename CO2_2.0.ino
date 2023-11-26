#include <DHT.h>
#include <ESP8266WiFi.h>
#include <MQ135.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

String apiKey = "GN7WHOTWYB8U5Z5F"; // Ingresa tu clave API de ThingSpeak
const char *ssid1 = "Estudiantes"; // Reemplaza con tu SSID de WiFi
const char *pass1 = "educar_2018";
const char *ssid2 = ":3"; // Reemplaza con tu segundo SSID de WiFi
const char *pass2 = "sorepichi";

#define DHTPIN 14 // Pin donde está conectado el DHT11
#define MQ135PIN A0 // Pin analógico donde está conectado el sensor MQ135

#define LED_PIN_GREEN 12 // Pin del LED verde
#define LED_PIN_RED 13 // Pin del LED rojo
#define BUZZER_PIN 0 // Pin del zumbador

DHT dht(DHTPIN, DHT11);
MQ135 gasSensor = MQ135(MQ135PIN);

WiFiClient client;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup()
{
  Serial.begin(115200);
  delay(10);
  dht.begin();

  Serial.println("Connecting to ");
  Serial.println(ssid1);

  WiFi.begin(ssid1, pass1);

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    if (millis() - startTime > 20000) {
      Serial.println("Failed to connect to network 1, trying network 2");
      WiFi.begin(ssid2, pass2);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("");
      Serial.println("WiFi connected to network 2");
      break;
    }
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected to network 1");
  }

  // Inicializa el display OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(2000);
  display.clearDisplay();

  // Configura los pines del LED y el zumbador
  pinMode(LED_PIN_GREEN, OUTPUT);
  pinMode(LED_PIN_RED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Inicialmente, enciende el LED verde y apaga el rojo
  digitalWrite(LED_PIN_GREEN, HIGH);
  digitalWrite(LED_PIN_RED, LOW);
}

void loop()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float ppm = gasSensor.getPPM(); // Leer el valor de ppm del sensor MQ135

  if (isnan(h) || isnan(t))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Comprueba si la temperatura, humedad o el ppm son altos
  if (t > 30 || h > 85 || ppm > 800)
  {
    // Si alguno de los valores es alto, cambia el LED a rojo y activa el zumbador
    digitalWrite(LED_PIN_GREEN, LOW);
    digitalWrite(LED_PIN_RED, HIGH);
    digitalWrite(BUZZER_PIN, HIGH); // Enciende el zumbador
  }
  else
  {
    // Si no, mantén el LED en verde y apaga el zumbador
    digitalWrite(LED_PIN_GREEN, HIGH);
    digitalWrite(LED_PIN_RED, LOW);
    digitalWrite(BUZZER_PIN, LOW); // Apaga el zumbador
  }

  if (client.connect("api.thingspeak.com", 80))
  {
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(t);
    postStr += "&field2=";
    postStr += String(h);
    postStr += "&field3=";
    postStr += String(ppm); // Agrega el valor de ppm del sensor MQ135

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);

    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" degrees Celsius, Humidity: ");
    Serial.print(h);
    Serial.print(" %, CO2 PPM: ");
    Serial.print(ppm);
    Serial.println(". Sent to ThingSpeak.");
  }

  client.stop();

  // Mostrar los datos en el display OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Temp: ");
  display.print(t);
  display.print("C");
  display.setCursor(0, 10);
  display.print("Humidity: ");
  display.print(h);
  display.print("%");
  display.setCursor(0, 20);
  display.print("CO2 PPM: ");
  display.print(ppm);
  display.display();

  Serial.println("Waiting...");

  // ThingSpeak necesita un retraso mínimo de 15 segundos entre actualizaciones
  delay(16000);
}

