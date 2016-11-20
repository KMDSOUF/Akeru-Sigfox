#include <SoftwareSerial.h>
#include "Akeru.h"
#include "Arduino.h"


volatile int flow_frequency; // Mesure les impulsions
unsigned int l_hour; // Calculer litres/hour
unsigned char flowsensor = 2; // capteur Input
unsigned long currentTime;
unsigned long cloopTime;
void flow ()//Fonction d'interruption
{
   flow_frequency++;
}

/* temper&hummid */
#define DHT11_PIN 0 

byte read_dht11_dat()
{
    byte i = 0;
    byte result=0;
    for(i=0; i< 8; i++){

        while(!(PINC & _BV(DHT11_PIN))); 
        delayMicroseconds(30);

        if(PINC & _BV(DHT11_PIN))
        result |=(1<<(7-i));
        while((PINC & _BV(DHT11_PIN)));
    }
    return result;
}

// structure pour le stockage des informations capturées
typedef struct 
{
  float temperature;
  float humidite;
  unsigned int humidite_eau;
  unsigned int debit;
} Payload;

// Capteur de temperatue aquatique
int sensorPin = A1; // Sélectionner la broche d'entrée du potentiomètre
unsigned int sensorValue = 0; // Variable pour stocker la valeur provenant du capteur

void setup()
{
   pinMode(flowsensor, INPUT);
   digitalWrite(flowsensor, HIGH);
   attachInterrupt(0, flow, RISING); // Initialisation interruption
   sei(); // Activer les interruptions
   currentTime = millis();
   cloopTime = currentTime;
  
     
    DDRC |= _BV(DHT11_PIN);
    PORTC |= _BV(DHT11_PIN);

    Serial.begin(9600);
    delay(100);

    /* initialiser akeru*/
    Akeru.begin();
}


void loop()
{
    byte dht11_dat[5];
    byte dht11_in;
    byte i;

    // Initialisation
    PORTC &= ~_BV(DHT11_PIN);
    delay(18);
    PORTC |= _BV(DHT11_PIN);
    delayMicroseconds(40);
    DDRC &= ~_BV(DHT11_PIN);
    delayMicroseconds(40);
    dht11_in = PINC & _BV(DHT11_PIN);
    delayMicroseconds(80);
    dht11_in = PINC & _BV(DHT11_PIN);
    delayMicroseconds(80);
    
    // reception des données
    for (i=0; i<5; i++)
    dht11_dat[i] = read_dht11_dat();

    DDRC |= _BV(DHT11_PIN);
    PORTC |= _BV(DHT11_PIN);

    byte dht11_check_sum = dht11_dat[0]+dht11_dat[1]+dht11_dat[2]+dht11_dat[3];
    

    Serial.print("Hummidite externe = ");
    Serial.print(dht11_dat[0]);
    Serial.print(".");
    Serial.print(dht11_dat[1]);
    Serial.println("%  ");
    Serial.print("Temperature = ");
    Serial.print(dht11_dat[2]);
    Serial.print(".");
    Serial.print(dht11_dat[3]);
    Serial.println("C  ");

    float temperature = dht11_dat[2];
    float humidite = dht11_dat[0];

    sensorValue = analogRead(sensorPin);
    Serial.print("Humidite aquatique = " );
    Serial.println(sensorValue);

    currentTime = millis();
    // Chaque seconde, calculer et imprimer des litres / heure
    cloopTime = currentTime; // Mises à jour cloopTime
    // Fréquence d'impulsion (Hz) = 7,5Q, Q égal au débit en L / min.
    l_hour = (flow_frequency * 60 / 7.5); // (Fréquence d'impulsion x 60 min) / 7,5Q = débit en L / heure
    flow_frequency = 0; // Réinitialiser le compteur
    Serial.print("Debit d'eau = ");
    Serial.print(l_hour);
    Serial.println(" L/hour");

    
    
    Serial.println("---------------------------");


    Payload p;

    // Attribuer les valeurs au variable de la structure
    p.temperature = temperature;
    p.humidite = humidite;
    p.humidite_eau = sensorValue;
    p.debit = l_hour;
    
    //Envoi des données sur actoboard via Sigfox
    Akeru.send(&p, sizeof(p));

  
    delay(1000);
}
