#include <WiFi.h>
#include <NetworkClient.h>
#include <WiFiAP.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <ZMPT101B.h>

#include <ESP_Mail_Client.h>
#include <LiquidCrystal_PCF8574.h>
#include <Preferences.h>

#include <RTClib.h>
#include "INA226.h"

#include "EmonLib.h"
#include <HardwareSerial.h>
#include "VoiceRecognitionV3.h"

#define onRecord0 (0)
#define offRecord0 (1)
#define onRecord1 (2)
#define offRecord1 (3)
#define onRecord2 (4)
#define offRecord2 (5)
#define onRecord3 (6)
#define offRecord3 (7)
#define onRecord4 (8)
#define offRecord4 (9)

int group = 0;


HardwareSerial mySerial(2);
VR myVR(16, 17);  // 2:RX 3:TX

uint8_t records[2];
uint8_t buf[64];


EnergyMonitor emon1;
EnergyMonitor emon2;


INA226 INA(0x40);
INA226 INA2(0x41);

Preferences preferences;

LiquidCrystal_PCF8574 lcd(0x27);

#define SENSITIVITY 500.0f

#define PIN_TENSION_1 32
#define PIN_TENSION_2 34

ZMPT101B voltageSensor_1(PIN_TENSION_1, 60.0);
ZMPT101B voltageSensor_2(PIN_TENSION_2, 60.0);

RTC_DS3231 rtc;

const int PRISE_1 = 4;
const int PRISE_2 = 14;
const int PRISE_3 = 18;
const int PRISE_4 = 2;

float courantPrise1 = 0.0;
float tensionPrise1 = 0.0;
float puissancePrise1 = 0.0;
float puissanceMaxPrise1 = 18.0;
String heureDebutPrise1 = "";
String heureFinPrise1 = "";

float courantPrise2 = 0.0;
float tensionPrise2 = 0.0;
float puissancePrise2 = 0.0;
float puissanceMaxPrise2 = 0.0;
String heureDebutPrise2 = "";
String heureFinPrise2 = "";

float courantPrise3 = 0.0;
float tensionPrise3 = 0.0;
float puissancePrise3 = 0.0;
float puissanceMaxPrise3 = 0.0;
String heureDebutPrise3 = "";
String heureFinPrise3 = "";

float courantPrise4 = 0.0;
float tensionPrise4 = 0.0;
float puissancePrise4 = 0.0;
float puissanceMaxPrise4 = 0.0;
String heureDebutPrise4 = "";
String heureFinPrise4 = "";

String statutPrise1 = "Eteinte";
String statutPrise2 = "Eteinte";
String statutPrise3 = "Eteinte";
String statutPrise4 = "Eteinte";

const char* ssid = "yourAP";
const char* password = "yourPassword";

// Assane Dieye - Kodanne Coulibaly

// // const char* ssid = "iPhone de Assane";                
// // const char* password = " ";     

// WiFiServer server(80);  // Crée un serveur web sur le port 80
// // NetworkServer server(80);

// // Configuration SMTP
// #define SMTP_HOST "smtp.gmail.com"  // Serveur SMTP (ex : Gmail)
// #define SMTP_PORT 465  // 465 pour SSL, 587 pour STARTTLS

// // Identifiants Gmail
// #define AUTHOR_EMAIL " "
// #define AUTHOR_PASSWORD " "  
// #define RECIPIENT_EMAIL " "

// // Création d'une session SMTP
// SMTPSession smtp;


String html = "";

int cpt = 0;
int P_1 = 0;
int P_2 = 0;
int P_3 = 0;
int P_4 = 0;

int e_1 = 0;
int e_2 = 0;
int e_3 = 0;
int e_4 = 0;

int f_1 = 0;
int f_2 = 0;
int f_3 = 0;
int f_4 = 0;

NetworkServer server(80);




void setup() {

  Serial.begin(115200);

  preferences.begin("mesPrises", false);


  puissanceMaxPrise1 = preferences.getFloat("pMaxP1", 18.0);
  puissanceMaxPrise2 = preferences.getFloat("pMaxP2", 0.0);
  puissanceMaxPrise3 = preferences.getFloat("pMaxP3", 0.0);
  puissanceMaxPrise4 = preferences.getFloat("pMaxP4", 0.0);

  heureDebutPrise1 = preferences.getString("hDebutP1", "00:00");
  heureDebutPrise2 = preferences.getString("hDebutP2", "00:00");
  heureDebutPrise3 = preferences.getString("hDebutP3", "00:00");
  heureDebutPrise4 = preferences.getString("hDebutP4", "00:00");

  heureFinPrise1 = preferences.getString("hFinP1", "00:00");
  heureFinPrise2 = preferences.getString("hFinP2", "00:00");
  heureFinPrise3 = preferences.getString("hFinP3", "00:00");
  heureFinPrise4 = preferences.getString("hFinP4", "00:00");



  Wire.begin();
  lcd.begin(16, 2);
  lcd.setBacklight(1);


  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Bienvenue");
  lcd.setCursor(0, 1);
  lcd.print("Initialisation...");


  mySerial.begin(9600, SERIAL_8N1, 16, 17);
  delay(2000);
  myVR.begin(9600);
  delay(2000);
  Serial.println("Elechouse Voice Recognition V3 Module\r\nControl Prise1 sample");


  if (!rtc.begin()) {
    lcd.print("RTC introuvable");
    while (1)
      ;
  }

  if (rtc.lostPower()) {
    lcd.clear();
    lcd.print("Init horloge...");
    delay(1000);
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // rtc.adjust(DateTime(2025, 4, 6, 14, 30, 0)); // à utiliser si tu veux définir manuellement
  }



  emon1.current(35, 0.7);  //pin , calibration
  emon2.current(33, 0.7);


  INA.setAverage(2);
  INA2.setAverage(2);

  delay(100);


  INA.setMaxCurrentShunt(0.1, 0.1);  // 100mA max avec shunt de 0.1Ω

  INA2.setMaxCurrentShunt(0.1, 0.1);  // 100mA max avec shunt de 0.1Ω

  voltageSensor_1.setSensitivity(SENSITIVITY);
  voltageSensor_2.setSensitivity(SENSITIVITY);


  pinMode(PRISE_1, OUTPUT);
  pinMode(PRISE_2, OUTPUT);
  pinMode(PRISE_3, OUTPUT);
  pinMode(PRISE_4, OUTPUT);


  digitalWrite(PRISE_1, LOW);
  digitalWrite(PRISE_2, LOW);
  digitalWrite(PRISE_3, LOW);
  digitalWrite(PRISE_4, LOW);


  if (myVR.clear() == 0) {
    Serial.println("Recognizer cleared.");
  } else {
    Serial.println("Not find VoiceRecognitionModule.");
    Serial.println("Please check connection and restart ESP32.");
    while (1)
      ;
  }

  loadGroup0();

  Serial.println();
  Serial.println("Configuring access point...");


  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while (1)
      ;
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  Serial.println("Server started");
}

// ----------
void loadGroup0() {
  records[0] = onRecord0;
  records[1] = offRecord0;
  records[2] = onRecord1;
  records[3] = offRecord1;
  records[4] = onRecord2;
  records[5] = offRecord2;
  records[6] = offRecord3;

  if (myVR.load(records, 7) >= 0) {
    Serial.println("Group 0 loaded.");
  } else {
    Serial.println("FaiPrise1 to load Group 0.");
  }
}

void loadGroup1() {
  records[0] = offRecord3;
  records[1] = onRecord4;
  records[2] = offRecord4;
  records[3] = offRecord3;
  records[4] = onRecord0;
  records[5] = offRecord0;
  records[6] = offRecord3;

  if (myVR.load(records, 7) >= 0) {
    Serial.println("Group 1 loaded.");
  } else {
    Serial.println("Fail Prise1 to load Group 1.");
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                  LOOP
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void loop() {

  DateTime dat = rtc.now();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Date: ");
  lcd.print(dat.day());
  lcd.print("/");
  lcd.print(dat.month());
  lcd.print("/");
  lcd.print(dat.year());
  lcd.setCursor(0, 1);
  lcd.print("Heure: ");
  lcd.print(dat.hour());
  lcd.print(":");
  if (dat.minute() < 10) lcd.print("0");
  lcd.print(dat.minute());

  bool trouve = false;

  int ret = myVR.recognize(buf, 50);

  if (ret > 0) {
    myVR.clear();
    group = 0;
    loadGroup0();

    if (buf[0] == 0xFF) {
      Serial.println("Aucun groupe actif");
    } else if (buf[0] & 0x80) {
      Serial.print("Groupe utilisateur : ");
      Serial.println(buf[0] & (~0x80), DEC);
    } else {
      Serial.print("Groupe système : ");
      Serial.println(buf[0], DEC);
    }

    switch (buf[1]) {
      case onRecord0:
        digitalWrite(PRISE_1, HIGH);
        e_1 = 1;
        statutPrise1 = "Allumé ";
        trouve = true;
        break;
      case onRecord1:
        digitalWrite(PRISE_1, HIGH);
        e_1 = 0;
        statutPrise1 = "Allumé ";
        digitalWrite(PRISE_2, HIGH);
        e_2 = 0;
        statutPrise2 = "Allumé ";
        digitalWrite(PRISE_3, HIGH);
        e_3 = 0;
        statutPrise3 = "Allumé ";
        digitalWrite(PRISE_4, HIGH);
        e_4 = 0;
        statutPrise4 = "Allumé ";

        break;

      case offRecord0:
        digitalWrite(PRISE_1, LOW);
        e_1 = 1;

        courantPrise1 = 0;
        tensionPrise1 = 0;
        puissancePrise1 = 0;

        trouve = true;
        statutPrise1 = "Eteinte ";
        break;
      case offRecord1:
        digitalWrite(PRISE_1, LOW);
        e_1 = 1;

        courantPrise1 = 0;
        tensionPrise1 = 0;
        puissancePrise1 = 0;
        statutPrise1 = "Eteinte ";
        digitalWrite(PRISE_2, LOW);
        e_2 = 1;

        courantPrise2 = 0;
        tensionPrise2 = 0;
        puissancePrise2 = 0;

        statutPrise2 = "Eteinte ";
        digitalWrite(PRISE_3, LOW);
        e_3 = 1;

        courantPrise3 = 0;
        tensionPrise3 = 0;
        puissancePrise3 = 0;

        statutPrise3 = "Eteinte ";
        digitalWrite(PRISE_4, LOW);
        e_4 = 1;

        courantPrise4 = 0;
        tensionPrise4 = 0;
        puissancePrise4 = 0;

        statutPrise4 = "Eteinte ";

        break;

      default:
        trouve = false;
        break;
    }

    // Afficher les détails de la commande
    printVR(buf);
    trouve = false;
  }
  // ---------------------

  puissancePrise1 = calculerPuissancePrise1();
  puissancePrise2 = calculerPuissancePrise2();
  puissancePrise3 = calculerPuissancePrise3();
  puissancePrise4 = calculerPuissancePrise4();


  if (puissancePrise1 <= 5) {
    puissancePrise1 = 0;
  }

  if (puissancePrise2 <= 5) {
    puissancePrise2 = 0;
  }

  if (puissancePrise3 <= 5) {
    puissancePrise3 = 0;
  }

  if (puissancePrise4 <= 5) {
    puissancePrise4 = 0;
  }

  // SURTENSION
  if (tensionPrise1 > 132) {  // SOIT 110% DE 120V
    digitalWrite(PRISE_1, LOW);
    P_1 = 1;
  }

  if (tensionPrise2 > 132) {
    digitalWrite(PRISE_2, LOW);
    P_2 = 1;
  }

  if (tensionPrise3 > 132) {
    digitalWrite(PRISE_3, LOW);
    P_3 = 1;
  }

  if (tensionPrise4 > 132) {
    digitalWrite(PRISE_4, LOW);
    P_4 = 1;
  }

  cpt++;


  if (cpt == 10) {

    cpt = 0;

    if (puissancePrise1 > puissanceMaxPrise1) {

      digitalWrite(PRISE_1, LOW);
      courantPrise1 = 0;
      tensionPrise1 = 0;
      puissancePrise1 = 0;
      statutPrise1 = "Eteinte pour dépassement de puissance";

      P_1 = 1;
    }
    if (puissancePrise2 > puissanceMaxPrise2) {


      digitalWrite(PRISE_2, LOW);
      courantPrise2 = 0;
      tensionPrise2 = 0;
      puissancePrise2 = 0;
      statutPrise2 = "Eteinte pour dépassement de puissance";

      P_2 = 1;
    }
    if (puissancePrise3 > puissanceMaxPrise3) {


      digitalWrite(PRISE_3, LOW);
      courantPrise3 = 0;
      tensionPrise3 = 0;
      puissancePrise3 = 0;
      statutPrise3 = "Eteinte pour dépassement de puissance";

      P_3 = 1;
    }

    if (puissancePrise4 > puissanceMaxPrise4) {


      digitalWrite(PRISE_4, LOW);
      courantPrise4 = 0;
      tensionPrise4 = 0;
      puissancePrise4 = 0;
      statutPrise4 = "Eteinte pour dépassement de puissance";

      P_4 = 1;
    }
  }




  DateTime now = rtc.now();

  //prise 1
  if (f_1 == 0) {
    if (P_1 == 0 && e_1 == 0) {
      validerHoraire(heureDebutPrise1, heureFinPrise1, PRISE_1, now);
      if (digitalRead(PRISE_1) == LOW) {
        statutPrise1 = "Eteinte : hors plage horaire";
      } else {
        statutPrise1 = "Allumé";
      }
    }
  }



  //prise 2
  if (f_1 == 0) {
    if (P_2 == 0 && e_2 == 0) {
      validerHoraire(heureDebutPrise2, heureFinPrise2, PRISE_2, now);
      if (digitalRead(PRISE_2) == LOW) {
        statutPrise2 = "Eteinte : hors plage horaire";
      } else {
        statutPrise2 = "Allumé";
      }
    }
  }

  //prise 3
  if (f_1 == 0) {
    if (P_3 == 0 && e_3 == 0) {
      validerHoraire(heureDebutPrise3, heureFinPrise3, PRISE_3, now);
      if (digitalRead(PRISE_3) == LOW) {
        statutPrise3 = "Eteinte : hors plage horaire";
      } else {
        statutPrise3 = "Allumé";
      }
    }
  }
  //prise 4
  if (f_1 == 0) {
    if (P_4 == 0 && e_4 == 0) {
      validerHoraire(heureDebutPrise4, heureFinPrise4, PRISE_4, now);
      if (digitalRead(PRISE_4) == LOW) {
        statutPrise4 = "Eteinte : hors plage horaire";
      } else {
        statutPrise4 = "Allumé";
      }
    }
  }
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //                  PAGE WEB
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  NetworkClient client = server.accept();
  String request = "";

  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();


        request += c;

        Serial.write(c);
        if (c == '\n') {
          if (currentLine.length() == 0) {

            currentLine = "";

            html = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n";

            html += "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta http-equiv='refresh' content='10'>";  //content='30'
            html += "<style>body{font-family:'Times New Roman',sans-serif;text-align:center;background-color:#f0f0f0;}";
            html += "h1{color:#333;}button{padding:10px 20px;margin:10px;font-size:16px;cursor:pointer;border:none;border-radius:5px;}";
            html += ".on{background-color:#4CAF50;color:white;}.off{background-color:#f44336;color:white;}";
            html += ".settings{background-color:#2196F3;color:white;}.control{display:inline-block;margin:20px;padding:20px;border:1px solid #ddd;border-radius:10px;background-color:#fff;}";
            html += ".menu{display:none;margin-top:10px;padding:10px;border:1px solid #ccc;border-radius:5px;background-color:#e7e7e7;text-align:left;}";
            html += ".info-block{display:inline-block;margin:20px;padding:20px;border:1px solid #ddd;border-radius:10px;background-color:#e7e7e7;text-align:left;width:80%;}</style>";
            html += "<script>function toggleMenu(menuId){const menu=document.getElementById(menuId);menu.style.display=(menu.style.display==='block')?'none':'block';}</script>";

            html += "<head></head><body><h1>UQAM - PROJET INTEGRATEUR II - HIVER 2025</h1>";
            html += "<h2>Multiprise intelligente</h2>";


            for (int prise = 1; prise <= 4; prise++) {
              html += "<div class='control'><h2>Prise " + String(prise) + "</h2>";

              float puissance = (prise == 1) ? puissancePrise1 : (prise == 2) ? puissancePrise2
                                                               : (prise == 3) ? puissancePrise3
                                                                              : puissancePrise4;
              String statut = (prise == 1) ? statutPrise1 : (prise == 2) ? statutPrise2
                                                          : (prise == 3) ? statutPrise3
                                                                         : statutPrise4;

              String debut = (prise == 1) ? heureDebutPrise1 : (prise == 2) ? heureDebutPrise2
                                                             : (prise == 3) ? heureDebutPrise3
                                                                            : heureDebutPrise4;

              String fin = (prise == 1) ? heureFinPrise1 : (prise == 2) ? heureFinPrise2
                                                         : (prise == 3) ? heureFinPrise3
                                                                        : heureFinPrise4;

              html += "<p>Consommation actuelle : " + String(puissance) + " W</p>";  //actuelle
              html += "<p>Horaire : " + debut + " - " + fin + " </p>";
              html += "<p>Etat : " + statut + "</p>";

              html += "<button class='on' onclick=\"location.href='/prise" + String(prise) + "/on'\">Allumer</button>";
              html += "<button class='off' onclick=\"location.href='/prise" + String(prise) + "/off'\">Eteindre</button>";
              html += "<button class='settings' onclick=\"toggleMenu('menu" + String(prise) + "')\">Reglage</button>";

              html += "<div id='menu" + String(prise) + "' class='menu'><form action='/prise" + String(prise) + "/settings' method='GET'>";

              html += "<label>Heure de début:<input type='time' name='heure_debut' value='" + debut + "'></label><br>";
              html += "<label>Heure de fin:<input type='time' name='heure_fin' value='" + fin + "'></label><br>";

              float puissanceMax = (prise == 1) ? puissanceMaxPrise1 : (prise == 2) ? puissanceMaxPrise2
                                                                     : (prise == 3) ? puissanceMaxPrise3
                                                                                    : puissanceMaxPrise4;
              html += "<label>Puissance maximale:<input type='number' name='puissance_max' min='1' max='5000' value='" + String(puissanceMax) + "'></label><br>";


              html += "<input type='submit' value='Enregistrer'></form></div></div>";
            }

            html += "<div class='info-block'><h2>Information Générale</h2><p>Bienvenue sur la plateforme de contrôle de ta multiprise.</p>";
            html += "<p>Utilisez les commandes ci-dessus pour gérer les prises.</p>";
            html += "<p>Le bouton 'Allumer' permet d’allumer la prise, tandis que le bouton 'Eteindre' la désactive. Le bouton 'Reglage' permet de définir un seuil de puissance consommée ainsi qu’un intervalle horaire pendant lequel la prise doit s’allumer ou s’éteindre automatiquement.</p>";
            html += "<p>Cette page web a été réalisée dans le cadre du cours MIC3117 - Analyse de circuits et projet intégrateur II dans le programme de baccalauréat en systèmes informatiques et électroniques (BSIE). Le projet consistait en la conception et la réalisation d'une multiprise intelligente. Cette dernière est destinée à garantir une sécurité optimale en protégeant les appareils contre les anomalies électriques, mais aussi à répondre à la nécessité d’optimiser l’énergie face à une consommation accrue.</p>";
            html += "<p>Il a été réalisé par <strong>Kodanne Coulibaly</strong> et <strong>Assane Dieye</strong> sous la supervision du professeur <strong> M. Christian Jesus B. Fayomi</strong>.</p></div>";

            html += "</body></html>";

            client.print(html);

            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
        //prise 1
        if (currentLine.endsWith("GET /prise1/on")) {
          statutPrise1 = "Allumé";
          P_1 = 0;
          e_1 = 0;
          f_1 = 1;
          digitalWrite(PRISE_1, HIGH);

          html = "HTTP/1.1 302 Found\r\nLocation: /\r\nConnection: close\r\n\r\n";
          client.print(html);
          break;

        } else if (currentLine.endsWith("GET /prise1/off")) {
          e_1 = 1;
          f_1 = 0;
          statutPrise1 = "Eteint";
          digitalWrite(PRISE_1, LOW);
          courantPrise1 = 0;
          tensionPrise1 = 0;
          puissancePrise1 = 0;

          html = "HTTP/1.1 302 Found\r\nLocation: /\r\nConnection: close\r\n\r\n";
          client.print(html);
          break;
        }
        //prise 2

        if (currentLine.endsWith("GET /prise2/on")) {
          statutPrise1 = "Allumé";
          P_2 = 0;
          e_2 = 0;
          f_2 = 1;
          digitalWrite(PRISE_2, HIGH);

          html = "HTTP/1.1 302 Found\r\nLocation: /\r\nConnection: close\r\n\r\n";
          client.print(html);
          break;

        } else if (currentLine.endsWith("GET /prise2/off")) {
          e_2 = 1;
          f_2 = 0;
          statutPrise1 = "Eteint";
          digitalWrite(PRISE_2, LOW);
          courantPrise1 = 0;
          tensionPrise1 = 0;
          puissancePrise1 = 0;

          html = "HTTP/1.1 302 Found\r\nLocation: /\r\nConnection: close\r\n\r\n";
          client.print(html);
          break;
        }

        //prise 3

        if (currentLine.endsWith("GET /prise3/on")) {
          statutPrise3 = "Allumé";
          P_3 = 0;
          e_3 = 0;
          f_3 = 1;
          digitalWrite(PRISE_3, HIGH);

          html = "HTTP/1.1 302 Found\r\nLocation: /\r\nConnection: close\r\n\r\n";
          client.print(html);
          break;

        } else if (currentLine.endsWith("GET /prise3/off")) {
          e_3 = 1;
          f_3 = 0;
          statutPrise3 = "Eteint";
          digitalWrite(PRISE_3, LOW);
          courantPrise3 = 0;
          tensionPrise3 = 0;
          puissancePrise3 = 0;

          html = "HTTP/1.1 302 Found\r\nLocation: /\r\nConnection: close\r\n\r\n";
          client.print(html);
          break;
        }


        //prise 4

        if (currentLine.endsWith("GET /prise4/on")) {
          statutPrise4 = "Allumé";
          P_4 = 0;
          e_4 = 0;
          f_4 = 1;
          digitalWrite(PRISE_4, HIGH);

          html = "HTTP/1.1 302 Found\r\nLocation: /\r\nConnection: close\r\n\r\n";
          client.print(html);
          break;

        } else if (currentLine.endsWith("GET /prise4/off")) {
          e_4 = 1;
          f_4 = 0;
          statutPrise4 = "Eteint";
          digitalWrite(PRISE_4, LOW);
          courantPrise4 = 0;
          tensionPrise4 = 0;
          puissancePrise4 = 0;

          html = "HTTP/1.1 302 Found\r\nLocation: /\r\nConnection: close\r\n\r\n";
          client.print(html);
          break;
        }
      }
    }

    if (request.indexOf("GET /prise1/settings") >= 0) {
      int idxPMax = request.indexOf("puissance_max=");
      if (idxPMax >= 0) {
        int debutValeur = idxPMax + strlen("puissance_max=");
        int finValeur = request.indexOf("&", debutValeur);
        if (finValeur == -1) finValeur = request.indexOf(" ", debutValeur);
        if (finValeur == -1) finValeur = request.length();

        String puissanceMax = request.substring(debutValeur, finValeur);
        puissanceMax.trim();

        float puissanceMaxValue = puissanceMax.toFloat();
        if (puissanceMaxValue > 0) {
          puissanceMaxPrise1 = puissanceMaxValue;
          preferences.putFloat("pMaxP1", puissanceMaxPrise1);

        } else {
          Serial.println("Erreur de conversion de puissance_max");
        }
      }

      // Récupération de l'heure de début
      int idxHeureDebut = request.indexOf("heure_debut=");
      if (idxHeureDebut >= 0) {
        int debutValeur = idxHeureDebut + strlen("heure_debut=");
        int finValeur = request.indexOf("&", debutValeur);
        if (finValeur == -1) finValeur = request.indexOf(" ", debutValeur);
        if (finValeur == -1) finValeur = request.length();

        // String heureDebut
        heureDebutPrise1 = request.substring(debutValeur, finValeur);
        heureDebutPrise1 = urlDecode(heureDebutPrise1);
        heureDebutPrise1.trim();

        preferences.putString("hDebutP1", heureDebutPrise1);
      }

      // Récupération de l'heure de fin
      int idxHeureFin = request.indexOf("heure_fin=");
      if (idxHeureFin >= 0) {
        int debutValeur = idxHeureFin + strlen("heure_fin=");
        int finValeur = request.indexOf("&", debutValeur);
        if (finValeur == -1) finValeur = request.indexOf(" ", debutValeur);
        if (finValeur == -1) finValeur = request.length();

        //String heureFin
        heureFinPrise1 = request.substring(debutValeur, finValeur);
        heureFinPrise1 = urlDecode(heureFinPrise1);
        heureFinPrise1.trim();

        preferences.putString("hFinP1", heureFinPrise1);
      }


    } else if (request.indexOf("GET /prise2/settings") >= 0) {
      int idxPMax = request.indexOf("puissance_max=");
      if (idxPMax >= 0) {
        int debutValeur = idxPMax + strlen("puissance_max=");
        int finValeur = request.indexOf("&", debutValeur);
        if (finValeur == -1) {
          finValeur = request.indexOf(" ", debutValeur);
          if (finValeur == -1) finValeur = request.length();
        }

        String puissanceMax = request.substring(debutValeur, finValeur);
        puissanceMax.trim();

        float puissanceMaxValue = puissanceMax.toFloat();
        if (puissanceMaxValue > 0) {
          puissanceMaxPrise2 = puissanceMaxValue;
          preferences.putFloat("pMaxP2", puissanceMaxPrise2);

        } else {
          Serial.println(" Erreur de conversion de puissance_max");
        }
      }

      // Recupération de l'heure de début
      int idxHeureDebut = request.indexOf("heure_debut=");
      if (idxHeureDebut >= 0) {
        int debutValeur = idxHeureDebut + strlen("heure_debut=");
        int finValeur = request.indexOf("&", debutValeur);
        if (finValeur == -1) finValeur = request.indexOf(" ", debutValeur);
        if (finValeur == -1) finValeur = request.length();

        // heureDebut
        heureDebutPrise2 = request.substring(debutValeur, finValeur);
        heureDebutPrise2 = urlDecode(heureDebutPrise2);
        heureDebutPrise1.trim();

        preferences.putString("hDebutP2", heureDebutPrise2);
      }

      // Récuperation de l'heure de fin
      int idxHeureFin = request.indexOf("heure_fin=");
      if (idxHeureFin >= 0) {
        int debutValeur = idxHeureFin + strlen("heure_fin=");
        int finValeur = request.indexOf("&", debutValeur);
        if (finValeur == -1) finValeur = request.indexOf(" ", debutValeur);
        if (finValeur == -1) finValeur = request.length();

        // heureFin
        heureFinPrise2 = request.substring(debutValeur, finValeur);
        heureFinPrise2 = urlDecode(heureFinPrise2);
        heureFinPrise2.trim();

        preferences.putString("hFinP2", heureFinPrise2);
      }
    } else if (request.indexOf("GET /prise3/settings") >= 0) {
      int idxPMax = request.indexOf("puissance_max=");
      if (idxPMax >= 0) {
        int debutValeur = idxPMax + strlen("puissance_max=");
        int finValeur = request.indexOf("&", debutValeur);
        if (finValeur == -1) {
          finValeur = request.indexOf(" ", debutValeur);
          if (finValeur == -1) finValeur = request.length();
        }

        String puissanceMax = request.substring(debutValeur, finValeur);
        puissanceMax.trim();

        float puissanceMaxValue = puissanceMax.toFloat();
        if (puissanceMaxValue > 0) {
          puissanceMaxPrise3 = puissanceMaxValue;
          preferences.putFloat("pMaxP3", puissanceMaxPrise3);

        } else {
          Serial.println(" Erreur de conversion de puissance_max");
        }
      }

      // Récuperation de l'heure de début
      int idxHeureDebut = request.indexOf("heure_debut=");
      if (idxHeureDebut >= 0) {
        int debutValeur = idxHeureDebut + strlen("heure_debut=");
        int finValeur = request.indexOf("&", debutValeur);
        if (finValeur == -1) finValeur = request.indexOf(" ", debutValeur);
        if (finValeur == -1) finValeur = request.length();

        //  heureDebut
        heureDebutPrise3 = request.substring(debutValeur, finValeur);
        heureDebutPrise3 = urlDecode(heureDebutPrise3);
        heureDebutPrise3.trim();

        preferences.putString("hDebutP3", heureDebutPrise3);
      }

      // Récuperation de l'heure de fin
      int idxHeureFin = request.indexOf("heure_fin=");
      if (idxHeureFin >= 0) {
        int debutValeur = idxHeureFin + strlen("heure_fin=");
        int finValeur = request.indexOf("&", debutValeur);
        if (finValeur == -1) finValeur = request.indexOf(" ", debutValeur);
        if (finValeur == -1) finValeur = request.length();

        // heureFin
        heureFinPrise3 = request.substring(debutValeur, finValeur);
        heureFinPrise3 = urlDecode(heureFinPrise3);
        heureFinPrise3.trim();

        preferences.putString("hFinP3", heureFinPrise3);
      }
    } else if (request.indexOf("GET /prise4/settings") >= 0) {
      int idxPMax = request.indexOf("puissance_max=");
      if (idxPMax >= 0) {
        int debutValeur = idxPMax + strlen("puissance_max=");
        int finValeur = request.indexOf("&", debutValeur);
        if (finValeur == -1) {
          finValeur = request.indexOf(" ", debutValeur);
          if (finValeur == -1) finValeur = request.length();
        }

        String puissanceMax = request.substring(debutValeur, finValeur);
        puissanceMax.trim();

        float puissanceMaxValue = puissanceMax.toFloat();
        if (puissanceMaxValue > 0) {
          puissanceMaxPrise4 = puissanceMaxValue;
          preferences.putFloat("pMaxP4", puissanceMaxPrise4);

        } else {
          Serial.println(" Erreur de conversion de puissance_max");
        }
      }

      // Récuperation de l'heure de début
      int idxHeureDebut = request.indexOf("heure_debut=");
      if (idxHeureDebut >= 0) {
        int debutValeur = idxHeureDebut + strlen("heure_debut=");
        int finValeur = request.indexOf("&", debutValeur);
        if (finValeur == -1) finValeur = request.indexOf(" ", debutValeur);
        if (finValeur == -1) finValeur = request.length();

        //  heureDebut
        heureDebutPrise4 = request.substring(debutValeur, finValeur);
        heureDebutPrise4 = urlDecode(heureDebutPrise4);
        heureDebutPrise1.trim();

        preferences.putString("hDebutP4", heureDebutPrise4);
      }

      // Récuperation de l'heure de fin
      int idxHeureFin = request.indexOf("heure_fin=");
      if (idxHeureFin >= 0) {
        int debutValeur = idxHeureFin + strlen("heure_fin=");
        int finValeur = request.indexOf("&", debutValeur);
        if (finValeur == -1) finValeur = request.indexOf(" ", debutValeur);
        if (finValeur == -1) finValeur = request.length();

        // heureFin
        heureFinPrise4 = request.substring(debutValeur, finValeur);
        heureFinPrise4 = urlDecode(heureFinPrise4);
        heureFinPrise4.trim();

        preferences.putString("hFinP4", heureFinPrise4);
      }
    }

    client.stop();
    Serial.println("Client Disconnected.");
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //                  FIN PAGE WEB
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++





  delay(1000);
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                  obtenirCourantPrise1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
float obtenirCourantPrise2() {

  double somme = 0;
  int n = 10;

  for (int i = 0; i < n; i++) {
    double Irms = emon1.calcIrms(1250);
    somme += Irms;
    delay(10);
  }

  double moyenneIrms = somme / n;

  float I = moyenneIrms;

  if (I <= 0.030 || tensionPrise2 <= 70) {
    I = 0;
  }

  return I;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                  obtenirTensionPrise1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
float obtenirTensionPrise1() {

  float voltage = voltageSensor_1.getRmsVoltage();
  if (voltage < 70) voltage = 0;
  return voltage;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                  obtenirCourantPrise1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
float obtenirCourantPrise1() {

  double somme2 = 0;
  int n2 = 10;

  for (int j = 0; j < n2; j++) {
    double Irms2 = emon2.calcIrms(1250);
    somme2 += Irms2;
    delay(10);
  }

  double moyenneIrms2 = somme2 / n2;

  float I = moyenneIrms2;

  if (I <= 0.035 || tensionPrise1 <= 70) {
    I = 0;
  } else {
    I = I - 0.015;
  }

  return I;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                  obtenirTensionPrise2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
float obtenirTensionPrise2() {

  float voltage = voltageSensor_2.getRmsVoltage();
  if (voltage < 70) voltage = 0;
  return voltage;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                  obtenirCourantPrise3
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
float obtenirCourantPrise3() {



  float V_BUS = INA2.getBusVoltage();  // Tension d'alimentation totale (VBUS)
  float I = INA2.getCurrent_mA();      // Courant



  return I;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                  obtenirTensionPrise3
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
float obtenirTensionPrise3() {

  float x = 0;

  // float V_BUS = INA2.getBusVoltage();          // Tension d'alimentation totale (VBUS)
  // float I = INA2.getCurrent_mA() / 1000.0; // Courant

  // return V_BUS;
  return x;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                  obtenirCourantPrise4
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
float obtenirCourantPrise4() {



  // float V_BUS = INA.getBusVoltage();  // Tension d'alimentation totale (VBUS)
  // float I = INA.getCurrent_mA();      // Courant

  float I = 0;

  return I;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                  obtenirTensionPrise4
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
float obtenirTensionPrise4() {

  float V_BUS = INA.getBusVoltage();  // Tension d'alimentation totale (VBUS)
  float I = INA.getCurrent_mA() / 1000.0;




  return V_BUS;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                  calculerPuissance
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// prise 1
float calculerPuissancePrise1() {

  courantPrise1 = obtenirCourantPrise1();
  tensionPrise1 = obtenirTensionPrise1();
  if (courantPrise1 == 0 || tensionPrise1 == 0) {
    puissancePrise1 = 0;
  } else {
    puissancePrise1 = courantPrise1 * tensionPrise1;
  }
  return puissancePrise1;
}

// prise 2
float calculerPuissancePrise2() {

  courantPrise2 = obtenirCourantPrise2();
  tensionPrise2 = obtenirTensionPrise2();
  if (courantPrise2 == 0 || tensionPrise2 == 0) {
    puissancePrise2 = 0;
  } else {
    puissancePrise2 = courantPrise2 * tensionPrise2;
  }

  return puissancePrise2;
}

// prise 3
float calculerPuissancePrise3() {

  courantPrise3 = obtenirCourantPrise3();
  tensionPrise3 = obtenirTensionPrise3();

  if (courantPrise3 == 0 || tensionPrise3 == 0) {
    puissancePrise3 = 0;
  } else {
    puissancePrise3 = courantPrise3 * tensionPrise3;
  }

  return puissancePrise3;
}

// prise 4
float calculerPuissancePrise4() {

  courantPrise4 = obtenirCourantPrise4();
  tensionPrise4 = obtenirTensionPrise4();

  if (courantPrise4 == 0 || tensionPrise4 == 0) {
    puissancePrise4 = 0;
  } else {
    puissancePrise4 = courantPrise4 * tensionPrise4;
  }

  return puissancePrise4;
}

String urlDecode(String input) {
  String decoded = "";
  char c;
  for (int i = 0; i < input.length(); i++) {
    if (input[i] == '+') {
      decoded += ' ';
    } else if (input[i] == '%' && i + 2 < input.length()) {
      char hex[3];
      hex[0] = input[i + 1];
      hex[1] = input[i + 2];
      hex[2] = 0;
      c = strtol(hex, NULL, 16);
      decoded += c;
      i += 2;
    } else {
      decoded += input[i];
    }
  }
  return decoded;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                  validerHoraire
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void validerHoraire(const String& heureDebut, const String& heureFin, int pinRelais, DateTime now) {
  int heureActuelleMinutes = now.hour() * 60 + now.minute();

  int heureDebutMinutes = heureDebut.substring(0, 2).toInt() * 60 + heureDebut.substring(3, 5).toInt();

  int heureFinMinutes = heureFin.substring(0, 2).toInt() * 60 + heureFin.substring(3, 5).toInt();

  String nomPrise = "Prise " + String(pinRelais);

  if (heureDebutMinutes < heureFinMinutes) {

    if (heureActuelleMinutes >= heureDebutMinutes && heureActuelleMinutes < heureFinMinutes) {

      digitalWrite(pinRelais, HIGH);  // ON

    } else {
      digitalWrite(pinRelais, LOW);  // OFF
    }
  } else {
    // Cas où la plage passe minuit
    if (heureActuelleMinutes >= heureDebutMinutes || heureActuelleMinutes < heureFinMinutes) {
      digitalWrite(pinRelais, HIGH);  // ON
    } else {
      digitalWrite(pinRelais, LOW);  // OFF
    }
  }
}


// //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// //                          Notifications
// //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// void notification(const char* sujet, const char* message){

//   // Configuration de l'authentification SMTP
//     ESP_Mail_Session session;
//     session.server.host_name = SMTP_HOST;
//     session.server.port = SMTP_PORT;
//     session.login.email = AUTHOR_EMAIL;
//     session.login.password = AUTHOR_PASSWORD;
//     session.login.user_domain = "";

//     // Configuration du message SMTP
//     SMTP_Message smtpMessage;
//     smtpMessage.sender.name = "UQAM - PROJET INTEGRATEUR II";
//     smtpMessage.sender.email = AUTHOR_EMAIL;
//     smtpMessage.subject = sujet;
//     smtpMessage.addRecipient("Destinataire", RECIPIENT_EMAIL);
//     smtpMessage.text.content = message;
//     smtpMessage.text.charSet = "utf-8";  // Encodage UTF-8
//     smtpMessage.text.transfer_encoding = Content_Transfer_Encoding::enc_base64;  // Encodage sécurisé

//     // Connexion SMTP
//     smtp.debug(1);  // Active les logs pour le débogage
//     if (!smtp.connect(&session)) {
//         Serial.println("Échec de la connexion SMTP");
//         return;
//     }

//     // Envoi du message
//     if (!MailClient.sendMail(&smtp, &smtpMessage)) {
//         Serial.println("Erreur d'envoi d'email : " + smtp.errorReason());
//     } else {
//         Serial.println("E-mail envoyé avec succès !");
//     }

//     // Fermeture de la session SMTP
//     smtp.closeSession();

// }

// -----------

// //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// //                    reconnaissance vocale
// //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void printVR(uint8_t* buf) {
  Serial.println("VR Index\tGroup\tRecordNum\tSignature");

  Serial.print(buf[2], DEC);
  Serial.print("\t\t");

  if (buf[0] == 0xFF) {
    Serial.print("NONE");
  } else if (buf[0] & 0x80) {
    Serial.print("UG ");
    Serial.print(buf[0] & (~0x80), DEC);
  } else {
    Serial.print("SG ");
    Serial.print(buf[0], DEC);
  }
  Serial.print("\t");

  Serial.print(buf[1], DEC);
  Serial.print("\t\t");
  if (buf[3] > 0) {
    printSignature(buf + 4, buf[3]);
  } else {
    Serial.print("NONE");
  }
  Serial.println();
}

void printSignature(uint8_t* buf, int len) {
  int i;
  for (i = 0; i < len; i++) {
    if (buf[i] > 0x19 && buf[i] < 0x7F) {
      Serial.write(buf[i]);
    } else {
      Serial.print("[");
      Serial.print(buf[i], HEX);
      Serial.print("]");
    }
  }
}
// -----------
