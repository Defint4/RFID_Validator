#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define RST_PIN         4           // Pin de reset, configurable
#define SS_PIN          6           // Pin SS, configurable
#define LED_WHITE       2           // Pin pour la LED blanche
#define LED_RED         5           // Pin pour la LED rouge
#define LED_GREEN       3           // Pin pour la LED verte
#define BUTTON_PIN      7           // Pin pour le bouton

#define SSID "PCmatthieu"
#define PASSW "123456789!"
const char* host     = "192.168.137.4";
const char* url      = "/index.php";

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Création d'une instance MFRC522


enum Mode {
  READ_MODE,
  WRITE_MODE
};

Mode currentMode = WRITE_MODE;

enum WriteState {
    WAIT_FOR_FAMILY_NAME,
    WAIT_FOR_FIRST_NAME,
    NONE
};

WriteState writeState = NONE;

String rfidData = "";

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 100;
bool lastButtonState = LOW;
bool buttonState = LOW;

bool ledState = LOW;
unsigned long lastReadTime = 0;
const unsigned long readDelay = 1000; // Un délai de 1 seconde (1000 millisecondes)
bool cardDetected = false;

MFRC522::MIFARE_Key key;



void setup() {
  pinMode(LED_WHITE, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  Serial.begin(115200);
  delay(1000); // Délai pour stabiliser la communication série

  // Initialisation du WiFi
  WiFi.begin(SSID, PASSW);
  Serial.println(F("\nConnecting"));

  // Attente de la connexion WiFi
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(F("."));
    digitalWrite(LED_RED, LOW);
    delay(500);
    digitalWrite(LED_RED, HIGH);
    delay(500);
  }

  Serial.println(F("Connecté avec succès !"));
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  // Initialisation du SPI et du lecteur MFRC522
  SPI.begin();
  mfrc522.PCD_Init();

  // Configuration initiale de la LED
  updateLED();

  // Initialisation de la clé pour le lecteur MFRC522
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}


void loop() {
  readButton(); // Lecture de l'état du bouton

  if (currentMode == READ_MODE) {
    performRead(); // Lire les données de la carte si en mode lecture
  } else if (currentMode == WRITE_MODE) {
    performWrite(); // Écrire les données sur la carte si en mode écriture
  }
}



void readButton() {
  bool reading = digitalRead(BUTTON_PIN);

  // Vérifier si l'état du bouton a changé
  if (reading != lastButtonState) {
    lastDebounceTime = millis(); // Réinitialiser le timer de debounce
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState && reading == HIGH) {
      Serial.println("Button Pressed");
      currentMode = (currentMode == READ_MODE) ? WRITE_MODE : READ_MODE;
      Serial.print("Changement de mode. Nouveau mode : ");
      Serial.println(currentMode == READ_MODE ? "READ_MODE" : "WRITE_MODE");
      updateLED(); // Mettre à jour l'état de la LED immédiatement
    }
    buttonState = reading;
  }

  lastButtonState = reading;
}




void updateLED() {
  static unsigned long previousMillis = 0;
  const long interval = 200; // Intervalle de clignotement pour la LED verte

  unsigned long currentMillis = millis();

  digitalWrite(LED_WHITE, (currentMode == READ_MODE) ? HIGH : LOW);

  if (currentMode == WRITE_MODE) {
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      ledState = !ledState;
      digitalWrite(LED_GREEN, ledState ? HIGH : LOW);
    }
  } else {
    if (ledState != LOW) {
      digitalWrite(LED_GREEN, LOW); // S'assurer que la LED verte est éteinte en mode READ
      ledState = LOW;
    }
  }

  digitalWrite(LED_RED, LOW); // La LED rouge reste éteinte par défaut
}



void readBlock(byte block, byte *buffer, byte &len) {
  Serial.print(F("Tentative d'authentification pour le bloc "));
  Serial.println(block);

  // Authentification avant la lecture
  MFRC522::StatusCode authStatus = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (authStatus == MFRC522::STATUS_OK) {
    Serial.print(F("Tentative de lecture du bloc "));
    Serial.println(block);

    // Lecture du bloc
    MFRC522::StatusCode readStatus = mfrc522.MIFARE_Read(block, buffer, &len);
    if (readStatus == MFRC522::STATUS_OK) {
      Serial.print(F("Block "));
      Serial.print(block);
      Serial.print(F(": "));
      for (uint8_t i = 0; i < len; i++) {
        Serial.print(buffer[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    } else {
      Serial.print(F("Échec de la lecture du bloc "));
      Serial.print(block);
      Serial.print(F(", Code d'erreur: "));
      Serial.println(readStatus);
    }
  } else {
    Serial.print(F("Échec de l'authentification pour le bloc "));
    Serial.print(block);
    Serial.print(F(", Code d'erreur: "));
    Serial.println(authStatus);
  }
}


void performRead() {

  if (millis() - lastReadTime < readDelay) {
    return;
  }

  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    cardDetected = false;
    return;
  }

  if (!cardDetected) {
    Serial.println(F("Carte RFID détectée."));
    cardDetected = true;
  }

  Serial.println(F("**Card Detected:**"));
  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid));

  byte buffer[18];
  byte len = 18;
  byte block;

  // Lecture du nom de famille
  block = 1;
  readBlock(block, buffer, len);

  // Lecture du prénom
  block = 4;
  readBlock(block, buffer, len);

  // Convertit l'UID de la carte en chaîne hexadécimale
  String uidStr = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) {
      uidStr += '0'; // Ajoute un 0 pour les nombres hexadécimaux à un chiffre
    }
    uidStr += String(mfrc522.uid.uidByte[i], HEX);
  }

  uidStr.toUpperCase(); // Convertit la chaîne hexadécimale en majuscules


  String postData = "mode=read&uid=" + uidStr;

  Serial.println("Donnée envoyée au serveur : ");
  Serial.println(postData);






   // Création de l'objet HTTPClient
  HTTPClient http;

  // URL complète de la requête HTTP
  String fullURL = "http://" + String(host) + String(url);

  // Démarre la connexion HTTP
  http.begin(fullURL);

  // Configuration de l'en-tête HTTP
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Effectue la requête HTTP POST
  int httpResponseCode = http.POST(postData);

  // Vérification de la réponse
  if (httpResponseCode == 200) {
    String response = http.getString();
    Serial.println("Server Response: " + response);

    // Si la réponse contient "True", allumez la LED verte
    if (response.indexOf("True") != -1) {
      digitalWrite(LED_GREEN, HIGH);
      delay(2000); // Allumez la LED verte pendant 2 secondes
      digitalWrite(LED_GREEN, LOW); // Éteignez la LED verte
    } else if (response.indexOf("False") != -1) {
      // Si la réponse contient "False", allumez la LED rouge
      digitalWrite(LED_RED, HIGH);
      delay(2000); // Allumez la LED rouge pendant 2 secondes
      digitalWrite(LED_RED, LOW); // Éteignez la LED rouge
    }
  } else {
    Serial.print("HTTP Request failed with error code: ");
    Serial.println(httpResponseCode);
  }

  // Fermeture de la connexion HTTP
  http.end();

  // Arrête la communication avec la carte RFID
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  lastReadTime = millis();
  cardDetected = false;

  Serial.println(F("Lecture terminée."));
  Serial.println(F("Opération terminée. Retour au mode d'attente."));
}

void performWrite() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    cardDetected = false;
    return;
  }

  if (!cardDetected) {
    Serial.println(F("Carte RFID détectée."));
    cardDetected = true;
  }

  // Conversion de l'UID de la carte en chaîne de caractères
  String uidStr = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) {
      uidStr += '0';
    }
    uidStr += String(mfrc522.uid.uidByte[i], HEX);
  }
  uidStr.toUpperCase();

  // Envoi de l'UID au serveur
  sendUIDToServer(uidStr);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  cardDetected = false;
  Serial.println(F("UID envoyé au serveur. Retour au mode d'attente."));
}

void sendUIDToServer(String uid) {
  // Création de l'objet HTTPClient
  HTTPClient http;

  // URL complète de la requête HTTP
  String fullURL = "http://" + String(host) + String(url);

  // Préparation des données à envoyer
  String postData = "mode=write&uid=" + uid;

  // Démarre la connexion HTTP
  http.begin(fullURL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Effectue la requête HTTP POST
  int httpResponseCode = http.POST(postData);

  // Vérification de la réponse
  if (httpResponseCode > 0) {
    String response = http.getString();
    if (response == "UID enregistré avec succès") {
      digitalWrite(LED_WHITE, HIGH);
      delay(500);
      digitalWrite(LED_WHITE, LOW);
    } else {
      digitalWrite(LED_RED, HIGH);
      delay(500);
      digitalWrite(LED_RED, LOW);
    }
    Serial.println("Réponse du serveur : " + response);
    
  } else {
    Serial.print("Erreur lors de l'envoi de la requête : ");
    Serial.println(httpResponseCode);
  }

  // Fermeture de la connexion HTTP
  http.end();
}

