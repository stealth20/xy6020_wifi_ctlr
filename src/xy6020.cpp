#include "xy6020.h"
#include "settings.h"

Xy6020::Xy6020(SettingsData &cfg, int8_t rx_pin, int8_t tx_pin,
               uint8_t slave_address)
    : mSlaveAddr(slave_address), mSoftSerial(rx_pin, tx_pin),
      mConnectionStatus(false), mCfg(cfg), mReadResultCounter(0) {
  for (int b = 0; b < READ_RESULTS_BUFFER_SIZE; ++b) {
    mReadResults[b] = false;
  }

  // Modbus init
  mSoftSerial.begin(115200, SWSERIAL_8N1);
  mSoftSerial.setTimeout(100);
  mModBus.begin(&mSoftSerial);
  mModBus.master();
  mTs = millis();
}

bool Xy6020::readHregCb(Modbus::ResultCode event, uint16_t transactionId,
                        void *data) {
  mReadResults[mReadResultCounter % READ_RESULTS_BUFFER_SIZE] =
      (event == Modbus::EX_SUCCESS);

  mConnectionStatus = false;
  for (int b = 0; b < READ_RESULTS_BUFFER_SIZE; ++b) {
    if (mReadResults[b]) {
      mConnectionStatus = true;
      break;
    }
  }
  // Serial.printf_P("HReg read: 0x%02X, Mem: %d\n", event, ESP.getFreeHeap());
  //  delay(50);
  yield();

  return true;
}

bool Xy6020::writeCb(Modbus::ResultCode event, uint16_t transactionId,
                     void *data) {
  Serial.printf_P("Request result: 0x%02X, Mem: %d\n", event,
                  ESP.getFreeHeap());
  return true;
}

void Xy6020::task() {
  if (millis() > mTs + 500) {
    mTs = millis();
    if (!mModBus.slave()) {
      mModBus.readHreg(mSlaveAddr, 0x0, mRegisters, OutputState + 1,
                       std::bind(&Xy6020::readHregCb, this,
                                 std::placeholders::_1, std::placeholders::_2,
                                 std::placeholders::_3));
    }
    // for (int r = 0; r < Xy6020RegistersMax; ++r) {
    // Serial.printf_P("%02X: %d\n", r, mRegisters[r]);
    //}
  }

  mModBus.task();
  yield();

  if (mCfg.enable_input_limits) {

    if (inputVoltage() < mCfg.switch_off_voltage &&
        mRegisters[OutputState] != 0) {
      Serial.println("Disable output on lower input level");
      setOutputEnabled(false);
    } else if (inputVoltage() > mCfg.switch_on_voltage &&
               mRegisters[OutputState] == 0) {
      Serial.println("Enable output on upper input level");
      setOutputEnabled(true);
    }
  }
}

bool Xy6020::setTargetVoltage(float voltage) {
  bool ret = true;
  if (voltage < 0) {
    voltage = 0;
    ret = false;
  } else if (voltage > 60) {
    voltage = 60;
    ret = false;
  }
  uint16_t value = voltage * 100;
  setRegister(TargetVoltage, value);
  return ret;
}

bool Xy6020::setMaxCurrent(float current) {
  bool ret = true;
  if (current < 0) {
    current = 0;
    ret = false;
  } else if (current > 20) {
    current = 20.0;
    ret = false;
  }
  uint16_t value = current * 100;
  setRegister(MaxCurrent, value);
  return ret;
}
bool Xy6020::setPower(float power) {
  auto max_voltage = targetVoltage();
  if (max_voltage <= 0) {
    return false;
  }
  if (power > mCfg.max_power) {
    power = mCfg.max_power;
  }
  auto target_current = power / max_voltage;
  if (target_current > 20) {
    target_current = 20;
  }
  setMaxCurrent(target_current);
  return true;
}
bool Xy6020::setMaxPower(float power) {
  if (power < 0) {
    return false;
  }
  bool need_correction = false;
  if (power < mCfg.max_power) {
    need_correction = true;
  }
  mCfg.max_power = power;
  if (need_correction) {
    return setPower(power);
  }
  return true;
}

void Xy6020::setOutputEnabled(bool state) { setRegister(OutputState, state); }

void Xy6020::waitForTransactionDone() {
  while (mModBus.slave()) { // Check if transaction is active
    mModBus.task();
    delay(10);
    yield();
  }
}

void Xy6020::setRegister(Register reg, uint16_t value) {
  Serial.printf_P("setRegister(%d, %d)", reg, value);
  waitForTransactionDone();
  mModBus.writeHreg(mSlaveAddr, reg, value,
                    std::bind(&Xy6020::writeCb, this, std::placeholders::_1,
                              std::placeholders::_2, std::placeholders::_3));
  waitForTransactionDone();
  delay(50);
  yield();
}
