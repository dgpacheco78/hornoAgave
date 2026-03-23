#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <time.h>
#include <DHT.h>
#include <max6675.h>

const char* ssid = "UTIM";
const char* password = "utim$$$2026";

const char* GOOGLE_SCRIPT_URL =
  "https://script.google.com/macros/s/AKfycbz4-vaqbfhkzpq4g_aNPbMaC8MYAubCxF6ZLELVvXoRZouxcCp8ZJonxHkpNQhMDHMu/exec";

const char* NTP_SERVER = "pool.ntp.org";
const long  GMT_OFFSET_SEC = -6 * 3600;
const int   DAYLIGHT_OFFSET_SEC = 0;

unsigned long lastLogMs = 0;
const unsigned long LOG_PERIOD_MS = 10UL * 60UL * 1000UL;

int thermoSO  = 19;
int thermoSCK = 18;
int thermoCS1 = 5;
int thermoCS2 = 17;
int thermoCS3 = 16;
int thermoCS4 = 4;

#define DHTPIN 27
#define DHTTYPE DHT11

WebServer server(80);
DHT dht(DHTPIN, DHTTYPE);

MAX6675 tc1(thermoSCK, thermoCS1, thermoSO);
MAX6675 tc2(thermoSCK, thermoCS2, thermoSO);
MAX6675 tc3(thermoSCK, thermoCS3, thermoSO);
MAX6675 tc4(thermoSCK, thermoCS4, thermoSO);

struct Temps {
  float termopar1, termopar2, termopar3, termopar4;
  float dht11;     // Temperatura ambiente (°C)
  float dht11_h;   // Humedad ambiente (%)
};

static String f2s(float v, unsigned int dec = 2) {
  if (isnan(v) || isinf(v)) return "null";
  return String((double)v, dec);
}

Temps readAllTemps() {
  Temps x;
  x.termopar1 = tc1.readCelsius();
  x.termopar2 = tc2.readCelsius();
  x.termopar3 = tc3.readCelsius();
  x.termopar4 = tc4.readCelsius();

  x.dht11   = dht.readTemperature(); // °C
  x.dht11_h = dht.readHumidity();    // %

  return x;
}

String nowISO_CDMX() {
  struct tm t;
  if (!getLocalTime(&t, 2000)) {
    return String("1970-01-01T00:00:00");
  }
  char buf[32];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &t);
  return String(buf);
}

bool postToGoogleSheets(const Temps& v) {
  if (WiFi.status() != WL_CONNECTED) return false;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;
  if (!https.begin(client, GOOGLE_SCRIPT_URL)) {
    Serial.println("HTTPS begin() fallo");
    return false;
  }

  https.addHeader("Content-Type", "application/json");

  // Orden: fecha-hora, Termopar1..4, Ambiente, Humedad
  String payload = "{";
  payload += "\"ts_cdmx\":\"" + nowISO_CDMX() + "\",";
  payload += "\"termopar1\":" + f2s(v.termopar1, 2) + ",";
  payload += "\"termopar2\":" + f2s(v.termopar2, 2) + ",";
  payload += "\"termopar3\":" + f2s(v.termopar3, 2) + ",";
  payload += "\"termopar4\":" + f2s(v.termopar4, 2) + ",";
  payload += "\"ambiente\":"  + f2s(v.dht11, 2) + ",";
  payload += "\"humedad\":"   + f2s(v.dht11_h, 2) + ",";
  // compatibilidad (si tu script todavía usa estos nombres)
  payload += "\"dht11\":"     + f2s(v.dht11, 2) + ",";
  payload += "\"dht11_h\":"   + f2s(v.dht11_h, 2);
  payload += "}";

  int code = https.POST(payload);
  String resp = https.getString();
  https.end();

  Serial.printf("POST Sheets code=%d\n", code);
  Serial.println(resp);

  return (code >= 200 && code < 400);
}

const char LOGO_MEZCAL_BASE64[] PROGMEM ="data:image/jpeg;base64,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";


const char INDEX_HTML[] PROGMEM =
"<!doctype html>\n"
"<html lang='es'>\n"
"<head>\n"
"<meta charset='utf-8'>\n"
"<meta name='viewport' content='width=device-width, initial-scale=1'>\n"
"<title>Página de monitoreo</title>\n"
"<style>\n"
"body{font-family:Arial;margin:0;background:#f2f5f9;color:#222;font-weight:bold}\n"
".header{background:white;padding:18px;text-align:center;box-shadow:0 2px 8px rgba(0,0,0,0.1)}\n"
".card{background:white;margin:20px auto;padding:20px;border-radius:14px;max-width:1200px;box-shadow:0 4px 14px rgba(0,0,0,0.12)}\n"
"table{width:100%;border-collapse:collapse;margin-bottom:20px}\n"
"th{background:#0b3d91;color:white;padding:12px}\n"
"td{padding:10px;border-bottom:1px solid #ddd}\n"
"td:first-child{font-weight:bold}\n"
"td:last-child{text-align:center;font-weight:bold;font-size:16px}\n"
".grid{display:grid;grid-template-columns:1fr;gap:18px}\n"
"@media(min-width:900px){.grid{grid-template-columns:1fr 1fr}}\n"
".panel{background:white;border-radius:12px;padding:10px;box-shadow:0 2px 8px rgba(0,0,0,0.12)}\n"
".panel h2{margin:6px 0 8px 0;color:#0b3d91;font-size:16px}\n"
"canvas{background:white;border-radius:10px}\n"
".ok{color:green;font-weight:bold}\n"
".bad{color:red;font-weight:bold}\n"
".footer{margin-top:10px;font-size:13px;color:#444}\n"
".header{display:flex;align-items:center;justify-content:space-between;margin-bottom:20px}\n"
".logo{height:80px}\n"
".titulo{text-align:center;flex:1}\n"
".btnTop{display:inline-block;padding:14px 18px;border-radius:14px;background:#6f2dbd;color:#fff;text-decoration:none;font-weight:900;font-size:16px;box-shadow:0 6px 16px rgba(0,0,0,0.18);white-space:nowrap}\n"
".btnTop:hover{opacity:0.92}\n"
"</style>\n"
"<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>\n"
"</head>\n"
"<body>\n"
"<div class='header' style='display:flex; align-items:center; padding:10px 20px; background:white; box-shadow:0 2px 8px rgba(0,0,0,0.1);'>\n"
"  <img src='%%LOGO_MEZCAL%%' class='logo' style='height:75px; margin-right:20px;'>\n"
"  <div class='titulo' style='text-align:left; flex-grow:1;'>\n"
"    <h1 style='margin:0; font-size:22px; color:#0b3d91;'>Sistema de Monitoreo de Temperaturas</h1>\n"
"    <p style='margin:0; font-size:14px; color:#333; font-weight:normal;'>Proceso de Destilación de Mezcal ubicado en la región de Izúcar de Matamoros, Puebla</p>\n"
"  </div>\n"
"  <a class='btnTop' href='https://docs.google.com/spreadsheets/d/1mGIOaRmM01Vyr_NIWpMOMjd-v-6aj02yV6WXn38rERQ/edit?usp=sharing' target='_blank' rel='noopener'>DATOS GUARDADOS</a>\n"
"  <div style='display:flex; align-items:center; gap:2px;'>\n"
"    <img src='https://www.uppuebla.edu.mx/wp-content/uploads/2022/08/LOGO-UPP-HD.png' class='logo' style='height:75px;'>\n"
"    <img src='https://th.bing.com/th/id/OIP.Bzl8Cne1qy6vXcEwhN_jtgHaHd?w=158&h=180&c=7&r=0&o=7&cb=defcachec2&pid=1.7&rm=3' class='logo' style='height:75px;'>\n"
"  </div>\n"
"</div>\n"
"<div class='card'>\n"
"<table>\n"
"<thead><tr><th>Tipo de Sensor</th><th>Temperatura Actual en °C</th></tr></thead>\n"
"<tbody>\n"
"<tr><td>Termopar 1</td><td id='t1'>--</td></tr>\n"
"<tr><td>Termopar 2</td><td id='t2'>--</td></tr>\n"
"<tr><td>Termopar 3</td><td id='t3'>--</td></tr>\n"
"<tr><td>Termopar 4</td><td id='t4'>--</td></tr>\n"
"<tr><td>Ambiente</td><td id='dhtT'>--</td></tr>\n"
"<tr><td>Humedad</td><td id='dhtH'>--</td></tr>\n"
"</tbody>\n"
"</table>\n"
"<div class='grid'>\n"
"<div class='panel'><h2>Termopar 1</h2><canvas id='c1'></canvas></div>\n"
"<div class='panel'><h2>Termopar 2</h2><canvas id='c2'></canvas></div>\n"
"<div class='panel'><h2>Termopar 3</h2><canvas id='c3'></canvas></div>\n"
"<div class='panel'><h2>Termopar 4</h2><canvas id='c4'></canvas></div>\n"
"<div class='panel'><h2>Temperatura</h2><canvas id='c5'></canvas></div>\n"
"<div class='panel'><h2>Humedad</h2><canvas id='c8'></canvas></div>\n"
"</div>\n"
"<div class='footer'>Estado de la conexión: <span id='st' class='bad'>Sin datos</span> | "
"IP actual en caso de querer acceder desde otro dispositivo conectado a la misma red: <span id='ip'>--</span></div>\n"
"</div>\n"
"<script>\n"
"const el=id=>document.getElementById(id);\n"
"function mkChart(id,label,color){\n"
"  const ctx=document.getElementById(id).getContext('2d');\n"
"  return new Chart(ctx,{type:'line',data:{labels:[],datasets:[{label:label,data:[],borderColor:color,backgroundColor:color,tension:0.25,pointRadius:0}]},options:{responsive:true,animation:false,plugins:{legend:{labels:{font:{size:14,weight:'bold'}}}},scales:{x:{ticks:{font:{size:12,weight:'bold'}}},y:{ticks:{font:{size:12,weight:'bold'}}}}}});\n"
"}\n"
"const ch1=mkChart('c1','T1','#e74c3c');\n"
"const ch2=mkChart('c2','T2','#3498db');\n"
"const ch3=mkChart('c3','T3','#2ecc71');\n"
"const ch4=mkChart('c4','T4','#f1c40f');\n"
"const ch5=mkChart('c5','Temperatura','#9b59b6');\n"
"const ch8=mkChart('c8','Humedad','#34495e');\n"
"function push(ch,val){const hora=new Date().toLocaleTimeString();ch.data.labels.push(hora);ch.data.datasets[0].data.push(val);if(ch.data.labels.length>300){ch.data.labels.shift();ch.data.datasets[0].data.shift();}ch.update();}\n"
"async function tick(){try{const r=await fetch('/api/temps');const j=await r.json();\n"
"  el('t1').textContent=j.termopar1;el('t2').textContent=j.termopar2;el('t3').textContent=j.termopar3;el('t4').textContent=j.termopar4;\n"
"  el('dhtT').textContent=j.dht11;el('dhtH').textContent=j.dht11_h;\n"
"  el('st').textContent='OK';el('st').className='ok';el('ip').textContent=j.ip;\n"
"  push(ch1,j.termopar1);push(ch2,j.termopar2);push(ch3,j.termopar3);push(ch4,j.termopar4);\n"
"  push(ch5,j.dht11);push(ch8,j.dht11_h);\n"
"}catch(e){el('st').textContent='Error';}}\n"
"setInterval(tick,1000);\n"
"tick();\n"
"</script>\n"
"</body>\n"
"</html>\n";

void handleRoot() {
  String html = String(INDEX_HTML);
  html.replace("%%LOGO_MEZCAL%%", String(LOGO_MEZCAL_BASE64));
  server.send(200, "text/html; charset=utf-8", html);
}

void handleTemps() {
  Temps v = readAllTemps();

  String json = "{";
  json += "\"termopar1\":" + f2s(v.termopar1, 2) + ",";
  json += "\"termopar2\":" + f2s(v.termopar2, 2) + ",";
  json += "\"termopar3\":" + f2s(v.termopar3, 2) + ",";
  json += "\"termopar4\":" + f2s(v.termopar4, 2) + ",";
  json += "\"dht11\":"    + f2s(v.dht11, 2) + ",";
  json += "\"dht11_h\":"  + f2s(v.dht11_h, 2) + ",";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
  json += "}";
  server.send(200, "application/json; charset=utf-8", json);
}

void setup() {
  Serial.begin(115200);
  delay(200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Conectando WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi conectado. IP: ");
  Serial.println(WiFi.localIP());

  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
  Serial.print("Sincronizando NTP");
  time_t nowT = time(nullptr);
  int tries = 0;
  while (nowT < 1700000000 && tries < 40) {
    delay(250);
    Serial.print(".");
    nowT = time(nullptr);
    tries++;
  }
  Serial.println();
  Serial.println("NTP listo");

  dht.begin();
  delay(500);

  server.on("/", handleRoot);
  server.on("/api/temps", handleTemps);
  server.begin();

  Serial.println("Servidor web iniciado en puerto 80");
  Serial.println("Web: http://" + WiFi.localIP().toString() + "/");

  lastLogMs = millis() - (LOG_PERIOD_MS - 3000);
}

void loop() {
  server.handleClient();

  unsigned long now = millis();
  if (now - lastLogMs >= LOG_PERIOD_MS) {
    lastLogMs = now;

    Temps v = readAllTemps();
    Serial.println("Guardando muestra (10 min) a Google Sheets...");
    Serial.print("ts_cdmx = ");
    Serial.println(nowISO_CDMX());

    bool ok = postToGoogleSheets(v);
    Serial.println(ok ? "OK guardado" : "Fallo guardado");
  }
}
