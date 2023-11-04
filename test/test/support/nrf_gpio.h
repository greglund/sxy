#ifndef NRF_GPIO_H__
#define NRF_GPIO_H__

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void NRF_GPIO_Type;

#define NRF_P0  ((NRF_GPIO_Type*) 0)
#define NRF_P1  ((NRF_GPIO_Type*) 4)


/** @brief Macro for mapping port and pin numbers to values understandable for nrf_gpio functions. */
#define NRF_GPIO_PIN_MAP(port, pin) (((port) << 5) | ((pin) & 0x1F))


/** @brief Pin direction definitions. */
typedef enum
{
    NRF_GPIO_PIN_DIR_INPUT, ///< Input.
    NRF_GPIO_PIN_DIR_OUTPUT, ///< Output.
} nrf_gpio_pin_dir_t;

/** @brief Connection of input buffer. */
typedef enum
{
    NRF_GPIO_PIN_INPUT_CONNECT,   ///< Connect input buffer.
    NRF_GPIO_PIN_INPUT_DISCONNECT ///< Disconnect input buffer.
} nrf_gpio_pin_input_t;

/**
 * @brief Enumerator used for selecting the pin to be pulled down or up at the time of pin
 * configuration.
 */
typedef enum
{
    NRF_GPIO_PIN_NOPULL, ///<  Pin pull-up resistor disabled.
    NRF_GPIO_PIN_PULLDOWN, ///<  Pin pull-down resistor enabled.
    NRF_GPIO_PIN_PULLUP,   ///<  Pin pull-up resistor enabled.
} nrf_gpio_pin_pull_t;

/** @brief Enumerator used for selecting output drive mode. */
typedef enum
{
    NRF_GPIO_PIN_S0S1, ///< !< Standard '0', standard '1'.
    NRF_GPIO_PIN_H0S1, ///< !< High-drive '0', standard '1'.
    NRF_GPIO_PIN_S0H1, ///< !< Standard '0', high-drive '1'.
    NRF_GPIO_PIN_H0H1, ///< !< High drive '0', high-drive '1'.
    NRF_GPIO_PIN_D0S1, ///< !< Disconnect '0' standard '1'.
    NRF_GPIO_PIN_D0H1, ///< !< Disconnect '0', high-drive '1'.
    NRF_GPIO_PIN_S0D1, ///< !< Standard '0', disconnect '1'.
    NRF_GPIO_PIN_H0D1, ///< !< High-drive '0', disconnect '1'.
} nrf_gpio_pin_drive_t;

/** @brief Enumerator used for selecting the pin to sense high or low level on the pin input. */
typedef enum
{
    NRF_GPIO_PIN_NOSENSE, ///<  Pin sense level disabled.
    NRF_GPIO_PIN_SENSE_LOW,      ///<  Pin sense low level.
    NRF_GPIO_PIN_SENSE_HIGH,     ///<  Pin sense high level.
} nrf_gpio_pin_sense_t;


/**
 * @brief Function for configuring the GPIO pin range as output pins with normal drive strength.
 *        This function can be used to configure pin range as simple output with gate driving GPIO_PIN_CNF_DRIVE_S0S1 (normal cases).
 *
 * @note For configuring only one pin as output, use @ref nrf_gpio_cfg_output.
 *       Sense capability on the pin is disabled and input is disconnected from the buffer as the pins are configured as output.
 *
 * @param pin_range_start  Specifies the start number (inclusive) in the range of pin numbers to be configured (allowed values 0-30).
 * @param pin_range_end    Specifies the end number (inclusive) in the range of pin numbers to be configured (allowed values 0-30).
 */
void nrf_gpio_range_cfg_output(uint32_t pin_range_start, uint32_t pin_range_end);

/**
 * @brief Function for configuring the GPIO pin range as input pins with given initial value set, hiding inner details.
 *        This function can be used to configure pin range as simple input.
 *
 * @note  For configuring only one pin as input, use @ref nrf_gpio_cfg_input.
 *        Sense capability on the pin is disabled and input is connected to buffer so that the GPIO->IN register is readable.
 *
 * @param pin_range_start  Specifies the start number (inclusive) in the range of pin numbers to be configured (allowed values 0-30).
 * @param pin_range_end    Specifies the end number (inclusive) in the range of pin numbers to be configured (allowed values 0-30).
 * @param pull_config      State of the pin range pull resistor (no pull, pulled down, or pulled high).
 */
void nrf_gpio_range_cfg_input(uint32_t            pin_range_start,
                                              uint32_t            pin_range_end,
                                              nrf_gpio_pin_pull_t pull_config);

/**
 * @brief Pin configuration function.
 *
 * The main pin configuration function.
 * This function allows to set any aspect in PIN_CNF register.
 *
 * @param pin_number Specifies the pin number.
 * @param dir        Pin direction.
 * @param input      Connect or disconnect the input buffer.
 * @param pull       Pull configuration.
 * @param drive      Drive configuration.
 * @param sense      Pin sensing mechanism.
 */
void nrf_gpio_cfg(
    uint32_t             pin_number,
    nrf_gpio_pin_dir_t   dir,
    nrf_gpio_pin_input_t input,
    nrf_gpio_pin_pull_t  pull,
    nrf_gpio_pin_drive_t drive,
    nrf_gpio_pin_sense_t sense);

/**
 * @brief Function for configuring the given GPIO pin number as output, hiding inner details.
 *        This function can be used to configure a pin as simple output with gate driving GPIO_PIN_CNF_DRIVE_S0S1 (normal cases).
 *
 * @note  Sense capability on the pin is disabled and input is disconnected from the buffer as the pins are configured as output.
 *
 * @param pin_number Specifies the pin number.
 */
void nrf_gpio_cfg_output(uint32_t pin_number);

/**
 * @brief Function for configuring the given GPIO pin number as input, hiding inner details.
 *        This function can be used to configure a pin as simple input.
 *
 * @note  Sense capability on the pin is disabled and input is connected to buffer so that the GPIO->IN register is readable.
 *
 * @param pin_number  Specifies the pin number.
 * @param pull_config State of the pin range pull resistor (no pull, pulled down, or pulled high).
 */
void nrf_gpio_cfg_input(uint32_t pin_number, nrf_gpio_pin_pull_t pull_config);

/**
 * @brief Function for resetting pin configuration to its default state.
 *
 * @param pin_number Specifies the pin number.
 */
void nrf_gpio_cfg_default(uint32_t pin_number);

/**
 * @brief Function for configuring the given GPIO pin number as a watcher. Only input is connected.
 *
 * @param pin_number Specifies the pin number.
 *
 */
void nrf_gpio_cfg_watcher(uint32_t pin_number);

/**
 * @brief Function for disconnecting input for the given GPIO.
 *
 * @param pin_number Specifies the pin number.
 */
void nrf_gpio_input_disconnect(uint32_t pin_number);

/**
 * @brief Function for configuring the given GPIO pin number as input, hiding inner details.
 *        This function can be used to configure pin range as simple input.
 *        Sense capability on the pin is configurable and input is connected to buffer so that the GPIO->IN register is readable.
 *
 * @param pin_number   Specifies the pin number.
 * @param pull_config  State of the pin pull resistor (no pull, pulled down, or pulled high).
 * @param sense_config Sense level of the pin (no sense, sense low, or sense high).
 */
void nrf_gpio_cfg_sense_input(uint32_t             pin_number,
                                              nrf_gpio_pin_pull_t  pull_config,
                                              nrf_gpio_pin_sense_t sense_config);

/**
 * @brief Function for configuring sense level for the given GPIO.
 *
 * @param pin_number   Specifies the pin number.
 * @param sense_config Sense configuration.
 */
void nrf_gpio_cfg_sense_set(uint32_t pin_number, nrf_gpio_pin_sense_t sense_config);

/**
 * @brief Function for setting the direction for a GPIO pin.
 *
 * @param pin_number Specifies the pin number for which to set the direction.
 * @param direction  Specifies the direction.
 */
void nrf_gpio_pin_dir_set(uint32_t pin_number, nrf_gpio_pin_dir_t direction);

/**
 * @brief Function for setting a GPIO pin.
 *
 * For this function to have any effect, the pin must be configured as an output.
 *
 * @param pin_number Specifies the pin number to be set.
 */
void nrf_gpio_pin_set(uint32_t pin_number);

/**
 * @brief Function for clearing a GPIO pin.
 *
 * For this function to have any effect, the pin must be configured as an output.
 *
 * @param pin_number Specifies the pin number to clear.
 */
void nrf_gpio_pin_clear(uint32_t pin_number);

/**
 * @brief Function for toggling a GPIO pin.
 *
 * For this function to have any effect, the pin must be configured as an output.
 *
 * @param pin_number Specifies the pin number to toggle.
 */
void nrf_gpio_pin_toggle(uint32_t pin_number);

/**
 * @brief Function for writing a value to a GPIO pin.
 *
 * For this function to have any effect, the pin must be configured as an output.
 *
 * @param pin_number Specifies the pin number to write.
 * @param value      Specifies the value to be written to the pin.
 * @arg 0 Clears the pin.
 * @arg >=1 Sets the pin.
 */
void nrf_gpio_pin_write(uint32_t pin_number, uint32_t value);

/**
 * @brief Function for reading the input level of a GPIO pin.
 *
 * If the value returned by this function is to be valid, the pin's input buffer must be connected.
 *
 * @param pin_number Specifies the pin number to read.
 *
 * @return 0 if the pin input level is low. Positive value if the pin is high.
 */
uint32_t nrf_gpio_pin_read(uint32_t pin_number);

/**
 * @brief Function for reading the output level of a GPIO pin.
 *
 * @param pin_number Specifies the pin number to read.
 *
 * @return 0 if the pin output level is low. Positive value if pin output is high.
 */
uint32_t nrf_gpio_pin_out_read(uint32_t pin_number);

/**
 * @brief Function for reading the sense configuration of a GPIO pin.
 *
 * @param pin_number Specifies the pin number to read.
 *
 * @return Sense configuration.
 */
nrf_gpio_pin_sense_t nrf_gpio_pin_sense_get(uint32_t pin_number);

/**
 * @brief Function for reading the direction configuration of a GPIO pin.
 *
 * @param pin_number Specifies the pin number to read.
 *
 * @return Direction configuration.
 */
nrf_gpio_pin_dir_t nrf_gpio_pin_dir_get(uint32_t pin_number);

/**
 * @brief Function for reading the status of GPIO pin input buffer.
 *
 * @param pin_number Pin number to be read.
 *
 * @retval Input buffer configuration.
 */
nrf_gpio_pin_input_t nrf_gpio_pin_input_get(uint32_t pin_number);

/**
 * @brief Function for reading the pull configuration of a GPIO pin.
 *
 * @param pin_number Specifies the pin number to read.
 *
 * @retval Pull configuration.
 */
nrf_gpio_pin_pull_t nrf_gpio_pin_pull_get(uint32_t pin_number);

/**
 * @brief Function for setting output direction on the selected pins on the given port.
 *
 * @param p_reg    Pointer to the structure of registers of the peripheral.
 * @param out_mask Mask specifying the pins to set as output.
 */
void nrf_gpio_port_dir_output_set(NRF_GPIO_Type * p_reg, uint32_t out_mask);

/**
 * @brief Function for setting input direction on selected pins on a given port.
 *
 * @param p_reg   Pointer to the structure of registers of the peripheral.
 * @param in_mask Mask that specifies the pins to be set as input.
 */
void nrf_gpio_port_dir_input_set(NRF_GPIO_Type * p_reg, uint32_t in_mask);

/**
 * @brief Function for writing the direction configuration of the GPIO pins in the given port.
 *
 * @param p_reg    Pointer to the structure of registers of the peripheral.
 * @param dir_mask Mask that specifies the direction of pins. Bit set means that the given pin is configured as output.
 */
void nrf_gpio_port_dir_write(NRF_GPIO_Type * p_reg, uint32_t dir_mask);

/**
 * @brief Function for reading the direction configuration of a GPIO port.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return Pin configuration of the current direction settings. Bit set means that the given pin is configured as output.
 */
uint32_t nrf_gpio_port_dir_read(NRF_GPIO_Type const * p_reg);

/**
 * @brief Function for reading the input signals of the GPIO pins on the given port.
 *
 * @param p_reg Pointer to the peripheral registers structure.
 *
 * @return Port input values.
 */
uint32_t nrf_gpio_port_in_read(NRF_GPIO_Type const * p_reg);

/**
 * @brief Function for reading the output signals of the GPIO pins on the given port.
 *
 * @param p_reg Pointer to the peripheral registers structure.
 *
 * @return Port output values.
 */
uint32_t nrf_gpio_port_out_read(NRF_GPIO_Type const * p_reg);

/**
 * @brief Function for writing the GPIO pins output on a given port.
 *
 * @param p_reg Pointer to the structure of registers of the peripheral.
 * @param value Output port mask.
 */
void nrf_gpio_port_out_write(NRF_GPIO_Type * p_reg, uint32_t value);

/**
 * @brief Function for setting high level on selected the GPIO pins on the given port.
 *
 * @param p_reg    Pointer to the structure of registers of the peripheral.
 * @param set_mask Mask with pins to be set as logical high level.
 */
void nrf_gpio_port_out_set(NRF_GPIO_Type * p_reg, uint32_t set_mask);

/**
 * @brief Function for setting low level on selected the GPIO pins on the given port.
 *
 * @param p_reg    Pointer to the structure of registers of the peripheral.
 * @param clr_mask Mask with pins to be set as logical low level.
 */
void nrf_gpio_port_out_clear(NRF_GPIO_Type * p_reg, uint32_t clr_mask);

/**
 * @brief Function for reading pin state of multiple consecutive ports.
 *
 * @param start_port Index of the first port to read.
 * @param length     Number of ports to read.
 * @param p_masks    Pointer to output array where port states will be stored.
 */
void nrf_gpio_ports_read(uint32_t start_port, uint32_t length, uint32_t * p_masks);

/**
 * @brief Function for reading latch state of multiple consecutive ports.
 *
 * @param start_port Index of the first port to read.
 * @param length     Number of ports to read.
 * @param p_masks    Pointer to output array where latch states will be stored.
 */
void nrf_gpio_latches_read(uint32_t   start_port,
                                           uint32_t   length,
                                           uint32_t * p_masks);

/**
 * @brief Function for reading and immediate clearing latch state of multiple consecutive ports.
 *
 * @param start_port Index of the first port to read and clear.
 * @param length     Number of ports to read and clear.
 * @param p_masks    Pointer to output array where latch states will be stored.
 */
void nrf_gpio_latches_read_and_clear(uint32_t   start_port,
                                                     uint32_t   length,
                                                     uint32_t * p_masks);

/**
 * @brief Function for reading latch state of single pin.
 *
 * @param pin_number Pin number.
 *
 * @return 0 if latch is not set. Positive value otherwise.
 */
uint32_t nrf_gpio_pin_latch_get(uint32_t pin_number);

/**
 * @brief Function for clearing latch state of a single pin.
 *
 * @param pin_number Pin number.
 */
void nrf_gpio_pin_latch_clear(uint32_t pin_number);

/**
 * @brief Function for checking if provided pin is present on the MCU.
 *
 * @param[in] pin_number Number of the pin to be checked.
 *
 * @retval true  Pin is present.
 * @retval false Pin is not present.
 */
bool nrf_gpio_pin_present_check(uint32_t pin_number);


/** @} */

#ifdef __cplusplus
}
#endif

#endif // NRF_GPIO_H__

