#ifdef WEB_DASHBOARD

#include <WebServer.h>
#include <WiFi.h>
#include "webdash.h"
#include "monitor.h"
#include "drivers/storage/storage.h"
#include "drivers/storage/nvMemory.h"
#include <ArduinoJson.h>
#include <ESPmDNS.h>

extern TSettings Settings;
extern unsigned long start;

static WebServer dashServer(80);
static nvMemory dashNvMem;

static const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>NerdMiner</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:'Courier New',monospace;background:#0a0a0a;color:#e0e0e0;padding:16px;max-width:480px;margin:0 auto}
h1{color:#00e676;font-size:1.4em;padding:12px 0;border-bottom:1px solid #1a1a1a;margin-bottom:12px}
h2{color:#00e676;font-size:1.1em;padding:10px 0 6px}
.g{display:grid;gap:8px}
.c{background:#141414;border:1px solid #222;border-radius:6px;padding:12px}
.l{color:#666;font-size:.75em;text-transform:uppercase;letter-spacing:1px}
.v{color:#00e676;font-size:1.3em;font-weight:bold;margin-top:4px}
.h{color:#00bcd4}
.s{font-size:.7em;color:#444;text-align:center;margin-top:16px;padding-top:12px;border-top:1px solid #1a1a1a}
.cfg{margin-top:12px}
.cfg .c{border-color:#1a1a1a}
.cfg .v{color:#888;font-size:.9em;word-break:break-all}
.f{margin-top:16px;background:#141414;border:1px solid #222;border-radius:6px;padding:16px}
.f label{color:#666;font-size:.75em;text-transform:uppercase;letter-spacing:1px;display:block;margin-bottom:4px;margin-top:10px}
.f label:first-child{margin-top:0}
.f input{width:100%;background:#0a0a0a;border:1px solid #333;border-radius:4px;padding:8px;color:#e0e0e0;font-family:'Courier New',monospace;font-size:.9em}
.f input:focus{outline:none;border-color:#00e676}
.btn{background:#00e676;color:#0a0a0a;border:none;border-radius:4px;padding:10px 20px;font-family:'Courier New',monospace;font-weight:bold;font-size:.9em;cursor:pointer;margin-top:14px;width:100%}
.btn:hover{background:#00c864}
.btn:disabled{background:#333;color:#666;cursor:not-allowed}
.msg{margin-top:8px;padding:8px;border-radius:4px;font-size:.85em;display:none}
.msg.ok{display:block;background:#0a2010;color:#00e676;border:1px solid #00e676}
.msg.err{display:block;background:#200a0a;color:#e65100;border:1px solid #e65100}
</style>
</head>
<body>
<h1>NerdMiner Dashboard</h1>
<div class="g" id="s"></div>
<div class="cfg">
<div class="g" id="cfg"></div>
</div>
<div class="f">
<h2>Settings</h2>
<div id="msg"></div>
<label>BTC Wallet Address</label>
<input type="text" id="wallet" maxlength="79">
<label>Pool URL</label>
<input type="text" id="pool" maxlength="79">
<label>Pool Port</label>
<input type="number" id="port" min="1" max="65535">
<label>Pool Password</label>
<input type="text" id="pass" maxlength="79">
<label>Timezone (UTC offset)</label>
<input type="number" id="tz" min="-12" max="12">
<button class="btn" id="sv" onclick="sv()">Save & Restart</button>
</div>
<div class="s">Auto-refreshes every 2s</div>
<script>
var loaded=false;
function u(){
fetch('/api/stats').then(r=>r.json()).then(d=>{
document.getElementById('s').innerHTML=
'<div class="c"><div class="l">Hashrate</div><div class="v h">'+d.hashrate+' KH/s</div></div>'+
'<div class="c"><div class="l">Shares</div><div class="v">'+d.shares+'</div></div>'+
'<div class="c"><div class="l">Valid Blocks</div><div class="v">'+d.valids+'</div></div>'+
'<div class="c"><div class="l">Best Difficulty</div><div class="v">'+d.bestdiff+'</div></div>'+
'<div class="c"><div class="l">Block Templates</div><div class="v">'+d.templates+'</div></div>'+
'<div class="c"><div class="l">Temperature</div><div class="v">'+d.temp+'</div></div>'+
'<div class="c"><div class="l">Total Hashes</div><div class="v">'+d.totalmh+' MH</div></div>'+
'<div class="c"><div class="l">Uptime</div><div class="v">'+d.uptime+'</div></div>';
document.getElementById('cfg').innerHTML=
'<div class="c"><div class="l">Pool</div><div class="v">'+d.pool+':'+d.port+'</div></div>'+
'<div class="c"><div class="l">Wallet</div><div class="v">'+d.wallet+'</div></div>'+
'<div class="c"><div class="l">IP Address</div><div class="v">'+d.ip+'</div></div>';
if(!loaded){
document.getElementById('wallet').value=d.wallet;
document.getElementById('pool').value=d.pool;
document.getElementById('port').value=d.port;
document.getElementById('pass').value=d.pass;
document.getElementById('tz').value=d.tz;
loaded=true;
}
}).catch(()=>{});
}
function sv(){
var b=document.getElementById('sv');
b.disabled=true;b.textContent='Saving...';
var body=JSON.stringify({
wallet:document.getElementById('wallet').value,
pool:document.getElementById('pool').value,
port:parseInt(document.getElementById('port').value),
pass:document.getElementById('pass').value,
tz:parseInt(document.getElementById('tz').value)
});
fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:body})
.then(r=>r.json()).then(d=>{
var m=document.getElementById('msg');
if(d.ok){m.className='msg ok';m.textContent='Saved! Restarting device...';}
else{m.className='msg err';m.textContent='Error: '+d.error;b.disabled=false;b.textContent='Save & Restart';}
}).catch(()=>{
var m=document.getElementById('msg');
m.className='msg err';m.textContent='Connection lost (device may be restarting)';
});
}
setInterval(u,2000);u();
</script>
</body>
</html>
)rawliteral";

static void handleRoot() {
    dashServer.send_P(200, "text/html", DASHBOARD_HTML);
}

static void handleApiStats() {
    unsigned long mElapsed = millis() - start;
    mining_data data = getMiningData(mElapsed);

    String json = "{";
    json += "\"hashrate\":\"" + data.currentHashRate + "\",";
    json += "\"shares\":\"" + data.completedShares + "\",";
    json += "\"valids\":\"" + data.valids + "\",";
    json += "\"bestdiff\":\"" + data.bestDiff + "\",";
    json += "\"templates\":\"" + data.templates + "\",";
    json += "\"temp\":\"" + data.temp + "\",";
    json += "\"totalmh\":\"" + data.totalMHashes + "\",";
    json += "\"uptime\":\"" + data.timeMining + "\",";
    json += "\"pool\":\"" + Settings.PoolAddress + "\",";
    json += "\"port\":" + String(Settings.PoolPort) + ",";
    json += "\"wallet\":\"" + String(Settings.BtcWallet) + "\",";
    json += "\"pass\":\"" + String(Settings.PoolPassword) + "\",";
    json += "\"tz\":" + String(Settings.Timezone) + ",";
    json += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
    json += "}";

    dashServer.send(200, "application/json", json);
}

static void handleApiConfigPost() {
    String body = dashServer.arg("plain");

    DynamicJsonDocument doc(512);
    DeserializationError err = deserializeJson(doc, body);
    if (err) {
        dashServer.send(200, "application/json", "{\"ok\":false,\"error\":\"Invalid JSON\"}");
        return;
    }

    String wallet = doc["wallet"] | "";
    String pool = doc["pool"] | "";
    int port = doc["port"] | 0;
    String pass = doc["pass"] | "x";
    int tz = doc["tz"] | 0;

    if (wallet.length() == 0 || pool.length() == 0 || port == 0) {
        dashServer.send(200, "application/json", "{\"ok\":false,\"error\":\"Missing required fields\"}");
        return;
    }

    // Update settings
    strncpy(Settings.BtcWallet, wallet.c_str(), sizeof(Settings.BtcWallet) - 1);
    Settings.BtcWallet[sizeof(Settings.BtcWallet) - 1] = '\0';
    Settings.PoolAddress = pool;
    Settings.PoolPort = port;
    strncpy(Settings.PoolPassword, pass.c_str(), sizeof(Settings.PoolPassword) - 1);
    Settings.PoolPassword[sizeof(Settings.PoolPassword) - 1] = '\0';
    Settings.Timezone = tz;

    // Save to flash
    dashNvMem.saveConfig(&Settings);

    dashServer.send(200, "application/json", "{\"ok\":true}");

    // Restart after a short delay to let the response send
    delay(500);
    ESP.restart();
}

void init_WebDashboard() {
    WiFi.setHostname("nerdminer");
    if (MDNS.begin("nerdminer")) {
        MDNS.addService("http", "tcp", 80);
        Serial.println(">> mDNS started: http://nerdminer.local");
    }
    dashServer.on("/", HTTP_GET, handleRoot);
    dashServer.on("/api/stats", HTTP_GET, handleApiStats);
    dashServer.on("/api/config", HTTP_POST, handleApiConfigPost);
    dashServer.begin();
    Serial.println(">> Web dashboard started at http://" + WiFi.localIP().toString());
}

void webDashboardProcess() {
    dashServer.handleClient();
}

#endif // WEB_DASHBOARD
