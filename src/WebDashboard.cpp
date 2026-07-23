#include "WebDashboard.h"

#include <ESPmDNS.h>
#include <WiFi.h>

#include <algorithm>

#include "AudioEventNames.h"
#include "Logger.h"
#include "ProjectConfig.h"
#include "WifiSecrets.h"

namespace vibhearing {
namespace {

constexpr char kDashboardHtml[] PROGMEM = R"HTML(
<!doctype html><html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>VibHearing</title><style>
:root{color-scheme:dark;--bg:#07111f;--card:#102036;--line:#23415f;--cyan:#55d8ff;--green:#68e0a0;--text:#eaf6ff;--muted:#91a8ba}
*{box-sizing:border-box}body{margin:0;background:linear-gradient(140deg,#07111f,#0d1c2e);color:var(--text);font:14px system-ui,sans-serif}
header{padding:24px max(18px,5vw);display:flex;justify-content:space-between;align-items:center}h1{margin:0;font-size:25px}h2{font-size:16px;margin:0 0 14px}.badge{padding:7px 11px;border:1px solid var(--line);border-radius:99px;color:var(--muted)}
main{width:min(1200px,92vw);margin:auto;display:grid;grid-template-columns:repeat(12,1fr);gap:14px;padding-bottom:40px}.card{background:#102036e8;border:1px solid var(--line);border-radius:15px;padding:17px;box-shadow:0 12px 35px #0004}.metrics{grid-column:span 12;display:grid;grid-template-columns:repeat(6,1fr);gap:10px}.metric{background:#0a1728;border-radius:11px;padding:13px}.metric small{display:block;color:var(--muted);margin-bottom:5px}.metric strong{font-size:20px;color:var(--cyan)}.wave{grid-column:span 8}.bands{grid-column:span 4}.settings{grid-column:span 8}.device{grid-column:span 4}canvas{width:100%;height:230px;background:#071321;border-radius:10px}.band-row{display:grid;grid-template-columns:72px 1fr 50px;gap:8px;align-items:center;margin:11px 0}.track{height:10px;background:#071321;border-radius:8px;overflow:hidden}.fill{height:100%;background:linear-gradient(90deg,var(--green),var(--cyan));width:0}.muted{color:var(--muted)}form{display:grid;grid-template-columns:repeat(2,1fr);gap:13px}label span{display:block;color:var(--muted);margin-bottom:5px}input{width:100%;background:#071321;color:var(--text);border:1px solid var(--line);padding:9px;border-radius:8px}label.check{display:flex;align-items:center;gap:9px}.check input{width:auto}button{border:0;border-radius:9px;padding:11px;background:var(--cyan);color:#04121c;font-weight:700;cursor:pointer}.full{grid-column:1/-1}dl{display:grid;grid-template-columns:1fr 1fr;gap:8px;margin:0}dt{color:var(--muted)}dd{margin:0;text-align:right}#message{min-height:20px;color:var(--green)}
@media(max-width:800px){.wave,.bands,.settings,.device{grid-column:span 12}.metrics{grid-template-columns:repeat(2,1fr)}form{grid-template-columns:1fr}}
</style></head><body><header><div><h1>VibHearing</h1><div class="muted">Acoustic telemetry and haptic control</div></div><div class="badge" id="connection">Connecting…</div></header><main>
<section class="card metrics"><div class="metric"><small>Event</small><strong id="event">—</strong></div><div class="metric"><small>Confidence</small><strong id="confidence">0%</strong></div><div class="metric"><small>SNR</small><strong id="snr">0.00</strong></div><div class="metric"><small>RMS</small><strong id="rms">0</strong></div><div class="metric"><small>Peak</small><strong id="peak">0</strong></div><div class="metric"><small>Haptic strength</small><strong id="strength">0%</strong></div></section>
<section class="card wave"><h2>Raw audio waveform</h2><canvas id="waveform" width="900" height="230"></canvas><p class="muted">256 normalized samples from the latest I2S frame; the view scales automatically.</p></section>
<section class="card bands"><h2>FFT frequency bands</h2><div id="bandList"></div></section>
<section class="card settings"><h2>Live settings</h2><form id="settingsForm">
<label class="check"><input type="checkbox" id="motorEnabled"><span>Motor enabled</span></label><label class="check"><input type="checkbox" id="serialMetricsEnabled"><span>Serial telemetry</span></label>
<label><span>Minimum voice SNR</span><input id="minimumVoiceSnr" type="number" min="1" max="10" step="0.05"></label><label><span>Minimum voice ratio</span><input id="minimumVoiceRatio" type="number" min="0.1" max="0.9" step="0.01"></label>
<label><span>Close voice SNR</span><input id="closeVoiceSnr" type="number" min="2" max="30" step="0.1"></label><label><span>Minimum confidence</span><input id="minimumEventConfidence" type="number" min="0.5" max="0.99" step="0.01"></label>
<label><span>Minimum motor PWM</span><input id="minimumHapticDuty" type="number" min="0" max="180" step="1"></label><label><span>Maximum motor PWM</span><input id="maximumHapticDuty" type="number" min="0" max="180" step="1"></label>
<label><span>Retrigger guard (ms)</span><input id="eventRetriggerGuardMs" type="number" min="200" max="5000" step="10"></label><button type="submit">Apply settings</button><div class="full" id="message"></div>
</form></section>
<section class="card device"><h2>Device</h2><dl><dt>IP</dt><dd id="ip">—</dd><dt>Wi-Fi RSSI</dt><dd id="rssi">—</dd><dt>Uptime</dt><dd id="uptime">—</dd><dt>Sample rate</dt><dd>16 kHz</dd><dt>FFT</dt><dd>256 samples</dd><dt>Motor pin</dt><dd>GPIO7</dd><dt>I2S</dt><dd>4 / 5 / 6</dd></dl></section>
</main><script>
const bands=['60–250','250–500','500–1k','1–2k','2–4k','4–8k'];const list=document.getElementById('bandList');bands.forEach((n,i)=>list.insertAdjacentHTML('beforeend',`<div class="band-row"><span>${n}</span><div class="track"><div class="fill" id="b${i}"></div></div><span id="bv${i}">0</span></div>`));
const set=(id,v)=>document.getElementById(id).textContent=v;function drawWave(a){const c=document.getElementById('waveform'),x=c.getContext('2d'),w=c.width,h=c.height;x.clearRect(0,0,w,h);x.strokeStyle='#23415f';x.beginPath();x.moveTo(0,h/2);x.lineTo(w,h/2);x.stroke();const max=Math.max(1,...a.map(Math.abs));x.strokeStyle='#55d8ff';x.lineWidth=2;x.beginPath();a.forEach((v,i)=>{const px=i*(w/(a.length-1)),py=h/2-(v/max)*(h*.42);i?x.lineTo(px,py):x.moveTo(px,py)});x.stroke()}
async function poll(){try{const r=await fetch('/api/status',{cache:'no-store'}),d=await r.json();set('connection',d.wifi?'Online':'Waiting for Wi-Fi');set('event',d.event);set('confidence',Math.round(d.confidence*100)+'%');set('strength',Math.round(d.strength*100)+'%');set('snr',d.snr.toFixed(2));set('rms',d.rms.toFixed(5));set('peak',d.peak.toFixed(5));set('ip',d.ip);set('rssi',d.rssi+' dBm');set('uptime',Math.floor(d.uptime/60)+' min');d.bands.forEach((v,i)=>{set('bv'+i,v.toFixed(3));document.getElementById('b'+i).style.width=Math.min(100,Math.log10(1+v*18)*58)+'%'});drawWave(d.waveform)}catch(e){set('connection','Connection lost')}setTimeout(poll,250)}
async function loadSettings(){const d=await(await fetch('/api/settings')).json();Object.entries(d).forEach(([k,v])=>{const e=document.getElementById(k);if(!e)return;e.type==='checkbox'?e.checked=v:e.value=v})}
document.getElementById('settingsForm').addEventListener('submit',async e=>{e.preventDefault();const p=new URLSearchParams();['motorEnabled','serialMetricsEnabled'].forEach(k=>p.set(k,document.getElementById(k).checked?'true':'false'));['minimumVoiceSnr','minimumVoiceRatio','closeVoiceSnr','minimumEventConfidence','minimumHapticDuty','maximumHapticDuty','eventRetriggerGuardMs'].forEach(k=>p.set(k,document.getElementById(k).value));const r=await fetch('/api/settings',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:p});set('message',r.ok?'Settings applied':'Settings error');await loadSettings()});loadSettings();poll();
</script></body></html>
)HTML";

bool parseBoolean(const String& value) { return value == "true" || value == "1"; }

}  // namespace

WebDashboard::WebDashboard(RuntimeSettings& settings) : settings_(settings) {}

void WebDashboard::begin() {
  registerRoutes();
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(config::kDeviceHostname);
  WiFi.begin(secrets::kWifiSsid, secrets::kWifiPassword);
  connectionAttemptMs_ = millis();
  Logger::info("Wi-Fi connection started");
}

void WebDashboard::update() {
  startServerIfReady();
  reconnectIfNeeded();
  if (serverStarted_) {
    server_.handleClient();
  }
}

void WebDashboard::updateAudio(const AudioFeatures& features,
                               const AudioEvent& event) {
  latestFeatures_ = features;
  latestEvent_ = event;
  hasAudio_ = true;
}

bool WebDashboard::connected() const { return WiFi.status() == WL_CONNECTED; }

void WebDashboard::registerRoutes() {
  server_.on("/", HTTP_GET, [this]() { sendDashboard(); });
  server_.on("/api/status", HTTP_GET, [this]() { sendStatus(); });
  server_.on("/api/settings", HTTP_GET, [this]() { sendSettings(); });
  server_.on("/api/settings", HTTP_POST, [this]() { applySettings(); });
  server_.onNotFound([this]() { server_.send(404, "application/json", "{\"error\":\"not_found\"}"); });
}

void WebDashboard::startServerIfReady() {
  if (serverStarted_ || !connected()) {
    return;
  }
  server_.begin();
  serverStarted_ = true;
  if (MDNS.begin(config::kDeviceHostname)) {
    MDNS.addService("http", "tcp", 80);
  }
  Serial.printf("[INFO] Dashboard ready: http://%s/ or http://%s.local/\n",
                WiFi.localIP().toString().c_str(), config::kDeviceHostname);
}

void WebDashboard::reconnectIfNeeded() {
  if (connected() ||
      millis() - connectionAttemptMs_ < config::kWifiConnectionTimeoutMs) {
    return;
  }
  WiFi.disconnect();
  WiFi.begin(secrets::kWifiSsid, secrets::kWifiPassword);
  connectionAttemptMs_ = millis();
  Logger::info("Wi-Fi reconnecting");
}

void WebDashboard::sendDashboard() {
  server_.sendHeader("Cache-Control", "no-store");
  server_.send_P(200, "text/html; charset=utf-8", kDashboardHtml);
}

void WebDashboard::sendStatus() {
  server_.sendHeader("Cache-Control", "no-store");
  server_.send(200, "application/json", statusJson());
}

void WebDashboard::sendSettings() {
  server_.sendHeader("Cache-Control", "no-store");
  server_.send(200, "application/json", settingsJson());
}

void WebDashboard::applySettings() {
  if (server_.hasArg("motorEnabled")) {
    settings_.motorEnabled = parseBoolean(server_.arg("motorEnabled"));
  }
  if (server_.hasArg("serialMetricsEnabled")) {
    settings_.serialMetricsEnabled =
        parseBoolean(server_.arg("serialMetricsEnabled"));
  }
  if (server_.hasArg("minimumVoiceSnr")) {
    settings_.minimumVoiceSnr =
        std::clamp(server_.arg("minimumVoiceSnr").toFloat(), 1.0F, 10.0F);
  }
  if (server_.hasArg("minimumVoiceRatio")) {
    settings_.minimumVoiceRatio =
        std::clamp(server_.arg("minimumVoiceRatio").toFloat(), 0.10F, 0.90F);
  }
  if (server_.hasArg("closeVoiceSnr")) {
    settings_.closeVoiceSnr =
        std::clamp(server_.arg("closeVoiceSnr").toFloat(), 2.0F, 30.0F);
  }
  if (server_.hasArg("minimumEventConfidence")) {
    settings_.minimumEventConfidence = std::clamp(
        server_.arg("minimumEventConfidence").toFloat(), 0.50F, 0.99F);
  }
  if (server_.hasArg("minimumHapticDuty")) {
    settings_.minimumHapticDuty = static_cast<uint8_t>(std::clamp(
        server_.arg("minimumHapticDuty").toInt(), 0L,
        static_cast<long>(config::kMotorSafeMaximumDuty)));
  }
  if (server_.hasArg("maximumHapticDuty")) {
    settings_.maximumHapticDuty = static_cast<uint8_t>(std::clamp(
        server_.arg("maximumHapticDuty").toInt(), 0L,
        static_cast<long>(config::kMotorSafeMaximumDuty)));
  }
  if (settings_.minimumHapticDuty > settings_.maximumHapticDuty) {
    std::swap(settings_.minimumHapticDuty, settings_.maximumHapticDuty);
  }
  if (server_.hasArg("eventRetriggerGuardMs")) {
    settings_.eventRetriggerGuardMs = static_cast<uint32_t>(std::clamp(
        server_.arg("eventRetriggerGuardMs").toInt(), 200L, 5000L));
  }
  server_.send(200, "application/json", settingsJson());
}

String WebDashboard::statusJson() const {
  String json;
  json.reserve(3600);
  const float snr = latestFeatures_.rms /
                    std::max(latestFeatures_.noiseFloor, 0.000001F);
  json += "{\"wifi\":";
  json += connected() ? "true" : "false";
  json += ",\"ip\":\"" + WiFi.localIP().toString() + "\"";
  json += ",\"rssi\":" + String(connected() ? WiFi.RSSI() : 0);
  json += ",\"uptime\":" + String(millis() / 1000U);
  json += ",\"event\":\"";
  json += hasAudio_ ? audioEventName(latestEvent_.type) : "none";
  json += "\",\"confidence\":" + String(latestEvent_.confidence, 4);
  json += ",\"strength\":" + String(latestEvent_.acousticStrength, 4);
  json += ",\"rms\":" + String(latestFeatures_.rms, 7);
  json += ",\"peak\":" + String(latestFeatures_.peak, 7);
  json += ",\"gain\":" + String(latestFeatures_.gain, 3);
  json += ",\"noiseFloor\":" + String(latestFeatures_.noiseFloor, 7);
  json += ",\"snr\":" + String(snr, 4);
  json += ",\"bands\":[";
  for (size_t index = 0; index < latestFeatures_.bands.size(); ++index) {
    if (index > 0U) json += ',';
    json += String(latestFeatures_.bands[index], 5);
  }
  json += "],\"waveform\":[";
  for (size_t index = 0; index < latestFeatures_.waveform.size(); ++index) {
    if (index > 0U) json += ',';
    json += String(latestFeatures_.waveform[index]);
  }
  json += "]}";
  return json;
}

String WebDashboard::settingsJson() const {
  String json;
  json.reserve(400);
  json += "{\"motorEnabled\":";
  json += settings_.motorEnabled ? "true" : "false";
  json += ",\"serialMetricsEnabled\":";
  json += settings_.serialMetricsEnabled ? "true" : "false";
  json += ",\"minimumVoiceSnr\":" + String(settings_.minimumVoiceSnr, 2);
  json += ",\"minimumVoiceRatio\":" +
          String(settings_.minimumVoiceRatio, 2);
  json += ",\"closeVoiceSnr\":" + String(settings_.closeVoiceSnr, 2);
  json += ",\"minimumEventConfidence\":" +
          String(settings_.minimumEventConfidence, 2);
  json += ",\"minimumHapticDuty\":" +
          String(settings_.minimumHapticDuty);
  json += ",\"maximumHapticDuty\":" +
          String(settings_.maximumHapticDuty);
  json += ",\"eventRetriggerGuardMs\":" +
          String(settings_.eventRetriggerGuardMs) + "}";
  return json;
}

}  // namespace vibhearing
