#include "main.h"

void cable_lock() {
  // This is the main function for the cable lock. The program works as follows.
  // 1. The program waits for the CP pin (active low) to go low, indicating that the cable is connected.
  // 2. As soon as this is detected, the program activates the motor for 0.5 seconds to engage the locking pin.
  // 3. After the pin is locked, the program closes the relay, allowing the CP signal to pass to the vehicle.
  // 4. The program then waits for the button pin (active low) to go low, indicating that the user wants to unlock the cable.
  // 5. As soon as this is detected, the program opens the relay, disabling charging.
  // 6. The program then activates the motor for 0.5 seconds in the opposite direction to disengage the locking pin, allowing the cable to be removed.
  // 7. Next we need to wait for the cable to be unplugged, which is detected by the CP pin going consistently high for 1 second.
  // 8. Go back to step 1 and wait for the next cable connection.

  HAL_GPIO_WritePin(GPIOB, MOTOR_MODE1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, MOTOR_MODE2_Pin, GPIO_PIN_RESET);

  while (1) {
    // If the cable is not yet connected, enter standby and wake on CP falling edge.
    // Wakeup from standby triggers a full reset, so execution resumes at main() and
    // re-enters this function; CP_Pin will already be low on that pass.
    // This is step 1
    if (HAL_GPIO_ReadPin(GPIOA, CP_Pin) == GPIO_PIN_SET) {
      __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF1);
      HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1_LOW);
      HAL_PWR_EnterSTANDBYMode();
    }

    // Activate motor to engage locking pin. This is step 2.
    HAL_GPIO_WritePin(GPIOB, MOTOR_SLEEP_Pin, GPIO_PIN_SET);   // Wake DRV8801 from sleep
    HAL_Delay(1);                                               // tWAKE: allow device to initialise
    HAL_GPIO_WritePin(GPIOB, MOTOR_PHASE_Pin, GPIO_PIN_SET);   // Set direction: engage
    HAL_GPIO_WritePin(GPIOB, MOTOR_ENABLE_Pin, GPIO_PIN_SET);  // Enable H-bridge outputs
    HAL_Delay(500);
    HAL_GPIO_WritePin(GPIOB, MOTOR_ENABLE_Pin, GPIO_PIN_RESET); // Disable outputs
    HAL_GPIO_WritePin(GPIOB, MOTOR_SLEEP_Pin, GPIO_PIN_RESET); // Return DRV8801 to sleep

    // Close the relay to enable charging. This is step 3.
    HAL_GPIO_WritePin(GPIOC, RELAY_Pin, GPIO_PIN_SET);

    // Illumiate the status LED to indicate that the cable is locked and charging is enabled.
    HAL_GPIO_WritePin(GPIOC, LED_Pin, GPIO_PIN_SET);

    // Wait for the button pin to go low, indicating that the user wants to unlock the cable
    // This is step 4
    while (HAL_GPIO_ReadPin(GPIOA, BUTTON_Pin) == GPIO_PIN_SET) {
      // Do nothing, just wait
    }

    // Switch off the status LED to indicate that the cable is unlocked and charging is disabled.
    HAL_GPIO_WritePin(GPIOC, LED_Pin, GPIO_PIN_RESET);

    // Open the relay to stop charging. This is step 5.
    HAL_GPIO_WritePin(GPIOC, RELAY_Pin, GPIO_PIN_RESET);

    // Activate motor to disengage locking pin. This is step 6.
    HAL_GPIO_WritePin(GPIOB, MOTOR_SLEEP_Pin, GPIO_PIN_SET);   // Wake DRV8801 from sleep
    HAL_Delay(1);                                               // tWAKE: allow device to initialise
    HAL_GPIO_WritePin(GPIOB, MOTOR_PHASE_Pin, GPIO_PIN_RESET); // Set direction: disengage
    HAL_GPIO_WritePin(GPIOB, MOTOR_ENABLE_Pin, GPIO_PIN_SET);  // Enable H-bridge outputs
    HAL_Delay(500);
    HAL_GPIO_WritePin(GPIOB, MOTOR_ENABLE_Pin, GPIO_PIN_RESET); // Disable outputs
    HAL_GPIO_WritePin(GPIOB, MOTOR_SLEEP_Pin, GPIO_PIN_RESET); // Return DRV8801 to sleep

    // Wait for CP pin to be consistently high for 1 second (cable fully removed)
    // This is step 7
    uint32_t high_since = 0;
    while (1) {
      if (HAL_GPIO_ReadPin(GPIOA, CP_Pin) == GPIO_PIN_SET) {
        if (high_since == 0) high_since = HAL_GetTick();
        if (HAL_GetTick() - high_since >= 1000) break;
      } else {
        high_since = 0;
      }
    }

    // Go back to step 1 and wait for the next cable connection.
    // This is step 8, which is achieved by looping back to the start of the while loop.
  }
}