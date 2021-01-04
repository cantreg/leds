#include <FastLED.h>
#include <colorutils.h>
#include <stdlib.h>
#include <time.h>

#define NUM_LEDS 24
#define NUM_FAN 18

#define PORT_LED_1 3
#define PORT_FAN_1 5
#define PORT_FAN_2 6
#define PORT_FAN_3 9

CPixelView<CRGB> *leds1 = new CPixelView<CRGB>(new CRGB[NUM_LEDS], NUM_LEDS);
CPixelView<CRGB> *fan1 = new CPixelView<CRGB>(new CRGB[NUM_FAN], NUM_FAN);
CPixelView<CRGB> *fan2 = new CPixelView<CRGB>(new CRGB[NUM_FAN], NUM_FAN);
CPixelView<CRGB> *fan3 = new CPixelView<CRGB>(new CRGB[NUM_FAN], NUM_FAN);

CRGB orange = CRGB(255, 60, 0);
CRGB orange2 = CRGB(30, 5, 0);
CRGB white = CRGB(240, 240, 240);
CRGB azure = CRGB(0, 40, 20);
CRGB blue = CRGB(0, 20, 20);
CRGB black = CRGB(0, 0, 0);

class Transition {
  public:
    String tname;

    CPixelView<CRGB> leds;

    bool isSequential = true;
    bool isParallel = false;

    bool isRunning = false;
    bool isFinished = false;

    virtual void transitionSetup() = 0;
    virtual void transition() = 0;
    virtual void transitionWait() {
    };
    virtual bool endCondition() = 0;

    Transition(CPixelView<CRGB> leds) : leds(leds) {
    }

    void makeTransition() {
      if (!isRunning) {
        this->isRunning = true;
        this->isFinished = false;
        this->transitionSetup();
      }
      this->transition();
      this->transitionEnd();
    }

    void transitionEnd() {
      if (endCondition()) {
        this->isRunning = false;
        this->isFinished = true;
        this->transitionWait();
      }
    }

};

class TransitionAllFadeToBlack : public Transition {
  public:
    uint8_t fade;
    TransitionAllFadeToBlack(CPixelView<CRGB> leds) : Transition(leds) {
      tname = "TransitionAllFadeToBlack";
    }
    void transitionSetup() {
      fade = 0;
    }
    void transition() {
      fade += 5;
      if (fade >= 180) fade = 180;
      this->leds.fadeToBlackBy(fade);
    }
    bool endCondition() {
      return fade >= 180;
    }
    void transitionWait() {
      FastLED.delay(800);
    }
};

class TransitionAllBlendBlue : public Transition {
  public:
    uint8_t blend_val;
    CRGB color;
    uint8_t duration;
    TransitionAllBlendBlue(CPixelView<CRGB> leds) : Transition(leds) {
      tname = "TransitionAllBlendBlue";
    }
    void transitionSetup() {
      this->blend_val = 0;
      this->color = blue;
      this->duration = 100;
    }
    void transition() {
      this->blend_val += 1;
      if(this->blend_val > 20) this->color = azure;
      if (this->blend_val >= duration) this->blend_val = duration;
      this->leds.nblend(this->color, this->blend_val);
      FastLED.delay(10);
    }
    bool endCondition() {
      return this->blend_val >= duration;
    }
    void transitionWait() {
      FastLED.delay(100);
    }
};

class TransitionFireFall : public Transition {
  public:
    const static int TR_NUM = 6;
    int c;
    CRGBArray<TR_NUM> templ;

    TransitionFireFall(CPixelView<CRGB> leds) : Transition(leds) {
      tname = "TransitionFireFall";
    }

    void transitionSetup() {
      c = 0;
      this->templ.fill_gradient_RGB(orange2, orange, orange, orange2);
    }
    void transition() {
      for (int j = 0; j < TR_NUM; j++) {
        if (c - j < leds.size() && c - j >= 0) {
          this->leds[c - j] = this->templ[TR_NUM - 1 - j];
        }
      }
      c++;
      FastLED.delay(10);
    }
    bool endCondition() {
      return this->c - this->TR_NUM >= leds.size();
    }
    ~TransitionFireFall() {
      delete[] templ;
    }
};


class TransitionFireFall2 : public TransitionFireFall {
  public:
    const static int TR_NUM = 3;
    CRGBArray<TR_NUM> templ;
    int c2;
    TransitionFireFall2(CPixelView<CRGB> leds) : TransitionFireFall(leds) {
      tname = "TransitionFireFall2";
    }
    void transitionSetup() {
      c = 0;
      c2 = leds.size() - 1 + TR_NUM - 1;
      this->templ.fill_gradient_RGB(orange2, orange, orange2);
    }
    void transition() {
      int ls = leds.size();
      for (int j = 0; j < TR_NUM; j++) {
        if ((c - j < ls / 2) && (c - j >= 0)) {
          this->leds[c - j] = this->templ[TR_NUM - 1 - j];
        }
        if ((c2 - j < ls) && (c2 - j >= ls / 2)) {
          this->leds[c2 - j] = this->templ[TR_NUM - 1 - j];
        }
      }
      c++;
      c2--;
      FastLED.delay(20);
    }
    bool endCondition() {
      return this->c - this->TR_NUM >= leds.size();
    }
};

class TransitionStarFall : public TransitionFireFall {
    const static int TR_NUM = 5;
    int lighting_num = 0;
  public:
    TransitionStarFall(CPixelView<CRGB> leds) : TransitionFireFall(leds) {
      tname = "TransitionStarFall";
    }
    void transitionSetup() {
      c = 0;
      this->templ.fill_solid(azure);
      this->templ[2] = white;
    }
    void transition() {
      this->leds.fill_solid(azure);
      this->templ.fill_solid(azure);
      if (c % 2 == 0) {
        this->templ[2] = white;
        this->templ[3] = white;
        this->templ[4] = white;
      } else {
        this->templ[0] = white;
        this->templ[1] = white;
        this->templ[2] = white;
      }

      for (int j = 0; j < TR_NUM; j++) {
        if (c - j < leds.size() && c - j >= 0) {
          this->leds[c - j] = this->templ[TR_NUM - 1 - j];
        }
      }
      c++;
    }
    void transitionWait() {
      FastLED.delay(600);
    }
};

class TransitionAllBlendOrange : public TransitionAllBlendBlue {
  public:
    TransitionAllBlendOrange(CPixelView<CRGB> leds) : TransitionAllBlendBlue(leds) {
      tname = "TransitionAllBlendOrange";
    }
    void transitionSetup() {
      this->blend_val = 0;
      this->color = orange;
      this->duration = 30;
    }
    void transition() {
      this->blend_val += 1;
      if (this->blend_val >= duration) this->blend_val = duration;
      this->leds.nblend(this->color, this->blend_val);
      FastLED.delay(10);
    }
};

class TransitionComposition {
  public:
    int count;
    Transition ** transitions;
    TransitionComposition (Transition ** transitions, int count) {
      this->transitions = transitions;
      this->count = count;
    }
    bool isFinished() {
      bool isFinished = true;
      for (int j = 0; j < this->count; j++) {
        Transition *t = this->transitions[j];
        isFinished &= t->isFinished;
      }
      return isFinished;
    }
    ~TransitionComposition() {
      for(int i = 0; i < this->count; i++) {
          delete this->transitions[i];
      }
      delete[] this->transitions;
    }
};

TransitionComposition* get_comp(int num) {
  switch (num) {
    case 0:
      return new TransitionComposition(new Transition*[4] { 
        new TransitionAllFadeToBlack(*leds1),
        new TransitionAllFadeToBlack(*fan1),
        new TransitionAllFadeToBlack(*fan2),
        new TransitionAllFadeToBlack(*fan3)
      }, 4);
      break;
    case 1:
      return new TransitionComposition(new Transition*[4] {        
        new TransitionAllBlendBlue(*leds1),
        new TransitionAllBlendBlue(*fan1),
        new TransitionAllBlendBlue(*fan2),
        new TransitionAllBlendBlue(*fan3)
        }, 4);
      break;
    case 2:
      return new TransitionComposition(new Transition*[1] { 
        new TransitionStarFall(*leds1) 
        }, 1);
      break;
    case 3:
      return new TransitionComposition(new Transition*[1] { 
        new TransitionStarFall(*fan1) 
        }, 1);
      break;
    case 4:
      return new TransitionComposition(new Transition*[2] { 
        new TransitionStarFall(*fan2), 
        new TransitionStarFall(*fan3)  
        }, 2);
      break;
    case 5:
      return NULL;
      break;
    case 6:
      return new TransitionComposition(new Transition*[1] { 
        new TransitionFireFall(*leds1) 
        }, 1);
      break;
    case 7:
      return new TransitionComposition(new Transition*[1] { 
        new TransitionFireFall2(*fan1) 
        }, 1);
      break;
    case 8:
      return new TransitionComposition(new Transition*[1] { 
        new TransitionFireFall2(*fan2) 
        }, 1);
      break;
    case 9:
      return new TransitionComposition(new Transition*[1] { 
        new TransitionFireFall2(*fan3) 
        }, 1);
      break;
    case 10:
      return new TransitionComposition(new Transition*[4] {        
        new TransitionAllBlendOrange(*leds1),
        new TransitionAllBlendOrange(*fan1),
        new TransitionAllBlendOrange(*fan2),
        new TransitionAllBlendOrange(*fan3)
        }, 4);
      break;
    default:
      return NULL;
  }
}


int comp_count = 11;
int comp_i = 0;
TransitionComposition *transitions_comp;

void setup() {
	
  srand(time(0));

  leds1->fill_solid(black);
  FastLED.addLeds<NEOPIXEL, PORT_LED_1>(*leds1, leds1->size());
  fan1->fill_solid(black);
  FastLED.addLeds<NEOPIXEL, PORT_FAN_1>(*fan1, fan1->size());
  fan2->fill_solid(black);
  FastLED.addLeds<NEOPIXEL, PORT_FAN_2>(*fan2, fan2->size());
  fan3->fill_solid(black);
  FastLED.addLeds<NEOPIXEL, PORT_FAN_3>(*fan3, fan3->size());
  
  FastLED.show();
  
  transitions_comp = get_comp(comp_i);

}

void loop() {

  if(transitions_comp != NULL) {
    for (int j = 0; j < transitions_comp->count; j++) {
      Transition *t = transitions_comp->transitions[j];
      if(t != NULL) {
        if (!t->isFinished) {
          t->makeTransition();
        }
      }
    }  
  }
  
  if (transitions_comp->isFinished() || transitions_comp == NULL) {
    if (comp_i == (comp_count - 1)) {
      comp_i = 0;
    } else {
      comp_i++;
    }
    delete transitions_comp;
    transitions_comp = get_comp(comp_i);
  }

  FastLED.show();
  FastLED.delay(10);
}