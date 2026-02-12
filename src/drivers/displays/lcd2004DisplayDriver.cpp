#include "displayDriver.h"

#ifdef LCD_2004_DISPLAY

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include "monitor.h"
#include "drivers/storage/storage.h"

extern monitor_data mMonitor;
extern TSettings Settings;

static LiquidCrystal_I2C lcd(LCD_I2C_ADDR, 20, 4);
static bool backlightOn = true;
static bool ledOn = true;
static unsigned long previousLedMillis = 0;
static unsigned long previousScreenMillis = 0;
#define SCREEN_CYCLE_MS 10000

// Helper to pad and print a 20-char line
static void printLine(int row, const String& text)
{
    lcd.setCursor(0, row);
    String padded = text;
    while (padded.length() < 20) padded += " ";
    lcd.print(padded.substring(0, 20));
}

void lcd2004_Init(void)
{
    Wire.begin(SDA_PIN, SCL_PIN);
    lcd.init();
    lcd.backlight();
    lcd.clear();
    Serial.println("LCD 2004 display initialized");
    pinMode(LED_PIN, OUTPUT);
}

void lcd2004_AlternateScreenState(void)
{
    backlightOn = !backlightOn;
    if (backlightOn) {
        lcd.backlight();
    } else {
        lcd.noBacklight();
    }
    ledOn = !ledOn;
}

void lcd2004_AlternateRotation(void)
{
    // No rotation for character LCD
}

void lcd2004_LoadingScreen(void)
{
    lcd.clear();
    lcd.setCursor(3, 1);
    lcd.print("NerdMiner v2");
    lcd.setCursor(4, 2);
    lcd.print("Starting...");
}

void lcd2004_SetupScreen(void)
{
    lcd.clear();
    lcd.setCursor(4, 1);
    lcd.print("Setup Mode");
    lcd.setCursor(3, 2);
    lcd.print("Connect WiFi");
}

// Screen 1: Mining stats
// Hash: 352.5 KH/s
// Temp: 53C  Tmpl: 12
// Best: 0.003500
// Share:0    Valid:0
void lcd2004_MiningScreen(unsigned long mElapsed)
{
    mining_data data = getMiningData(mElapsed);

    printLine(0, "Hash: " + data.currentHashRate + " KH/s");
    printLine(1, "Temp: " + data.temp + "C  Tmpl:" + data.templates);
    printLine(2, "Best: " + data.bestDiff);
    printLine(3, "Share:" + data.completedShares + " Valid:" + data.valids);
}

// Screen 2: Pool info and uptime
// public-pool.io
// Port: 21496
// Up: 0d 01:23:45
// Share:0    Valid:0
void lcd2004_PoolScreen(unsigned long mElapsed)
{
    mining_data data = getMiningData(mElapsed);

    String pool = Settings.PoolAddress;
    printLine(0, pool);
    printLine(1, "Port: " + String(Settings.PoolPort));
    printLine(2, "Up: " + data.timeMining);
    printLine(3, "Share:" + data.completedShares + " Valid:" + data.valids);
}

// Screen 3: Network info
// WiFi: MyNetworkSSID
// IP: 192.168.138.108
// nerdminer.local
// RSSI: -65 dBm
void lcd2004_NetworkScreen(unsigned long mElapsed)
{
    printLine(0, "WiFi:" + WiFi.SSID());
    printLine(1, "IP:" + WiFi.localIP().toString());
    printLine(2, "nerdminer.local");
    printLine(3, "RSSI:" + String(WiFi.RSSI()) + " dBm");
}

void lcd2004_DoLedStuff(unsigned long frame)
{
    unsigned long currentMillis = millis();

    if (!ledOn) {
        digitalWrite(LED_PIN, INACTIVE_LED);
        return;
    }

    switch (mMonitor.NerdStatus) {
    case NM_waitingConfig:
        digitalWrite(LED_PIN, ACTIVE_LED);
        break;
    case NM_Connecting:
        if (currentMillis - previousLedMillis >= 500) {
            previousLedMillis = currentMillis;
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        }
        break;
    case NM_hashing:
        if (currentMillis - previousLedMillis >= 100) {
            previousLedMillis = currentMillis;
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        }
        break;
    }
}

void lcd2004_AnimateCurrentScreen(unsigned long frame)
{
    unsigned long currentMillis = millis();
    if (currentMillis - previousScreenMillis >= SCREEN_CYCLE_MS) {
        previousScreenMillis = currentMillis;
        lcd2004DisplayDriver.current_cyclic_screen =
            (lcd2004DisplayDriver.current_cyclic_screen + 1) %
            lcd2004DisplayDriver.num_cyclic_screens;
    }
}

CyclicScreenFunction lcd2004CyclicScreens[] = {
    lcd2004_MiningScreen,
    lcd2004_PoolScreen,
    lcd2004_NetworkScreen
};

DisplayDriver lcd2004DisplayDriver = {
    lcd2004_Init,
    lcd2004_AlternateScreenState,
    lcd2004_AlternateRotation,
    lcd2004_LoadingScreen,
    lcd2004_SetupScreen,
    lcd2004CyclicScreens,
    lcd2004_AnimateCurrentScreen,
    lcd2004_DoLedStuff,
    SCREENS_ARRAY_SIZE(lcd2004CyclicScreens),
    0,
    20,
    4,
};

#endif // LCD_2004_DISPLAY
