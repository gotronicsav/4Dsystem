#include <genieArduino.h>
#include <OneWire.h>

Genie genie;
#define RESETLINE 4

int b, mode;
int slider_val;
const int chauf = 8;

int const temp = 12;
OneWire Capt_temp(temp);

void setup()
{
  Serial.begin(9600);
  genie.Begin(Serial);

  genie.AttachEventHandler(myGenieEventHandler);

  pinMode(RESETLINE, OUTPUT);
  digitalWrite(RESETLINE, 1);
  delay(100);
  digitalWrite(RESETLINE, 0);
  delay (3500);

  genie.WriteContrast(15);

  pinMode(chauf, OUTPUT);
  genie.WriteStr(0, "MODE OFF");
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, 1, 10);
  mode = 1;
  slider_val = 10;
}


void loop()
{
  int temperature = getTemp();
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, 0, temperature); //on affiche la température ambiante captée

  while (b < 1000) {
    genie.DoEvents();   //TEMPO
    b++;
    delay(1);
  }
  b = 0;

  if (mode == 0) digitalWrite(chauf, HIGH);  //boutton ON >> chauffage ON >> relais à l'état haut #com2

  if (mode == 1) digitalWrite(chauf, LOW);  ////boutton OFF >> chauffage OFF >> relais à l'état bas #com3

  if (mode == 2) { //boutton AUTO
    //la température est inférieur à la consigne (hystérésis de 1)
    if (temperature < slider_val - 1) digitalWrite(chauf, HIGH);  //chauffage ON >> #com2

    //la température est supérieur à la consigne (hystérésis de 1)
    if (temperature > slider_val + 1) digitalWrite(chauf, LOW);  //chauffage OFF >> #com3

  }
}


void myGenieEventHandler(void)
{

  genieFrame Event;
  genie.DequeueEvent(&Event);

  bool button0_state = false;
  bool button1_state = false;
  bool button2_state = false;

  if (Event.reportObject.cmd == GENIE_REPORT_EVENT) //on capte les informations envoyées par l'Afficheur
  {

    if (Event.reportObject.object == GENIE_OBJ_4DBUTTON) //on capte l'état des bouttons
    {

      if (Event.reportObject.index == 0) //on capte l'état du boutton d'index 0 (ON)
      {
        button0_state = genie.GetEventData(&Event);
        if (button0_state == true)
        {
          genie.WriteStr(0, "MODE ON"); //on actualise le texte annociateur du boutton appuyé #com1
          mode = 0; //chauffage ON
        }
      }

      if (Event.reportObject.index == 1) //on capte l'état du boutton d'index 1 (OFF)
      {
        button1_state = genie.GetEventData(&Event);
        if (button1_state == true)
        {
          genie.WriteStr(0, "MODE OFF"); //#com1
          mode = 1; //chauffage OFF
        }
      }

      if (Event.reportObject.index == 2) //on capte l'état du boutton d'index 2 (AUTO)
      {
        button2_state = genie.GetEventData(&Event);
        if (button2_state == true)
        {
          genie.WriteStr(0, "MODE AUTO"); //#com1
          mode = 2; //chauffage AUTO
        }
      }

    }


    if (Event.reportObject.object == GENIE_OBJ_SLIDER) //on capte l'état du slider
    {
      if (Event.reportObject.index == 0)
      {
        slider_val = genie.GetEventData(&Event) + 10; //on sauvegarde la valeur du slider +10 d'offset
        genie.WriteObject(GENIE_OBJ_LED_DIGITS, 1, slider_val); //on affiche cette valeur au texte de consigne
      }
    }

  }

}

int getTemp() { //calcul de la température [programme constructeur]

  byte data[12];
  byte addr[8];

  if ( !Capt_temp.search(addr)) Capt_temp.reset_search();

  Capt_temp.reset();
  Capt_temp.select(addr);
  Capt_temp.write(0x44, 1);

  byte present = Capt_temp.reset();
  Capt_temp.select(addr);
  Capt_temp.write(0xBE);


  for (int i = 0; i < 9; i++) data[i] = Capt_temp.read();

  Capt_temp.reset_search();

  byte MSB = data[1];
  byte LSB = data[0];

  int Vtemp = ((MSB << 8) | LSB);
  int Temp = Vtemp / 16;

  return Temp;

}

