#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <stdio.h>

// Feather RP2040 STEMMA QT / Qwiic Pins
#define I2C_PORT i2c1
#define I2C_SDA 2
#define I2C_SCL 3

#define MAX30101_ADDR 0x57
#define REG_FIFO_WR_PTR 0x04
#define REG_FIFO_RD_PTR 0x06
#define REG_FIFO_DATA 0x07
#define REG_FIFO_CONFIG 0x08
#define REG_MODE_CONFIG 0x09
#define REG_SPO2_CONFIG 0x0A
#define REG_LED1_PA 0x0C
#define REG_LED2_PA 0x0D

// --- I2C Helpers ---
void write_reg(uint8_t reg, uint8_t val) {
  uint8_t buf[2] = {reg, val};
  i2c_write_blocking(I2C_PORT, MAX30101_ADDR, buf, 2, false);
}

uint8_t read_reg(uint8_t reg) {
  uint8_t val;
  i2c_write_blocking(I2C_PORT, MAX30101_ADDR, &reg, 1, true);
  i2c_read_blocking(I2C_PORT, MAX30101_ADDR, &val, 1, false);
  return val;
}

// --- Sensor Initialization ---
void max30101_init() {
  // 1. Soft Reset
  write_reg(REG_MODE_CONFIG, 0x40);
  while (read_reg(REG_MODE_CONFIG) & 0x40) {
    sleep_ms(1);
  }

  // 2. FIFO Config: Sample Average = 4, FIFO Rollover = Enable
  write_reg(REG_FIFO_CONFIG, 0x50);

  // 3. Mode Config: SpO2 Mode (Red + IR LEDs)
  write_reg(REG_MODE_CONFIG, 0x03);

  // 4. SpO2 Config: ADC Range = 4096nA, Sample Rate = 100Hz, LED Pulse Width =
  // 411us
  write_reg(REG_SPO2_CONFIG, 0x27);

  // 5. LED Pulse Amplitude: ~7.2mA for Red and IR (Adjust if signal is too
  // weak/strong)
  write_reg(REG_LED1_PA, 0x24);
  write_reg(REG_LED2_PA, 0x24);

  // 6. Clear Pointers
  write_reg(REG_FIFO_WR_PTR, 0x00);
  write_reg(REG_FIFO_RD_PTR, 0x00);
}

int main() {
  stdio_init_all();

  // Initialize I2C at 400 kHz
  i2c_init(I2C_PORT, 400 * 1000);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);

  sleep_ms(2000); // Wait for serial connection
  printf("Starting MAX30101 Heart Rate Monitor...\n");

  max30101_init();

  // Algorithm Variables
  float dc_filtered = 0;
  const float alpha = 0.95f;
  uint32_t last_beat_time = 0;
  float bpm = 0;
  bool beat_state = false;

  while (true) {
    uint8_t wr_ptr = read_reg(REG_FIFO_WR_PTR);
    uint8_t rd_ptr = read_reg(REG_FIFO_RD_PTR);

    // Check if new data is in the FIFO
    if (wr_ptr != rd_ptr) {
      uint8_t reg = REG_FIFO_DATA;
      uint8_t buffer[6];

      i2c_write_blocking(I2C_PORT, MAX30101_ADDR, &reg, 1, true);
      i2c_read_blocking(I2C_PORT, MAX30101_ADDR, buffer, 6, false);

      // Reconstruct 18-bit values
      uint32_t red =
          ((buffer[0] << 16) | (buffer[1] << 8) | buffer[2]) & 0x03FFFF;
      uint32_t ir =
          ((buffer[3] << 16) | (buffer[4] << 8) | buffer[5]) & 0x03FFFF;

      // Heart Rate Algorithm (IR sensor is most reliable for pulse)
      if (ir > 50000) { // Threshold indicating a finger is present
        // 1. Remove DC offset (skin/bone) using a moving average
        if (dc_filtered == 0)
          dc_filtered = (float)ir;
        dc_filtered = (dc_filtered * alpha) + (ir * (1.0f - alpha));

        // 2. Extract AC pulse wave
        float ac_signal = ir - dc_filtered;

        // 3. Detect downward spike (blood pulse absorbing IR light)
        if (ac_signal < -150.0f && !beat_state) {
          beat_state = true;

          uint32_t current_time = to_ms_since_boot(get_absolute_time());
          uint32_t dt = current_time - last_beat_time;
          last_beat_time = current_time;

          // Filter out impossible heart rates (30 to 200 BPM)
          if (dt > 300 && dt < 2000) {
            float current_bpm = 60000.0f / dt;
            // Smooth the BPM output
            bpm = (bpm == 0) ? current_bpm : (bpm * 0.8f + current_bpm * 0.2f);
            printf("BPM: %.1f\n", bpm);
          }
        } else if (ac_signal > 0.0f) {
          beat_state = false; // Reset threshold
        }
      } else {
        if (dc_filtered != 0) {
          printf("Finger removed.\n");
          dc_filtered = 0;
          bpm = 0;
        }
      }
    }
    sleep_ms(10); // Polling delay
  }
  return 0;
}
