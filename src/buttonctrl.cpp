#include "buttonctrl.h"

ButtonCtrl::ButtonCtrl(Xy6020 &xy) : mXy(xy) {
  mPins[Button::VMinus] = BTN_V_MINUS;
  mPins[Button::VPlus] = BTN_V_PLUS;
  mPins[Button::IMinus] = BTN_I_MINUS;
  mPins[Button::IPlus] = BTN_I_PLUS;
}

void ButtonCtrl::init() {
  for (int b = 0; b < Button::ButtonCountMax; ++b) {
    pinMode(mPins[b], INPUT_PULLUP);
    for (int s = 0; s < MAX_STATES; ++s) {
      mLastStates[b][s] = true;
    }
  }
  mTs = millis();
}

ButtonCtrl::ButtonState ButtonCtrl::getButtonState(Button btn) {
  auto current_state = digitalRead(mPins[btn]);

  bool long_press = true;

  for (int s = 1; s < MAX_STATES; ++s) {
    if (mLastStates[btn][s]) {
      long_press = false;
    }
    mLastStates[btn][s - 1] = mLastStates[btn][s];
  }
  mLastStates[btn][MAX_STATES - 1] = current_state;

  if (long_press) {
    return ButtonState::LongPress;
  }
  if (current_state && !mLastStates[btn][MAX_STATES - 2]) {
    return ButtonState::Released;
  }

  return ButtonState::Idle;
}

void ButtonCtrl::task() {
  if (millis() > mTs + 80) {
    mTs = millis();
  } else {
    return;
  }
  for (int b = 0; b < Button::ButtonCountMax; ++b) {
    auto button = static_cast<Button>(b);
    auto state = getButtonState(button);
    if (state == ButtonState::Idle) {
      continue;
    }
    double value = 0.0;
    if (state == ButtonState::Released) {
      //Serial.println("Released");
      value = 0.1;
    } else if (state == ButtonState::LongPress) {
      //Serial.println("LongPress");
      value = 0.3;
    }

    switch (button) {
    case Button::VMinus:
      value *= -1.0;
    case Button::VPlus: {
      auto voltage = mXy.targetVoltage();
      voltage += value;
      mXy.setTargetVoltage(voltage);
    } break;
    case Button::IMinus:
      value *= -1.0;
    case Button::IPlus: {
      auto current = mXy.maxCurrent();
      current += value;
      mXy.setMaxCurrent(current);
    } break;
    default:
      break;
    }
  }
}
