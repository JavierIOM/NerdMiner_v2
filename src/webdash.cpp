#ifdef WEB_DASHBOARD

#include <WebServer.h>
#include <WiFi.h>
#include "webdash.h"
#include "monitor.h"
#include "drivers/storage/storage.h"

extern TSettings Settings;
extern unsigned long start;

static WebServer dashServer(80);

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
.g{display:grid;gap:8px}
.c{background:#141414;border:1px solid #222;border-radius:6px;padding:12px}
.l{color:#666;font-size:.75em;text-transform:uppercase;letter-spacing:1px}
.v{color:#00e676;font-size:1.3em;font-weight:bold;margin-top:4px}
.h{color:#00bcd4}
.s{font-size:.7em;color:#444;text-align:center;margin-top:16px;padding-top:12px;border-top:1px solid #1a1a1a}
.cfg{margin-top:12px}
.cfg .c{border-color:#1a1a1a}
.cfg .v{color:#888;font-size:.9em;word-break:break-all}
</style>
</head>
<body>
<h1>NerdMiner Dashboard</h1>
<div class="g" id="s"></div>
<div class="cfg">
<div class="g" id="cfg"></div>
</div>
<div class="s">Auto-refreshes every 2s</div>
<script>
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
}).catch(()=>{});
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
    json += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
    json += "}";

    dashServer.send(200, "application/json", json);
}

void init_WebDashboard() {
    dashServer.on("/", HTTP_GET, handleRoot);
    dashServer.on("/api/stats", HTTP_GET, handleApiStats);
    dashServer.begin();
    Serial.println(">> Web dashboard started at http://" + WiFi.localIP().toString());
}

void webDashboardProcess() {
    dashServer.handleClient();
}

#endif // WEB_DASHBOARD
