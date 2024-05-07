#include "xy6020.h"

#define BTN_V_PLUS 12
#define BTN_V_MINUS 13
#define BTN_I_PLUS 14
#define BTN_I_MINUS 16

class ButtonCtrl {
  static const uint8_t MAX_STATES = 5;
  enum class ButtonState { Idle, Released, LongPress };
  enum Button { VPlus = 0, VMinus, IPlus, IMinus, ButtonCountMax };

  Xy6020 &mXy;
  bool mLastStates[ButtonCountMax][MAX_STATES];
  uint8_t mPins[ButtonCountMax];
  unsigned long mTs;

  ButtonState getButtonState(Button btn);

public:
  ButtonCtrl(Xy6020 &xy);
  void init();
  void task();
};
