#include <LinkedList.h>

#include <UTFT.h>
#include <URTouch.h>
#include <SPI.h>


using namespace::std;
extern uint8_t Arial_round_16x24[];
extern uint8_t Retro8x16[];
extern uint8_t Grotesk24x48[];

int x, y;

String currentPage;

UTFT myGLCD (SSD1289,38,39,40,41);
URTouch myTouch(6, 5, 4, 3, 2);

String scr = "";

void setup() {
delay(100);

randomSeed(analogRead(0));

pinMode(13, INPUT);
pinMode(12, INPUT);



myGLCD.InitLCD();
myGLCD.clrScr();
myTouch.InitTouch();
myTouch.setPrecision(PREC_MEDIUM);
//Scramble();
drawHomeScreen();

currentPage = "home";
}

int run = 0;
double current_millis;


byte selectedcategory = 1;
String categories[] = {"2x2", "3x3"};
class Category
{
private:

public:
  String category;
  LinkedList<double> times_list = LinkedList<double>();
  LinkedList<String> scrambles = LinkedList<String>();

};

Category kategoriak[2];

class Timer
{
private:
public:
    double time = 0;
    void Start()
    {
        
        while (run==1)
        {
            
             time = round((millis()-current_millis)/10);
             time = time/100;
             

             Write_out_time();
            
            if((digitalRead(13) == HIGH && digitalRead(12) == HIGH) || time >599.9)
            {
              delay(5);
              run++;
            }
        }
        
    }
    void Stop()
    {
      kategoriak[selectedcategory].times_list.add(time);
      kategoriak[selectedcategory].scrambles.add(scr);
         scr = "";
         myGLCD.setColor(238, 255, 204);
        myGLCD.fillRoundRect(15,10,315,55);

        myGLCD.setColor(0,0,0);
        Write_Scramble(Scramble(), 20,20, 1, 1);
        run = 0;
       Write_out_average5();
        Write_out_list();
    }
    void Write_out_average5()
    {
      myGLCD.setFont(Arial_round_16x24);
      while (kategoriak[selectedcategory].times_list.size() >=6)
        {
          kategoriak[selectedcategory].times_list.remove(0);
        }
          myGLCD.setBackColor(238, 255, 204);
      if (kategoriak[selectedcategory].times_list.size() == 5)
        {
          myGLCD.print("Ao5:        " , 80, 165);
          myGLCD.print("Ao5: " + Average5(), 80, 165);          
        }
        else
        {
          myGLCD.print("Ao5:        " , 80, 165);
        }
    }

    void Write_out_time()
    {
      myGLCD.setFont(Grotesk24x48);
      myGLCD.setBackColor(238, 255, 204);
      
      if (time < 0) //Ezek az ifek csak a pozíció miatt kell
        {
          myGLCD.print(Normal_form(time), 115,80);
        }
      else if (time < 59.9)
        {
          myGLCD.print(Normal_form(time), 95,80);
        }
       else if (time > 60)
        {
          myGLCD.print(Normal_form(time), 65,80);
        }
    }

    void Write_out_list()
    {
      myGLCD.setFont(Retro8x16);

      myGLCD.setColor(221, 187, 187); //négyzetnek
      myGLCD.fillRoundRect(240,58,315,160); 

      myGLCD.setBackColor(221, 187, 187); //betűnek

      myGLCD.setColor(0,0,0);
      for (int i = 0; i < kategoriak[selectedcategory].times_list.size(); i++)
      {
        myGLCD.print(String(i+1) + "." + Normal_form(kategoriak[selectedcategory].times_list[i]),241, 60+(i*20));
      }
      
    }
    void Time_plus2(int plus2)
    {
      time += plus2;
      Write_out_time();
    }
    void Time_DNF()
    {
      if (time != 0)
      {
        time = time * -1;
      }
      else
      {
        time = -1;
      }
      
    }
};

Timer Time;

class Buttons
{
private:

public:
    int x1;
    int y1;
    int x2;
    int y2;

    String button_name;
    


    void draw(int X1, int Y1, int X2, int Y2, String Name)
    {
      x1 = X1;
      y1 = Y1;
      x2 = X2;
      y2 = Y2;

      button_name = Name;


      myGLCD.setColor(221, 187, 187);
      myGLCD.fillRoundRect(x1, y1, x2, y2);
      myGLCD.setColor(0,0,0);
      myGLCD.drawRoundRect(x1, y1, x2, y2);
      myGLCD.setFont(Arial_round_16x24);
      myGLCD.setBackColor(221, 187, 187);
    }
    void print(String szoveg, int x, int y)
    {
      myGLCD.print(szoveg, x, y);
    }
};


LinkedList<Buttons> buttons_list = LinkedList<Buttons>();

bool AddButtons(String tmp)
{
  for (int i = 0; i <  buttons_list.size(); i++)
  {
    if (tmp == buttons_list[i].button_name)
    {
      return true;
    }
    
  }
  return false;
}

bool touched = false;
bool released = false;
long start = 0;
long stop = 0;
int holdtime = 0;

bool _15seconds = false;
bool plus2 = false;
bool DNF = false;

bool inspect2 = false; //ha 15-17 az inspection
bool inspectdnf = false;

void loop() 
{
  myTouch.read();
  x = myTouch.getX();
  y = myTouch.getY();
  holdtime = 0;
  if (currentPage == "home")
  {
    updatestate();
    
      //Kategória választó kiválasztása és megrajzolása
      if((x>=240) && (x<=320) && (y>=190) && (y<=239) && run !=1 && _15seconds == false)
      {
        currentPage = "categories";
        drawCategories();
      }

      if((digitalRead(12) == LOW || digitalRead(13) == LOW) && holdtime > 500)
      {
        _15seconds = false; //Nem megy a 15 másodperc
        plus2 = false; //Nem +2-es a rakás
        DNF = false; //Nem dnf a rakás
        inspectdnf = false;
        delay(5);    
        myGLCD.setFont(Arial_round_16x24);
        myGLCD.setBackColor(238, 255, 204);
      


        current_millis= millis(); //az indítás pillanatában az idő
        run++; //run változó növelése 1-gyel jelezve, hogy elindult a mérés
        Time.Start(); //Time osztály Start függvényének meghívása
        if(run > 1)
        {
          Time.Stop();          
          touched = false;
          released = false;
          start = 0;
          stop = 0;
        }
      }

      //+2
      if (((x>=0) && (x<=60) && (y>=90) && (y<=120) && plus2 == false && DNF == false && _15seconds == false && Time.time > 0)  || (inspect2 == true && plus2 == false && DNF == false && _15seconds == false))
      {
        drawBG();
        kategoriak[selectedcategory].times_list[kategoriak[selectedcategory].times_list.size()-1]+=2; 
        Time.Time_plus2(2);
        Time.Write_out_average5();
        Time.Write_out_list();
        delay(20);
        plus2 = true;
        inspect2 = false;

      } //-2
      else if ((x>=1) && (x<=62) && (y>=90) && (y<=120) && plus2 == true && DNF == false && _15seconds == false && Time.time > 0)
      {
        drawBG();
        kategoriak[selectedcategory].times_list[kategoriak[selectedcategory].times_list.size()-1]-=2; 
        Time.Time_plus2(-2);
        Time.Write_out_average5();
        Time.Write_out_list();
        delay(20);
        plus2 = false;

      }
      //DNF
      if ((x>=1) && (x<=62) && (y>=140) && (y<=170) && plus2 == false && _15seconds == false && inspectdnf == false && Time.time != 0)
      {
        drawBG();

        delay(20);
        kategoriak[selectedcategory].times_list[kategoriak[selectedcategory].times_list.size()-1] *= -1; 
        Time.Time_DNF();
        DNF = !DNF;
        Time.Write_out_average5();
        Time.Write_out_list();
      }

      //Del
      if ((x>=1) && (x<=62) && (y>=200) && (y<=230) && _15seconds == false && kategoriak[selectedcategory].times_list.size() >=1)
      {
        plus2 = false;
        DNF = false;
        kategoriak[selectedcategory].times_list.remove(kategoriak[selectedcategory].times_list.size()-1);
        if (Time.time != 0)
        {
          drawBG();
        }
        
        Time.time = 0;
        delay(20);
        Time.Write_out_average5();
        Time.Write_out_list();
      }
      //240,58,315,160
      if ((x>=240) && (x<=315) && (y>=58) && (y<=160) && _15seconds == false)
      {
        currentPage = "listed_times";
        drawListed_times();
      }
      
  }

  
  
  
  if (currentPage == "categories")
  {
    for (int i = 0; i < buttons_list.size(); i++)
    {
      if((x > buttons_list[i].x1) && (y > buttons_list[i].y1) && (x < buttons_list[i].x2) && (y < buttons_list[i].y2) && GetIndexOf(buttons_list[i].button_name) != -5)
      {
        selectedcategory = GetIndexOf(buttons_list[i].button_name);
        delay(30); 
        //Scramble();
        drawHomeScreen();
        Time.time = 0.00;
        currentPage = "home";
        x = 0;
        y = 0;
        Time.Write_out_average5();
        Time.Write_out_time();
      }

    }
  }

  if (currentPage == "listed_times")
  {
    //230,190,319,239
    if ((x>=230) && (x<=320) && (y>=190) && (y<=239))
    {
      currentPage = "home";
      drawHomeScreen();
      drawBG();
    }
    
  }
  
}

long start15 = 0;
bool drawn = false;
void updatestate()
{
  if (myTouch.dataAvailable() && (x<=240) && (x>90) && (y<=190) && run != 1)
  {
    delay(100);
    start15 = millis()+18000;
    _15seconds = !_15seconds;
    drawBG();
  }
  
  if(_15seconds)
  {
    if ((start15-millis())/1000 > 2)
    {
      myGLCD.print(String((start15-millis())/1000 - 2), 140,80);
    }
    
    if ((start15-millis())/1000 - 2 < 10 && drawn == false)
    {
        drawBG();
        drawn = true;
    }
    if ((start15-millis())/1000 <= 2 && (start15-millis())/1000 >= 0)
    {
      myGLCD.print("+2", 140,80);
      inspect2 = true;

    }
    if ((start15-millis())/1000 <= 0)
    {
      inspect2 = false;
      Time.Time_DNF();
      DNF = true;
      Time.Stop();
      _15seconds = false;
      inspectdnf = true;

    }
    
    
    
  }
  else
  {
    Time.Write_out_time();
    drawn = false;
  }

//0.5 "LED-ek"
if (digitalRead(13) == HIGH && digitalRead(12) == HIGH)
  {
    myGLCD.setColor(255,0,0);
    myGLCD.fillCircle(150,220,10);
    if (touched == false)
    {
      touched = true;
      start = millis();
    }
    released = true;
    if (millis()-start > 500)
    {
      myGLCD.setColor(0,255,0);
      myGLCD.fillCircle(170,220,10);
    }
    
  }
  else if (digitalRead(13) == LOW && digitalRead(12) == LOW)
  {
    myGLCD.setColor(238, 255, 204);
    myGLCD.fillCircle(150,220,10);
    myGLCD.fillCircle(170,220,10);
    if (released == true)
    {
      released = false;
      stop = millis();
      drawBG();
    }
    touched = false;
    holdtime = stop - start;
  }

  myGLCD.setColor(1,1,1);
  myGLCD.setFont(Grotesk24x48);
  myGLCD.setBackColor(238, 255, 204);

  


  
}

void drawHomeScreen(){
//Főképernyő
myGLCD.setBackColor(0, 0, 0);
myGLCD.setColor(255, 255, 255);
myGLCD.setColor(0,0,0);

myGLCD.fillScr(238, 255, 204);

myGLCD.setBackColor(238, 255, 204);
myGLCD.setFont(Grotesk24x48);
myGLCD.print("0.00", 95, 80);

//Átlag
myGLCD.setFont(Arial_round_16x24);
myGLCD.setBackColor(238, 255, 204);
myGLCD.print("Ao5: ", 80, 165);

Time.Write_out_list();


//Scramble();
//Write_Scramble(kategoriak[selectedcategory].scrambles[kategoriak[selectedcategory].scrambles.size()-1], 20,20, 1, 1);
 scr = "";
Write_Scramble(Scramble(), 20,20, 1, 1);

drawCategorySelector();
drawPenalties();
drawRemove();
}

void drawCategorySelector()
{
  Buttons category_picker = Buttons();
  category_picker.draw(240,190,319,239,"cats");
  category_picker.print(categories[selectedcategory],255,205);
}
void drawPenalties()
{
  Buttons penalty = Buttons();
  penalty.draw(1,90,62,120,"+2");
  penalty.print(penalty.button_name,11,95);

  /*if (!AddButtons(penalty.button_name))
  {
    buttons_list.add(penalty);
  }*/

  penalty.draw(1,140,62,170, "DNF");
  penalty.print(penalty.button_name,6,145);

  /*if (!AddButtons(penalty.button_name))
  {
    buttons_list.add(penalty);
  }*/

}

void drawRemove()
{
  Buttons rn = Buttons();
  rn.draw(1,200,62,230, "Del");
  rn.print(rn.button_name, 6, 205);

  /*if (!AddButtons(rn.button_name))
  {
    buttons_list.add(rn);
  }*/
}

void drawCategories()
{
  myGLCD.clrScr();


  for (int i = 0; i < sizeof(categories)/sizeof(String); i++)
  {
      Buttons Categories_button = Buttons();


      Categories_button.draw(35, 135-(i*80), 285, 175-(i*80), categories[i]);
      Categories_button.print(categories[i], 135, 145-(i*80));

      if (!AddButtons(Categories_button.button_name))
      {
        buttons_list.add(Categories_button);
      }
      
  }
  
}

void drawListed_times()
{
  myGLCD.clrScr();
  myGLCD.fillScr(238, 255, 204);
  myGLCD.setFont(Retro8x16);

  for (int i = 0; i < kategoriak[selectedcategory].times_list.size() && i < 3; i++)
  {
    myGLCD.print(String(i+1) + "." + Normal_form(kategoriak[selectedcategory].times_list[i]),2, 2+(i*70));
    //myGLCD.print(kategoriak[selectedcategory].scrambles[i],2, 22+(i*40));
    Write_Scramble(kategoriak[selectedcategory].scrambles[i],2, 22+(i*70),0,1);
  }
  
  Buttons back = Buttons();
  back.draw(230,210,319,239,"Back");
  back.print("Back",245,215);

  /*if (!AddButtons(back.button_name))
    {
      buttons_list.add(back);
    }*/
      
}

int GetIndexOf(String tmp)
{
  for (int i = 0; i < sizeof(categories)/sizeof(String); i++)
  {
    if (categories[i] == tmp)
    {
      return i;
    }
    
  }
  return -5;
}


String Average5()
{
  int k = 0;
  double min = kategoriak[selectedcategory].times_list[0];

/*Ez azért kell hogyha a lista első eleme DNF akkor az ne legyen
egyenlő a -1-gyel mert akkor nem működik a minimum kiválasztás*/
  while (kategoriak[selectedcategory].times_list[k]< 0)
    {
      k++;
      min = kategoriak[selectedcategory].times_list[k];
    }
  double max = kategoriak[selectedcategory].times_list[0];
  double ao5 = 0;
  

  for (int i = 0; i < kategoriak[selectedcategory].times_list.size(); i++)
  {
    //Azt ellenőrzi hogyha 2-nél több DNF van akkor megállítja a ciklust és DNF lesz az átlag
    if (max < 0 && kategoriak[selectedcategory].times_list[i] < 0 && i > 0)
    {
      ao5 = -1;
      min = 0;
      max = 0;
      break;
    }
    
    //Ha az adott elem nem DNF
    if (kategoriak[selectedcategory].times_list[i] > 0)
    {   /*Itt is ellenőrizni kell hogy nem DNF-e a max mert az első elem lehetett az.
        egyébként sima maximum és mininmum kiválasztás*/
        if (kategoriak[selectedcategory].times_list[i] > max && max > 0)
        {
          max = kategoriak[selectedcategory].times_list[i];
        }
        else if (kategoriak[selectedcategory].times_list[i] < min)
        {
          min = kategoriak[selectedcategory].times_list[i];
        }
    }
    else //Ez meg akkor kell ha az adott elem DNF (-1) akkor az legyen a max
    {
        max = kategoriak[selectedcategory].times_list[i];
    }
    //Összeadjuk az elemeket
    ao5 += kategoriak[selectedcategory].times_list[i];
    
  }

  ao5 = (ao5-min-max)/3; //Kivonjuk a legkisebbet és a legnagyobbat majd osztunk hárommal
  String output = String(ao5);
  output = Normal_form(ao5);
  myGLCD.setBackColor(238, 255, 204);
  return output;
  
}

String Normal_form(double data)
{
  String output = String(data);
  if (data < 0)
  {
    return "DNF";
  }
  else if (data > 59.999)
  {
    if (data-(int)(data/60)*60 < 10)
    {
      output = String((int)(data/60)) + ":0" + String(data-(int)(data/60)*60);
    }
    else
    {
      output = String((int)(data/60)) + ":" + String(data-(int)(data/60)*60);
    }
  }
  return output;
}

void drawBG()
{
  myGLCD.setColor(238, 255, 204);
  myGLCD.fillRoundRect(65,70,238,130);

  myGLCD.setColor(0,0,0);
}

String Scramble()
{
    //LinkedList<String> scr = LinkedList<String>();
    
    myGLCD.setBackColor(238, 255, 204); //Háttér színének állítása
    myGLCD.setColor(0,0,0); //Használt szín állítása (pl betűszín)
    myGLCD.setFont(Retro8x16); //Betűstílus kiválasztása

    String letters3x3[3][2] ={{"R","L"},{"U","D"},{"F","B"}}; 
    String letters2x2[3] ={"R","U","F"}; //3 elemű tömb ami a betűketjeleket tartalmazza 2x2-höz
    String symbols[3] = {"","'","2"}; //3 elemű tömb ami az egyéb jeleket tartalmazza

    byte prv = -1; //Az előző betű eltárolásásra szolgáló változó
    //String prv2 = ""; //Az előző előtti betű eltárolásásra szolgáló változó

    //Szélesség és magasság tárolásásra szolgáló változók
    int HEIGHT = 20;
    int WIDTH = 20;
    if (categories[selectedcategory] == "3x3") //Ha a 3x3-mas kategória lett kiválasztva 
    {
      
      for (int i = 1; i <= 21; i++)
      {
          //String rndletter = letters3x3[random(0,3), random(0,2)]; Véletlenszerű betű
          byte rndletterindex = random(0,3);
          String rndsymbol = symbols[random(0,3)]; //Véletlenszerű karakter

          //Ameddig az előző vagy előző előtti betű egyenlő a legeneráltal addig generáljon újat
          while (rndletterindex == prv) 
          {
              rndletterindex = random(0,3);
          }
          //prv2 = prv; 
          prv = rndletterindex;

          //betű és karakter kiírása a megadott koordinátákra
          //myGLCD.print(letters3x3[rndletterindex][random(0,2)] + rndsymbol + " ", WIDTH, HEIGHT);
          scr = scr + letters3x3[rndletterindex][random(0,2)] + rndsymbol + " ";
          WIDTH += 22;

          //Ez az if azért kell mert így lehet sort törni
          if(i % 13 == 0)
          {
              HEIGHT += 20;
              WIDTH = 80;
              scr += "/";
          }
      }
      //kategoriak[selectedcategory].scrambles.add(scr);
      return scr;
    }
    if (categories[selectedcategory] == "2x2")
    {
      HEIGHT = 20;
      WIDTH = 80;
      for (int i = 1; i <= 9; i++)
      {
          byte rndletterindex = random(0,3);
          String rndsymbol = symbols[random(0,3)];
          while (rndletterindex == prv)
          {
              rndletterindex = random(0,3);
          }
          prv = rndletterindex;

          //myGLCD.print(letters2x2[rndletterindex] + rndsymbol + " ", WIDTH, HEIGHT);
          WIDTH += 22;

          scr = scr + letters2x2[rndletterindex] + rndsymbol + " ";
      }
      //kategoriak[selectedcategory].scrambles.add(scr);
    }
    return scr;
}
void Write_Scramble(String scramble, int WIDTH, int HEIGHT, byte tabcount, byte linebreakcount)
{
  myGLCD.setBackColor(238, 255, 204);
  
  /*int asd = scramble.length();
  myGLCD.print(String(asd),100,200);*/
  if (scramble.indexOf("/") >= 0)
  {
    myGLCD.print(scramble.substring(0,scramble.indexOf("/")), WIDTH, HEIGHT);
  myGLCD.print(scramble.substring(scramble.indexOf("/")+1, scramble.length()-1), WIDTH+tabcount*60, HEIGHT+linebreakcount*20);
  }
  else
  {
    myGLCD.print(scramble,WIDTH+tabcount*45,HEIGHT);
  }
  
 
  
}