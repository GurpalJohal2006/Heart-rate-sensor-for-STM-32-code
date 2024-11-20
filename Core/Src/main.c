#include "main.h"
#include "max30102_for_stm32_hal.h"
#include <stdio.h>

I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;
GPIO_InitTypeDef buzzer_gpio; // Define GPIO for buzzer
max30102_t max30102;

#define HEART_RATE_THRESHOLD 50 // Threshold to trigger the buzzer
#define BUZZER_PIN GPIO_PIN_5   // PA5 is used fora the buzzer
#define BUZZER_PORT GPIOA

// Function prototypes
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
void check_and_buzz(uint32_t heart_rate);

// UART output function for debugging
void max30102_plot(uint32_t ir_sample, uint32_t red_sample) {
    char buffer[50];
    sprintf(buffer, "IR: %lu, RED: %lu\r\n", ir_sample, red_sample);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}

// Main program
int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART1_UART_Init();

    // Initialize the MAX30102
    max30102_init(&max30102, &hi2c1);
    max30102_reset(&max30102);
    max30102_clear_fifo(&max30102);

    // Configure MAX30102
    max30102_set_fifo_config(&max30102, 0x03, 1, 0x0F); // FIFO settings
    max30102_set_led_current_1(&max30102, 6.2);
    max30102_set_led_current_2(&max30102, 6.2);
    max30102_set_mode(&max30102, 0x03); // SpO2 mode

    while (1) {
        if (max30102_has_interrupt(&max30102)) {
            max30102_interrupt_handler(&max30102);

            // Extract the latest IR and RED samples (assuming the library populates these arrays)
            uint32_t ir_sample = max30102._ir_samples[0];
            uint32_t red_sample = max30102._red_samples[0];

            // Here, we would typically have a heart rate calculation function.
            // For simplicity, let's assume `calculate_heart_rate` returns a bpm value.
            uint32_t aheart_rate = calculate_heart_rate(ir_sample, red_sample);

            // Output the heart rate via UART
            char buffer[50];
            sprintf(buffer, "Heart Rate: %lu bpm\r\n", heart_rate);
            HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

            // Check if heart rate is below threshold, then activate the buzzer
            check_and_buzz(heart_rate);
        }
    }
}

// Check heart rate and trigger buzzer if below threshold
void check_and_buzz(uint32_t heart_rate) {
    if (heart_rate < HEART_RATE_THRESHOLD) {
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET); // Turn buzzer on
        HAL_Delay(500); // Buzzer on duration
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET); // Turn buzzer off
    }
}

// Dummy function to calculate heart rate from IR/RED samples
uint32_t calculate_heart_rate(uint32_t ir_sample, uint32_t red_sample) {
    // Replace with actual heart rate calculation algorithm
    return 60; // Placeholder value
}

// Peripheral Initialization Code
static void MX_GPIO_Init(void) {
    // Initialize GPIO for Buzzer (PA5)
    __HAL_RCC_GPIOA_CLK_ENABLE();
    buzzer_gpio.Pin = BUZZER_PIN;
    buzzer_gpio.Mode = GPIO_MODE_OUTPUT_PP;
    buzzer_gpio.Pull = GPIO_NOPULL;
    buzzer_gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BUZZER_PORT, &buzzer_gpio);
}

static void MX_I2C1_Init(void) {
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);
}

static void MX_USART1_UART_Init(void) {
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 9600;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);
}

void SystemClock_Config(void) {
    // Add system clock configuration code here
    //added it 
}
