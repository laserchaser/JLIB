#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

/*******************************************************************************
 *
 *  Binary button, pressed or not pressed, debounce logic and patterns module.
 *  Requires proper initialization and the service routine to be called
 *  periodically.
 *
 ******************************************************************************/

#ifndef BIBUTTON_J_H
#define BIBUTTON_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Default number of ticks, assumed to be measured in milliseconds, that a
 * button needs to retain a given state for it to be considered debounced.
 */

#define BIBUTTON_DEBOUNCE_TICKS_DEFAULT   5U

/*
 * Default number of ticks, assumed to be measured in milliseconds, that a
 * button needs to retain a given state after being debounced for it to be
 * considered held either pressed or not pressed for a full phase.
 */

#define BIBUTTON_HOLD_TICKS_DEFAULT       250U

/*
 * A button log is composed of up to 64 entries, where each bit in a 64-bit
 * unsigned integer is a button "phase" or state of pressed or not pressed. A
 * single button pattern is stored and compared with the same log logic.
 */

typedef uint64_t BIBUTTON_log_t;
typedef uint64_t BIBUTTON_pattern_t;

/*******************************************************************************
 *
 * BIBUTTON_pattern_flags_t
 *
 * DESCRIPTION:
 *  Module pattern flags.
 *
 * disabled
 *  Patterns are enabled by default. If set, the pattern handling logic for
 *  the pattern is skipped.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t disabled                      : 1;
    uint8_t reserved1                     : 7;
  };
}
BIBUTTON_pattern_flags_t;

/*******************************************************************************
 *
 * BIBUTTON_pattern_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module pattern flags.
 *
 * length
 *  Length of the pattern. Must be a value between 1 and 64.
 *
 * pattern
 *  The pattern to look for represented by bit values in a single 64-bit
 *  unsigned integer.
 *
 * mask
 *  Calculated at pattern registration, provides a quick way to mask off the
 *  relevant pattern bits depending on pattern length.
 *
 * callback
 *  Function called if/when the pattern is detected.
 *
 * callback_context
 *  Parameter to pass to the callback function - this allows for a single
 *  callback to handle several patterns.
 *
 * next_pattern
 *  Linked list to the next BIBUTTON_pattern_t structure, NULL if the current
 *  node is the end of the list. New patterns will be inserted from longest
 *  length to shortest (see pattern registration function notes).
 *
 ******************************************************************************/

typedef struct
{
  BIBUTTON_pattern_flags_t flags;
  uint8_t length;
  BIBUTTON_pattern_t pattern;
  BIBUTTON_pattern_t mask;
  void (*callback)(uint32_t);
  uint32_t callback_context;
  void* next_pattern;
}
BIBUTTON_pattern_instance_t;

/*******************************************************************************
 *
 * BIBUTTON_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 * disabled
 *  Disables the button if set.
 *
 * pressed
 *  Set if the button is currently pressed according to the hardware read, else,
 *  cleared if the button is currently released according to hardware.
 *
 * debounced_pressed
 *  Set if the button is debounced as pressed, else, cleared if the button is
 *  debounced as released.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t disabled                      : 1;
    uint8_t pressed                       : 1;
    uint8_t debounced_pressed             : 1;
    uint8_t reserved3                     : 5;
  };
}
BIBUTTON_flags_t;

/*******************************************************************************
 *
 * BIBUTTON_hal_get_button_state_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will read the pressed state of the button.
 *
 * RETURN:
 *  True if the button is pressed, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*BIBUTTON_hal_get_button_state_t)(void);

/*******************************************************************************
 *
 * BIBUTTON_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module flags.
 *
 * active_log_length
 *  Number of the last logged button states which are considered "active",
 *  meaning they occurred since the last user or pattern match clear and are
 *  within the log's maximum length of 64-entries.
 *
 * log
 *  The button log entry, where set bits represent a pressed state and cleard
 *  bits represent a released state.
 *
 * debounce_ticks_required
 *  Number of ticks required for the button to be considered debounced.
 *
 * debounce_ticks_count
 *  Keeps track of the number of tick counts which have passed since a new
 *  button debounce began.
 *
 * hold_ticks_required
 *  Number of ticks required for the button to be considered held.
 *
 * hold_ticks_count
 *  Keeps track of the number of tick counts which have passed since a button
 *  debounce has completed. This also counts the ticks which occur during a
 *  failed debounce, though, those ticks are not added until after the
 *  debounce fails.
 *
 * registered_patterns
 *  Linked list of registered patterns. Pattern checking and call-backs are
 *  handled during the service routine.
 *
 * *_hal_*
 *  User-provided functions. See typedef comments.
 *
 ******************************************************************************/

typedef struct
{
  BIBUTTON_flags_t flags;
  uint8_t active_log_length;
  BIBUTTON_log_t log;
  uint16_t debounce_ticks_required;
  uint16_t debounce_ticks_count;
  uint16_t hold_ticks_required;
  uint16_t hold_ticks_count;
  BIBUTTON_pattern_instance_t* registered_patterns;
  BIBUTTON_hal_get_button_state_t get_button_state;
}
BIBUTTON_instance_t;

/*******************************************************************************
 *
 * BIBUTTON_initialize_basic
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values.
 *
 * PARAMETERS:
 *  See BIBUTTON_instance_t.
 *
 ******************************************************************************/

void BIBUTTON_initialize_basic(BIBUTTON_instance_t* instance,
                               BIBUTTON_hal_get_button_state_t get_button_state);

/*******************************************************************************
 *
 * BIBUTTON_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values.
 *
 * PARAMETERS:
 *  See BIBUTTON_instance_t.
 *
 ******************************************************************************/

void BIBUTTON_initialize(BIBUTTON_instance_t* instance,
                         uint16_t debounce_ticks_required,
                         uint16_t hold_ticks_required,
                         bool button_disabled,
                         BIBUTTON_hal_get_button_state_t get_button_state);

/*******************************************************************************
 *
 * BIBUTTON_add_pattern
 *
 * DESCRIPTION:
 *  Registers a new pattern with a binary button. Multiple patterns can be
 *  registered with the same binary button. The user is responsible for ensuring
 *  that the patterns do not collide with each other. Newly registered patterns
 *  will be ordered from longest to shortest - this prevents shorter patterns
 *  from blocking longer patterns. Example: 1101 versus 11101.
 *
 * PARAMETERS:
 *  See BIBUTTON_pattern_instance_t.
 *
 * RETURN:
 *  True if the pattern was registered, else, false.
 *
 ******************************************************************************/

bool BIBUTTON_add_pattern(BIBUTTON_instance_t* instance,
                          BIBUTTON_pattern_instance_t* pattern_instance,
                          BIBUTTON_pattern_t pattern,
                          uint8_t pattern_length,
                          void (*callback)(uint32_t),
                          uint32_t callback_context);

/*******************************************************************************
 *
 * BIBUTTON_service
 *
 * DESCRIPTION:
 *  Services the module instance - checks for state changes, debounces, etc...
 *  This function is meant to be called periodically from the system tick
 *  interrupt. Each call is considered a "tick" and, ideally, the ticks should
 *  be equal to 1-millisecond each.
 *
 ******************************************************************************/

void BIBUTTON_service(BIBUTTON_instance_t* instance);

/*******************************************************************************
 *
 * BIBUTTON_enable
 *
 * DESCRIPTION:
 *  Enables the button if not already enabled.
 *
 * PARAMETERS:
 *  clear
 *   If true, clears the counters and sets the button debounced state to the
 *   current hardware state of the button. If false, simply clears the disabled
 *   flag and the button resumes its logic as if it were never disabled.
 *
 ******************************************************************************/

void BIBUTTON_enable(BIBUTTON_instance_t* instance, bool clear);

/*******************************************************************************
 *
 * BIBUTTON_clear_log
 *
 * DESCRIPTION:
 *  Disables the button - will cause an early return when serviced - no logic
 *  operations performed.
 *
 ******************************************************************************/

void BIBUTTON_disable(BIBUTTON_instance_t* instance);

/*******************************************************************************
 *
 * BIBUTTON_is_button_debounced_pressed
 *
 * DESCRIPTION:
 *  Gets the debounced state of the button.
 *
 * RETURN:
 *  True if the button is debounced pressed, else, false.
 *
 ******************************************************************************/

bool BIBUTTON_is_button_debounced_pressed(BIBUTTON_instance_t* instance);

/*******************************************************************************
 *
 * BIBUTTON_clear_log
 *
 * DESCRIPTION:
 *  Clears the button event log.
 *
 ******************************************************************************/

void BIBUTTON_clear_log(BIBUTTON_instance_t* instance);

/*******************************************************************************
 *
 * BIBUTTON_is_button_pressed
 *
 * DESCRIPTION:
 *  Gets the hardware state of the button.
 *
 * RETURN:
 *  True if the button is pressed according to hardware (non-debounced), else,
 *  false.
 *
 * NOTES:
 *  Helper function primarily called from BIBUTTON_service. Exposed here for
 *  unit testing.
 *
 ******************************************************************************/

bool BIBUTTON_is_button_pressed(BIBUTTON_instance_t* instance);

/*******************************************************************************
 *
 * BIBUTTON_log_event
 *
 * DESCRIPTION:
 *  Adds a 0 (released) or 1 (pressed) bit to the end of the button event log.
 *  The existing log is shifted to the left and the new event is added as the
 *  least-significant-bit.
 *
 * PARAMETERS:
 *  pressed
 *   True if the button is pressed (add 1 to log), else, false (add 0 to log).
 *
 * NOTES:
 *  Helper function primarily called from BIBUTTON_service. Exposed here for
 *  unit testing.
 *
 ******************************************************************************/

void BIBUTTON_log_event(BIBUTTON_instance_t* instance, bool pressed);

/*******************************************************************************
 *
 * BIBUTTON_debounce_and_hold_handler
 *
 * DESCRIPTION:
 *  Called from the service routine and handles debounce and button steady
 *  state logic.
 *
 * NOTES:
 *  Helper function primarily called from BIBUTTON_service. Exposed here for
 *  unit testing.
 *
 ******************************************************************************/

void BIBUTTON_debounce_and_hold_handler(BIBUTTON_instance_t* instance);

/*******************************************************************************
 *
 * BIBUTTON_pattern_handler
 *
 * DESCRIPTION:
 *  Called from the service routine and handles pattern logic. Iterates through
 *  the enabled patterns for this button. On the first match, clears the active
 *  log count and calls the callback function with the callback parameter.
 *
 * NOTES:
 *  Helper function primarily called from BIBUTTON_service. Exposed here for
 *  unit testing.
 *
 ******************************************************************************/

void BIBUTTON_pattern_handler(BIBUTTON_instance_t* instance);

#ifdef __cplusplus
}
#endif
#endif // BIBUTTON_J_H

/*******************************************************************************
 *
 *  Simple mutex module for shared data busses.
 *
 ******************************************************************************/

#ifndef BUS_MUTEX_J_H
#define BUS_MUTEX_J_H

/*******************************************************************************
 *
 *  Soft timer module with microsecond-ish precision. A single hardware timer
 *  initialized by the user is required. The soft timers are then created with
 *  a ticket system - the user creates a ticket and periodically polls to
 *  determine if the ticket has expired.
 *
 *  Ther user configured hardware timer should ideally have a nanosecond
 *  resolution which is evenly divisible into microseconds, and with a period
 *  large enough that the ISR is not called too often. The timer ISR should
 *  be called at the end of each period and needs to call the instance period
 *  expiration handler.
 *
 ******************************************************************************/

#ifndef UTIMER_J_H
#define UTIMER_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *
 * UTIMER_ticket_t
 *
 * DESCRIPTION:
 *  Captures a snapshot of the hardware timer tick value and soft timer period
 *  and hold calculated expiration values.
 *
 * start_ticks_capture
 *  Copy of the hardware timer tick value when the ticket is created.
 *
 * start_periods_capture
 *  Copy of the UTIMER periods counter when the ticket is created.
 *
 * expiration_ticks
 *  The hardware timer tick value which will indicate expiration.
 *
 * expiration_periods
 *  The soft timer period value which will indicate expiration.
 *
 * expiration_us
 *  Not needed for internal operation, but placed here to keep track of the
 *  user's original ticket value for debugging purposes.
 *
 ******************************************************************************/

typedef struct
{
  uint64_t start_ticks_capture;
  uint64_t start_periods_capture;
  uint64_t expiration_ticks;
  uint64_t expiration_periods;
  uint64_t expiration_us;
}
UTIMER_ticket_t;

/*******************************************************************************
 *
 * UTIMER_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t reserved0                     : 8;
  };
}
UTIMER_flags_t;

/*******************************************************************************
 *
 * UTIMER_hal_get_hardware_counter_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will get the tick count of the hardware clock.
 *
 * RETURN:
 *  Tick count of the hardware clock.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef uint64_t (*UTIMER_hal_get_hardware_counter_t)(void);

/*******************************************************************************
 *
 * UTIMER_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module flags.
 *
 * ticks_per_microsecond
 *  Hardware timer ticks for every microsecond. For microsecond accuracy, this
 *  NEEDS to be a value greater than, or equal to, 1.
 *
 * ticks_per_period
 *  Hardware timer ticks for every period.
 *
 * period_counter
 *  Keeps track of the number of periods passed by the hardware timer. This
 *  value should be incremented by the hardware timer ISR.
 *
 * *_hal_*
 *  User-provided functions. See typedef comments.
 *
 ******************************************************************************/

typedef struct
{
  UTIMER_flags_t flags;
  uint64_t ticks_per_microsecond;
  uint64_t ticks_per_period;
  volatile uint64_t period_counter;
  UTIMER_hal_get_hardware_counter_t get_hardware_counter;
}
UTIMER_instance_t;

/*******************************************************************************
 *
 * UTIMER_period_isr_handler
 *
 * DESCRIPTION:
 *  Handler for the hardware timer period interrupt. The user code must call
 *  this function from their hardware timer period ISR.
 *
 * IMPORTANT:
 *  The user MUST ensure that the hardware timer period interrupt is enabled
 *  and that this handler is called from the ISR.
 *
 ******************************************************************************/

void UTIMER_period_isr_handler(UTIMER_instance_t* instance);

/*******************************************************************************
 *
 * UTIMER_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values.
 *
 * PARAMETERS:
 *  See UTIMER_instance_t.
 *
 ******************************************************************************/

void UTIMER_initialize(UTIMER_instance_t* instance,
                       uint64_t ticks_per_microsecond,
                       uint64_t ticks_per_period,
                       UTIMER_hal_get_hardware_counter_t get_hardware_counter);

/*******************************************************************************
 *
 * UTIMER_ticket_create
 *
 * DESCRIPTION:
 *  Creates a new ticket with a snapshot of the current hardware tick count
 *  and period counter and calculates the tick count and period for expiration.
 *
 * PARAMETERS:
 *  ticket
 *   Pointer to a user-provided ticket.
 *
 *  expiration_us
 *   Number of microseconds until expiration. Can be set as 0 if the timer is
 *   being used as a stopwatch.
 *
 ******************************************************************************/

void UTIMER_ticket_create(UTIMER_instance_t* instance,
                          UTIMER_ticket_t* ticket,
                          uint64_t expiration_us);

/*******************************************************************************
 *
 * UTIMER_ticket_has_expired
 *
 * DESCRIPTION:
 *  Checks if a ticket has expired.
 *
 * PARAMETERS:
 *  ticket
 *   Pointer to a user-provided ticket.
 *
 * RETURN:
 *  True if the ticket's expiration has come or past, else, false.
 *
 ******************************************************************************/

bool UTIMER_ticket_has_expired(UTIMER_instance_t* instance, UTIMER_ticket_t* ticket);

/*******************************************************************************
 *
 * UTIMER_ticket_elapsed_time
 *
 * DESCRIPTION:
 *  Calculates the total time elapsed, in microseconds, since the creation of
 *  a ticket.
 *
 * PARAMETERS:
 *  ticket
 *   Pointer to a user-provided ticket.
 *
 * RETURN:
 *  Number of microseconds which have passed since the ticket was created.
 *
 ******************************************************************************/

uint64_t UTIMER_ticket_elapsed_time(UTIMER_instance_t* instance, UTIMER_ticket_t* ticket);

#ifdef __cplusplus
}
#endif
#endif // UTIMER_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *
 *  I2C master module. Supports both 7-bit and 10-bit addressing.
 *
 ******************************************************************************/

#ifndef SER_I2C_J_H
#define SER_I2C_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The default timeout between data transactions which will cause the
 * transaction to abort. (Essentially, a watchdog). A timeout value of 0 will
 * disable a transaction on timeout.
 */

#define SERI2C_TIMEOUT_DEFAULT_uS         100000U
#define SERI2C_TIMEOUT_DISABLED_uS        0U

/*
 * Define bit-masks for the 7-bit and 10-bit slave address types.
 */

#define SERI2C_7BIT_ADDRESS_MASK          0x007FU
#define SERI2C_10BIT_ADDRESS_MASK         0x03FFU

/*******************************************************************************
 *
 * SERI2C_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 * busy
 *  Set when there is an active transaction. Cleared when the transaction
 *  is completed.
 *
 * transmit_register
 *  Set if the transaction is a register read/write and the first byte sent
 *  needs to be the register address. Cleared once the register address is
 *  sent.
 *
 * repeated_start
 *  Set if the transaction has done a repeated start. This is needed for
 *  10-bit addressing.
 *
 * task_state
 *  State machine state.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t busy                          : 1;
    uint8_t transmit_register             : 1;
    uint8_t repeated_start                : 1;
    uint8_t task_state                    : 5;
  };
}
SERI2C_flags_t;

/*******************************************************************************
 *
 * SERI2C_error_flags_t
 *
 * DESCRIPTION:
 *  Module error flags.
 *
 * timeout
 *  The maximum allowable time between receiving or transmitting the next data
 *  element has passed.
 *
 * nak_response
 *  Slave device responded with a NACK causing the transaction to be cut short.
 *
 * collision
 *  Collision detected on the I2C line.
 *
 * rx_overflow
 *  Receive buffer overflow.
 *
 * other
 *  All other types of errors.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t timeout                       : 1;
    uint8_t nak_response                  : 1;
    uint8_t collision                     : 1;
    uint8_t rx_overflow                   : 1;
    uint8_t other                         : 1;
    uint8_t reserved5                     : 3;
  };
}
SERI2C_error_flags_t;

/*******************************************************************************
 *
 * SERI2C_hal_is_rx_ready_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if the hardware Rx buffer has data available.
 *
 * RETURN:
 *  True if the hardware Rx buffer has data available, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*SERI2C_hal_is_rx_ready_t)(void);

/*******************************************************************************
 *
 * SERI2C_hal_is_tx_ready_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if the hardware Tx buffer has room available.
 *
 * RETURN:
 *  True if the hardware Tx buffer has room available, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*SERI2C_hal_is_tx_ready_t)(void);

/*******************************************************************************
 *
 * SERI2C_hal_read_rx_register_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will read the next available element from the hardware Rx buffer.
 *
 * RETURN:
 *  The next read element from the Rx hardware buffer.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef uint8_t (*SERI2C_hal_read_rx_register_t)(void);

/*******************************************************************************
 *
 * SERI2C_hal_write_tx_register_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will write a value to the hardware Tx buffer.
 *
 * PARAMETERS:
 *  value
 *   Element to be written to the hardware Tx buffer.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SERI2C_hal_write_tx_register_t)(uint8_t);

/*******************************************************************************
 *
 * SERI2C_hal_send_start_condition_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will send a start condition.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SERI2C_hal_send_start_condition_t)(void);

/*******************************************************************************
 *
 * SERI2C_hal_is_send_start_condition_completed_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will determine if sending the start condition has finished.
 *
 * RETURN:
 *  True if the start condition has finished, else, false;
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*SERI2C_hal_is_send_start_condition_completed_t)(void);

/*******************************************************************************
 *
 * SERI2C_hal_send_restart_condition_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will send a restart condition.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SERI2C_hal_send_restart_condition_t)(void);

/*******************************************************************************
 *
 * SERI2C_hal_is_send_restart_condition_completed_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will determine if sending the restart condition has finished.
 *
 * RETURN:
 *  True if the restart condition has finished, else, false;
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*SERI2C_hal_is_send_restart_condition_completed_t)(void);

/*******************************************************************************
 *
 * SERI2C_hal_send_stop_condition_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will sends a stop condition.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SERI2C_hal_send_stop_condition_t)(void);

/*******************************************************************************
 *
 * SERI2C_hal_is_send_stop_condition_completed_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will determine if sending the stop condition has finished.
 *
 * RETURN:
 *  True if the stop condition has finished, else, false;
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*SERI2C_hal_is_send_stop_condition_completed_t)(void);

/*******************************************************************************
 *
 * SERI2C_hal_send_ack_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will send an ACK response.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SERI2C_hal_send_ack_t)(void);

/*******************************************************************************
 *
 * SERI2C_hal_is_send_ack_completed_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will determine if sending the ACK rsponse has completed.
 *
 * RETURN:
 *  True if the ACK response has finished, else, false;
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*SERI2C_hal_is_send_ack_completed_t)(void);

/*******************************************************************************
 *
 * SERI2C_hal_send_nak_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will send an NAK response.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SERI2C_hal_send_nak_t)(void);

/*******************************************************************************
 *
 * SERI2C_hal_is_send_nak_completed_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will determine if sending the NAK response has completed.
 *
 * RETURN:
 *  True if the NAK response has finished, else, false;
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*SERI2C_hal_is_send_nak_completed_t)(void);

/*******************************************************************************
 *
 * SERI2C_hal_is_ack_received_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will determine if an ACK has been received.
 *
 * RETURN:
 *  True if an ACK has been received, else, false;
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*SERI2C_hal_is_ack_received_t)(void);

/*******************************************************************************
 *
 * SERI2C_hal_error_check_nak_received_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will determine if an NAK response has been received. (Not always an
 *  error.)
 *
 * RETURN:
 *  True if an NAK has been received, else, false;
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*SERI2C_hal_error_check_nak_received_t)(void);

/*******************************************************************************
 *
 * SERI2C_hal_error_check_collision_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will determine if a collision on the bus has been detected.
 *
 * RETURN:
 *  True if a collision was detected, else, false;
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef bool (*SERI2C_hal_error_check_collision_t)(void);

/*******************************************************************************
 *
 * SERI2C_hal_error_check_rx_overflow_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will determine if a receive buffer overflow has been detected.
 *
 * RETURN:
 *  True if a receive buffer overflow was detected, else, false;
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef bool (*SERI2C_hal_error_check_rx_overflow_t)(void);

/*******************************************************************************
 *
 * SERI2C_hal_error_check_other_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will determine if any other type of error has been detected.
 *
 * RETURN:
 *  True if any other type of error was detected, else, false;
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef bool (*SERI2C_hal_error_check_other_t)(void);

/*******************************************************************************
 *
 * SERI2C_hal_clear_errors_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will clear all I2C errors.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*SERI2C_hal_clear_errors_t)(void);

/*******************************************************************************
 *
 * SERI2C_hal_enable_rx_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will enable the Rx line (this is needed for various versions of
 *  chips).
 *
 * PARAMETERS:
 *  last_byte
 *   True if the next byte to be read is the last, else, false. This is a
 *   tweak from the original library code to accomodate chips which need to
 *   program the ACK or NAK response ahead of time.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*SERI2C_hal_enable_rx_t)(bool);

/*******************************************************************************
 *
 * SERI2C_hal_new_task_reset_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will clear or reset various registers in preparation for a new task.
 *  (Eg. FIFO, FSM, etc...)
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*SERI2C_hal_new_task_reset_t)(void);

/*******************************************************************************
 *
 * SERI2C_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module flags.
 *
 * errors
 *  Module error flags.
 *
 * utimer
 *  User-provided initialized instance of a UTIMER.
 *
 * utimer_ticket
 *  Data structure used with the UTIMER instance.
 *
 * register_length
 *  The length, in bytes, up to the byte-length of the register parameter, that
 *  make up the real register value which needs to be transmitted.
 *
 * register_bytes_remaining
 *  Keeps track of the number of register bytes which still need to be
 *  transmitted during the register write portion of the I2C transaction.
 *
 * slave_address
 *  Stores the slave address of the current slave device.
 *
 * rx_buffer
 *  Element buffer which will hold the element data from the RX transaction.
 *
 * tx_buffer
 *  Element buffer which contains the element data for the TX transaction.
 *
 * register_value
 *  Stores the register offset value of the current slave device if this is a
 *  register read or write operation.
 *
 * timeout_us
 *  Value which will be loaded into the utimer_ticket. Initialized to the
 *  module default but can be modified by the user.
 *
 * rx_element_count
 *  Number of elements to be received in the RX transaction.
 *
 * tx_element_count
 *  Number of elements to be transmitted in the TX transaction.
 *
 * rx_element_counter
 *  Number of elements which have been received during the RX transaction.
 *
 * tx_element_counter
 *  Number of elements which have been transmitted during the TX transaction.
 *
 * *_hal_*
 *  User-provided functions. See typedef comments.
 *
 ******************************************************************************/

typedef struct
{
  SERI2C_flags_t flags;
  SERI2C_error_flags_t errors;
  UTIMER_instance_t* utimer;
  UTIMER_ticket_t utimer_ticket;
  uint8_t register_length;
  uint8_t register_bytes_remaining;
  uint16_t slave_address;
  uint8_t* rx_buffer;
  uint8_t* tx_buffer;
  uint32_t register_value;
  uint32_t timeout_us;
  uint32_t rx_element_count;
  uint32_t tx_element_count;
  uint32_t rx_element_counter;
  uint32_t tx_element_counter;
  SERI2C_hal_is_rx_ready_t is_rx_ready;
  SERI2C_hal_is_tx_ready_t is_tx_ready;
  SERI2C_hal_read_rx_register_t read_rx_register;
  SERI2C_hal_write_tx_register_t write_tx_register;
  SERI2C_hal_send_start_condition_t send_start_condition;
  SERI2C_hal_is_send_start_condition_completed_t is_send_start_condition_completed;
  SERI2C_hal_send_restart_condition_t send_restart_condition;
  SERI2C_hal_is_send_restart_condition_completed_t is_send_restart_condition_completed;
  SERI2C_hal_send_stop_condition_t send_stop_condition;
  SERI2C_hal_is_send_stop_condition_completed_t is_send_stop_condition_completed;
  SERI2C_hal_send_ack_t send_ack;
  SERI2C_hal_is_send_ack_completed_t is_send_ack_completed;
  SERI2C_hal_send_nak_t send_nak;
  SERI2C_hal_is_send_nak_completed_t is_send_nak_completed;
  SERI2C_hal_is_ack_received_t is_ack_received;
  SERI2C_hal_error_check_nak_received_t error_check_nak_received;
  SERI2C_hal_error_check_collision_t error_check_collision;
  SERI2C_hal_error_check_rx_overflow_t error_check_rx_overflow;
  SERI2C_hal_error_check_other_t error_check_other;
  SERI2C_hal_clear_errors_t clear_errors;
  SERI2C_hal_enable_rx_t enable_rx;
  SERI2C_hal_new_task_reset_t new_task_reset;
}
SERI2C_instance_t;

/*******************************************************************************
 *
 * SERI2C_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values.
 *
 * PARAMETERS:
 *  See SERI2C_instance_t.
 *
 ******************************************************************************/

void SERI2C_initialize(SERI2C_instance_t* instance,
                       UTIMER_instance_t* utimer,
                       SERI2C_hal_is_rx_ready_t is_rx_ready,
                       SERI2C_hal_is_tx_ready_t is_tx_ready,
                       SERI2C_hal_read_rx_register_t read_rx_register,
                       SERI2C_hal_write_tx_register_t write_tx_register,
                       SERI2C_hal_send_start_condition_t send_start_condition,
                       SERI2C_hal_is_send_start_condition_completed_t is_send_start_condition_completed,
                       SERI2C_hal_send_restart_condition_t send_restart_condition,
                       SERI2C_hal_is_send_restart_condition_completed_t is_send_restart_condition_completed,
                       SERI2C_hal_send_stop_condition_t send_stop_condition,
                       SERI2C_hal_is_send_stop_condition_completed_t is_send_stop_condition_completed,
                       SERI2C_hal_send_ack_t send_ack,
                       SERI2C_hal_is_send_ack_completed_t is_send_ack_completed,
                       SERI2C_hal_send_nak_t send_nak,
                       SERI2C_hal_is_send_nak_completed_t is_send_nak_completed,
                       SERI2C_hal_is_ack_received_t is_ack_received,
                       SERI2C_hal_error_check_nak_received_t error_check_nak_received,
                       SERI2C_hal_error_check_collision_t error_check_collision,
                       SERI2C_hal_error_check_rx_overflow_t error_check_rx_overflow,
                       SERI2C_hal_error_check_other_t error_check_other,
                       SERI2C_hal_clear_errors_t clear_errors,
                       SERI2C_hal_enable_rx_t enable_rx,
                       SERI2C_hal_new_task_reset_t new_task_reset);

/*******************************************************************************
 *
 * SERI2C_set_transaction_timeout
 *
 * DESCRIPTION:
 *  Sets the transaction timeout value. Every time an element is received or
 *  transmitted the timeout timer will be reset with this value. If a timeout
 *  occurs, the operation is aborted.
 *
 * PARAMETERS:
 *  timeout_us
 *   Timeout to set in microseconds.
 *
 ******************************************************************************/

void SERI2C_set_transaction_timeout(SERI2C_instance_t* instance,
                                    uint32_t timeout_us);

/*******************************************************************************
 *
 * SERI2C_begin_new_write_read
 *
 * DESCRIPTION:
 *  Attempts to begin a new write task followed by a read task. The request
 *  will be accepted if the instance is not currently busy with a task.
 *
 * PARAMETERS:
 *  slave_address
 *   I2C hardware address of the slave device. Can either be 7-bit or 10-bit.
 *
 *  tx_buffer
 *   Buffer which contains the data to be written to the slave device.
 *
 *  tx_length
 *   Number of bytes to write. (Tx buffer must be this size or greater.)
 *
 *  rx_buffer
 *   Buffer which will hold the data read from the slave device.
 *
 *  rx_length
 *   Number of bytes to read. (Rx buffer must be this size or greater.)
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool SERI2C_begin_new_write_read(SERI2C_instance_t* instance,
                                 uint16_t slave_address,
                                 uint8_t* tx_buffer,
                                 uint32_t tx_length,
                                 uint8_t* rx_buffer,
                                 uint32_t rx_length);

/*******************************************************************************
 *
 * SERI2C_begin_new_read
 *
 * DESCRIPTION:
 *  Attempts to begin a new read transaction. This can only be done if the
 *  instance is currently not busy.
 *
 * PARAMETERS:
 *  See SERI2C_begin_new_write_read.
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool SERI2C_begin_new_read(SERI2C_instance_t* instance,
                           uint16_t slave_address,
                           uint8_t* rx_buffer,
                           uint32_t rx_length);

/*******************************************************************************
 *
 * SERI2C_begin_new_write
 *
 * DESCRIPTION:
 *  Attempts to begin a new write transaction. This can only be done if the
 *  instance is currently not busy.
 *
 * PARAMETERS:
 *  See SERI2C_begin_new_write_read.
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool SERI2C_begin_new_write(SERI2C_instance_t* instance,
                            uint16_t slave_address,
                            uint8_t* tx_buffer,
                            uint32_t tx_length);

/*******************************************************************************
 *
 * SERI2C_begin_new_register_read
 *
 * DESCRIPTION:
 *  Attempts to begin a new register read transaction. This can only be done
 *  if the instance is currently not busy.
 *
 * PARAMETERS:
 *  See SERI2C_begin_new_write_read.
 *
 *  register_value
 *   The register value. This will be the first data written to the slave
 *   after the slave is addressed for writing.
 *
 *  register_length
 *   The length, in bytes, of the register value. This accomodates the possible
 *   need for longer register requirements, such as EEPROM memory addressing.
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool SERI2C_begin_new_register_read(SERI2C_instance_t* instance,
                                    uint16_t slave_address,
                                    uint32_t register_value,
                                    uint8_t register_length,
                                    uint8_t* rx_buffer,
                                    uint32_t rx_length);

/*******************************************************************************
 *
 * SERI2C_begin_new_register_write
 *
 * DESCRIPTION:
 *  Attempts to begin a new register write task. The request will be accepted
 *  if the instance is not currently busy with a task.
 *
 * PARAMETERS:
 *  See SERI2C_begin_new_write_read.
 *  See SERI2C_begin_new_register_read.
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool SERI2C_begin_new_register_write(SERI2C_instance_t* instance,
                                     uint16_t slave_address,
                                     uint32_t register_value,
                                     uint8_t register_length,
                                     uint8_t* tx_buffer,
                                     uint32_t tx_length);

/*******************************************************************************
 *
 * SERI2C_service
 *
 * DESCRIPTION:
 *  Services the task state machine. Must be called repeatedly until the task
 *  is completed.
 *
 * RETURN:
 *  False if there is an ongoing task which has not completed, else, true.
 *
 ******************************************************************************/

bool SERI2C_service(SERI2C_instance_t* instance);

/*******************************************************************************
 *
 * SERI2C_abort
 *
 * DESCRIPTION:
 *  Aborts the current task.
 *
 ******************************************************************************/

void SERI2C_abort(SERI2C_instance_t* instance);

/*******************************************************************************
 *
 * SERI2C_is_busy
 *
 * DESCRIPTION:
 *  Determines if an instance is currently busy with a task.
 *
 * RETURN:
 *  True if the instance is busy with a task, else, false.
 *
 ******************************************************************************/

bool SERI2C_is_busy(SERI2C_instance_t* instance);

#ifdef __cplusplus
}
#endif
#endif // SER_I2C_J_H

/*******************************************************************************
 *
 *  I2C Batch master module. Supports both 7-bit and 10-bit addressing.
 *
 ******************************************************************************/

#ifndef SER_I2CBAT_J_H
#define SER_I2CBAT_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The default timeout between data transactions which will cause the
 * transaction to abort. (Essentially, a watchdog). A timeout value of 0 will
 * disable a transaction on timeout.
 */

#define SERI2CBAT_TIMEOUT_DEFAULT_uS      100000U
#define SERI2CBAT_TIMEOUT_DISABLED_uS     0U

/*
 * Define bit-masks for the 7-bit and 10-bit slave address types.
 */

#define SERI2CBAT_7BIT_ADDRESS_MASK       0x007FU
#define SERI2CBAT_10BIT_ADDRESS_MASK      0x03FFU

/*******************************************************************************
 *
 * SERI2CBAT_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 * busy
 *  Set when there is an active transaction. Cleared when the transaction
 *  is completed.
 *
 * executing_batch
 *  Set when a batch of commands are being executed.
 *
 * restart_required
 *  Set if the transaction requires a restart transition between writing and
 *  reading.
 *
 * task_state
 *  State machine state.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t busy                          : 1;
    uint8_t executing_batch               : 1;
    uint8_t restart_required              : 1;
    uint8_t reserved3                     : 1;
    uint8_t task_state                    : 4;
  };
}
SERI2CBAT_flags_t;

/*******************************************************************************
 *
 * SERI2CBAT_error_flags_t
 *
 * DESCRIPTION:
 *  Module error flags.
 *
 * timeout
 *  The maximum allowable time between receiving or transmitting the next data
 *  element has passed.
 *
 * nak_response
 *  Slave device responded with a NACK causing the transaction to be cut short.
 *
 * collision
 *  Collision detected on the I2C line.
 *
 * rx_overflow
 *  Receive buffer overflow.
 *
 * other
 *  All other types of errors.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t timeout                       : 1;
    uint8_t nak_response                  : 1;
    uint8_t collision                     : 1;
    uint8_t rx_overflow                   : 1;
    uint8_t other                         : 1;
    uint8_t reserved5                     : 3;
  };
}
SERI2CBAT_error_flags_t;

/*******************************************************************************
 *
 * SERI2CBAT_hal_is_rx_ready_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if the hardware Rx buffer has data available.
 *
 * RETURN:
 *  True if the hardware Rx buffer has data available, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*SERI2CBAT_hal_is_rx_ready_t)(void);

/*******************************************************************************
 *
 * SERI2CBAT_hal_is_tx_ready_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if the hardware Tx buffer has room available.
 *
 * RETURN:
 *  True if the hardware Tx buffer has room available, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*SERI2CBAT_hal_is_tx_ready_t)(void);

/*******************************************************************************
 *
 * SERI2CBAT_hal_read_rx_register_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will read the next available element from the hardware Rx buffer.
 *
 * RETURN:
 *  The next read element from the Rx hardware buffer.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef uint8_t (*SERI2CBAT_hal_read_rx_register_t)(void);

/*******************************************************************************
 *
 * SERI2CBAT_hal_write_tx_register_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will write a value to the hardware Tx buffer.
 *
 * PARAMETERS:
 *  value
 *   Element to be written to the hardware Tx buffer.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SERI2CBAT_hal_write_tx_register_t)(uint8_t);

/*******************************************************************************
 *
 * SERI2CBAT_hal_enqueue_start_command_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will enqueue the start command.
 *
 * PARAMETERS:
 *  index
 *   The next batch queue index.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SERI2CBAT_hal_enqueue_start_command_t)(uint8_t);

/*******************************************************************************
 *
 * SERI2CBAT_hal_enqueue_restart_command_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will enqueue the restart command.
 *
 * PARAMETERS:
 *  index
 *   The next batch queue index.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SERI2CBAT_hal_enqueue_restart_command_t)(uint8_t);

/*******************************************************************************
 *
 * SERI2CBAT_hal_enqueue_read_command_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will enqueue the read command.
 *
 * PARAMETERS:
 *  index
 *   The next batch queue index.
 *
 *  length
 *   The length, in bytes, to read.
 *
 *  ack_response
 *   Indicates if the byte(s) associated with this command read shoul return
 *   an ACK (true) or NAK (false) response. If the total number of bytes to
 *   read is greater than 1, this module will always enqueue a separate read
 *   command for the last byte by itself with this argument false (NAK).
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SERI2CBAT_hal_enqueue_read_command_t)(uint8_t, uint16_t, bool);

/*******************************************************************************
 *
 * SERI2CBAT_hal_enqueue_write_command_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will enqueue the write command.
 *
 * PARAMETERS:
 *  index
 *   The next batch queue index.
 *
 *  length
 *   The length, in bytes, to write.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SERI2CBAT_hal_enqueue_write_command_t)(uint8_t, uint16_t);

/*******************************************************************************
 *
 * SERI2CBAT_hal_enqueue_end_command_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will enqueue the 'end' command. (This is a command which indicates
 *  an end of the current batch but not an end to the data transaction. It
 *  lets the system know another batch will be enqueued and executed soon.)
 *
 * PARAMETERS:
 *  index
 *   The next batch queue index.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SERI2CBAT_hal_enqueue_end_command_t)(uint8_t);

/*******************************************************************************
 *
 * SERI2CBAT_hal_enqueue_stop_command_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will enqueue the stop command.
 *
 * PARAMETERS:
 *  index
 *   The next batch queue index.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SERI2CBAT_hal_enqueue_stop_command_t)(uint8_t);

/*******************************************************************************
 *
 * SERI2CBAT_hal_trigger_batch_execute_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will begin execution of the batch commands.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SERI2CBAT_hal_trigger_batch_execute_t)(void);

/*******************************************************************************
 *
 * SERI2CBAT_hal_trigger_batch_abort_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will abort batch execution and clear the command queue.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SERI2CBAT_hal_trigger_batch_abort_t)(void);

/*******************************************************************************
 *
 * SERI2CBAT_hal_is_batch_completed_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will determine if the batch operation has finished.
 *
 * PARAMETERS:
 *  index
 *   The index of the last command in the batch queue.
 *
 * RETURN:
 *  True if the the batch has finished, else, false;
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*SERI2CBAT_hal_is_batch_completed_t)(uint8_t);

/*******************************************************************************
 *
 * SERI2CBAT_hal_error_check_nak_received_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will determine if an NAK response has been received. (Not always an
 *  error.)
 *
 * RETURN:
 *  True if an NAK has been received, else, false;
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*SERI2CBAT_hal_error_check_nak_received_t)(void);

/*******************************************************************************
 *
 * SERI2CBAT_hal_error_check_collision_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will determine if a collision on the bus has been detected.
 *
 * RETURN:
 *  True if a collision was detected, else, false;
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef bool (*SERI2CBAT_hal_error_check_collision_t)(void);

/*******************************************************************************
 *
 * SERI2CBAT_hal_error_check_rx_overflow_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will determine if a receive buffer overflow has been detected.
 *
 * RETURN:
 *  True if a receive buffer overflow was detected, else, false;
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef bool (*SERI2CBAT_hal_error_check_rx_overflow_t)(void);

/*******************************************************************************
 *
 * SERI2CBAT_hal_error_check_other_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will determine if any other type of error has been detected.
 *
 * RETURN:
 *  True if any other type of error was detected, else, false;
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef bool (*SERI2CBAT_hal_error_check_other_t)(void);

/*******************************************************************************
 *
 * SERI2CBAT_hal_clear_errors_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will clear all I2C errors.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*SERI2CBAT_hal_clear_errors_t)(void);

/*******************************************************************************
 *
 * SERI2CBAT_hal_new_task_reset_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will clear or reset various registers in preparation for a new task.
 *  (Eg. FIFO, FSM, etc...)
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*SERI2CBAT_hal_new_task_reset_t)(void);

/*******************************************************************************
 *
 * SERI2CBAT_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module flags.
 *
 * errors
 *  Module error flags.
 *
 * utimer
 *  User-provided initialized instance of a UTIMER.
 *
 * utimer_ticket
 *  Data structure used with the UTIMER instance.
 *
 * register_length
 *  The length, in bytes, up to the byte-length of the register parameter, that
 *  make up the real register value which needs to be transmitted.
 *
 * cmd_queue_length
 *  The length, in number of commands, which can be enqueued for a single
 *  batch transaction. Must be at least 2 to allow at least one command
 *  for a continuing transaction followed by the 'end' command.
 *
 * cmd_queue_counter
 *  Number of enqueued commands.
 *
 * addr_reg_buffer_length
 *  Number of bytes to write in the addr_reg buffer.
 *
 * addr_reg_buffer
 *  Holds the slave address and register write information. This is a maximum
 *  of 6-bytes (2 for the slave address and 4 for the register).
 *
 * cmd_rw_length
 *  The length, in bytes, which can be read or written for a single read/write
 *  command. Must be at least 4 to allow the maximum supported register size
 *  for a single write command.
 *
 * slave_address
 *  Stores the slave address of the current slave device.
 *
 * rx_buffer
 *  Element buffer which will hold the element data from the RX transaction.
 *
 * tx_buffer
 *  Element buffer which contains the element data for the TX transaction.
 *
 * register_value
 *  Stores the register offset value of the current slave device if this is a
 *  register read or write operation.
 *
 * timeout_us
 *  Value which will be loaded into the utimer_ticket. Initialized to the
 *  module default but can be modified by the user.
 *
 * buffered_bytes_per_iteration
 *  Maximum number of bytes that will be written to the hardware TX buffer
 *  or read from the hardware RX buffer per iteration.
 *
 * batch_rx_element_count
 *  Number of elements to be received in the current RX batch.
 *
 * batch_tx_element_count
 *  Number of elements to be transmitted in the current TX batch.
 *
 * batch_rx_element_counter
 *  Number of elements which have been received during the current RX batch.
 *
 * batch_tx_element_counter
 *  Number of elements which have been transmitted during the current TX batch.
 *
 * rx_element_count
 *  Number of elements to be received in the entire RX transaction.
 *
 * tx_element_count
 *  Number of elements to be transmitted in the entire TX transaction.
 *
 * rx_element_counter
 *  Number of elements which have been received during the RX transaction.
 *
 * tx_element_counter
 *  Number of elements which have been transmitted during the TX transaction.
 *
 * rx_element_enqueued_counter
 *  Number of elements which have been enqueued to be received during the
 *  RX transaction.
 *
 * tx_element_enqueued_counter
 *  Number of elements which have been enqueued to be be transmitted during
 *  the TX transaction.
 *
 * *_hal_*
 *  User-provided functions. See typedef comments.
 *
 ******************************************************************************/

typedef struct
{
  SERI2CBAT_flags_t flags;
  SERI2CBAT_error_flags_t errors;
  UTIMER_instance_t* utimer;
  UTIMER_ticket_t utimer_ticket;
  uint8_t register_length;
  uint8_t cmd_queue_length;
  uint8_t cmd_queue_counter;
  uint8_t re_addr_buffer_count;
  uint8_t re_addr_buffer_counter;
  uint8_t re_addr_buffer;
  uint8_t addr_reg_buffer_count;
  uint8_t addr_reg_buffer_counter;
  uint8_t addr_reg_buffer[6];
  uint16_t cmd_rw_length;
  uint16_t slave_address;
  uint8_t* rx_buffer;
  uint8_t* tx_buffer;
  uint32_t register_value;
  uint32_t timeout_us;
  uint32_t buffered_bytes_per_iteration;
  uint32_t batch_rx_element_count;
  uint32_t batch_tx_element_count;
  uint32_t batch_rx_element_counter;
  uint32_t batch_tx_element_counter;
  uint32_t rx_element_count;
  uint32_t tx_element_count;
  uint32_t rx_element_counter;
  uint32_t tx_element_counter;
  uint32_t rx_element_enqueued_counter;
  uint32_t tx_element_enqueued_counter;
  SERI2CBAT_hal_is_rx_ready_t is_rx_ready;
  SERI2CBAT_hal_is_tx_ready_t is_tx_ready;
  SERI2CBAT_hal_read_rx_register_t read_rx_register;
  SERI2CBAT_hal_write_tx_register_t write_tx_register;
  SERI2CBAT_hal_enqueue_start_command_t enqueue_start_command;
  SERI2CBAT_hal_enqueue_restart_command_t enqueue_restart_command;
  SERI2CBAT_hal_enqueue_read_command_t enqueue_read_command;
  SERI2CBAT_hal_enqueue_write_command_t enqueue_write_command;
  SERI2CBAT_hal_enqueue_end_command_t enqueue_end_command;
  SERI2CBAT_hal_enqueue_stop_command_t enqueue_stop_command;
  SERI2CBAT_hal_trigger_batch_execute_t trigger_batch_execute;
  SERI2CBAT_hal_trigger_batch_abort_t trigger_batch_abort;
  SERI2CBAT_hal_is_batch_completed_t is_batch_completed;
  SERI2CBAT_hal_error_check_nak_received_t error_check_nak_received;
  SERI2CBAT_hal_error_check_collision_t error_check_collision;
  SERI2CBAT_hal_error_check_rx_overflow_t error_check_rx_overflow;
  SERI2CBAT_hal_error_check_other_t error_check_other;
  SERI2CBAT_hal_clear_errors_t clear_errors;
  SERI2CBAT_hal_new_task_reset_t new_task_reset;
}
SERI2CBAT_instance_t;

/*******************************************************************************
 *
 * SERI2CBAT_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values.
 *
 * PARAMETERS:
 *  See SERI2CBAT_instance_t.
 *
 ******************************************************************************/

void SERI2CBAT_initialize(SERI2CBAT_instance_t* instance,
                          UTIMER_instance_t* utimer,
                          uint8_t cmd_queue_length,
                          uint16_t cmd_rw_length,
                          uint32_t buffered_bytes_per_iteration,
                          SERI2CBAT_hal_is_rx_ready_t is_rx_ready,
                          SERI2CBAT_hal_is_tx_ready_t is_tx_ready,
                          SERI2CBAT_hal_read_rx_register_t read_rx_register,
                          SERI2CBAT_hal_write_tx_register_t write_tx_register,
                          SERI2CBAT_hal_enqueue_start_command_t enqueue_start_command,
                          SERI2CBAT_hal_enqueue_restart_command_t enqueue_restart_command,
                          SERI2CBAT_hal_enqueue_read_command_t enqueue_read_command,
                          SERI2CBAT_hal_enqueue_write_command_t enqueue_write_command,
                          SERI2CBAT_hal_enqueue_end_command_t enqueue_end_command,
                          SERI2CBAT_hal_enqueue_stop_command_t enqueue_stop_command,
                          SERI2CBAT_hal_trigger_batch_execute_t trigger_batch_execute,
                          SERI2CBAT_hal_trigger_batch_abort_t trigger_batch_abort,
                          SERI2CBAT_hal_is_batch_completed_t is_batch_completed,
                          SERI2CBAT_hal_error_check_nak_received_t error_check_nak_received,
                          SERI2CBAT_hal_error_check_collision_t error_check_collision,
                          SERI2CBAT_hal_error_check_rx_overflow_t error_check_rx_overflow,
                          SERI2CBAT_hal_error_check_other_t error_check_other,
                          SERI2CBAT_hal_clear_errors_t clear_errors,
                          SERI2CBAT_hal_new_task_reset_t new_task_reset);

/*******************************************************************************
 *
 * SERI2CBAT_set_transaction_timeout
 *
 * DESCRIPTION:
 *  Sets the transaction timeout value. Every time an element is received or
 *  transmitted the timeout timer will be reset with this value. If a timeout
 *  occurs, the operation is aborted.
 *
 * PARAMETERS:
 *  timeout_us
 *   Timeout to set in microseconds.
 *
 ******************************************************************************/

void SERI2CBAT_set_transaction_timeout(SERI2CBAT_instance_t* instance,
                                       uint32_t timeout_us);

/*******************************************************************************
 *
 * SERI2CBAT_begin_new_write_read
 *
 * DESCRIPTION:
 *  Attempts to begin a new write task followed by a read task. The request
 *  will be accepted if the instance is not currently busy with a task.
 *
 * PARAMETERS:
 *  slave_address
 *   I2C hardware address of the slave device. Can either be 7-bit or 10-bit.
 *
 *  tx_buffer
 *   Buffer which contains the data to be written to the slave device.
 *
 *  tx_length
 *   Number of bytes to write. (Tx buffer must be this size or greater.)
 *
 *  rx_buffer
 *   Buffer which will hold the data read from the slave device.
 *
 *  rx_length
 *   Number of bytes to read. (Rx buffer must be this size or greater.)
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool SERI2CBAT_begin_new_write_read(SERI2CBAT_instance_t* instance,
                                    uint16_t slave_address,
                                    uint8_t* tx_buffer,
                                    uint32_t tx_length,
                                    uint8_t* rx_buffer,
                                    uint32_t rx_length);

/*******************************************************************************
 *
 * SERI2CBAT_begin_new_read
 *
 * DESCRIPTION:
 *  Attempts to begin a new read transaction. This can only be done if the
 *  instance is currently not busy.
 *
 * PARAMETERS:
 *  See SERI2CBAT_begin_new_write_read.
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool SERI2CBAT_begin_new_read(SERI2CBAT_instance_t* instance,
                              uint16_t slave_address,
                              uint8_t* rx_buffer,
                              uint32_t rx_length);

/*******************************************************************************
 *
 * SERI2CBAT_begin_new_write
 *
 * DESCRIPTION:
 *  Attempts to begin a new write transaction. This can only be done if the
 *  instance is currently not busy.
 *
 * PARAMETERS:
 *  See SERI2CBAT_begin_new_write_read.
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool SERI2CBAT_begin_new_write(SERI2CBAT_instance_t* instance,
                               uint16_t slave_address,
                               uint8_t* tx_buffer,
                               uint32_t tx_length);

/*******************************************************************************
 *
 * SERI2CBAT_begin_new_register_read
 *
 * DESCRIPTION:
 *  Attempts to begin a new register read transaction. This can only be done
 *  if the instance is currently not busy.
 *
 * PARAMETERS:
 *  See SERI2CBAT_begin_new_write_read.
 *
 *  register_value
 *   The register value. This will be the first data written to the slave
 *   after the slave is addressed for writing.
 *
 *  register_length
 *   The length, in bytes, of the register value. This accomodates the possible
 *   need for longer register requirements, such as EEPROM memory addressing.
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool SERI2CBAT_begin_new_register_read(SERI2CBAT_instance_t* instance,
                                       uint16_t slave_address,
                                       uint32_t register_value,
                                       uint8_t register_length,
                                       uint8_t* rx_buffer,
                                       uint32_t rx_length);

/*******************************************************************************
 *
 * SERI2CBAT_begin_new_register_write
 *
 * DESCRIPTION:
 *  Attempts to begin a new register write task. The request will be accepted
 *  if the instance is not currently busy with a task.
 *
 * PARAMETERS:
 *  See SERI2CBAT_begin_new_write_read.
 *  See SERI2CBAT_begin_new_register_read.
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool SERI2CBAT_begin_new_register_write(SERI2CBAT_instance_t* instance,
                                        uint16_t slave_address,
                                        uint32_t register_value,
                                        uint8_t register_length,
                                        uint8_t* tx_buffer,
                                        uint32_t tx_length);

/*******************************************************************************
 *
 * SERI2CBAT_service
 *
 * DESCRIPTION:
 *  Services the task state machine. Must be called repeatedly until the task
 *  is completed.
 *
 * RETURN:
 *  False if there is an ongoing task which has not completed, else, true.
 *
 ******************************************************************************/

bool SERI2CBAT_service(SERI2CBAT_instance_t* instance);

/*******************************************************************************
 *
 * SERI2CBAT_abort
 *
 * DESCRIPTION:
 *  Aborts the current task.
 *
 ******************************************************************************/

void SERI2CBAT_abort(SERI2CBAT_instance_t* instance);

/*******************************************************************************
 *
 * SERI2CBAT_is_busy
 *
 * DESCRIPTION:
 *  Determines if an instance is currently busy with a task.
 *
 * RETURN:
 *  True if the instance is busy with a task, else, false.
 *
 ******************************************************************************/

bool SERI2CBAT_is_busy(SERI2CBAT_instance_t* instance);

#ifdef __cplusplus
}
#endif
#endif // SER_I2CBAT_J_H

/*******************************************************************************
 *
 *  I2C slave module. Supports both 7-bit and 10-bit addressing. The hardware
 *  layer is abstracted away with function pointers - it is required that the
 *  user provide specific device hardware interaction functions.
 *
 *  Unlike the I2C master, the slave does not know when it will be addressed
 *  nor how many bytes the master is going to ask for or write. Therefore, the
 *  I2C slave will not contain pointers to data buffers but will provide
 *  callback functions for read and write transactions.
 *
 ******************************************************************************/

#ifndef SER_I2C_SLAVE_J_H
#define SER_I2C_SLAVE_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The default timeout required to pass before the clock stretch on the client
 * is released. This only needs to be a short time for the registers to be
 * committed (I am not sure even how necessary it is, but following Microchip's
 * example).
 */

#define SERI2CSLAVE_CLOCK_STRETCH_DEFAULT_uS  1U
#define SERI2CSLAVE_CLOCK_STRETCH_DISABLED_uS 0U

/*
 * Define bit-masks for the 7-bit and 10-bit slave address types.
 */

#define SERI2CSLAVE_7BIT_ADDRESS_MASK         0x007FU
#define SERI2CSLAVE_10BIT_ADDRESS_MASK        0x03FFU

/*
 * Enumerate the states of the I2C service state machine. Since the slave waits
 * for the master's lead, not many states are needed.
 */

typedef enum
{
  SERI2CSLAVE_WAITING_TO_BE_ADDRESSED         = 0,
  SERI2CSLAVE_DEVICE_ADDRESSED_ACK_PENDING    = 1,
  SERI2CSLAVE_MASTER_WRITE_TO_SLAVE           = 2,
  SERI2CSLAVE_MASTER_READ_FROM_SLAVE          = 3
}
SERI2CSLAVE_state_t;

/*******************************************************************************
 *
 * SERI2CSLAVE_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 * busy
 *  Set when there is an active transaction. Cleared when the transaction
 *  is completed.
 *
 * read_transaction
 *  Set if the current I2C transaction is master reading from the slave, else,
 *  cleared if the current I2C transaction is master writing to slave.
 *
 * clock_stretching
 *  Set if the client is currently waiting for the utimer to expire before
 *  releasing the clock stretching.
 *
 * transaction_state
 *  Keeps track of the I2C transaction state in the service routine. Values
 *  match those defined in SERI2CSLAVE_state_t.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t busy                          : 1;
    uint8_t read_transaction              : 1;
    uint8_t clock_stretching              : 1;
    uint8_t reserved3                     : 1;
    uint8_t transaction_state             : 3;
    uint8_t reserved7                     : 1;
  };
}
SERI2CSLAVE_flags_t;

/*******************************************************************************
 *
 * SERI2CSLAVE_error_flags_t
 *
 * DESCRIPTION:
 *  Module error flags.
 *
 * i2c_general
 *  All other types of I2C errors, such as buffer overflow.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t i2c_general                   : 1;
    uint8_t reserved1                     : 7;
  };
}
SERI2CSLAVE_error_flags_t;

/*******************************************************************************
 *
 * SERI2CSLAVE_master_read_from_slave_callback_t
 *
 * DESCRIPTION:
 *  Function template which is called when the master requests a data read
 *  from the slave device.
 *
 * PARAMETERS:
 *  value
 *   Pointer to a single byte which will be put onto the slave Tx buffer and
 *   sent to the master.
 *
 *  bytes_read_from_slave
 *   The total number of bytes read from the slave in the current operation.
 *
 ******************************************************************************/

typedef void (*SERI2CSLAVE_master_read_from_slave_callback_t)(uint8_t*, uint32_t);


/*******************************************************************************
 *
 * SERI2CSLAVE_master_write_to_slave_callback_t
 *
 * DESCRIPTION:
 *  Function template which is called when data is received from the master
 *  and needs to be handled by the slave.
 *
 * PARAMETERS:
 *  value
 *   Pointer to a single byte value which was just pulled off the slave Rx
 *   buffer.
 *
 *  bytes_written_to_slave
 *   The total number of bytes written to the slave in the current operation.
 *
 * RETURN:
 *  True if the received value was accepted and an ACK should be returned,
 *  else, false to return a NAK.
 *
 ******************************************************************************/

typedef bool (*SERI2CSLAVE_master_write_to_slave_callback_t)(uint8_t*, uint32_t);

/*******************************************************************************
 *
 * SERI2CSLAVE_hal_is_rx_ready_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template which is called to check if
 *  the hardware Rx buffer has data available.
 *
 * RETURN:
 *  True if the hardware Rx buffer has data available, else, false.
 *
 ******************************************************************************/

typedef bool (*SERI2CSLAVE_hal_is_rx_ready_t)(void);

/*******************************************************************************
 *
 * SERI2CSLAVE_hal_is_tx_ready_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template which is called to check if
 *  the hardware Tx buffer has room available.
 *
 * RETURN:
 *  True if the hardware Tx buffer has room, else, false.
 *
 ******************************************************************************/

typedef bool (*SERI2CSLAVE_hal_is_tx_ready_t)(void);

/*******************************************************************************
 *
 * SERI2CSLAVE_hal_read_rx_register_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template which is called to read the
 *  next available byte from the hardware Rx buffer.
 *
 * RETURN:
 *  The next read byte from the Rx hardware buffer.
 *
 ******************************************************************************/

typedef uint8_t (*SERI2CSLAVE_hal_read_rx_register_t)(void);

/*******************************************************************************
 *
 * SERI2CSLAVE_hal_write_tx_register_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template which is called to write the
 *  parameter byte into the hardware Tx buffer.
 *
 * PARAMETERS:
 *  value
 *   Byte to be written to the hardware Tx buffer.
 *
 ******************************************************************************/

typedef void (*SERI2CSLAVE_hal_write_tx_register_t)(uint8_t);

/*******************************************************************************
 *
 * SERI2CSLAVE_hal_send_ack_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template which sends an ACK response.
 *
 ******************************************************************************/

typedef void (*SERI2CSLAVE_hal_send_ack_t)(void);

/*******************************************************************************
 *
 * SERI2CSLAVE_hal_is_send_ack_completed_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template which determines if the ACK
 *  response has finished sending.
 *
 * RETURN:
 *  True if the ACK response has finished sending, else, false;
 *
 ******************************************************************************/

typedef bool (*SERI2CSLAVE_hal_is_send_ack_completed_t)(void);

/*******************************************************************************
 *
 * SERI2CSLAVE_hal_send_nak_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template which sends an NAK response.
 *
 ******************************************************************************/

typedef void (*SERI2CSLAVE_hal_send_nak_t)(void);

/*******************************************************************************
 *
 * SERI2CSLAVE_hal_is_ack_received_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template which determines if an ACK
 *  has been received.
 *
 * RETURN:
 *  True if the ACK response has been received, else, false;
 *
 ******************************************************************************/

typedef bool (*SERI2CSLAVE_hal_is_ack_received_t)(void);

/*******************************************************************************
 *
 * SERI2CSLAVE_hal_is_stop_received_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template which determines if an STOP
 *  has been received.
 *
 * RETURN:
 *  True if the STOP response has been received, else, false;
 *
 ******************************************************************************/

typedef bool (*SERI2CSLAVE_hal_is_stop_received_t)(void);

/*******************************************************************************
 *
 * SERI2CSLAVE_hal_is_device_addressed_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template which determines if the
 *  slave device has been addressed.
 *
 * RETURN:
 *  True if the slave device has been addressed, else, false;
 *
 ******************************************************************************/

typedef bool (*SERI2CSLAVE_hal_is_device_addressed_t)(void);

/*******************************************************************************
 *
 * SERI2CSLAVE_hal_is_device_10bit_addressed_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template which determines if the
 *  slave device has been addressed via 10bit address.
 *
 * RETURN:
 *  True if the slave device has been addressed, else, false;
 *
 ******************************************************************************/

typedef bool (*SERI2CSLAVE_hal_is_device_10bit_addressed_t)(void);

/*******************************************************************************
 *
 * SERI2CSLAVE_hal_release_clock_stretch_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template which releases the slave
 *  clockstretch.
 *
 ******************************************************************************/

typedef void (*SERI2CSLAVE_hal_release_clock_stretch_t)(void);

/*******************************************************************************
 *
 * SERI2CSLAVE_hal_error_check_general_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template which checks for any errors
 *  in the I2C transaction.
 *
 * RETURN:
 *  True if any errors exist, else, false.
 *
 ******************************************************************************/

typedef bool (*SERI2CSLAVE_hal_error_check_general_t)(void);

/*******************************************************************************
 *
 * SERI2CSLAVE_hal_clear_errors_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template which clears any errors.
 *
 ******************************************************************************/

typedef void (*SERI2CSLAVE_hal_clear_errors_t)(void);

/*******************************************************************************
 *
 * SERI2CSLAVE_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module flags.
 *
 * errors
 *  Module error flags.
 *
 * utimer
 *  User-provided initialized instance of a UTIMER.
 *
 * utimer_ticket
 *  Data structure used with the UTIMER instance.
 *
 * device_address
 *  Stores the I2C device address.
 *
 * clock_stretch_us
 *  Value which will be loaded into the utimer_ticket. Initialized to the
 *  module default but can be modified by the user.
 *
 * bytes_written_to_slave
 *  Keeps track of the number of bytes written to the slave (master write to
 *  slave) in the current transaction. This value is passed to the master
 *  write to slave callback.
 *
 * bytes_read_from_slave
 *  Keeps track of the number of bytes read from the slave (master read from
 *  slave) in the current transaction. This value is passed to the master
 *  read from slave callback.
 *
 * *_callback_*
 * *_hal_*
 *  User-provided functions. See typedef comments.
 *
 ******************************************************************************/

typedef struct
{
  SERI2CSLAVE_flags_t flags;
  SERI2CSLAVE_error_flags_t errors;
  UTIMER_instance_t* utimer;
  UTIMER_ticket_t utimer_ticket;
  uint16_t device_address;
  uint32_t clock_stretch_us;
  uint32_t bytes_written_to_slave;
  uint32_t bytes_read_from_slave;
  SERI2CSLAVE_master_read_from_slave_callback_t master_read_from_slave_callback;
  SERI2CSLAVE_master_write_to_slave_callback_t master_write_to_slave_callback;
  SERI2CSLAVE_hal_is_rx_ready_t is_rx_ready;
  SERI2CSLAVE_hal_is_tx_ready_t is_tx_ready;
  SERI2CSLAVE_hal_read_rx_register_t read_rx_register;
  SERI2CSLAVE_hal_write_tx_register_t write_tx_register;
  SERI2CSLAVE_hal_send_ack_t send_ack;
  SERI2CSLAVE_hal_is_send_ack_completed_t is_send_ack_completed;
  SERI2CSLAVE_hal_send_nak_t send_nak;
  SERI2CSLAVE_hal_is_ack_received_t is_ack_received;
  SERI2CSLAVE_hal_is_stop_received_t is_stop_received;
  SERI2CSLAVE_hal_is_device_addressed_t is_device_addressed;
  SERI2CSLAVE_hal_is_device_10bit_addressed_t is_device_10bit_addressed;
  SERI2CSLAVE_hal_release_clock_stretch_t release_clock_stretch;
  SERI2CSLAVE_hal_error_check_general_t error_check_general;
  SERI2CSLAVE_hal_clear_errors_t clear_errors;
}
SERI2CSLAVE_instance_t;

/*******************************************************************************
 *
 * SERI2CSLAVE_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance.
 *
 * PARAMETERS:
 *  See SERI2CSLAVE_instance_t.
 *
 ******************************************************************************/

void SERI2CSLAVE_initialize(SERI2CSLAVE_instance_t* instance,
                            UTIMER_instance_t* utimer,
                            uint16_t device_address,
                            SERI2CSLAVE_master_read_from_slave_callback_t master_read_from_slave_callback,
                            SERI2CSLAVE_master_write_to_slave_callback_t master_write_to_slave_callback,
                            SERI2CSLAVE_hal_is_rx_ready_t is_rx_ready,
                            SERI2CSLAVE_hal_is_tx_ready_t is_tx_ready,
                            SERI2CSLAVE_hal_read_rx_register_t read_rx_register,
                            SERI2CSLAVE_hal_write_tx_register_t write_tx_register,
                            SERI2CSLAVE_hal_send_ack_t send_ack,
                            SERI2CSLAVE_hal_is_send_ack_completed_t is_send_ack_completed,
                            SERI2CSLAVE_hal_send_nak_t send_nak,
                            SERI2CSLAVE_hal_is_ack_received_t is_ack_received,
                            SERI2CSLAVE_hal_is_stop_received_t is_stop_received,
                            SERI2CSLAVE_hal_is_device_addressed_t is_device_addressed,
                            SERI2CSLAVE_hal_is_device_10bit_addressed_t is_device_10bit_addressed,
                            SERI2CSLAVE_hal_release_clock_stretch_t release_clock_stretch,
                            SERI2CSLAVE_hal_error_check_general_t error_check_general,
                            SERI2CSLAVE_hal_clear_errors_t clear_errors);

/*******************************************************************************
 *
 * SERI2CSLAVE_set_clock_stretch_timeout
 *
 * DESCRIPTION:
 *  Sets the clock stretch timeout value. The timeout value is what is used
 *  when a clock stretch is active to determine how long the stretch should be
 *  held after all registers are loaded. A value of 0 can be set to disable the
 *  timeout.
 *
 * PARAMETERS:
 *  clock_stretch_us
 *   Clock stretch to set in microseconds.
 *
 ******************************************************************************/

void SERI2CSLAVE_set_clock_stretch_timeout(SERI2CSLAVE_instance_t* instance,
                                           uint32_t clock_stretch_us);

/*******************************************************************************
 *
 * SERI2CSLAVE_service
 *
 * DESCRIPTION:
 *  Services the I2C transaction state machine. Since the slave does not
 *  start the transaction, this function must be continuously called to listen
 *  for master requests.
 *
 ******************************************************************************/

void SERI2CSLAVE_service(SERI2CSLAVE_instance_t* instance);

/*******************************************************************************
 *
 * SERI2C_is_busy
 *
 * DESCRIPTION:
 *  Determines if an instance is currently busy with a transaction.
 *
 * RETURN:
 *  True if the instance is busy with a transaction, else, false.
 *
 * NOTES:
 *  Due to the nature of the slave device, this flag may only be accurate if
 *  the master device(s) are playing nicely. This flag is more for debugging
 *  purposes.
 *
 ******************************************************************************/

bool SERI2CSLAVE_is_busy(SERI2CSLAVE_instance_t* instance);

#ifdef __cplusplus
}
#endif
#endif // SER_I2C_SLAVE_J_H

/*******************************************************************************
 *
 *  SPI master module. Supports various data frame widths including 8, 9, 16,
 *  and 32 bit. Requires proper initialization and the service routine to be
 *  called repeatedly after a new task request until the task is completed.
 *
 ******************************************************************************/

#ifndef SER_SPI_J_H
#define SER_SPI_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The default maximum number of elements which will be written to, or read
 * from, the hardware data buffer during a single non-blocking iteration of
 * the service routine.
 */

#define SERSPI_ELEMENTS_PER_ITERATION_DEFAULT 16U

/*
 * The default timeout between data transactions which will cause the task
 * to abort. (Essentially, a watchdog). A timeout value of 0 will disable the
 * task timeout logic.
 */

#define SERSPI_TIMEOUT_DEFAULT_uS         100000U
#define SERSPI_TIMEOUT_DISABLED_uS        0U

/*
 * The default allowed lead for Tx elements over Rx elements. The instance
 * value should be modified by the user to reflect the hardware buffer length.
 */

#define SERSPI_TX_LEAD_DEFAULT            8U

/*
 * The value sent by the master to the slave when there is no more relevant
 * data elements to send but there is still data to be read from the slave.
 */

#define SERSPI_TX_DUMMY_VALUE             0xFFFFFFFFU

/*******************************************************************************
 *
 * SERSPI_data_width_t
 *
 * DESCRIPTION:
 *  Enumerate the data widths available for SPI. There are 9-bit and 12-bit
 *  options too, however, these can simply fall into the 16-bit option.
 *
 ******************************************************************************/

typedef enum
{
  SERSPI_DATA_WIDTH_8_BITS                = 0,
  SERSPI_DATA_WIDTH_16_BITS,
  SERSPI_DATA_WIDTH_32_BITS,
  SERSPI_DATA_WIDTH_COUNT
}
SERSPI_data_width_t;

/*******************************************************************************
 *
 * SERSPI_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 * busy
 *  Set when the module is busy with a task and cleared when the task is
 *  completed.
 *
 * data_width
 *  Indicates the bit-width for the SPI transaction. Values match those defined
 *  in SERSPI_data_width_t.
 *
 * task_state
 *  State machine state.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t busy                          : 1;
    uint8_t data_width                    : 2;
    uint8_t reserved3                     : 1;
    uint8_t task_state                    : 2;
    uint8_t reserved6                     : 2;
  };
}
SERSPI_flags_t;

/*******************************************************************************
 *
 * SERSPI_error_flags_t
 *
 * DESCRIPTION:
 *  Module error flags.
 *
 * timeout
 *  The maximum allowable time between receiving or transmitting the next data
 *  element has passed.
 *
 * rx_overflow
 *  Receive buffer overflow error.
 *
 * frame
 *  Frame error (the data-frame format does not match expected protocol).
 *
 * other
 *  All other types of errors.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t timeout                       : 1;
    uint8_t rx_overflow                   : 1;
    uint8_t frame                         : 1;
    uint8_t other                         : 1;
    uint8_t reserved4                     : 4;
  };
}
SERSPI_error_flags_t;

/*******************************************************************************
 *
 * SERSPI_hal_is_rx_ready_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if the hardware Rx buffer has data available.
 *
 * RETURN:
 *  True if the hardware Rx buffer has data available, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*SERSPI_hal_is_rx_ready_t)(void);

/*******************************************************************************
 *
 * SERSPI_hal_is_tx_ready_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if the hardware Tx buffer has room available.
 *
 * RETURN:
 *  True if the hardware Tx buffer has room available, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*SERSPI_hal_is_tx_ready_t)(void);

/*******************************************************************************
 *
 * SERSPI_hal_read_rx_register_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will read the next available element from the hardware Rx buffer.
 *
 * RETURN:
 *  The next read element from the Rx hardware buffer.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef uint32_t (*SERSPI_hal_read_rx_register_t)(void);

/*******************************************************************************
 *
 * SERSPI_hal_write_tx_register_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will write a value to the hardware Tx buffer.
 *
 * PARAMETERS:
 *  value
 *   Element to be written to the hardware Tx buffer.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SERSPI_hal_write_tx_register_t)(uint32_t);

/*******************************************************************************
 *
 * SERSPI_hal_is_spi_busy_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if the SPI module is still busy. This can be checked by
 *  seeing if the Tx buffer is emptied or, sometimes, hardware may provide a
 *  register flag which is set/cleared when the module is completely finished.
 *
 *  In burst mode, this checks if the burst operation has completed.
 *
 * RETURN:
 *  True if the SPI module is busy, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*SERSPI_hal_is_spi_busy_t)(void);

/*******************************************************************************
 *
 * SERSPI_hal_error_check_rx_overflow_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if the buffer overflow error interrupt flag was triggered.
 *
 * RETURN:
 *  True if the flag is set, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef bool (*SERSPI_hal_error_check_rx_overflow_t)(void);

/*******************************************************************************
 *
 * SERSPI_hal_error_check_frame_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if the frame or break error interrupt flag was triggered.
 *
 * RETURN:
 *  True if the flag is set, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef bool (*SERSPI_hal_error_check_frame_t)(void);

/*******************************************************************************
 *
 * SERSPI_hal_error_check_other_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if any other errors have occurred.
 *
 * RETURN:
 *  True if any additional error flag(s) are set, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef bool (*SERSPI_hal_error_check_other_t)(void);

/*******************************************************************************
 *
 * SERSPI_hal_clear_error_flags_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will clear Rx interrupt error flags.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*SERSPI_hal_clear_error_flags_t)(void);

/*******************************************************************************
 *
 * SERSPI_hal_new_task_reset_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will clear or reset various registers in preparation for a new task.
 *  (Eg. FIFO, FSM, etc...)
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*SERSPI_hal_new_task_reset_t)(void);

/*******************************************************************************
 *
 * SERSPI_hal_burst_write_mosi_buffer_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will write data to the hardware write buffer (MOSI) which will be
 *  output during the next burst operation.
 *
 * PARAMETERS:
 *  buffer
 *   Pointer to a buffer in device memory, which may be offset, which contains
 *   the data which needs to be copied into the MOSI buffer.
 *
 *  length
 *   Length, in bytes, which should be read from the input buffer and placed
 *   into the MOSI hardware buffer. The length will always be less-than-or-
 *   equal-to the burst buffer length set in initialization.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SERSPI_hal_burst_write_mosi_buffer_t)(uint8_t*, uint32_t);

/*******************************************************************************
 *
 * SERSPI_hal_burst_write_mosi_buffer_dummy_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will write dummy data to the hardware write buffer (MOSI) which will
 *  be output during a burst operation. Dummy data is used when there are more
 *  bytes to read than there are to write. The user can decide what the dummy
 *  data will be, though, it is generally a series of 0xFF's.
 *
 * PARAMETERS:
 *  length
 *   Length, in bytes, of dummy values which should be written to the MOSI
 *   hardware buffer.
 *
 *  offset
 *   The starting offset int the MOSI hardware buffer that the dummy bytes
 *   should be written. This will always be at the beginning of the
 *   hardware buffer or come after valid MOSI data in the buffer.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SERSPI_hal_burst_write_mosi_buffer_dummy_t)(uint32_t, uint32_t);

/*******************************************************************************
 *
 * SERSPI_hal_burst_read_miso_buffer_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will read data from the hardware read buffer (MISO) which should
 *  have been filled during the burst task.
 *
 * PARAMETERS:
 *  buffer
 *   Buffer which the values in the hardware MISO buffer should be copied
 *   to.
 *
 *  length
 *   Length, in bytes, which should be read from the MISO hardware buffer and
 *   placed into the passed-in buffer. The length will always be less-than-or-
 *   equal-to the burst buffer length set in initialization.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SERSPI_hal_burst_read_miso_buffer_t)(uint8_t*, uint32_t);

/*******************************************************************************
 *
 * SERSPI_hal_burst_set_length_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will set the number of bytes which should be written/read during the
 *  burst operation. For this module, the MOSI and MISO values are kept the
 *  same and the MOSI is supplemented with dummy bytes as needed.
 *
 * PARAMETERS:
 *  length
 *   Number of bytes which should be written/read during the next burst.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SERSPI_hal_burst_set_length_t)(uint32_t);

/*******************************************************************************
 *
 * SERSPI_hal_burst_start_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will start the burst task.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SERSPI_hal_burst_start_t)(void);

/*******************************************************************************
 *
 * SERSPI_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module flags.
 *
 * errors
 *  Module error flags.
 *
 * utimer
 *  User-provided initialized instance of a UTIMER.
 *
 * utimer_ticket
 *  Data structure used with the UTIMER instance.
 *
 * timeout_us
 *  Value which will be loaded into the utimer_ticket. Initialized to the
 *  module default but can be modified by the user.
 *
 * rx_buffer
 *  Element buffer which will hold the element data from the transaction. The
 *  buffer data type is determined by the initialized data width.
 *
 * tx_buffer
 *  Element buffer which contains the element data from the transaction. The
 *  buffer data type is determined by the initialized data width.
 *
 * rx_element_count
 *  Number of elements to be received in the transaction.
 *
 * tx_element_count
 *  Number of elements to be transmitted in the transaction.
 *
 * tx_dummy_element_count
 *  Number of dummy elements to be transmitted in the transaction. This is used
 *  if the number of elements to read is greater than the number of elements
 *  to write. (The master can only receive data upon sending data.)
 *
 * rx_element_counter
 *  Number of elements which have been received during the transaction.
 *
 * tx_element_counter
 *  Number of elements which have been transmitted during the transaction.
 *
 * tx_dummy_element_counter
 *  Number of dummy elements which have been transmitted during the transaction.
 *
 * max_elements_per_iteration
 *  The maximum number of elements which will be processed, for each Rx and
 *  Tx, in a single iteration of the state machine. A lower value will block
 *  for a less time. In burst mode, this represents the length of the hardware
 *  buffer (or should not exceed the hardware buffer length).
 *
 * tx_lead_over_rx_allowance
 *  For peripherals which have hardware buffers, the maximum extra number of
 *  processed elements on the Tx which can be enqueued ahead of the handled
 *  Rx elements.
 *
 * tx_lead_over_rx_counter
 *  Keeps track of the number of Tx elements over Rx elements. Incremented for
 *  every Tx element and decremented for every Rx element.
 *
 * burst_length
 *  The number of bytes which have been loaded into the SPI burst data
 *  register(s).
 *
 * service_handler
 *  Function pointer to the service routine applicable to the initialized
 *  service mode.
 *
 * *_hal_*
 *  User-provided functions. See typedef comments.
 *
 ******************************************************************************/

typedef struct
{
  SERSPI_flags_t flags;
  SERSPI_error_flags_t errors;
  UTIMER_instance_t* utimer;
  UTIMER_ticket_t utimer_ticket;
  uint32_t timeout_us;
  void* rx_buffer;
  void* tx_buffer;
  uint32_t rx_element_count;
  uint32_t tx_element_count;
  uint32_t tx_dummy_element_count;
  uint32_t rx_element_counter;
  uint32_t tx_element_counter;
  uint32_t tx_dummy_element_counter;
  uint32_t max_elements_per_iteration;
  uint8_t tx_lead_over_rx_allowance;
  int8_t tx_lead_over_rx_counter;
  uint32_t burst_length;
  bool (*service_handler)(void* instance);
  SERSPI_hal_is_rx_ready_t is_rx_ready;
  SERSPI_hal_is_tx_ready_t is_tx_ready;
  SERSPI_hal_read_rx_register_t read_rx_register;
  SERSPI_hal_write_tx_register_t write_tx_register;
  SERSPI_hal_is_spi_busy_t is_spi_busy;
  SERSPI_hal_error_check_rx_overflow_t error_check_rx_overflow;
  SERSPI_hal_error_check_frame_t error_check_frame;
  SERSPI_hal_error_check_other_t error_check_other;
  SERSPI_hal_clear_error_flags_t clear_error_flags;
  SERSPI_hal_new_task_reset_t new_task_reset;
  SERSPI_hal_burst_write_mosi_buffer_t burst_write_mosi_buffer;
  SERSPI_hal_burst_write_mosi_buffer_dummy_t burst_write_mosi_buffer_dummy;
  SERSPI_hal_burst_read_miso_buffer_t burst_read_miso_buffer;
  SERSPI_hal_burst_set_length_t burst_set_length;
  SERSPI_hal_burst_start_t burst_start;
}
SERSPI_instance_t;

/*******************************************************************************
 *
 * SERSPI_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values.
 *
 * PARAMETERS:
 *  See SERSPI_instance_t.
 *
 ******************************************************************************/

void SERSPI_initialize(SERSPI_instance_t* instance,
                       UTIMER_instance_t* utimer,
                       SERSPI_data_width_t data_width,
                       uint32_t max_elements_per_iteration,
                       SERSPI_hal_is_rx_ready_t is_rx_ready,
                       SERSPI_hal_is_tx_ready_t is_tx_ready,
                       SERSPI_hal_read_rx_register_t read_rx_register,
                       SERSPI_hal_write_tx_register_t write_tx_register,
                       SERSPI_hal_is_spi_busy_t is_spi_busy,
                       SERSPI_hal_error_check_rx_overflow_t error_check_rx_overflow,
                       SERSPI_hal_error_check_frame_t error_check_frame,
                       SERSPI_hal_error_check_other_t error_check_other,
                       SERSPI_hal_clear_error_flags_t clear_error_flags,
                       SERSPI_hal_new_task_reset_t new_task_reset);

/*******************************************************************************
 *
 * SERSPI_initialize_burst
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values. Burst method.
 *
 * PARAMETERS:
 *  See SERSPI_instance_t.
 *
 ******************************************************************************/

void SERSPI_initialize_burst(SERSPI_instance_t* instance,
                             UTIMER_instance_t* utimer,
                             uint32_t max_elements_per_iteration,
                             SERSPI_hal_burst_write_mosi_buffer_t burst_write_mosi_buffer,
                             SERSPI_hal_burst_write_mosi_buffer_dummy_t burst_write_mosi_buffer_dummy,
                             SERSPI_hal_burst_read_miso_buffer_t burst_read_miso_buffer,
                             SERSPI_hal_burst_set_length_t burst_set_length,
                             SERSPI_hal_burst_start_t burst_start,
                             SERSPI_hal_is_spi_busy_t is_spi_busy,
                             SERSPI_hal_error_check_rx_overflow_t error_check_rx_overflow,
                             SERSPI_hal_error_check_frame_t error_check_frame,
                             SERSPI_hal_error_check_other_t error_check_other,
                             SERSPI_hal_clear_error_flags_t clear_error_flags,
                             SERSPI_hal_new_task_reset_t new_task_reset);

/*******************************************************************************
 *
 * SERSPI_set_transaction_timeout
 *
 * DESCRIPTION:
 *  Sets the transaction timeout value. Every time an element is received or
 *  transmitted the timeout timer will be reset with this value. If a timeout
 *  occurs, the task is aborted.
 *
 * PARAMETERS:
 *  timeout_us
 *   Timeout to set in microseconds.
 *
 ******************************************************************************/

void SERSPI_set_task_timeout(SERSPI_instance_t* instance, uint32_t timeout_us);

/*******************************************************************************
 *
 * SERSPI_begin_new_write_read
 *
 * DESCRIPTION:
 *  Attempts to begin a new write task followed by a read task. The request
 *  will be accepted if the instance is not in free-flow mode and not currently
 *  busy with a task.
 *
 * PARAMETERS:
 *  tx_buffer
 *   Pointer to a buffer which contains the data to be written to the slave
 *   device. The buffer data type is determined by the initialized data width.
 *
 *  tx_length
 *   Number of Tx elements to write.
 *
 *  rx_buffer
 *   Pointer to a buffer which will hold the data read from the slave device.
 *   The buffer data type is determined by the initialized data width.
 *
 *  rx_length
 *   Number of Rx elements to read.
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool SERSPI_begin_new_write_read(SERSPI_instance_t* instance,
                                 void* tx_buffer,
                                 uint32_t tx_length,
                                 void* rx_buffer,
                                 uint32_t rx_length);

/*******************************************************************************
 *
 * SERSPI_begin_new_read
 *
 * DESCRIPTION:
 *  Attempts to begin a new read task. The request will be accepted if the
 *  instance is not in free-flow mode and not currently busy with a task.
 *
 * PARAMETERS:
 *  rx_buffer
 *   Pointer to a buffer which will hold the data read from the slave device.
 *   The buffer data type is determined by the initialized data width.
 *
 *  rx_length
 *   Number of Rx elements to read.
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool SERSPI_begin_new_read(SERSPI_instance_t* instance,
                           void* rx_buffer,
                           uint32_t rx_length);

/*******************************************************************************
 *
 * SERSPI_begin_new_write
 *
 * DESCRIPTION:
 *  Attempts to begin a new write transaction. This can only be done if the
 *  instance is currently not busy.
 *
 * PARAMETERS:
 *  tx_buffer
 *   Pointer to a buffer which contains the data to be written to the slave
 *   device. The buffer data type is determined by the initialized data width.
 *
 *  tx_length
 *   Number of Tx elements to write.
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool SERSPI_begin_new_write(SERSPI_instance_t* instance,
                            void* tx_buffer,
                            uint32_t tx_length);

/*******************************************************************************
 *
 * SERSPI_service
 *
 * DESCRIPTION:
 *  Services the task state machine. Must be called repeatedly until the task
 *  is completed.
 *
 * RETURN:
 *  False if there is an ongoing task which has not completed, else, true.
 *
 ******************************************************************************/

bool SERSPI_service(SERSPI_instance_t* instance);

/*******************************************************************************
 *
 * SERSPI_abort
 *
 * DESCRIPTION:
 *  Aborts the current task.
 *
 ******************************************************************************/

void SERSPI_abort(SERSPI_instance_t* instance);

/*******************************************************************************
 *
 * SERSPI_is_busy
 *
 * DESCRIPTION:
 *  Determines if an instance is currently busy with a task.
 *
 * RETURN:
 *  True if the instance is busy with a task, else, false.
 *
 ******************************************************************************/

bool SERSPI_is_busy(SERSPI_instance_t* instance);

#ifdef __cplusplus
}
#endif
#endif // SER_SPI_J_H

/*******************************************************************************
 *
 *  Universal-Asynchronous-Receive-Transfer (UART) module. Supports both a
 *  controlled Tx/Rx method and a "free-flow" method, the later intended to be
 *  used for terminal access. Both methods require proper initialization and
 *  the service routine to be called repeatedly after a new task request until
 *  the task is completed.
 *
 ******************************************************************************/

#ifndef SER_UART_J_H
#define SER_UART_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The default maximum number of elements which will be written to, or read
 * from, the hardware data buffer during a single non-blocking iteration of
 * the service routine.
 */

#define SERUART_ELEMENTS_PER_ITERATION_DEFAULT 8U

/*
 * The default timeout between data transactions which will cause the task
 * to abort. (Essentially, a watchdog). A timeout value of 0 will disable the
 * task timeout logic.
 */

#define SERUART_TIMOEOUT_DEFAULT_uS       100000U
#define SERUART_TIMEOUT_DISABLED_uS       0U

/*******************************************************************************
 *
 * SERUART_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 * rx_busy
 *  Set when there is an active Rx task. Cleared when the Rx task.
 *  is completed.
 *
 * tx_busy
 *  Set when there is an active Tx task. Cleared when the Tx task
 *  is completed.
 *
 * data_9bit
 *  Set if the data elements are 9-bits in length, else, cleared if standard
 *  8-bit data elements.
 *
 * tx_finishing
 *  Set for non free-flow Tx tasks when the last of the data has been loaded
 *  onto the hardware buffer and we are waiting for the data to finish
 *  transferring.
 *
 * free_flow
 *  Set if there is no expected Tx/Rx element count and we want to run
 *  continuously -- expected for user terminal interface.
 *
 * rx_buffer_is_queue
 *  Set if the Rx buffer pointer should be treated as a Queue instance, else,
 *  the buffer is treated as a normal array.
 *
 * tx_buffer_is_queue
 *  Set if the Tx buffer pointer should be treated as a Queue instance, else,
 *  the buffer is treated as a normal array.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t rx_busy                       : 1;
    uint8_t tx_busy                       : 1;
    uint8_t data_9bit                     : 1;
    uint8_t tx_finishing                  : 1;
    uint8_t free_flow                     : 1;
    uint8_t rx_buffer_is_queue            : 1;
    uint8_t tx_buffer_is_queue            : 1;
    uint8_t reserved7                     : 1;
  };
}
SERUART_flags_t;

/*******************************************************************************
 *
 * SERUART_error_flags_t
 *
 * DESCRIPTION:
 *  Module error flags.
 *
 * timeout
 *  The maximum allowable time between receiving or transmitting the next data
 *  element has passed.
 *
 * rx_overflow
 *  Receive buffer overflow error.
 *
 * frame
 *  Frame error (the data-frame format does not match expected protocol).
 *
 * parity
 *  Parity error (the parity bit of the data is not correct).
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t timeout                       : 1;
    uint8_t rx_overflow                   : 1;
    uint8_t frame                         : 1;
    uint8_t parity                        : 1;
    uint8_t reserved4                     : 4;
  };
}
SERUART_error_flags_t;

/*******************************************************************************
 *
 * SERUART_hal_is_rx_ready_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if the hardware Rx buffer has data available.
 *
 * RETURN:
 *  True if the hardware Rx buffer has data available, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*SERUART_hal_is_rx_ready_t)(void);

/*******************************************************************************
 *
 * SERUART_hal_is_tx_ready_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if the hardware Tx buffer has room available.
 *
 * RETURN:
 *  True if the hardware Tx buffer has room available, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*SERUART_hal_is_tx_ready_t)(void);

/*******************************************************************************
 *
 * SERUART_hal_is_tx_empty_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if the hardware Tx buffer is empty. This is used to check
 *  if a transmit task has completed.
 *
 * RETURN:
 *  True if the hardware Tx buffer is empty, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef bool (*SERUART_hal_is_tx_empty_t)(void);

/*******************************************************************************
 *
 * SERUART_hal_read_rx_register_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will read the next available element from the hardware Rx buffer.
 *
 * RETURN:
 *  The next read element from the Rx hardware buffer.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef uint16_t (*SERUART_hal_read_rx_register_t)(void);

/*******************************************************************************
 *
 * SERUART_hal_write_tx_register_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will write a value to the hardware Tx buffer.
 *
 * PARAMETERS:
 *  value
 *   Element to be written to the hardware Tx buffer.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SERUART_hal_write_tx_register_t)(uint16_t);

/*******************************************************************************
 *
 * SERUART_hal_error_check_rx_overflow_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if the buffer overflow error interrupt flag was triggered.
 *
 * RETURN:
 *  True if the flag is set, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef bool (*SERUART_hal_error_check_rx_overflow_t)(void);

/*******************************************************************************
 *
 * SERUART_hal_error_check_rx_frame_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if the frame or break error interrupt flag was triggered.
 *
 * RETURN:
 *  True if the flag is set, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef bool (*SERUART_hal_error_check_rx_frame_t)(void);

/*******************************************************************************
 *
 * SERUART_hal_error_check_rx_parity_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if the parity error interrupt flag was triggered.
 *
 * RETURN:
 *  True if the flag is set, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef bool (*SERUART_hal_error_check_rx_parity_t)(void);

/*******************************************************************************
 *
 * _hal_clear_rx_error_flags_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will clear Rx interrupt error flags.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*SERUART_hal_clear_rx_error_flags_t)(void);

/*******************************************************************************
 *
 * SERUART_hal_clear_tx_error_flags_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will clear Tx interrupt error flags.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*SERUART_hal_clear_tx_error_flags_t)(void);

/*******************************************************************************
 *
 * SERUART_hal_new_rx_task_reset_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will clear or reset various Rx registers in preparation for a new
 *  Rx task. (Eg. FIFO, FSM, etc...)
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*SERUART_hal_new_rx_task_reset_t)(void);

/*******************************************************************************
 *
 * SERUART_hal_new_tx_task_reset_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will clear or reset various Tx registers in preparation for a new
 *  Tx task. (Eg. FIFO, FSM, etc...)
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*SERUART_hal_new_tx_task_reset_t)(void);

/*******************************************************************************
 *
 * SERUART_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module flags.
 *
 * errors
 *  Module error flags.
 *
 * utimer
 *  User-provided initialized instance of a UTIMER.
 *
 * utimer_ticket
 *  Data structure used with the UTIMER instance.
 *
 * timeout_us
 *  Value which will be loaded into the utimer_ticket. Initialized to the
 *  module default but can be modified by the user.
 *
 * rx_buffer
 *  Either an element buffer or queue instance which will hold the element data
 *  from the Rx task.
 *
 * tx_buffer
 *  Either an element buffer or queue instance which contains the element data
 *  for the Tx task.
 *
 * rx_element_count
 *  Number of elements to be received in the Rx task. Only applicable to non
 *  free-flow mode.
 *
 * rx_element_counter
 *  Number of elements which have been received during the Rx task. Only
 *  applicable to non free-flow mode.
 *
 * tx_element_count
 *  Number of elements to be transmitted in the Tx task. Only applicable to non
 *  free-flow mode.
 *
 * tx_element_counter
 *  Number of elements which have been transmitted during the Tx task. Only
 *  applicable to non free-flow mode.
 *
 * max_elements_per_iteration
 *  The maximum number of elements which will be processed, for each Rx and
 *  Tx, in a single iteration of the state machine. A lower value will block
 *  for a less time.
 *
 * *_hal_*
 *  User-provided functions. See typedef comments.
 *
 ******************************************************************************/

typedef struct
{
  SERUART_flags_t flags;
  SERUART_error_flags_t errors;
  UTIMER_instance_t* utimer;
  UTIMER_ticket_t utimer_ticket;
  uint32_t timeout_us;
  void* rx_buffer;
  void* tx_buffer;
  uint32_t rx_element_count;
  uint32_t rx_element_counter;
  uint32_t tx_element_count;
  uint32_t tx_element_counter;
  uint32_t max_elements_per_iteration;
  SERUART_hal_is_rx_ready_t is_rx_ready;
  SERUART_hal_is_tx_ready_t is_tx_ready;
  SERUART_hal_is_tx_empty_t is_tx_empty;
  SERUART_hal_read_rx_register_t read_rx_register;
  SERUART_hal_write_tx_register_t write_tx_register;
  SERUART_hal_error_check_rx_overflow_t error_check_rx_overflow;
  SERUART_hal_error_check_rx_frame_t error_check_rx_frame;
  SERUART_hal_error_check_rx_parity_t error_check_rx_parity;
  SERUART_hal_clear_rx_error_flags_t clear_rx_error_flags;
  SERUART_hal_clear_tx_error_flags_t clear_tx_error_flags;
  SERUART_hal_new_rx_task_reset_t new_rx_task_reset;
  SERUART_hal_new_tx_task_reset_t new_tx_task_reset;
}
SERUART_instance_t;

/*******************************************************************************
 *
 * SERUART_initialize_basic
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values. Basic mode.
 *
 * PARAMETERS:
 *  See SERUART_instance_t.
 *
 ******************************************************************************/

void SERUART_initialize_basic(SERUART_instance_t* instance,
                              UTIMER_instance_t* utimer,
                              SERUART_hal_is_rx_ready_t is_rx_ready,
                              SERUART_hal_is_tx_ready_t is_tx_ready,
                              SERUART_hal_is_tx_empty_t is_tx_empty,
                              SERUART_hal_read_rx_register_t read_rx_register,
                              SERUART_hal_write_tx_register_t write_tx_register);

/*******************************************************************************
 *
 * SERUART_initialize_freeflow
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values. Free-flow mode.
 *
 * PARAMETERS:
 *  See SERUART_instance_t.
 *
 * NOTES:
 *  Buffers must be QUEUE instances.
 *
 ******************************************************************************/

void SERUART_initialize_freeflow(SERUART_instance_t* instance,
                                 void* rx_queue,
                                 void* tx_queue,
                                 SERUART_hal_is_rx_ready_t is_rx_ready,
                                 SERUART_hal_is_tx_ready_t is_tx_ready,
                                 SERUART_hal_read_rx_register_t read_rx_register,
                                 SERUART_hal_write_tx_register_t write_tx_register);

/*******************************************************************************
 *
 * SERUART_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values.
 *
 * PARAMETERS:
 *  See SERUART_instance_t.
 *
 ******************************************************************************/

void SERUART_initialize(SERUART_instance_t* instance,
                        UTIMER_instance_t* utimer,
                        bool data_9bit_mode,
                        bool free_flow_mode,
                        uint32_t max_elements_per_iteration,
                        void* rx_buffer,
                        void* tx_buffer,
                        SERUART_hal_is_rx_ready_t is_rx_ready,
                        SERUART_hal_is_tx_ready_t is_tx_ready,
                        SERUART_hal_is_tx_empty_t is_tx_empty,
                        SERUART_hal_read_rx_register_t read_rx_register,
                        SERUART_hal_write_tx_register_t write_tx_register,
                        SERUART_hal_error_check_rx_overflow_t error_check_rx_overflow,
                        SERUART_hal_error_check_rx_frame_t error_check_rx_frame,
                        SERUART_hal_error_check_rx_parity_t error_check_rx_parity,
                        SERUART_hal_clear_rx_error_flags_t clear_rx_error_flags,
                        SERUART_hal_clear_tx_error_flags_t clear_tx_error_flags,
                        SERUART_hal_new_rx_task_reset_t new_rx_task_reset,
                        SERUART_hal_new_tx_task_reset_t new_tx_task_reset);

/*******************************************************************************
 *
 * SERUART_set_task_timeout
 *
 * DESCRIPTION:
 *  Sets the task timeout value. Every time an element is received or
 *  transmitted the timeout timer will be reset with this value. If a timeout
 *  occurs, the task is aborted.
 *
 * PARAMETERS:
 *  timeout_us
 *   Timeout to set in microseconds.
 *
 * NOTE:
 *  Prototype provided in the header for unit test purposes.
 *
 ******************************************************************************/

void SERUART_set_task_timeout(SERUART_instance_t* instance, uint32_t timeout_us);

/*******************************************************************************
 *
 * SERUART_begin_new_rx
 *
 * DESCRIPTION:
 *  Attempts to begin a new Rx task. The request will be accepted if the
 *  instance is not in free-flow mode and not currently busy with a task.
 *
 * PARAMETERS:
 *  buffer
 *   Pointer to a buffer which will hold received data. The buffer will
 *   be treated as a QUEUE_instance if the queue bit is set. The queue element
 *   length will be dependent upon how the user initialized the module instant,
 *   for 8-bit or 9-bit element values.
 *
 *  element_count
 *   Number of elements in the buffer.
 *
 *  is_buffer_a_queue
 *   Flag indicating if the buffer should be treated as a QUEUE instance.
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool SERUART_begin_new_rx(SERUART_instance_t* instance,
                          void* buffer,
                          uint32_t element_count,
                          bool is_buffer_a_queue);

/*******************************************************************************
 *
 * SERUART_begin_new_tx
 *
 * DESCRIPTION:
 *  Attempts to begin a new Rx task. The request will be accepted if the
 *  instance is not in free-flow mode and not currently busy with a task.
 *
 * PARAMETERS:
 *  buffer
 *   Pointer to a buffer which contains the data to transmit. The buffer will
 *   be treated as a QUEUE_instance if the queue bit is set. The queue element
 *   length will be dependent upon how the user initialized the module instant,
 *   for 8-bit or 9-bit element values.
 *
 *  element_count
 *   Number of elements in the buffer.
 *
 *  is_buffer_a_queue
 *   Flag indicating if the buffer should be treated as a QUEUE instance.
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool SERUART_begin_new_tx(SERUART_instance_t* instance,
                          void* buffer,
                          uint32_t element_count,
                          bool is_buffer_a_queue);

/*******************************************************************************
 *
 * SERUART_service
 *
 * DESCRIPTION:
 * Services the Rx and Tx task state machine. Must be called repeatedly until
 * the task(s) are completed. Must be called indefinitely in free-flow mode.
 *
 * RETURN:
 *  False if there is an ongoing task which has not completed, else, true.
 *
 ******************************************************************************/

bool SERUART_service(SERUART_instance_t* instance);

/*******************************************************************************
 *
 * SERUART_abort_rx
 *
 * DESCRIPTION:
 *  Aborts the Rx task.
 *
 ******************************************************************************/

void SERUART_abort_rx(SERUART_instance_t* instance);

/*******************************************************************************
 *
 * SERUART_abort_tx
 *
 * DESCRIPTION:
 *  Aborts the Tx task.
 *
 ******************************************************************************/

void SERUART_abort_tx(SERUART_instance_t* instance);

/*******************************************************************************
 *
 * SERUART_is_tx_busy
 *
 * DESCRIPTION:
 *  Determines if an instance is currently busy with a Tx task.
 *
 * RETURN:
 *  True if the instance is busy with a Tx task, else, false.
 *
 ******************************************************************************/

bool SERUART_is_tx_busy(SERUART_instance_t* instance);

/*******************************************************************************
 *
 * SERUART_is_rx_busy
 *
 * DESCRIPTION:
 *  Determines if an instance is currently busy with a Rx task.
 *
 * RETURN:
 *  True if the instance is busy with a Rx task, else, false.
 *
 ******************************************************************************/

bool SERUART_is_rx_busy(SERUART_instance_t* instance);

#ifdef __cplusplus
}
#endif
#endif // SER_UART_J_H

/*******************************************************************************
 *
 * BUSMUTEX_bus_id_t
 *
 * DESCRIPTION:
 *  Shared busses that can exist on a system. The "NULL" ID is listed first
 *  and indicates that a mutex is not required for that bus.
 *
 ******************************************************************************/

typedef enum
{
  BUSMUTEX_BUS_ID_NULL                    = 0,
  BUSMUTEX_BUS_ID_SPI_0,
  BUSMUTEX_BUS_ID_SPI_1,
  BUSMUTEX_BUS_ID_SPI_2,
  BUSMUTEX_BUS_ID_SPI_3,
  BUSMUTEX_BUS_ID_SPI_4,
  BUSMUTEX_BUS_ID_SPI_5,
  BUSMUTEX_BUS_ID_I2C_0,
  BUSMUTEX_BUS_ID_I2C_1,
  BUSMUTEX_BUS_ID_I2C_2,
  BUSMUTEX_BUS_ID_I2C_3,
  BUSMUTEX_BUS_ID_I2C_4,
  BUSMUTEX_BUS_ID_I2C_5,
  BUSMUTEX_BUS_ID_COUNT
}
BUSMUTEX_bus_id_t;

/*******************************************************************************
 *
 * BUSMUTEX_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t reserved0                     : 8;
  };
}
BUSMUTEX_flags_t;

/*******************************************************************************
 *
 * BUSMUTEX_hal_enter_critical
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will enter the critical section (non-interruptable).
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*BUSMUTEX_hal_enter_critical_t)(void);

/*******************************************************************************
 *
 * BUSMUTEX_hal_exit_critical
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will exit the critical section.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*BUSMUTEX_hal_exit_critical_t)(void);

/*******************************************************************************
 *
 * BUSMUTEX_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module flags.
 *
 * bus_mutex
 *  Array of mutex flags. A flag is set when a mutex is claimed.
 *
 * *_hal_*
 *  User-provided functions. See typedef comments.
 *
 ******************************************************************************/

typedef struct
{
  BUSMUTEX_flags_t flags;
  bool bus_mutex[BUSMUTEX_BUS_ID_COUNT];
  BUSMUTEX_hal_enter_critical_t enter_critical;
  BUSMUTEX_hal_exit_critical_t exit_critical;
}
BUSMUTEX_instance_t;

/*******************************************************************************
 *
 * BUSMUTEX_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values.
 *
 * PARAMETERS:
 *  See BUSMUTEX_instance_t.
 *
 ******************************************************************************/

void BUSMUTEX_initialize(BUSMUTEX_instance_t* instance,
                         BUSMUTEX_hal_enter_critical_t enter_critical,
                         BUSMUTEX_hal_exit_critical_t exit_critical);

/*******************************************************************************
 *
 * BUSMUTEX_is_available
 *
 * DESCRIPTION:
 *  Determines if a bus ID is currently available and open to be requested.
 *
 * PARAMETERS:
 *  bus_id
 *   BUS ID to check availability.
 *
 * RETURN
 *  True if the bus is available, else, false.
 *
 ******************************************************************************/

bool BUSMUTEX_is_available(BUSMUTEX_instance_t* instance, BUSMUTEX_bus_id_t bus_id);

/*******************************************************************************
 *
 * BUSMUTEX_request_mutex
 *
 * DESCRIPTION:
 *  Attempts to secure the mutex of a bus ID.
 *
 * PARAMETERS:
 *  bus_id
 *   BUS ID mutex being requested.
 *
 * RETURN
 *  True if the bus ID mutex was secured, else, false.
 *
 ******************************************************************************/

bool BUSMUTEX_request_mutex(BUSMUTEX_instance_t* instance, BUSMUTEX_bus_id_t bus_id);

/*******************************************************************************
 *
 * BUSMUTEX_release_mutex
 *
 * DESCRIPTION:
 *  Attempts to release the mutex of a bus ID.
 *
 * PARAMETERS:
 *  bus_id
 *   BUS ID mutex being released.
 *
 * RETURN
 *  True if the bus ID mutex was released, else, false (if the mutex was not
 *  in use).
 *
 ******************************************************************************/

bool BUSMUTEX_release_mutex(BUSMUTEX_instance_t* instance, BUSMUTEX_bus_id_t bus_id);

#ifdef __cplusplus
}
#endif
#endif // BUS_MUTEX_J_H

/*******************************************************************************
 *
 *  Generic queue which supports any data type. The user provides the element
 *  size and a buffer. Requires proper initialization.
 *
 *  The option is available to make the queue thread-safe. This is done by
 *  not crossing variables which will be used in separate contexts. This comes
 *  at a cost of efficiency, and that the actual usable queue length will be
 *  (N-1). The application must also abide by the following rules:
 *    1. Wrapping is disabled.
 *    2. Only one context (thread or interrupt) enqueues.
 *    3. Only one context (thread or interrupt) dequeues.
 *
 ******************************************************************************/

#ifndef QUEUE_J_H
#define QUEUE_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *
 * QUEUE_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 * wrapping_enabled
 *  The oldest entries of the queue will be overwritten with new enqueued
 *  entries if the queue is full. NOTE: Cannot be used in thread-safe mode.
 *
 * thread_safe
 *  The queue will be thread-safe for two application contexts (i.e. Application
 *  and Interrupt, Application1 and Application2, etc...). NOTE: Cannot be used
 *  with wrapping.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t wrapping_enabled              : 1;
    uint8_t thread_safe                   : 1;
    uint8_t reserved2                     : 6;
  };
}
QUEUE_flags_t;

/*******************************************************************************
 *
 * QUEUE_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module flags.
 *
 * buffer
 *  Pointer to a user-provided buffer which will provide the physical memory
 *  for the queue. NOTE: Thread-safe queues require a buffer which can hold
 *  at least 2-elements.
 *
 * buffer_length
 *  Length, in bytes, of the user-provided buffer.
 *
 * element_size
 *  Size of a single queue element in bytes.
 *
 * queue_length_in_elements
 *  Calculated number of slots the queue contains based upon the queue length
 *  and the element size.
 *
 * element_counter
 *  Keeps track of the number of active queue entries. (Only used for quicker
 *  operations in non thread-safe mode.)
 *
 * head_index
 *  Keeps track of the head of the queue as an index/offset in number of
 *  elements from the start address of the user-provided buffer.
 *
 * tail_index
 *  Keeps track of the tail of the queue as an index/offset in number of
 *  elements from the start address of the user-provided buffer.
 *
 ******************************************************************************/

typedef struct
{
  QUEUE_flags_t flags;
  void* buffer;
  uint32_t buffer_length;
  uint32_t element_size;
  uint32_t queue_length_in_elements;
  uint32_t element_counter;
  uint32_t head_index;
  uint32_t tail_index;
}
QUEUE_instance_t;

/*******************************************************************************
 *
 * QUEUE_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values. SPI method.
 *
 * PARAMETERS:
 *  See QUEUE_instance_t.
 *
 * NOTES:
 *  The length of the queue will be determined by dividing the buffer size by
 *  the element size. It is ideal for the element size to be an equal divisor
 *  of the buffer, however, it is not necessary.
 *
 ******************************************************************************/

void QUEUE_initialize(QUEUE_instance_t* instance,
                      void* buffer,
                      uint32_t buffer_length,
                      uint32_t element_size,
                      bool wrapping_enabled,
                      bool thread_safe);

/*******************************************************************************
 *
 * QUEUE_enqueue
 *
 * DESCRIPTION:
 *  Attempts to enqueue a single element.
 *
 * PARAMETERS:
 *  element
 *   Pointer to the element to enqueue. A copy of the element will be stored on
 *   the queue.
 *
 * RETURN:
 *  True if space was available and the element was enqueued, else, false.
 *
 ******************************************************************************/

bool QUEUE_enqueue(QUEUE_instance_t* instance, void* element);

/*******************************************************************************
 *
 * QUEUE_dequeue
 *
 * DESCRIPTION:
 *  Attempts to dequeue a single element.
 *
 * PARAMETERS:
 *  element
 *   Pointer to a location to store a copy of the dequeued element.
 *
 * RETURN:
 *  True if the queue was not empty and an element was dequeued, else, false.
 *
 ******************************************************************************/

bool QUEUE_dequeue(QUEUE_instance_t* instance, void* element);

/*******************************************************************************
 *
 * QUEUE_peek
 *
 * DESCRIPTION:
 *  Peeks at the next element in the queue, returning a copy of the element but
 *  not removing it from the queue.
 *
 * PARAMETERS:
 *  element
 *   Pointer to the location to store a copy of the element.
 *
 * RETURN:
 *  True if the queue was not empty and a copy was made of the next element
 *  to be dequeued, else, false.
 *
 ******************************************************************************/

bool QUEUE_peek(QUEUE_instance_t* instance, void* element);

/*******************************************************************************
 *
 * QUEUE_enqueue_buffer
 *
 * DESCRIPTION:
 *  element_buffer
 *   Pointer to a buffer of elements to enqueue. Copies of the element(s) will
 *   be stored on the queue.
 *
 *  element_count
 *   Number of elements to enqueue from the element_buffer.
 *
 * RETURN:
 *  Actual number of elements enqueued which may be smaller than element_count
 *  if room was not available on the queue.
 *
 ******************************************************************************/

uint32_t QUEUE_enqueue_buffer(QUEUE_instance_t* instance,
                              void* element_buffer,
                              uint32_t element_count);

/*******************************************************************************
 *
 * QUEUE_dequeue_buffer
 *
 * DESCRIPTION:
 *  element_buffer
 *   Pointer to the buffer location to store a copy of the dequeued elements.
 *
 *  element_count
 *   Number of elements to dequeue to the element_buffer.
 *
 * RETURN:
 *  Actual number of elements dequeued which may be smaller than element_count
 *  if the queue had less elements enqueued.
 *
 ******************************************************************************/

uint32_t QUEUE_dequeue_buffer(QUEUE_instance_t* instance,
                              void* element_buffer,
                              uint32_t element_count);

/*******************************************************************************
 *
 * QUEUE_get_element_position
 *
 * DESCRIPTION:
 *  element
 *   Pointer to an element which will be compared against the elements in the
 *   queue.
 *
 *  position
 *   The position offset from the head of the queue of the first element which
 *   matches the compare element.
 *
 * RETURN:
 *  True if the element was found in the queue, else, false.
 *
 ******************************************************************************/

bool QUEUE_get_element_position(QUEUE_instance_t* instance,
                                void* element,
                                uint32_t* position);

/*******************************************************************************
 *
 * QUEUE_is_full
 *
 * DESCRIPTION:
 *  Checks if the queue is full.
 *
 * RETURNS:
 *  True if the queue is full, else, false.
 *
 ******************************************************************************/

bool QUEUE_is_full(QUEUE_instance_t* instance);

/*******************************************************************************
 *
 * QUEUE_is_empty
 *
 * DESCRIPTION:
 *  Checks if the queue is empty.
 *
 * RETURNS:
 *  True if the queue is empty, else, false.
 *
 ******************************************************************************/

bool QUEUE_is_empty(QUEUE_instance_t* instance);

/*******************************************************************************
 *
 * QUEUE_get_count
 *
 * DESCRIPTION:
 *  Gets the number of elements currently enqueued.
 *
 * RETURNS:
 *  Number of elements enqueued.
 *
 ******************************************************************************/

uint32_t QUEUE_get_count(QUEUE_instance_t* instance);

/*******************************************************************************
 *
 * QUEUE_clear
 *
 * DESCRIPTION:
 *  In non thread-safe mode, zero-fills the queue buffer and sets the element
 *  counter and index offsets to 0. In thread-safe mode, dequeues all elements.
 *
 ******************************************************************************/

void QUEUE_clear(QUEUE_instance_t* instance);

#ifdef __cplusplus
}
#endif
#endif // QUEUE_J_H

/*******************************************************************************
 *
 *  Special queue for shared bus tasks built from the Queue and BusMutex
 *  modules. Each instance is only responsible for a single bus ID, hence, a
 *  bus queue should exist for every shared bus. Requires proper initialization
 *  and the dequeue routine to be called repeatedly.
 *
 ******************************************************************************/

#ifndef BUS_QUEUE_J_H
#define BUS_QUEUE_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *
 * BUSQUEUE_task_callback_t
 *
 * DESCRIPTION:
 *  Function template for a user-provided function which is called to perform
 *  a new task on the instance shared bus.
 *
 * PARAMETERS:
 *  context
 *   User-provided context.
 *
 * RETURN:
 *  True if the task was started and can be removed from the queue, else,
 *  false.
 *
 ******************************************************************************/

typedef bool (*BUSQUEUE_task_callback_t)(uint32_t);

/*******************************************************************************
 *
 * BUSQUEUE_element_t
 *
 * DESCRIPTION:
 *  Data structure which is handled by the queue.
 *
 * context
 *  Value which will be passed into the user task callback function.
 *
 * task_callback
 *  User task function.
 *
 ******************************************************************************/

typedef struct
{
  uint32_t context;
  BUSQUEUE_task_callback_t task_callback;
}
BUSQUEUE_element_t;

/*******************************************************************************
 *
 * BUSQUEUE_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t reserved0                     : 8;
  };
}
BUSQUEUE_flags_t;

/*******************************************************************************
 *
 * BUSQUEUE_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module flags.
 *
 * bus_mutex
 *  User-provided initialized instance of a BUSMUTEX.
 *
 * bus_id
 *  The bus ID of the shared bus associated with this instance.
 *
 * queue
 *  Queue instance contained and controlled by BusQueue. The user provides
 *  the buffer which is used by the queue.
 *
 ******************************************************************************/

typedef struct
{
  BUSQUEUE_flags_t flags;
  BUSMUTEX_instance_t* bus_mutex;
  BUSMUTEX_bus_id_t bus_id;
  QUEUE_instance_t queue;
}
BUSQUEUE_instance_t;

/*******************************************************************************
 *
 * BUSQUEUE_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values.
 *
 * PARAMETERS:
 *  See BUSQUEUE_instance_t.
 *
 *  queue_buffer
 *   Pointer to user-provided buffer which will be used for the instance queue.
 *   The buffer size should be large enough to hold at least one or more
 *   BUSQUEUE_element_t.
 *
 *  queue_buffer_length
 *   Length, in bytes, of the queue buffer.
 *
 ******************************************************************************/

void BUSQUEUE_initialize(BUSQUEUE_instance_t* instance,
                         BUSMUTEX_instance_t* bus_mutex,
                         BUSMUTEX_bus_id_t bus_id,
                         uint8_t* queue_buffer,
                         uint32_t queue_buffer_length);

/*******************************************************************************
 *
 * BUSQUEUE_enqueue
 *
 * DESCRIPTION:
 *  Attempts to enqueue an new element.
 *
 * PARAMETERS:
 *  task_callback
 *   Task function to call when the element is dequeued.
 *
 *  context
 *   Value which will be passed into the user task callback function.
 *
 *  discard_if_exists
 *   If true, checks if the element already exists on the bus queue. If so,
 *   the new task element is discarded.
 *
 * RETURN
 *  True if space was available and the element was enqueued, else, false.
 *
 ******************************************************************************/

bool BUSQUEUE_enqueue(BUSQUEUE_instance_t* instance,
                      BUSQUEUE_task_callback_t task_callback,
                      uint32_t context,
                      bool discard_if_exists);

/*******************************************************************************
 *
 * BUSQUEUE_dequeue
 *
 * DESCRIPTION:
 *  Checks if the bus id, associated with the instance, is available. If so,
 *  peeks at the next element and calls its task function callback. If the
 *  callback returns true the element is dequeued.
 *
 * PARAMETERS:
 *  force_dequeue
 *   Will force the next element to dequeue even if the bus is busy and
 *   cannot start a new task. The dequeued task is simply deleted.
 *
 * RETURN
 *  True if the element was dequeued, else, false.
 *
 ******************************************************************************/

bool BUSQUEUE_dequeue(BUSQUEUE_instance_t* instance, bool force_dequeue);

#ifdef __cplusplus
}
#endif
#endif // BUS_MUTEX_J_H

/*******************************************************************************
 *
 *  Periodic task registrar and handler. Requires proper initialization and
 *  the service routine to be called periodically, ideally every 1-millisecond.
 *
 ******************************************************************************/

#ifndef CHRONO_J_H
#define CHRONO_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *
 * CHRONO_task_flags_t
 *
 * DESCRIPTION:
 *  Module registered task flags.
 *
 * enabled
 *  Enables or disables the registered Chrono task - the task function
 *  callback will not be called and the task counter will not be incremented.
 *
 * stop_next
 *  If set, the task enabled flag will be set false after the task is executed.
 *
 * one_shot
 *  If set, the task will be removed from the task list after the task is
 *  executed.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t enabled                       : 1;
    uint8_t stop_next                     : 1;
    uint8_t one_shot                      : 1;
    uint8_t reserved3                     : 5;
  };
}
CHRONO_task_flags_t;

/*******************************************************************************
 *
 * CHRONO_task_t
 *
 * DESCRIPTION:
 *  Module registered task.
 *
 * flags
 *  Module task flags.
 *
 * task_function
 *  The task function callback which will be called at the task periodic
 *  interval.
 *
 * period_ticks
 *  Number of ticks which define a single periodic interval for the task.
 *
 * tick_counter
 *  Incremented by the chrono instance. The task_function is called once
 *  this value equals period_ticks. Reset to 0 after task_function is called.
 *
 * next_task
 *  Provides a linked-list data structure for flexible adding and removing
 *  a variable number of tasks to/from the Chrono instance list.
 *
 ******************************************************************************/

typedef struct
{
  CHRONO_task_flags_t flags;
  void (*task_function)(void);
  uint32_t period_ticks;
  uint32_t tick_counter;
  void* next_task;
}
CHRONO_task_t;

/*******************************************************************************
 *
 * CHRONO_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 * enabled
 *  Enables or disables the chrono instance.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t enabled                       : 1;
    uint8_t reserved1                     : 7;
  };
}
CHRONO_flags_t;

/*******************************************************************************
 *
 * CHRONO_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module flags.
 *
 * task_count
 *  Keeps track of the number of tasks in the linked list. Not really needed
 *  for operation purposes, rather, here for information an debug purposes.
 *
 * task_list
 *  Linked list of registered tasks. The user is responsible for maintaining
 *  the task structure data - if the Chrono instance is in global space
 *  than so should the task data structures.
 *
 ******************************************************************************/

typedef struct
{
  CHRONO_flags_t flags;
  uint32_t task_count;
  CHRONO_task_t* task_list;
}
CHRONO_instance_t;

/*******************************************************************************
 *
 * CHRONO_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values. State machine method.
 *
 ******************************************************************************/

void CHRONO_initialize(CHRONO_instance_t* instance);

/*******************************************************************************
 *
 * CHRONO_enable
 *
 * DESCRIPTION:
 *  Enables a module instance.
 *
 ******************************************************************************/

void CHRONO_enable(CHRONO_instance_t* instance);

/*******************************************************************************
 *
 * CHRONO_disable
 *
 * DESCRIPTION:
 *  Disables a module instance.
 *
 ******************************************************************************/

void CHRONO_disable(CHRONO_instance_t* instance);

/*******************************************************************************
 *
 * CHRONO_add_task
 *
 * DESCRIPTION:
 *  Adds a new task to the task list. Does not add the task if the task already
 *  exists. Clears the tick counter and places the task at the end of the
 *  list.
 *
 * PARMAETERS:
 *  task
 *   Pointer to a task structure in user memory space. The module will NOT copy
 *   the data structure.
 *
 * RETURN:
 *  True if the task was added, else, false (task is already on the list).
 *
 ******************************************************************************/

bool CHRONO_add_task(CHRONO_instance_t* instance, CHRONO_task_t* task);

/*******************************************************************************
 *
 * CHRONO_remove_task
 *
 * DESCRIPTION:
 *  Removes a task, if it exists, from an instance task list.
 *
 * PARMAETERS:
 *  task
 *   Pointer to a task structure in user memory space.
 *
 * RETURN:
 *  True if the task was removed, else, false (task was not on the list).
 *
 ******************************************************************************/

bool CHRONO_remove_task(CHRONO_instance_t* instance, CHRONO_task_t* task);

/*******************************************************************************
 *
 * CHRONO_service
 *
 * DESCRIPTION:
 *  Iterates through the task list, increments the counters of enabled tasks,
 *  calls the task functions, etc... Should be called periodically at a set
 *  interval which this module defines as a "tick." If called within interrupt
 *  context, the user should ensure that all tasks are non-blocking.
 *
 * PARAMETERS:
 *  ticks
 *   The number of ticks to increment all periodic chrono tasks. This should
 *   be 1 if the service handler is called periodically without delays. The
 *   value can be greater than 1 if the service function was not able to run
 *   during a set period and needs to catch up on its ticks.
 *
 ******************************************************************************/

void CHRONO_service(CHRONO_instance_t* instance, uint32_t ticks);

#ifdef __cplusplus
}
#endif
#endif // CHRONO_J_H

/*******************************************************************************
 *
 *  DMX 512 transmitting and receiving module. Supports both non-blocking
 *  state machine and DMA methods. Both methods require proper initialization
 *  and the service routine to be called repeatedly after a new task request
 *  until the task is completed.
 *
 *  The instance mode (receive/transmit) is uninitialized at initialization
 *  and must be set to either receive or transmit before starting a new task.
 *  The mode can be easily switched between receive and transmit.
 *
 ******************************************************************************/

#ifndef DMX512_J_H
#define DMX512_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The maximum number of data bytes which can be transmitted or received in the
 * data section of the protocol packet. DMX 512 does not require the maximum
 * number of data bytes to be transmitted.
 */

#define DMX512_DATA_LENGTH_MAX            512U

/*******************************************************************************
 *
 * DMX512_mode_t
 *
 * DESCRIPTION:
 *  Enumerates the available tranmission modes.
 *
 ******************************************************************************/

typedef enum
{
  DMX512_MODE_UNINITIALIZED               = 0,
  DMX512_MODE_RECEIVE,
  DMX512_MODE_TRANSMIT
}
DMX512_mode_t;

/*******************************************************************************
 *
 * DMX512_start_code_t
 *
 * DESCRIPTION:
 *  Enumerates the standard start code values.
 *
 ******************************************************************************/

typedef enum
{
  DMX512_START_CODE_STANDARD              = 0x00,
  DMX512_START_CODE_TEXT                  = 0x17,
  DMX512_START_CODE_SYSTEM_INFO           = 0xCF,
  DMX512_START_CODE_RDM                   = 0xCC,
}
DMX512_start_code_t;

/*******************************************************************************
 *
 * DMX512_transaction_data_t
 *
 * DESCRIPTION:
 *  The start code and data portion of the DMX 512 packet.
 *
 * start_code
 *  The first valid byte of the transaction which indicates the type of DMX
 *  data being transmitted. The user can compare against this value, once the
 *  transaction completes, to determine what to do with the received DMX data.
 *
 * dmx
 *  The data portion of the DMX 512 transaction which immediately follows the
 *  start code.
 *
 ******************************************************************************/

typedef struct __attribute__((packed, aligned(1)))
{
  uint8_t start_code;
  uint8_t dmx[DMX512_DATA_LENGTH_MAX];
}
DMX512_transaction_data_t;

/*******************************************************************************
 *
 * DMX512_transaction_flags_t
 *
 * DESCRIPTION:
 *  Module DMX 512 transaction flags which indicate status and errors. The
 *  user application can look at these flags when determining how to handle
 *  "completed" received DMX 512 packets.
 *
 * timeout
 *  The maximum allowable time since the beginning of a receive or transmit of
 *  a DMX packet has passed.
 *
 * dma_transfer
 *  An error occurred during the DMA transfer.
 *
 * uart_overflow
 *  The UART hardware receive buffer overflowed during the receive.
 *
 * uart_frame
 *  A UART frame error was detected during the receive.
 *
 * uart_break_missed
 *  The DMX break was not detected during the receive.
 *
 ******************************************************************************/

typedef union
{
  uint16_t all;
  struct
  {
    uint16_t timeout                      : 1;
    uint16_t dma_transfer_error           : 1;
    uint16_t uart_overflow_error          : 1;
    uint16_t uart_frame_error             : 1;
    uint16_t uart_break_missed            : 1;
    uint16_t reserved5                    : 11;
  };
}
DMX512_transaction_flags_t;

/*******************************************************************************
 *
 * DMX512_transaction_t
 *
 * DESCRIPTION:
 *  Transaction data and additional meta for a single DMX 512 transaction.
 *
 * flags
 *  See DMX512_transaction_flags_t.
 *
 * data_length
 *  The number of bytes to write in a transmit. This includes both the start
 *  code and the DMX data.
 *
 * data
 *  See DMX512_transaction_data_t.
 *
 ******************************************************************************/

typedef struct
{
  DMX512_transaction_flags_t flags;
  uint16_t data_length;
  DMX512_transaction_data_t data;
}
DMX512_transaction_t;

/*******************************************************************************
 *
 * DMX512_transaction_buffers_t
 *
 * DESCRIPTION:
 *  A three buffer system is used to support working versus stable data and DMX
 *  channel data versus other non-channel data (RDM, TEXT, etc...). At any time
 *  only one of the buffers is the "working" buffer. The other two are stable
 *  buffers, one for DMX and the other for all other types of data. This setup
 *  assumes that DMX channel data will be most prevelant, and that all other
 *  data types can share a single double buffer.
 *
 * transaction
 *  See DMX512_transaction_t
 *
 ******************************************************************************/

typedef struct
{
  DMX512_transaction_t transaction[3];
}
DMX512_transaction_buffers_t;

/*******************************************************************************
 *
 * DMX512_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 * busy
 *  Set when the module is busy with a task and cleared when the task is
 *  completed.
 *
 * mode
 *  The current mode of the instance, either receiver or transmitter.
 *
 * dma
 *  Set true if module is initialized to use DMA.
 *
 * task_state
 *  State machine state.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t busy                          : 1;
    uint8_t mode                          : 2;
    uint8_t dma                           : 1;
    uint8_t task_state                    : 4;
  };
}
DMX512_flags_t;

/*******************************************************************************
 *
 * DMX512_receive_complete_callback_t
 *
 * DESCRIPTION:
 *  Function template for a user-provided function which is called after a
 *  receive transaction is completed.
 *
 * PARAMETERS:
 *  transaction
 *   Completed transaction information and data
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*DMX512_receive_complete_callback_t)(DMX512_transaction_t*);

/*******************************************************************************
 *
 * DMX512_transmit_complete_callback_t
 *
 * DESCRIPTION:
 *  Function template for a user-provided function which is called after a
 *  transmit transaction is completed.
 *
 * PARAMETERS:
 *  transaction
 *   Completed transaction information and data
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*DMX512_transmit_complete_callback_t)(DMX512_transaction_t*);

/*******************************************************************************
 *
 * DMX512_pre_task_callback_t
 *
 * DESCRIPTION:
 *  Function template for a user-provided function which is called when a new
 *  task is accepted and initialized but before the task actually begins. This
 *  allows the user to perform any last additional configurations before the
 *  actual logic of the task begins.
 *
 *  Only called when the instance is in transmit mode. In receive mode, a new
 *  receive task is triggered by the UART fault interrupt.
 *
 * PARAMETERS:
 *  context
 *   User-provided context.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*DMX512_pre_task_callback_t)(uint32_t);

/*******************************************************************************
 *
 * DMX512_post_task_callback_t
 *
 * DESCRIPTION:
 *  Function template for a user-provided function which is called after a
 *  task completes.
 *
 * PARAMETERS:
 *  context
 *   User-provided context.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*DMX512_post_task_callback_t)(uint32_t);

/*******************************************************************************
 *
 * DMX512_hal_set_dmx_direction_gpio_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will handle DMX direction change. This can include changing a
 *  direction GPIO or reconfiguring the line to hook to the UART Tx or Rx.
 *
 * PARAMETERS:
 *  mode
 *   Receive or transmit.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*DMX512_hal_set_dmx_direction_t)(DMX512_mode_t);

/*******************************************************************************
 *
 * DMX512_hal_generate_tx_break_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will cause the Tx line to force a break condition (bring the line
 *  low). The method to accomplish this will differ on each processor. One
 *  way is to temporarily configure the line as a GPIO output and drive it
 *  low.
 *
 *  This operation will only be allowed if the instance is in transmit mode.
 *
 * PARAMETERS:
 *  enable
 *   True to enable the Tx break (force line low), else, false to configure
 *   the line as UART Tx.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*DMX512_hal_generate_tx_break_t)(bool);

/*******************************************************************************
 *
 * DMX512_hal_is_rx_ready_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if the hardware Rx buffer has data available.
 *
 * RETURN:
 *  True if the hardware Rx buffer has data available, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*DMX512_hal_is_rx_ready_t)(void);

/*******************************************************************************
 *
 * DMX512_hal_is_tx_ready_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if the hardware Tx buffer has room available.
 *
 * RETURN:
 *  True if the hardware Tx buffer has room available, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*DMX512_hal_is_tx_ready_t)(void);

/*******************************************************************************
 *
 * DMX512_hal_is_tx_empty_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if the hardware Tx buffer is empty. This is used to check
 *  if a transmit task has completed.
 *
 * RETURN:
 *  True if the hardware Tx buffer is empty, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*DMX512_hal_is_tx_empty_t)(void);

/*******************************************************************************
 *
 * DMX512_hal_read_rx_register_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will read the next available byte from the hardware Rx buffer.
 *
 * RETURN:
 *  The next read byte from the Rx hardware buffer.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef uint8_t (*DMX512_hal_read_rx_register_t)(void);

/*******************************************************************************
 *
 * DMX512_hal_write_tx_register_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will write a byte into the hardware Tx buffer.
 *
 * PARAMETERS:
 *  value
 *   Byte to be written to the hardware Tx buffer.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*DMX512_hal_write_tx_register_t)(uint8_t);

/*******************************************************************************
 *
 * DMX512_hal_error_check_rx_overflow_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if the buffer overflow error interrupt flag was triggered.
 *
 * RETURN:
 *  True if the flag is set, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*DMX512_hal_error_check_rx_overflow_t)(void);

/*******************************************************************************
 *
 * DMX512_hal_error_check_rx_break_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if the break error interrupt flag was triggered. This is not
 *  really an error in DMX context, but is expected, and needed, to operate
 *  correctly. We expect each new received packet to be preceeded with  a break
 *  which will trigger a break interrupt.
 *
 * RETURN:
 *  True if the flag is set, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*DMX512_hal_error_check_rx_break_t)(void);

/*******************************************************************************
 *
 * DMX512_hal_error_check_rx_frame_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if the frame error interrupt flag was triggered.
 *
 * RETURN:
 *  True if the flag is set, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*DMX512_hal_error_check_rx_frame_t)(void);

/*******************************************************************************
 *
 * DMX512_hal_clear_rx_error_flags_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will clear Rx interrupt error flags. Depending on the processor, this
 *  could be interpreted as just clearing the fault interrupt itself. In the
 *  algorithm, this function and DMX512_hal_clear_rx_fault_interrupt are called
 *  one after another.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*DMX512_hal_clear_rx_error_flags_t)(void);

/*******************************************************************************
 *
 * DMX512_hal_enable_rx_fault_interrupt_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will enable and disable the Rx fault interrupt(s).
 *
 * PARAMETERS:
 *  enable
 *   True to enable the fault interrupt(s), else, false to disable.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*DMX512_hal_enable_rx_fault_interrupt_t)(bool);

/*******************************************************************************
 *
 * DMX512_hal_clear_rx_fault_interrupt_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will clear the Rx fault interrupt(s). This must clear all interrupt
 *  flags were set in the fault enable function.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*DMX512_hal_clear_rx_fault_interrupt_t)(void);

/*******************************************************************************
 *
 * DMX512_hal_clear_dma_transfer_complete_interrupt_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will clear the DMA transfer complete interrupt (and other DMA complete
 *  related interrupts).
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*DMX512_hal_clear_dma_transfer_complete_interrupt_t)(void);

/*******************************************************************************
 *
 * DMX512_hal_configure_dma_receive_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will configure the DMA to receive data from the UART Rx.
 *
 * PARAMETERS:
 *  dest_addr
 *   Address in memory where the data will be written to.
 *
 *  dest_length
 *   Length, in bytes, of the destination buffer.
 *
 * RETURN:
 *  True if the configuration was successful and started, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*DMX512_hal_configure_dma_receive_t)(void*, uint32_t);

/*******************************************************************************
 *
 * DMX512_hal_configure_dma_transmit_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will configure the DMA to transmit data to the UART Tx.
 *
 * PARAMETERS:
 *  src_addr
 *   Address in memory where the data will be read from.
 *
 *  src_length
 *   Length, in bytes, of the source buffer.
 *
 * RETURN:
 *  True if the configuration was successful and started, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*DMX512_hal_configure_dma_transmit_t)(void*, uint32_t);

/*******************************************************************************
 *
 * DMX512_hal_disable_dma_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will stop/disable the DMA.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*DMX512_hal_disable_dma_t)(void);

/*******************************************************************************
 *
 * DMX512_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module flags.
 *
 * utimer
 *  User-provided initialized instance of a UTIMER.
 *
 * utimer_ticket
 *  Data structure used with the UTIMER instance.
 *
 * transaction_buffers
 *  Pointer to a user provided buffer, of size DMX512_transaction_buffers_t,
 *  which is DMA compatible and will provide buffering for all transactions.
 *
 * stable_dmx
 *  Points to the last completed and stable DMX data transaction data structure.
 *
 * stable_other
 *  Points to the last completed and stable non-DMX data (RDM, TEXT, etc...)
 *  transaction data structure.
 *
 * stable
 *  Points to the last completed data transaction data structure, either the
 *  stable_dmx or stable_other.
 *
 * working
 *  Points to the data structure which is currently being modified or utilized
 *  for a transaction.
 *
 * break_us
 *  Time, in microseconds, that the line is held low for the "break" portion of
 *  the protocol. This value is defaulted at initialization, but can be modified
 *  by the user to suit their timing needs.
 *
 * mark_after_break_us
 *  Time, in microseconds, that the line is held high for the "MAB" portion of
 *  the protocol. This value is defaulted at initialization, but can be modified
 *  by the user to suit their timing needs.
 *
 * tx_post_timeout_us
 *  Time, in microseconds, that the alrogithm times out after completely sending
 *  the DMX data to the UART peripheral before starting the break condition.
 *  This is intended to provide sufficient time for the peripheral to completely
 *  finish transmitting the Tx data. This value is defaulted at initialization,
 *  but can be modified by the user to suit their timing needs.
 *
 * dmx_byte_counter
 *  Keeps track of the number of received/transmitted DMX bytes during
 *  non-DMA transfers.
 *
 * callback_context
 *  Context passed into the user pre/post task callbacks.
 *
 * service_handler
 *  Internal-use pointer to the active state machine service.
 *
 * *_task_*
 * *_hal_*
 *  User-provided functions. See typedef comments.
 *
 ******************************************************************************/

typedef struct
{
  volatile DMX512_flags_t flags;
  UTIMER_instance_t* utimer;
  volatile UTIMER_ticket_t utimer_ticket;
  DMX512_transaction_buffers_t* transaction_buffers;
  DMX512_transaction_t* stable_dmx;
  DMX512_transaction_t* stable_other;
  DMX512_transaction_t* stable;
  DMX512_transaction_t* working;
  uint16_t break_us;
  uint16_t mark_after_break_us;
  uint16_t tx_post_timeout_us;
  uint16_t dmx_byte_counter;
  uint32_t callback_context;
  bool (*service_handler)(void*);
  DMX512_receive_complete_callback_t receive_complete_callback;
  DMX512_transmit_complete_callback_t transmit_complete_callback;
  DMX512_pre_task_callback_t pre_task_callback;
  DMX512_post_task_callback_t post_task_callback;
  DMX512_hal_set_dmx_direction_t set_dmx_direction;
  DMX512_hal_generate_tx_break_t generate_tx_break;
  DMX512_hal_is_rx_ready_t is_rx_ready;
  DMX512_hal_is_tx_ready_t is_tx_ready;
  DMX512_hal_is_tx_empty_t is_tx_empty;
  DMX512_hal_read_rx_register_t read_rx_register;
  DMX512_hal_write_tx_register_t write_tx_register;
  DMX512_hal_error_check_rx_overflow_t error_check_rx_overflow;
  DMX512_hal_error_check_rx_break_t error_check_rx_break;
  DMX512_hal_error_check_rx_frame_t error_check_rx_frame;
  DMX512_hal_clear_rx_error_flags_t clear_rx_error_flags;
  DMX512_hal_enable_rx_fault_interrupt_t enable_rx_fault_interrupt;
  DMX512_hal_clear_rx_fault_interrupt_t clear_rx_fault_interrupt;
  DMX512_hal_clear_dma_transfer_complete_interrupt_t clear_dma_transfer_complete_interrupt;
  DMX512_hal_configure_dma_receive_t configure_dma_receive;
  DMX512_hal_configure_dma_transmit_t configure_dma_transmit;
  DMX512_hal_disable_dma_t disable_dma;
}
DMX512_instance_t;

/*******************************************************************************
 *
 * DMX512_uart_fault_isr_handler
 *
 * DESCRIPTION:
 *  Handler for the UART fault interrupt (break, overflow, and frame). The
 *  user code must call this function from their UART fault ISR.
 *
 * IMPORTANT:
 *  The user MUST ensure that the UART fault (break, overflow, and frame
 *  errors) interrupt is enabled and that this handler is called from the ISR.
 *
 ******************************************************************************/

void DMX512_uart_fault_isr_handler(DMX512_instance_t* instance);

/*******************************************************************************
 *
 * DMX512_dma_transfer_complete_isr_handler
 *
 * DESCRIPTION:
 *  Handler for the DMA transfer complete interrupt. The user code must call
 *  this function from their DMA transfer complete ISR.
 *
 * IMPORTANT:
 *  If the DMA method is used, the user MUST ensure that the DMA transmission
 *  complete interrupt is enabled and that this handler is called from that
 *  ISR.
 *
 ******************************************************************************/

void DMX512_dma_transfer_complete_isr_handler(DMX512_instance_t* instance);

/*******************************************************************************
 *
 * DMX512_dma_transfer_error_isr_handler
 *
 * DESCRIPTION:
 *  Handler for the DMA transfer error interrupt. The user code must call this
 *  function from their DMA transfer error ISR.
 *
 * IMPORTANT:
 *  If the DMA method is used, the user MUST ensure that the DMA transmission
 *  error interrupt is enabled and that this handler is called from that ISR.
 *
 ******************************************************************************/

void DMX512_dma_transfer_error_isr_handler(DMX512_instance_t* instance);

/*******************************************************************************
 *
 * DMX512_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values. State machine method.
 *
 * PARAMETERS:
 *  See DMX512_instance_t.
 *
 ******************************************************************************/

void DMX512_initialize(DMX512_instance_t* instance,
                       UTIMER_instance_t* utimer,
                       DMX512_transaction_buffers_t* transaction_buffers,
                       DMX512_receive_complete_callback_t receive_complete_callback,
                       DMX512_transmit_complete_callback_t transmit_complete_callback,
                       DMX512_pre_task_callback_t pre_task_callback,
                       DMX512_post_task_callback_t post_task_callback,
                       DMX512_hal_set_dmx_direction_t set_dmx_direction,
                       DMX512_hal_generate_tx_break_t generate_tx_break,
                       DMX512_hal_is_rx_ready_t is_rx_ready,
                       DMX512_hal_is_tx_ready_t is_tx_ready,
                       DMX512_hal_is_tx_empty_t is_tx_empty,
                       DMX512_hal_read_rx_register_t read_rx_register,
                       DMX512_hal_write_tx_register_t write_tx_register,
                       DMX512_hal_error_check_rx_overflow_t error_check_rx_overflow,
                       DMX512_hal_error_check_rx_break_t error_check_rx_break,
                       DMX512_hal_error_check_rx_frame_t error_check_rx_frame,
                       DMX512_hal_clear_rx_error_flags_t clear_rx_error_flags,
                       DMX512_hal_enable_rx_fault_interrupt_t enable_rx_fault_interrupt,
                       DMX512_hal_clear_rx_fault_interrupt_t clear_rx_fault_interrupt);

/*******************************************************************************
 *
 * DMX512_initialize_dma
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values. DMA method.
 *
 * PARAMETERS:
 *  See DMX512_instance_t.
 *
 ******************************************************************************/

void DMX512_initialize_dma(DMX512_instance_t* instance,
                           UTIMER_instance_t* utimer,
                           DMX512_transaction_buffers_t* transaction_buffers,
                           DMX512_receive_complete_callback_t receive_complete_callback,
                           DMX512_transmit_complete_callback_t transmit_complete_callback,
                           DMX512_pre_task_callback_t pre_task_callback,
                           DMX512_post_task_callback_t post_task_callback,
                           DMX512_hal_set_dmx_direction_t set_dmx_direction,
                           DMX512_hal_generate_tx_break_t generate_tx_break,
                           DMX512_hal_is_rx_ready_t is_rx_ready,
                           DMX512_hal_read_rx_register_t read_rx_register,
                           DMX512_hal_error_check_rx_overflow_t error_check_rx_overflow,
                           DMX512_hal_error_check_rx_break_t error_check_rx_break,
                           DMX512_hal_error_check_rx_frame_t error_check_rx_frame,
                           DMX512_hal_clear_rx_error_flags_t clear_rx_error_flags,
                           DMX512_hal_enable_rx_fault_interrupt_t enable_rx_fault_interrupt,
                           DMX512_hal_clear_rx_fault_interrupt_t clear_rx_fault_interrupt,
                           DMX512_hal_clear_dma_transfer_complete_interrupt_t clear_dma_transfer_complete_interrupt,
                           DMX512_hal_configure_dma_receive_t configure_dma_receive,
                           DMX512_hal_configure_dma_transmit_t configure_dma_transmit,
                           DMX512_hal_disable_dma_t disable_dma);

/*******************************************************************************
 *
 * DMX512_set_mode
 *
 * DESCRIPTION:
 *  Changes the DMX mode to receive or transmit. Any active task(s) will be
 *  aborted.
 *
 * PARAMETER:
 *  mode
 *   Receive or transmit mode.
 *
 ******************************************************************************/

void DMX512_set_mode(DMX512_instance_t* instance, DMX512_mode_t mode);

/*******************************************************************************
 *
 * DMX512_begin_new_dmx_transmit
 *
 * DESCRIPTION:
 *  Attempts to begin a new DMX transmit task. The request will be accepted if
 *  the instance is in transmit mode and not currently busy with a task.
 *
 * PARAMETERS:
 *  start_code
 *   The first byte of the data portion of the transaction.
 *
 *  data
 *   The bytes following the start code (e.g. DMX values).
 *
 *  data_length
 *   The length, in bytes, of the data array (DMX values only, not the
 *   start byte).
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool DMX512_begin_new_dmx_transmit(DMX512_instance_t* instance,
                                   uint8_t start_code,
                                   uint8_t* data,
                                   uint16_t data_length);

/*******************************************************************************
 *
 * DMX512_service
 *
 * DESCRIPTION:
 *  Services the task state machine. Must be called repeatedly until the task
 *  is completed.
 *
 * RETURN:
 *  False if there is an ongoing task which has not completed, else, true.
 *
 ******************************************************************************/

bool DMX512_service(DMX512_instance_t* instance);

/*******************************************************************************
 *
 * DMX512_is_busy
 *
 * DESCRIPTION:
 *  Determines if an instance is currently busy with a task.
 *
 * RETURN:
 *  True if the instance is busy with a task, else, false.
 *
 ******************************************************************************/

bool DMX512_is_busy(DMX512_instance_t* instance);

#ifdef __cplusplus
}
#endif
#endif // DMX512_J_H

/*******************************************************************************
 *
 *  EEPROM with generic HAL interface. Requires proper initialization and the
 *  service routine to be called repeatedly after a new task request until the
 *  task is completed.
 *
 *  While EEPROMs are built by several manufactures, most follow the same
 *  interface of a 2-4 byte address followed by a read or write operation. The
 *  maximum number of bytes that can be written per a single transaction is the
 *  size of the EEPROM page. Once written, the EEPROM needs time to commit the
 *  received data to NVM. A read operation does not have this same limitation
 *  and can span accross pages.
 *
 ******************************************************************************/

#ifndef EEPROM_J_H
#define EEPROM_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *
 * EEPROM_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 * busy
 *  Set when the module is busy with a task and cleared when the task is
 *  completed.
 *
 * erase_task
 *  Set if the current task is an erase task. Allows the erase task to utilize
 *  the write service handler with minor differences.
 *
 * task_state
 *  State machine state.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t busy                          : 1;
    uint8_t erase_task                    : 1;
    uint8_t reserved2                     : 2;
    uint8_t task_state                    : 3;
    uint8_t reserved7                     : 1;
  };
}
EEPROM_flags_t;

/*******************************************************************************
 *
 * EEPROM_pre_task_callback_t
 *
 * DESCRIPTION:
 *  Function template for a user-provided function which is called when a new
 *  task is accepted and initialized but before the task actually begins. This
 *  allows the user to perform any last additional configurations before the
 *  actual logic of the task begins.
 *
 * PARAMETERS:
 *  context
 *   User-provided context.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*EEPROM_pre_task_callback_t)(uint32_t);

/*******************************************************************************
 *
 * EEPROM_post_task_callback_t
 *
 * DESCRIPTION:
 *  Function template for a user-provided function which is called after a
 *  task completes.
 *
 * PARAMETERS:
 *  context
 *   User-provided context.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*EEPROM_post_task_callback_t)(uint32_t);

/*******************************************************************************
 *
 * EEPROM_hal_driver_read_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will read the EEPROM.
 *
 * PARAMETERS:
 *  address
 *   Starting memory address.
 *
 *  address_reg_length
 *   The length, in bytes, of the memory address or register.
 *
 *  buffer
 *   Buffer which will hold the read data.
 *
 *  length
 *   The length, in bytes, of data to read.
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*EEPROM_hal_driver_read_t)(uint32_t, uint8_t, uint8_t*, uint32_t);

/*******************************************************************************
 *
 * EEPROM_hal_driver_write_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will write the EEPROM.
 *
 * PARAMETERS:
 *  address
 *   Starting memory address.
 *
 *  address_reg_length
 *   The length, in bytes, of the memory address or register.
 *
 *  buffer
 *   Buffer which holds the data to be written.
 *
 *  length
 *   The length, in bytes, of data to write.
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*EEPROM_hal_driver_write_t)(uint32_t, uint8_t, uint8_t*, uint32_t);

/*******************************************************************************
 *
 * EEPROM_hal_driver_service_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will service the driver.
 *
 * RETURN:
 *  False if there is an ongoing task which has not completed, else, true.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*EEPROM_hal_driver_service_t)(void);

/*******************************************************************************
 *
 * EEPROM_hal_driver_timeout_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will determine if the driver has had a task timeout.
 *
 * RETURN:
 *  True if a timeout has occurred, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*EEPROM_hal_driver_timeout_t)(void);

/*******************************************************************************
 *
 * EEPROM_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module flags.
 *
 * utimer
 *  User-provided initialized instance of a UTIMER.
 *
 * utimer_ticket
 *  Data structure used with the UTIMER instance.
 *
 * bus_mutex
 *  User-provided initialized instance of a BUSMUTEX.
 *
 * bus_id
 *  The BUS ID associated with the BUSMUTEX instance.
 *
 * address_reg_length
 *  The length, in bytes, of the EEPROM address/register.
 *
 * page_length
 *  The length, in bytes, of a single EEPROM page.
 *
 * total_length
 *  The length, in bytes, of the entrie EEPROM.
 *
 * buffer
 *  Pointer to the user buffer which contains the data to written or will
 *  contain the read data.
 *
 * rw_count
 *  The number of bytes to write to, or read from, the data buffer.
 *
 * rw_counter
 *  Keeps track of the number of bytes that have been written to, or read from,
 *  the data buffer.
 *
 * rw_address
 *  The starting address of the RW task.
 *
 * page_commit_timeout_us
 *  The time, in microseconds, which must be waited between page writes to give
 *  the EEPROM time to commit its volatile page buffer to NVM.
 *
 * callback_context
 *  Context passed into the user pre/post operation callbacks.
 *
 * service_handler
 *  Function pointer to the service routine applicable to the initialized
 *  service mode.
 *
 * *_task_*
 *  User-provided functions. See typedef comments.
 *
 * *_hal_*
 *  User-provided functions. See typedef comments.
 *
 ******************************************************************************/

typedef struct
{
  EEPROM_flags_t flags;
  UTIMER_instance_t* utimer;
  UTIMER_ticket_t utimer_ticket;
  BUSMUTEX_instance_t* bus_mutex;
  BUSMUTEX_bus_id_t bus_id;
  uint8_t address_reg_length;
  uint16_t page_length;
  uint32_t total_length;
  uint8_t* buffer;
  uint32_t rw_count;
  uint32_t rw_counter;
  uint32_t rw_address;
  uint32_t page_commit_timeout_us;
  uint32_t callback_context;
  bool (*service_handler)(void* instance);
  EEPROM_pre_task_callback_t pre_task_callback;
  EEPROM_post_task_callback_t post_task_callback;
  EEPROM_hal_driver_read_t driver_read;
  EEPROM_hal_driver_write_t driver_write;
  EEPROM_hal_driver_service_t driver_service;
  EEPROM_hal_driver_timeout_t driver_timeout;
}
EEPROM_instance_t;

/*******************************************************************************
 *
 * EEPROM_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values.
 *
 * PARAMETERS:
 *  See EEPROM_instance_t.
 *
 ******************************************************************************/

void EEPROM_initialize(EEPROM_instance_t* instance,
                       UTIMER_instance_t* utimer,
                       BUSMUTEX_instance_t* bus_mutex,
                       BUSMUTEX_bus_id_t bus_id,
                       uint8_t address_reg_length,
                       uint16_t page_length,
                       uint32_t total_length,
                       uint32_t page_commit_timeout_us,
                       EEPROM_pre_task_callback_t pre_task_callback,
                       EEPROM_post_task_callback_t post_task_callback,
                       EEPROM_hal_driver_read_t driver_read,
                       EEPROM_hal_driver_write_t driver_write,
                       EEPROM_hal_driver_service_t driver_service,
                       EEPROM_hal_driver_timeout_t driver_timeout);

/*******************************************************************************
 *
 * EEPROM_purge
 *
 * DESCRIPTION:
 *  Attempts to begin a new EEPROM erase task with the address range of the
 *  entire EEPROM.
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool EEPROM_purge(EEPROM_instance_t* instance);

/*******************************************************************************
 *
 * EEPROM_erase
 *
 * DESCRIPTION:
 *  Attempts to begin a new EEPROM erase task.
 *
 * PARAMETERS:
 *  start_address
 *   The starting memory address.
 *
 *  length
 *   The length, in bytes, to erase.
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool EEPROM_erase(EEPROM_instance_t* instance,
                  uint32_t start_address,
                  uint32_t length);

/*******************************************************************************
 *
 * EEPROM_begin_new_write
 *
 * DESCRIPTION:
 *  Attempts to begin a new EEPROM write task.
 *
 * PARAMETERS:
 *  start_address
 *   The starting EEPROM memory address where the data will be written.
 *
 *  buffer
 *   User buffer which contains the data to be written to the EEPROM.
 *
 *  length
 *   The length, in bytes, of the buffer (number of bytes to write).
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool EEPROM_begin_new_write(EEPROM_instance_t* instance,
                            uint32_t start_address,
                            uint8_t* buffer,
                            uint32_t length);

/*******************************************************************************
 *
 * EEPROM_begin_new_read
 *
 * DESCRIPTION:
 *  Attempts to begin a new EEPROM read task.
 *
 * PARAMETERS:
 *  start_address
 *   The starting EEPROM memory address where the data will be read.
 *
 *  buffer
 *   User buffer which will hold the data read from the EEPROM.
 *
 *  length
 *   The length, in bytes, of the buffer (number of bytes to read).
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool EEPROM_begin_new_read(EEPROM_instance_t* instance,
                           uint32_t start_address,
                           uint8_t* buffer,
                           uint32_t length);

/*******************************************************************************
 *
 * EEPROM_service
 *
 * DESCRIPTION:
 *  Services the task state machine. Must be called repeatedly until the task
 *  is completed.
 *
 * RETURN:
 *  False if there is an ongoing task which has not completed, else, true.
 *
 ******************************************************************************/

bool EEPROM_service(EEPROM_instance_t* instance);

/*******************************************************************************
 *
 * EEPROM_is_busy
 *
 * DESCRIPTION:
 *  Determines if an instance is currently busy with a task.
 *
 * RETURN:
 *  True if the instance is busy with a task, else, false.
 *
 ******************************************************************************/

bool EEPROM_is_busy(EEPROM_instance_t* instance);

#ifdef __cplusplus
}
#endif
#endif // EEPROM_J_H

/*******************************************************************************
 *
 *  Font structures for standard gfx font. Adafruit provides a tool which can
 *  convert between True-type to gfx here: https://rop.nl/truetype2gfx/
 *
 ******************************************************************************/

#ifndef GFX2D_FONT_J_H
#define GFX2D_FONT_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *
 * GFX2DFONT_glyph_t
 *
 * DESCRIPTION:
 *  Provides meta information about a particular glyph/character which resides
 *  within the actual font bitmap.
 *
 * bitmap_offset
 *  Offset into the bitmap where the glyph data begins.
 *
 * width
 *  Width of the glyph in pixels.
 *
 * height
 *  Height of the glyph in pixels.
 *
 * x_advance
 *  Distance to advance the cursor after each printed glyph. For mono-spaced
 *  fonts, this normally equals the width of the widest character.
 *
 * x_offset
 *  Distance, in pixels, from cursor position to upper-left corner. Not always
 *  zero because the glyph may be somewhat centered in its allowed space.
 *
 * y_offset
 *  Distance, in pixels, from cursor position to upper-left corner. The cursor
 *  is located in the bottom-left corner of the glyph space, so this value is
 *  almost always negative (positive for something like '_').
 *
 ******************************************************************************/

typedef struct
{
  uint16_t bitmap_offset;
  uint8_t width;
  uint8_t height;
  uint8_t x_advance;
  int8_t x_offset;
  int8_t y_offset;
}
GFX2DFONT_glyph_t;

/*******************************************************************************
 *
 * GFX2DFONT_font_t
 *
 * DESCRIPTION:
 *  An entire font module in which a bitmap contains all the glyph/characters,
 *  and meta information describes the glyph offsets in the bitmap.
 *
 * bitmap
 *  Bitmap containing all font glyph/characters.
 *
 * glyph
 *  Array of glyph, one for each glyph/character, which provides meta
 *  information of the location in the bitmap.
 *
 * first_ascii
 *  First ASCII character/glyph supported by the font.
 *
 * last_ascii
 *  Last ASCII character/glyph supported by the font.
 *
 * y_advance
 *  Distance to move cursor down for the next line of text.
 *
 ******************************************************************************/

typedef struct
{
  uint8_t* bitmap;
  GFX2DFONT_glyph_t* glyph;
  uint16_t first_ascii;
  uint16_t last_ascii;
  uint8_t y_advance;
}
GFX2DFONT_font_t;

/*
 * Default font.
 */

extern const GFX2DFONT_font_t GFX2DFONT_DEFAULT_FONT;

#ifdef __cplusplus
}
#endif
#endif // GFX2D_FONT_J_H

/*******************************************************************************
 *
 *  Simple graphics library which can render basic shapes and images. Elements
 *  are rendered into a memory buffer. It is then up to external user code to
 *  send the buffer to the display.
 *
 *  With segmentation the target display can be rendered in parts. This is used
 *  when there is not enough memory to host an entire virtual display buffer.
 *  Since each segment requires a re-render of the display frame, it is ideal
 *  to have a large enough buffer which can hold the entire virtual display
 *  frame buffer.
 *
 *  This library supports all combinations of up to 32-bit RGB formats (e.g.
 *  3:3:2, 3:3:3, 4:4:4, 5:6:5, 6:6:6, 8:8:8, etc...) The RGB bit makeup of the
 *  pixel is irrelevant to the library logic. Instead, the user is required to
 *  provide a function which interprets a provided RBGA color into the desired
 *  pixel format.
 *
 *  While this library supports non-byte-aligned formats (i.e. when the sum of
 *  the RGBA bits is not divisible by 8), the algorithm throughput for these
 *  formats is considerably slower, on the order to 20x slower or more.
 *
 ******************************************************************************/

#ifndef GFX2D_J_H
#define GFX2D_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *
 * GFX2D_rgb_bitmap_t
 *
 * DESCRIPTION:
 *  Enumerates the various supported bitmap types for draw_rgb_bitmap.
 *
 ******************************************************************************/

typedef enum
{
  GFX2D_RGB_BITMAP_332                    = 0,
  GFX2D_RGB_BITMAP_565,
  GFX2D_RGB_BITMAP_888
}
GFX2D_rgb_bitmap_t;

/*******************************************************************************
 *
 * GFX2D_circle_quadrant_t
 *
 * DESCRIPTION:
 *  Used for determining which quadrants of a circle need to be drawn.
 *
 ******************************************************************************/

typedef enum
{
  GFX2D_CIRCLE_QUADRANT_NONE              = 0, // 0b0000
  GFX2D_CIRCLE_QUADRANT_TOP_LEFT          = 1, // 0b0001
  GFX2D_CIRCLE_QUADRANT_TOP_RIGHT         = 2, // 0b0010
  GFX2D_CIRCLE_QUADRANT_BOTTOM_LEFT       = 4, // 0b0100
  GFX2D_CIRCLE_QUADRANT_BOTTOM_RIGHT      = 8, // 0b1000
  GFX2D_CIRCLE_QUADRANT_ALL               = 15 // 0b1111
}
GFX2D_circle_quadrant_t;

/*******************************************************************************
 *
 * GFX2D_rgba_t
 *
 * DESCRIPTION:
 *  Traditional 32-bit color, RGB with an Alpha channel. This type is used for
 *  user input and does not necessarily reflect the rendered color based on
 *  pixel color settings.
 *
 ******************************************************************************/

typedef union
{
  uint32_t all;
  struct
  {
    uint32_t r                            : 8;
    uint32_t g                            : 8;
    uint32_t b                            : 8;
    uint32_t a                            : 8;
  };
}
GFX2D_rgba_t;

/*******************************************************************************
 *
 * GFX2D_font_t
 *
 * DESCRIPTION:
 *  Collection of variables for text drawing.
 *
 * font
 *  Points to the font module to use.
 *
 * cursor_x
 *  The current x-start location of the next character to write.
 *
 * cursor_y
 *  The current y-start location of the next character to write.
 *
 * color
 *  See GFX2D_rgba_t. (Uint32_t used for quicker access.)
 *
 * x_magnification
 *  Magnification of the text in the x-direction.
 *
 * y_magnification
 *  Magnification of the text in the y-direction.
 *
 ******************************************************************************/

typedef struct
{
  GFX2DFONT_font_t* font;
  int16_t cursor_x;
  int16_t cursor_y;
  uint32_t color;
  uint8_t x_magnification;
  uint8_t y_magnification;
}
GFX2D_font_t;

/*******************************************************************************
 *
 * GFX2D_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 * byte_aligned
 *  Indicates whether bits_per_pixel is evenly divisible by 8, that is, it has
 *  byte alignment. This flag is set or cleared during initialization.
 *
 * invert
 *  Inverts the pixel location output to support a 180-degree screen rotation.
 *
 * wrap_text
 *  Will cause text to wrap around to the next line automatically if the end
 *  of the screen is reached.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t byte_aligned                  : 1;
    uint8_t invert                        : 1;
    uint8_t wrap_text                     : 1;
    uint8_t reserved3                     : 5;
  };
}
GFX2D_flags_t;

/*******************************************************************************
 *
 * GFX2D_rgba_to_pixel_t
 *
 * DESCRIPTION:
 *  Function template for a user-provided function which is called to convert
 *  a GFX2D_rgba_t into the desired pixel format. If the number of bits per
 *  pixel is less than 32, then the output pixels should only occupy the least
 *  significant bits.
 *
 *  The user should also include any byte-swapping as required to support big
 *  versus little endian formatting.
 *
 * PARAMETERS:
 *  rgba
 *   See GFX2D_rgba_t. (Uint32_t used for quicker access.)
 *
 * RETURN:
 *  Formatted pixel color.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef uint32_t (*GFX2D_rgba_to_pixel_t)(uint32_t);

/*******************************************************************************
 *
 * GFX2D_canvas_fill_handler_t
 *
 * DESCRIPTION:
 *  Function template for internal helper functions for GFX2D_fill_canvas. The
 *  canvas fill function is used to clear the screen and set a background color.
 *  Hence, having a direct call to an optimized handler can significantly improve
 *  performance.
 *
 * PARAMETERS:
 *  instance
 *   Pointer to GFX2D_instance_t.
 *
 *  color
 *   Formatted pixel color.
 *
 * NOTES:
 *  For internal use only.
 *
 ******************************************************************************/

typedef void (*GFX2D_canvas_fill_handler_t)(void*, uint32_t);

/*******************************************************************************
 *
 * GFX2D_draw_hline_handler_t
 *
 * DESCRIPTION:
 *  Function template for internal helper functions for GFX2D_draw_hline. The
 *  hline function is used for the various filled objects, such as triangle,
 *  rectangle, and circle. Hence, having a direct call to an optimized handler
 *  can significantly improve performance.
 *
 * PARAMETERS:
 *  instance
 *   Pointer to GFX2D_instance_t.
 *
 *  x
 *   X-Coords starting point of line.
 *
 *  y
 *   Y-Coords starting point of line.
 *
 *  length
 *   Length of the line in pixels.
 *
 *  color
 *   Formatted pixel color.
 *
 * NOTES:
 *  For internal use only.
 *
 ******************************************************************************/

typedef void (*GFX2D_draw_hline_handler_t)(void*,
                                           int16_t,
                                           int16_t,
                                           int16_t,
                                           uint32_t);

/*******************************************************************************
 *
 * GFX2D_draw_vline_handler_t
 *
 * DESCRIPTION:
 *  Function template for internal helper functions for GFX2D_draw_vline.
 *
 * PARAMETERS:
 *  instance
 *   Pointer to GFX2D_instance_t.
 *
 *  x
 *   X-Coords starting point of line.
 *
 *  y
 *   Y-Coords starting point of line.
 *
 *  length
 *   Length of the line in pixels.
 *
 *  color
 *   Formatted pixel color.
 *
 * NOTES:
 *  For internal use only.
 *
 ******************************************************************************/

typedef void (*GFX2D_draw_vline_handler_t)(void*,
                                           int16_t,
                                           int16_t,
                                           int16_t,
                                           uint32_t);

/*******************************************************************************
 *
 * GFX2D_draw_filled_rectangle_handler_t
 *
 * DESCRIPTION:
 *  Function template for internal helper functions for GFX2D_draw_filled_-
 *  rectangle. The filled rectangle function is used for drawing panels and
 *  other large areas. Hence, having a direct call to an optimized handler can
 *  significantly improve performance.
 *
 * PARAMETERS:
 *  instance
 *   Pointer to GFX2D_instance_t.
 *
 *  x
 *   X-Coords starting point of line.
 *
 *  y
 *   Y-Coords starting point of line.
 *
 *  width
 *   Width of the rectangle in pixels.
 *
 *  height
 *   Height of the rectangle in pixels.
 *
 *  color
 *   Formatted pixel color.
 *
 * NOTES:
 *  For internal use only.
 *
 ******************************************************************************/

typedef void (*GFX2D_draw_filled_rectangle_handler_t)(void*,
                                                      int16_t,
                                                      int16_t,
                                                      int16_t,
                                                      int16_t,
                                                      uint32_t);

/*******************************************************************************
 *
 * GFX2D_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * NOTE:
 *  The drawing canvas is a segmented portion of the target display frame. If
 *  resources allow, the canvas dimensions should be the same as the target for
 *  maximum efficiency. Otherwise, the canvas represents a portion of the target
 *  display frame defined by a x,y offset and width and height.
 *
 * flags
 *  Module flags.
 *
 * bits_per_pixel
 *  The number of bits which make up a single pixel, ranging from 1 to 32.
 *
 * bytes_per_pixel
 *  The number of bytes which make up a single pixel. This value is only used
 *  for byte-aligned formats.
 *
 * display_buffer
 *  Pointer to user-provided buffer which will hold the rendered graphics.
 *
 * display_buffer_length_bytes
 *  Total length, in bytes, of the display buffer.
 *
 * display_buffer_length_pixels
 *  Total length, in pixels, of the display buffer.
 *
 * display_target_width
 *  The width, in pixels, of the target display.
 *
 * display_target_height
 *  The height, in pixels, of the target display.
 *
 * canvas_x0
 *  The starting x-offset relative to the target display frame which is mapped
 *  to the canvas x-origin.
 *
 * canvas_y0
 *  The starting y-offset relative to the target display frame which is mapped
 *  to the canvas y-origin.
 *
 * canvas_width
 *  Width, in pixels, of the drawing canvas.
 *
 * canvas_height
 *  Height, in pixels, of the drawing canvas.
 *
 * canvas_bits_per_row
 *  The number of bits which compose a single drawing canvas row.
 *
 * canvas_bytes_per_row
 *  The number of bytes which compose a single drawing canvas row. This value
 *  is only used for byte-aligned formats.
 *
 * canvas_length_bytes
 *  Total length, in bytes, of the drawing canvas.
 *
 * canvas_length_pixels
 *  Total length, in pixels, of the drawing canvas.
 *
 * font
 *  Collection of text related variables.
 *
 * rgba_to_pixel
 *  See GFX2D_rgba_to_pixel_t.
 *
 * canvas_fill_handler
 *  See GFX2D_canvas_fill_handler_t.
 *
 * draw_hline_handler
 *  See GFX2D_draw_hline_handler_t.
 *
 * draw_vline_handler
 *  See GFX2D_draw_vline_handler_t.
 *
 * draw_filled_rectangle_handler
 *  See GFX2D_draw_filled_rectangle_handler_t.
 *
 ******************************************************************************/

typedef struct
{
  GFX2D_flags_t flags;
  uint8_t bits_per_pixel;
  uint8_t bytes_per_pixel;
  uint8_t* display_buffer;
  uint32_t display_buffer_length_bytes;
  uint32_t display_buffer_length_pixels;
  int16_t display_target_width;
  int16_t display_target_height;
  int16_t canvas_x0;
  int16_t canvas_y0;
  int16_t canvas_width;
  int16_t canvas_height;
  uint32_t canvas_bits_per_row;
  uint32_t canvas_bytes_per_row;
  uint32_t canvas_length_bytes;
  uint32_t canvas_length_pixels;
  GFX2D_font_t font;
  GFX2D_rgba_to_pixel_t rgba_to_pixel;
  GFX2D_canvas_fill_handler_t canvas_fill_handler;
  GFX2D_draw_hline_handler_t draw_hline_handler;
  GFX2D_draw_vline_handler_t draw_vline_handler;
  GFX2D_draw_filled_rectangle_handler_t draw_filled_rectangle_handler;
}
GFX2D_instance_t;

/*******************************************************************************
 *
 * GFX2D_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values. The drawing canvas is defaulted to a single pixel. The user
 *  should call GFX2D_set_canvas_dimensions after initialization to set the draw
 *  dimensions.
 *
 * PARAMETERS:
 *  See GFX2D_instance_t.
 *
 ******************************************************************************/

void GFX2D_initialize(GFX2D_instance_t* instance,
                      uint8_t* display_buffer,
                      uint32_t display_buffer_length_bytes,
                      int16_t display_target_width,
                      int16_t display_target_height,
                      uint8_t bits_per_pixel,
                      GFX2D_rgba_to_pixel_t rgba_to_pixel);

/*******************************************************************************
 *
 * GFX2D_set_canvas_dimensions
 *
 * DESCRIPTION:
 *  Sets the drawing canvas origin and pixel width and height. If the user
 *  display buffer is large enough to accomodate an entire virtual display frame
 *  for the target display, it is ideal to only call this function once with an
 *  origin of 0,0 and a width and height equal to the target display.
 *
 * PARAMETERS:
 *  See GFX2D_instance_t.
 *
 * RETURN:
 *  True if the canvas dimensions were set, else, false if the dimensions
 *  require more space than available in the display buffer. In this case, the
 *  width and height are set to 1.
 *
 * NOTE:
 *  Dimensions which overflow the target display width and height will be
 *  trimmed. The function will still return true.
 *
 ******************************************************************************/

bool GFX2D_set_canvas_dimensions(GFX2D_instance_t* instance,
                                 int16_t canvas_x0,
                                 int16_t canvas_y0,
                                 int16_t canvas_width,
                                 int16_t canvas_height);

/*******************************************************************************
 *
 * GFX2D_draw_pixel
 *
 * DESCRIPTION:
 *  Draws a single pixel.
 *
 * PARAMETERS:
 *  x
 *   X-Coords of pixel.
 *
 *  y
 *   Y-Coords of pixel.
 *
 *  color
 *   See GFX2D_rgba_t. (Uint32_t used for quicker access.)
 *
 * RETURN:
 *  True if the pixel was drawn, else, false.
 *
 ******************************************************************************/

bool GFX2D_draw_pixel(GFX2D_instance_t* instance,
                      int16_t x,
                      int16_t y,
                      uint32_t color);

/*******************************************************************************
 *
 * GFX2D_fill_canvas
 *
 * DESCRIPTION:
 *  Fills the canvas buffer with a specified color.
 *
 * PARAMETERS:
 *  color
 *   See GFX2D_rgba_t. (Uint32_t used for quicker access.)
 *
 ******************************************************************************/

void GFX2D_fill_canvas(GFX2D_instance_t* instance, uint32_t color);

/*******************************************************************************
 *
 * GFX2D_draw_hline
 *
 * DESCRIPTION:
 *  Draws a horizontal line.
 *
 * PARAMETERS:
 *  x
 *   X-Coords starting point of line.
 *
 *  y
 *   Y-Coords starting point of line.
 *
 *  length
 *   Length of the line in pixels.
 *
 *  color
 *   See GFX2D_rgba_t. (Uint32_t used for quicker access.)
 *
 ******************************************************************************/

void GFX2D_draw_hline(GFX2D_instance_t* instance,
                      int16_t x,
                      int16_t y,
                      int16_t length,
                      uint32_t color);

/*******************************************************************************
 *
 * GFX2D_draw_vline
 *
 * DESCRIPTION:
 *  Draws a vertical line.
 *
 * PARAMETERS:
 *  x
 *   X-Coords starting point of line.
 *
 *  y
 *   Y-Coords starting point of line.
 *
 *  length
 *   Length of the line in pixels.
 *
 *  color
 *   See GFX2D_rgba_t. (Uint32_t used for quicker access.)
 *
 ******************************************************************************/

void GFX2D_draw_vline(GFX2D_instance_t* instance,
                      int16_t x,
                      int16_t y,
                      int16_t length,
                      uint32_t color);

/*******************************************************************************
 *
 * GFX2D_draw_line
 *
 * DESCRIPTION:
 *  Draws a line.
 *
 * PARAMETERS:
 *  x0
 *   X-Coords starting point of line.
 *
 *  y0
 *   Y-Coords starting point of line.
 *
 *  x1
 *   X-Coords ending point of line.
 *
 *  y1
 *   Y-Coords ending point of line.
 *
 *  color
 *   See GFX2D_rgba_t. (Uint32_t used for quicker access.)
 *
 ******************************************************************************/

void GFX2D_draw_line(GFX2D_instance_t* instance,
                     int16_t x0,
                     int16_t y0,
                     int16_t x1,
                     int16_t y1,
                     uint32_t color);

/*******************************************************************************
 *
 * GFX2D_draw_triangle
 *
 * DESCRIPTION:
 *  Draws a triangle.
 *
 * PARAMETERS:
 *  x0
 *   X-Coords of first triangle point.
 *
 *  y0
 *   Y-Coords of first triangle point..
 *
 *  x1
 *   X-Coords of second triangle point.
 *
 *  y1
 *   Y-Coords of second triangle point.
 *
 *  x2
 *   X-Coords of third triangle point.
 *
 *  y2
 *   Y-Coords of third triangle point.
 *
 *  color
 *   See GFX2D_rgba_t. (Uint32_t used for quicker access.)
 *
 ******************************************************************************/

void GFX2D_draw_triangle(GFX2D_instance_t* instance,
                         int16_t x0,
                         int16_t y0,
                         int16_t x1,
                         int16_t y1,
                         int16_t x2,
                         int16_t y2,
                         uint32_t color);

/*******************************************************************************
 *
 * GFX2D_draw_filled_triangle
 *
 * DESCRIPTION:
 *  Draws a filled triangle.
 *
 * PARAMETERS:
 *  x0
 *   X-Coords of first triangle point.
 *
 *  y0
 *   Y-Coords of first triangle point.
 *
 *  x1
 *   X-Coords of second triangle point.
 *
 *  y1
 *   Y-Coords of second triangle point.
 *
 *  x2
 *   X-Coords of third triangle point.
 *
 *  y2
 *   Y-Coords of third triangle point.
 *
 *  color
 *   See GFX2D_rgba_t. (Uint32_t used for quicker access.)
 *
 ******************************************************************************/

void GFX2D_draw_filled_triangle(GFX2D_instance_t* instance,
                                int16_t x0,
                                int16_t y0,
                                int16_t x1,
                                int16_t y1,
                                int16_t x2,
                                int16_t y2,
                                uint32_t color);

/*******************************************************************************
 *
 * GFX2D_draw_circle_arc
 *
 * DESCRIPTION:
 *  Mainly used as a helper function, but can be called independently. Draws
 *  the quadrant arcs of a circle depending on the input bit quadrant flag.
 *
 * PARAMETERS:
 *  x0
 *   X-Coords of center point of circle.
 *
 *  y0
 *   Y-Coords of center point of circle.
 *
 *  radius
 *   Radius of the circle.
 *
 *  quadrant
 *   Bit flag(s) indicating which arc(s), or quadrant(s) of a circle, to draw.
 *
 *  color
 *   See GFX2D_rgba_t. (Uint32_t used for quicker access.)
 *
 ******************************************************************************/

void GFX2D_draw_circle_arc(GFX2D_instance_t* instance,
                           int16_t x0,
                           int16_t y0,
                           int16_t radius,
                           GFX2D_circle_quadrant_t quadrant,
                           uint32_t color);

/*******************************************************************************
 *
 * GFX2D_draw_filled_circle_arc
 *
 * DESCRIPTION:
 *  Mainly used as a helper function, but can be called independently. Draws
 *  the quadrant arcs of a circle depending on the input bit quadrant flag.
 *
 * PARAMETERS:
 *  x0
 *   X-Coords of center point of circle.
 *
 *  y0
 *   Y-Coords of center point of circle.
 *
 *  radius
 *   Radius of the circle.
 *
 *  quadrant
 *   Bit flag(s) indicating which arc(s), or quadrant(s) of a circle, to draw.
 *
 *  color
 *   See GFX2D_rgba_t. (Uint32_t used for quicker access.)
 *
 ******************************************************************************/

void GFX2D_draw_filled_circle_arc(GFX2D_instance_t* instance,
                                  int16_t x0,
                                  int16_t y0,
                                  int16_t radius,
                                  GFX2D_circle_quadrant_t quadrant,
                                  uint32_t color);

/*******************************************************************************
 *
 * GFX2D_draw_circle
 *
 * DESCRIPTION:
 *  Draws a circle.
 *
 * PARAMETERS:
 *  x0
 *   X-Coords of center point.
 *
 *  y0
 *   Y-Coords of center point.
 *
 *  radius
 *   Radius of the circle.
 *
 *  color
 *   See GFX2D_rgba_t. (Uint32_t used for quicker access.)
 *
 ******************************************************************************/

void GFX2D_draw_circle(GFX2D_instance_t* instance,
                       int16_t x0,
                       int16_t y0,
                       int16_t radius,
                       uint32_t color);

/*******************************************************************************
 *
 * GFX2D_draw_filled_circle
 *
 * DESCRIPTION:
 *  Draws a filled circle.
 *
 * PARAMETERS:
 *  x0
 *   X-Coords of center point.
 *
 *  y0
 *   Y-Coords of center point.
 *
 *  radius
 *   Radius of the circle.
 *
 *  color
 *   See GFX2D_rgba_t. (Uint32_t used for quicker access.)
 *
 ******************************************************************************/

void GFX2D_draw_filled_circle(GFX2D_instance_t* instance,
                              int16_t x0,
                              int16_t y0,
                              int16_t radius,
                              uint32_t color);

/*******************************************************************************
 *
 * GFX2D_draw_rectangle
 *
 * DESCRIPTION:
 *  Draws a rectangle.
 *
 * PARAMETERS:
 *  x
 *   X-Coords starting point of rectangle.
 *
 *  y
 *   Y-Coords starting point of rectangle.
 *
 *  width
 *   Width of the rectangle in pixels.
 *
 *  height
 *   Height of the rectangle in pixels.
 *
 *  color
 *   See GFX2D_rgba_t. (Uint32_t used for quicker access.)
 *
 ******************************************************************************/

void GFX2D_draw_rectangle(GFX2D_instance_t* instance,
                          int16_t x,
                          int16_t y,
                          int16_t width,
                          int16_t height,
                          uint32_t color);

/*******************************************************************************
 *
 * GFX2D_draw_filled_rectangle
 *
 * DESCRIPTION:
 *  Draws a filled rectangle.
 *
 * PARAMETERS:
 *  x
 *   X-Coords starting point of rectangle.
 *
 *  y
 *   Y-Coords starting point of rectangle.
 *
 *  width
 *   Width of the rectangle in pixels.
 *
 *  height
 *   Height of the rectangle in pixels.
 *
 *  color
 *   See GFX2D_rgba_t. (Uint32_t used for quicker access.)
 *
 ******************************************************************************/

void GFX2D_draw_filled_rectangle(GFX2D_instance_t* instance,
                                int16_t x,
                                int16_t y,
                                int16_t width,
                                int16_t height,
                                uint32_t color);

/*******************************************************************************
 *
 * GFX2D_draw_rounded_rectangle
 *
 * DESCRIPTION:
 *  Draws a rectangle with rounded edges.
 *
 * PARAMETERS:
 *  x
 *   X-Coords starting point of rectangle.
 *
 *  y
 *   Y-Coords starting point of rectangle.
 *
 *  width
 *   Width of the rectangle in pixels.
 *
 *  height
 *   Height of the rectangle in pixels.
 *
 *  color
 *   See GFX2D_rgba_t. (Uint32_t used for quicker access.)
 *
 ******************************************************************************/

void GFX2D_draw_rounded_rectangle(GFX2D_instance_t* instance,
                                  int16_t x,
                                  int16_t y,
                                  int16_t width,
                                  int16_t height,
                                  int16_t radius,
                                  uint32_t color);

/*******************************************************************************
 *
 * GFX2D_draw_filled_rounded_rectangle
 *
 * DESCRIPTION:
 *  Draws a filled rectangle with rounded edges.
 *
 * PARAMETERS:
 *  x
 *   X-Coords starting point of rectangle.
 *
 *  y
 *   Y-Coords starting point of rectangle.
 *
 *  width
 *   Width of the rectangle in pixels.
 *
 *  height
 *   Height of the rectangle in pixels.
 *
 *  color
 *   See GFX2D_rgba_t. (Uint32_t used for quicker access.)
 *
 ******************************************************************************/

void GFX2D_draw_filled_rounded_rectangle(GFX2D_instance_t* instance,
                                         int16_t x,
                                         int16_t y,
                                         int16_t width,
                                         int16_t height,
                                         int16_t radius,
                                         uint32_t color);

/*******************************************************************************
 *
 * GFX2D_draw_binary_bitmap
 *
 * DESCRIPTION:
 *  Draws a binary color bitmap at the specified x,y location. If the color and
 *  background color are the same value, then the background color will not be
 *  drawn and will instead act as a transparent pixel.
 *
 * PARAMETERS:
 *  bitmap
 *   Pointer to the binary bitmap.
 *
 *  x0, y0
 *   Target display write origin.
 *
 *  bitmap_x0, bitmap_y0
 *   Bitmap read origin.
 *
 *  bitmap_width_draw
 *   The width, in pixels, of the bitmap portion to draw.
 *
 *  bitmap_height_draw
 *   The height, in pixels, of the bitmap portion to draw.
 *
 *  bitmap_width_total
 *   The width, in pixels, of the complete bitmap image.
 *
 *  bitmap_height_total
 *   The height, in pixels, of the complete bitmap image.
 *
 *  color
 *   Color set to a pixel when the corresponding bitmap bit is a 1.
 *
 *  background_color
 *   Color set to a pixel when the corresponding bitmap bit is a 0. If the
 *   background color is the same as color, then the pixel is not drawn in
 *   the case of 0 (transparent).
 *
 ******************************************************************************/

void GFX2D_draw_binary_bitmap(GFX2D_instance_t* instance,
                              const uint8_t* bitmap,
                              int16_t x0,
                              int16_t y0,
                              uint16_t bitmap_x0,
                              uint16_t bitmap_y0,
                              uint16_t bitmap_width_draw,
                              uint16_t bitmap_height_draw,
                              uint16_t bitmap_width_total,
                              uint16_t bitmap_height_total,
                              uint32_t color,
                              uint32_t background_color);

/*******************************************************************************
 *
 * GFX2D_draw_rgb_bitmap
 *
 * DESCRIPTION:
 *  Draws an RGB bitmap at the specified x,y location.
 *
 * PARAMETERS:
 *  rgb_bitmap
 *   Bitmap format (332, 565, 888).
 *
 *  alpha_mask
 *   Bit-mask indicating which pixels should be transparent (0) and which
 *   should be opaque (1). Each bit represents a pixel, so the length of the
 *   bitmap array in bytes should be the cieling of the number of bitmap pixels
 *   divided by 8. This parameter can be left NULL if not needed.
 *
 *  See GFX2D_draw_binary_bitmap.
 *
 ******************************************************************************/

void GFX2D_draw_rgb_bitmap(GFX2D_instance_t* instance,
                           GFX2D_rgb_bitmap_t rgb_bitmap,
                           const uint8_t* bitmap,
                           const uint8_t* alpha_mask,
                           int16_t x0,
                           int16_t y0,
                           uint16_t bitmap_x0,
                           uint16_t bitmap_y0,
                           uint16_t bitmap_width_draw,
                           uint16_t bitmap_height_draw,
                           uint16_t bitmap_width_total,
                           uint16_t bitmap_height_total);

/*******************************************************************************
 *
 * GFX2D_set_text_magnification
 *
 * DESCRIPTION:
 *  Sets the x and y text magnifications.To keep the text in proper proportions,
 *  the x and y values should be the same.
 *
 * PARAMETERS:
 *  x_magnification
 *   Text magnification in the x-direction.
 *
 *  y_magnification
 *   Text magnification in the y-direction.
 *
 ******************************************************************************/

void GFX2D_set_text_magnification(GFX2D_instance_t* instance,
                                  uint8_t x_magnification,
                                  uint8_t y_magnification);

/*******************************************************************************
 *
 * GFX2D_set_font
 *
 * DESCRIPTION:
 *  Sets the font.
 *
 * PARAMETERS:
 *  font
 *   Pointer to font structure.
 *
 ******************************************************************************/

void GFX2D_set_font(GFX2D_instance_t* instance, GFX2DFONT_font_t* font);

/*******************************************************************************
 *
 * GFX2D_set_font_color
 *
 * DESCRIPTION:
 *  Sets the font color.
 *
 * PARAMETERS:
 *  color
 *   See GFX2D_rgba_t. (Uint32_t used for quicker access.)
 *
 ******************************************************************************/

void GFX2D_set_font_color(GFX2D_instance_t* instance, uint32_t color);

/*******************************************************************************
 *
 * GFX2D_set_inverted
 *
 * DESCRIPTION:
 *  Enables or disables inverted rendering.
 *
 * PARAMETERS:
 *  enable_inverted
 *   True to enable inverted rendering, else, false to disable.
 *
 ******************************************************************************/

void GFX2D_set_inverted(GFX2D_instance_t* instance, bool enable_inverted);

/*******************************************************************************
 *
 * GFX2D_set_text_wrap
 *
 * DESCRIPTION:
 *  Enables or disables text wrapping - text will automatically go to the next
 *  line if the end of the row is reached.
 *
 * PARAMETERS:
 *  enable_wrap
 *   True to enable wrapping, else, false to disable.
 *
 ******************************************************************************/

void GFX2D_set_text_wrap(GFX2D_instance_t* instance, bool enable_wrap);

/*******************************************************************************
 *
 * GFX2D_draw_char
 *
 * DESCRIPTION:
 *  Draws a single character from the font module.
 *
 * PARAMETERS:
 *  c
 *   ASCII character to draw. Characters/glyph not supported by the font
 *   module will not be drawn.
 *
 ******************************************************************************/

void GFX2D_draw_char(GFX2D_instance_t* instance, uint8_t c);

#ifdef __cplusplus
}
#endif
#endif // GFX2D_J_H

/*******************************************************************************
 *
 *  Ili9341 display module which utilizes the Gfx2d library and sends pixel
 *  data through SPI DMA. Requires proper initialization and the service
 *  routine to be called repeatedly after a new task request until the task
 *  is completed.
 *
 ******************************************************************************/

#ifndef ILI9XXX_J_H
#define ILI9XXX_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The default timeout for chip select and unselect.
 */

#define ILI9341_CHIP_SELECT_TIMEOUT_uS    100000U

/*******************************************************************************
 *
 * ILI9341_custom_command_t
 *
 * DESCRIPTION:
 *  Custom command values which are used during the device initialization to
 *  indicate certain behaviors. These values are either not found or not used
 *  in the regular function of the device.
 *
 ******************************************************************************/

typedef enum
{
  ILI9341_CUSTOM_COMMAND_END_OF_LIST      = 0x00,
  ILI9341_CUSTOM_COMMAND_DELAY_MS         = 0xFF
}
ILI9341_custom_command_t;

/*******************************************************************************
 *
 * ILI9341_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 * busy
 *  Set when the module is busy with a task and cleared when the task is
 *  completed.
 *
 * dma_busy
 *  Set when the DMA is given data to load into the SPI transmit register.
 *  Cleared in interrupt context when the DMA transfer is completed.
 *
 * reg_write
 *  Set when the current task is a register write and cleared when the task is
 *  a register read.
 *
 * single_segment
 *  Set during initialization if the dispaly buffer is large enough to hold
 *  the entire target display frame. This allows us to do some one-time
 *  calculations during initialization and save time during the display update.
 *
 * task_state
 *  State machine state.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t busy                          : 1;
    uint8_t dma_busy                      : 1;
    uint8_t reg_write                     : 1;
    uint8_t single_segment                : 1;
    uint8_t task_state                    : 4;
  };
}
ILI9341_flags_t;

/*******************************************************************************
 *
 * ILI9341_error_flags_t
 *
 * DESCRIPTION:
 *  Module error flags.
 *
 * timeout
 *  The maximum allowable time between receiving or transmitting the next data
 *  element has passed.
 *
 * other
 *  All other types of errors.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t timeout                       : 1;
    uint8_t other                         : 1;
    uint8_t reserved2                     : 6;
  };
}
ILI9341_error_flags_t;

/*******************************************************************************
 *
 * ILI9341_draw_handler_t
 *
 * DESCRIPTION:
 *  Function template for a user-provided function which is called to render
 *  the display buffer utilizing the GFX2D module.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*ILI9341_draw_handler_t)(void);

/*******************************************************************************
 *
 * ILI9341_pre_task_callback_t
 *
 * DESCRIPTION:
 *  Function template for a user-provided function which is called when a new
 *  task is accepted and initialized but before the task actually begins. This
 *  allows the user to perform any last additional configurations before the
 *  actual logic of the task begins.
 *
 * PARAMETERS:
 *  context
 *   User-provided context.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*ILI9341_pre_task_callback_t)(uint32_t);

/*******************************************************************************
 *
 * ILI9341_post_task_callback_t
 *
 * DESCRIPTION:
 *  Function template for a user-provided function which is called after a
 *  task completes.
 *
 * PARAMETERS:
 *  context
 *   User-provided context.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*ILI9341_post_task_callback_t)(uint32_t);

/*******************************************************************************
 *
 * ILI9341_hal_set_chip_select_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will enable or disable the device chip select. A boolean return is
 *  provided to allow more iteration time to select/unselect the chip. This
 *  function will continue to be called until true is returned.
 *
 * PARAMETERS:
 *  enable
 *   True to select the device, else, false.
 *
 * RETURN:
 *  True if the chip selection was completed, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*ILI9341_hal_set_chip_select_t)(bool);

/*******************************************************************************
 *
 * ILI9341_hal_set_dc_select_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will set the data-command line. (generally data:1, command:0).
 *
 * PARAMETERS:
 *  enable
 *   True to enable data mode, else, false to enable command mode.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 *  I had considered making this a boolean return to allow "slow" setting of
 *  the data/command select, but ultimately decided against it. While the
 *  chip select is only triggered at the start and end of a transaction, the
 *  DC is triggered throughout the transaction - it really should have a GPIO,
 *  or something equivalent, assigned to drive it.
 *
 *
 ******************************************************************************/

typedef void (*ILI9341_hal_set_dc_select_t)(bool);

/*******************************************************************************
 *
 * ILI9341_hal_configure_dma_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will configure and start the DMA transfer for pixel data.
 *
 * PARAMETERS:
 *  src_addr
 *   Starting address of the pixel buffer.
 *
 *  src_length
 *   Length of the source buffer in bytes.
 *
 * RETURN:
 *  True if the configuration was successful and started, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*ILI9341_hal_configure_dma_t)(void*, uint32_t);

/*******************************************************************************
 *
 * ILI9341_hal_disable_dma_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will disable the DMA.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*ILI9341_hal_disable_dma_t)(void);

/*******************************************************************************
 *
 * ILI9341_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module flags.
 *
 * errors
 *  Errors which occurred in the last transaction.
 *
 * gfx2d
 *  User-provided initialized instance of a GFX2D.
 *
 * utimer
 *  User-provided initialized instance of a UTIMER.
 *
 * spi
 *  User-provided initialized instance of a SERSPI.
 *
 * utimer_ticket
 *  Data structure used with the UTIMER instance.
 *
 * bus_mutex
 *  User-provided initialized instance of a BUSMUTEX.
 *
 * bus_id
 *  The BUS ID associated with the BUSMUTEX instance.
 *
 * render_y0
 *  Keeps track of the last rendered y-offset.
 *
 * render_rows
 *  The number of rows starting from render_y0 which are buffered and sent to
 *  the display in a single rendering iteration.
 *
 * render_page_buffer
 *  Holds the register and parameters for setting the starting page/row.
 *
 * display_adjust_y
 *  Adjusts the column and row starting and ending positions. (This is used for
 *  cheap screens which may not be properly aligned within their frames).
 *
 * reg_address
 *  Register address to read/write in register rw task.
 *
 * reg_buffer
 *  Buffer which holds the value to write to the register or will hold the
 *  value read from the register in a register rw task.
 *
 * reg_length
 *  Length of the reg_buffer.
 *
 * chip_select_timeout_us
 *  The time allowed for the chip select to complete before aborting. The value
 *  is initialized to the defined default, but can be directly modified by the
 *  user.
 *
 * dma_bytes_per_transfer
 *  The maximum number of bytes which can be sent in a single DMA transaction.
 *
 * dma_transfer_timeout_us
 *  The timeout allowed for the DMA transfer to complete before giving up.
 *
 * dma_transfer_counter
 *  The number of bytes which have been transferred by the DMA.
 *
 * dma_transfer_count
 *  The total number of bytes which need to be transferred by the DMA.
 *
 * dma_transfer_last_packet_length
 *  The length, in bytes, of the last data packet which will be transmitted by
 *  the DMA. This is calculated early on to avoid repetitive math during the
 *  transaction process.
 *
 * dma_src_buffer_offset
 *  Offset into the source buffer which is being transmitted by DMA.
 *
 * callback_context
 *  Context passed into the user pre/post operation callbacks.
 *
 * service_handler
 *  Function pointer to the service routine applicable to the initialized
 *  service mode.
 *
 * draw_handler
 *  See ILI9341_draw_handler_t
 *
 * *_task_*
 * *_hal_*
 *  User-provided functions. See typedef comments.
 *
 ******************************************************************************/

typedef struct
{
  volatile ILI9341_flags_t flags;
  ILI9341_error_flags_t errors;
  GFX2D_instance_t* gfx2d;
  UTIMER_instance_t* utimer;
  SERSPI_instance_t* spi;
  UTIMER_ticket_t utimer_ticket;
  BUSMUTEX_instance_t* bus_mutex;
  BUSMUTEX_bus_id_t bus_id;
  uint16_t render_y0;
  uint16_t render_rows;
  uint8_t render_page_buffer[4];
  int8_t display_adjust_y;
  uint8_t reg_address;
  void* reg_buffer;
  uint32_t reg_length;
  uint32_t chip_select_timeout_us;
  uint32_t dma_bytes_per_transfer;
  uint32_t dma_transfer_timeout_us;
  uint32_t dma_transfer_counter;
  uint32_t dma_transfer_count;
  uint32_t dma_transfer_last_packet_length;
  uint32_t dma_src_buffer_offset;
  uint32_t callback_context;
  bool (*service_handler)(void* instance);
  ILI9341_draw_handler_t draw_handler;
  ILI9341_pre_task_callback_t pre_task_callback;
  ILI9341_post_task_callback_t post_task_callback;
  ILI9341_hal_set_chip_select_t set_chip_select;
  ILI9341_hal_set_dc_select_t set_dc_select;
  ILI9341_hal_configure_dma_t configure_dma;
  ILI9341_hal_disable_dma_t disable_dma;
}
ILI9341_instance_t;

/*******************************************************************************
 *
 * ILI9341_dma_transfer_complete_handler
 *
 * DESCRIPTION:
 *  Handler for the DMA transfer complete interrupt. The user code must call
 *  this function from their DMA transfer complete ISR.
 *
 * IMPORTANT:
 *  The user MUST ensure that the DMA transmission complete interrupt is
 *  enabled and that this handler is called from that ISR.
 *
 ******************************************************************************/

void ILI9341_dma_transfer_complete_handler(ILI9341_instance_t* instance);

/*******************************************************************************
 *
 * ILI9341_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values.
 *
 * PARAMETERS:
 *  See ILI9341_instance_t.
 *
 ******************************************************************************/

void ILI9341_initialize(ILI9341_instance_t* instance,
                        GFX2D_instance_t* gfx2d,
                        UTIMER_instance_t* utimer,
                        SERSPI_instance_t* spi,
                        BUSMUTEX_instance_t* bus_mutex,
                        BUSMUTEX_bus_id_t bus_id,
                        uint32_t dma_bytes_per_transfer,
                        uint32_t dma_transfer_timeout_us,
                        ILI9341_draw_handler_t draw_handler,
                        ILI9341_pre_task_callback_t pre_task_callback,
                        ILI9341_post_task_callback_t post_task_callback,
                        ILI9341_hal_set_chip_select_t set_chip_select,
                        ILI9341_hal_set_dc_select_t set_dc_select,
                        ILI9341_hal_configure_dma_t configure_dma,
                        ILI9341_hal_disable_dma_t disable_dma);

/*******************************************************************************
 *
 * ILI9341_configure_display
 *
 * DESCRIPTION:
 *  Sends a series of commands to the display to initialize it and turn it on.
 *  The passed in array must follow a special format which indicates the
 *  commands and arguments to send. The format is as follows:
 *
 *    <command>, <arg_count>, <...arguments...>,
 *
 *  If the argument count is 0 then the arguments field is omitted. Additional
 *  custom commands can be found in ILI9341_custom_command_t. The delay command
 *  takes a single argument, which represents the number of milliseconds to
 *  delay, which takes the place of the arg_count field. Eg.
 *
 *    <delay_command>, <ms>,
 *
 *  The user must terminate the list with a 0x00 command.
 *
 *    <0x00>
 *
 *  This command is expected to be called soon after system initialization. To
 *  keep things simple, this function will block until the configuration is
 *  completed.
 *
 * PARAMETERS:
 *  command_list
 *   List of bytes which follow the sequence described above.
 *
 * NOTES:
 *  Included part of the command_list should be commands 0x2A and 0x2B which
 *  set the draw matrix on the display. For this library, the user should
 *  set the matrix to be the entire screen area. For example, a 128 by 160
 *  screen might look like this:
 *    0x2A, 4, 0x00, 0x00, 0x00, 0x7F,
 *    0x2B, 4, 0x00, 0x00, 0x00, 0x9F,
 *
 ******************************************************************************/

void ILI9341_configure_display(ILI9341_instance_t* instance, const uint8_t* command_list);

/*******************************************************************************
 *
 * ILI9341_begin_new_register_write
 *
 * DESCRIPTION:
 *  Attempts to begin a new register write task.
 *
 * PARAMETERS:
 *  reg_address
 *   The register address or access code.
 *
 *  buffer
 *   The buffer which holds the value to be written to the register.
 *
 *  length
 *   The length, in bytes, of the buffer (number of bytes to write).
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool ILI9341_begin_new_register_write(ILI9341_instance_t* instance,
                                      uint8_t reg_address,
                                      void* buffer,
                                      uint32_t length);

/*******************************************************************************
 *
 * ILI9341_begin_new_register_read
 *
 * DESCRIPTION:
 *  Attempts to begin a new register read task.
 *
 * PARAMETERS:
 *  reg_address
 *   The register address or access code.
 *
 *  buffer
 *   The buffer which will hold the value read from the register.
 *
 *  length
 *   The length, in bytes, of the buffer (number of bytes to write).
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool ILI9341_begin_new_register_read(ILI9341_instance_t* instance,
                                     uint8_t reg_address,
                                     void* buffer,
                                     uint32_t length);

/*******************************************************************************
 *
 * ILI9341_begin_new_display_update
 *
 * DESCRIPTION:
 *  Attempts to begin a new display update task.
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool ILI9341_begin_new_display_update(ILI9341_instance_t* instance);

/*******************************************************************************
 *
 * ILI9341_service
 *
 * DESCRIPTION:
 *  Services the task state machine. Must be called repeatedly until the task
 *  is completed.
 *
 * RETURN:
 *  False if there is an ongoing task which has not completed, else, true.
 *
 ******************************************************************************/

bool ILI9341_service(ILI9341_instance_t* instance);

/*******************************************************************************
 *
 * ILI9341_is_busy
 *
 * DESCRIPTION:
 *  Determines if an instance is currently busy with a task.
 *
 * RETURN:
 *  True if the instance is busy with a task, else, false.
 *
 ******************************************************************************/

bool ILI9341_is_busy(ILI9341_instance_t* instance);

#ifdef __cplusplus
}
#endif
#endif // ILI9XXX_J_H

/*******************************************************************************
 *
 *  RGB LED module which supports off, on, pattern, and ramping modes. Requires
 *  proper initialization and the service routine to be called periodically.
 *
 ******************************************************************************/

#ifndef RGB_J_H
#define RGB_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Helper pre-compile function for creating a OFF 64-bit UI code.
 */

#define RGB_MODE_OFF_UI_CODE()            0ULL

/*
 * Helper pre-compile function for creating a ON 64-bit UI code with the use
 * of an palette index -- compare to RGB_led_mode_on_ui_t for the bit field
 * locations.
 */

#define RGB_MODE_ON_PALETTE_UI_CODE(RGB_ARG_PALETTE_INDEX) \
(                                           \
  0ULL                                    | \
  (uint64_t)RGB_ARG_PALETTE_INDEX << 32U  | \
  1ULL << 40U                               \
)

/*
 * Helper pre-compile function for creating a ON 64-bit UI code with the use
 * of an RGB color-- compare to RGB_led_mode_on_ui_t for the bit field
 * locations.
 */

#define RGB_MODE_ON_COLOR_UI_CODE(RGB_ARG_RED, RGB_ARG_GREEN, RGB_ARG_BLUE) \
(                                           \
  0ULL                                    | \
  (uint64_t)RGB_ARG_RED << 0U             | \
  (uint64_t)RGB_ARG_GREEN << 8U           | \
  (uint64_t)RGB_ARG_BLUE << 16U             \
)

/*
 * Helper pre-compile function for creating a PATTERN 64-bit UI code -- compare
 * to RGB_led_mode_pattern_ui_t for the bit field locations.
 */

#define RGB_MODE_PATTERN_UI_CODE(RGB_ARG_PATTERN, RGB_ARG_PHASES,     \
                                 RGB_ARG_DIVISOR, RGB_ARG_ITERATIONS, \
                                 RGB_ARG_FORCE_SYNC)                  \
(                                           \
  0ULL                                    | \
  (uint64_t)RGB_ARG_PHASES << 0U          | \
  (uint64_t)RGB_ARG_DIVISOR << 4U         | \
  (uint64_t)RGB_ARG_FORCE_SYNC << 7U      | \
  (uint64_t)RGB_ARG_ITERATIONS << 8U      | \
  (uint64_t)RGB_ARG_PATTERN << 16U          \
)

/*
 * Helper pre-compile function for creating a RAMP 64-bit UI code -- compare to
 * RGB_led_mode_ramp_ui_t for the bit field locations.
 */

#define RGB_MODE_RAMP_UI_CODE(RGB_ARG_PALETTE_START, RGB_ARG_PALETTE_END,  \
                              RGB_ARG_ITERATIONS, RGB_ARG_START_DELAY,     \
                              RGB_ARG_UP, RGB_ARG_UP_HOLD,                 \
                              RGB_ARG_DOWN, RGB_ARG_DOWN_HOLD)             \
(                                           \
  0ULL                                    | \
  (uint64_t)RGB_ARG_PALETTE_START << 0U   | \
  (uint64_t)RGB_ARG_PALETTE_END << 4U     | \
  (uint64_t)RGB_ARG_ITERATIONS << 8U      | \
  (uint64_t)RGB_ARG_START_DELAY << 16U    | \
  (uint64_t)RGB_ARG_UP << 24U             | \
  (uint64_t)RGB_ARG_UP_HOLD << 32U        | \
  (uint64_t)RGB_ARG_DOWN << 40U           | \
  (uint64_t)RGB_ARG_DOWN_HOLD << 48U        \
)

/*
 * Instances of the RGB module are meant to be serviced periodically at a set
 * interval - generally serviced by the system tick interrupt. The default
 * value defined here is ideal for a 1-ms periodic tick as it divides the
 * pattern phases somewhat evenly across 1-second. (1000 / 12) = 83.333 The
 * user can adjust the variable value to speed up or slow down the pattern
 * and ramp animations.
 */

#define RGB_TICKS_PER_QUANTA_DEFAULT      83U

/*
 * The individual patterns of each LED are kept somewhat synchronous with each
 * other in an single instance by a instance-level step-counter. There are 12-
 * steps in a single instance period - each step is measured as a single
 * quanta which is defined by a tick count which is incremented each time the
 * instance service routine is called.
 */

#define RGB_PATTERN_STEPS_PER_PERIOD      12U

/*
 * The maximum pattern length, or phases, that a single LED can have. While
 * this value is the same as RGB_PATTERN_STEPS_PER_PERIOD, it is not treated
 * the same. Depending on the pattern divisor, a single iteration of the
 * pattern may or may not last longer than the instance period.
 */

#define RGB_PATTERN_MAX_LENGTH            12U

/*
 * In PATTERN and RAMP modes, an iteration value of 0 means that the animation
 * should run forever.
 */

#define RGB_INFINITE_ITERATIONS           0U

/*
 * Number of bits which compose the semaphore flag and associated maximum value
 * of the semaphore.
 */

#define RGB_SEMAPHORE_BIT_LENGTH          8U
#define RGB_SEMAPHORE_MAX_VALUE           ((1 << RGB_SEMAPHORE_BIT_LENGTH) - 1)

/*******************************************************************************
 *
 * RGB_led_color_t
 *
 * DESCRIPTION:
 *  A 24-bit color is composed of a red, green, and blue intensity. Intensities
 *  will range from 0 to 255 (8-bits each).
 *
 * red
 *  The red intensity.
 *
 * green
 *  The green intensity.
 *
 * blue
 *  The blue intensity.
 *
 ******************************************************************************/

typedef union
{
  uint32_t all;
  struct
  {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t reserved;
  };
}
RGB_led_color_t;

/*******************************************************************************
 *
 * RGB_led_color_hr_t
 *
 * DESCRIPTION:
 *  In ramping mode, the interpolation between colors can be very subtle such
 *  that several quanta is needed to increment to the next ramp value in the
 *  range of 0 to 255. To simplify the math and keep track of the progress of
 *  the interpolation, the color intensities can be stored as 24-bit values.
 *  (We do not want to use the full 32-bits to avoid overflow during the math
 *  operations for interpolation -- besides, 24-bit resolution is high enough
 *  and may require less work than 31-bit.)
 *
 * red
 *  The red intensity in high-resolution 24-bit format.
 *
 * green
 *  The green intensity in high-resolution 24-bit format.
 *
 * blue
 *  The blue intensity in high-resolution 24-bit format.
 *
 ******************************************************************************/

typedef struct
{
  int32_t red;
  int32_t green;
  int32_t blue;
}
RGB_led_color_hr_t;

/*******************************************************************************
 *
 * RGB_led_mode_id_t
 *
 * DESCRIPTION:
 *  An LED can be in one of 4-modes: OFF, ON, PATTERN, and RAMP.
 *
 ******************************************************************************/

typedef enum
{
  RGB_LED_MODE_ID_OFF                     = 0,
  RGB_LED_MODE_ID_ON,
  RGB_LED_MODE_ID_PATTERN,
  RGB_LED_MODE_ID_RAMP
}
RGB_led_mode_id_t;

/*******************************************************************************
 *
 * RGB_pattern_divisor_t
 *
 * DESCRIPTION:
 *  A pattern iteration can be slowed down by setting the divisor. The divisor
 *  must be able to evenly divide the number of steps in a pattern period.
 *  Hence, the allowed divisors are /1, /2, /3, /4, /6, /12. A truth table
 *  will be used on each pattern handler phase to check if the next phase of
 *  the LED pattern should become active based upon the instance pattern step.
 *
 ******************************************************************************/

typedef enum
{
  RGB_PATTERN_DIVISOR_1                   = 0,
  RGB_PATTERN_DIVISOR_2,
  RGB_PATTERN_DIVISOR_3,
  RGB_PATTERN_DIVISOR_4,
  RGB_PATTERN_DIVISOR_6,
  RGB_PATTERN_DIVISOR_12,
  RGB_PATTERN_DIVISOR_COUNT
}
RGB_pattern_divisor_t;

/*******************************************************************************
 *
 * RGB_led_palette_slot_t
 *
 * DESCRIPTION:
 *  Referring to an painter palette which provides quick access to working
 *  colors, each LED will have its own palette of customizable colors. The
 *  palette slot count is purposefully capped at 16 so that it a single palette
 *  index can be packed into 4-bits. To save memory space, the first several
 *  slots are shared amongst all the LEDs in the instance. The last few are
 *  unique for each LED.
 *
 * NOTE:
 *  The number of shared slots may be reduced by moving the position of
 *  RGB_LED_PALETTE_SHARED_COUNT. Checks are already in place to properly
 *  handle the palette index based upon shared versus unique palette.
 *
 ******************************************************************************/

typedef enum
{
  RGB_LED_PALETTE_SLOT_0                  = 0,
  RGB_LED_PALETTE_SLOT_1                  = 1,
  RGB_LED_PALETTE_SLOT_2                  = 2,
  RGB_LED_PALETTE_SLOT_3                  = 3,
  RGB_LED_PALETTE_SLOT_4                  = 4,
  RGB_LED_PALETTE_SLOT_5                  = 5,
  RGB_LED_PALETTE_SLOT_6                  = 6,
  RGB_LED_PALETTE_SLOT_7                  = 7,
  RGB_LED_PALETTE_SLOT_8                  = 8,
  RGB_LED_PALETTE_SLOT_9                  = 9,
  RGB_LED_PALETTE_SLOT_A                  = 10,
  RGB_LED_PALETTE_SLOT_B                  = 11,
  RGB_LED_PALETTE_SHARED_COUNT,
  RGB_LED_PALETTE_SLOT_C                  = 12,
  RGB_LED_PALETTE_SLOT_D                  = 13,
  RGB_LED_PALETTE_SLOT_E                  = 14,
  RGB_LED_PALETTE_SLOT_F                  = 15,
  RGB_LED_PALETTE_SLOT_COUNT,
  RGB_LED_PALETTE_UNIQUE_COUNT            = (RGB_LED_PALETTE_SLOT_COUNT -
                                             RGB_LED_PALETTE_SHARED_COUNT)
}
RGB_led_palette_slot_t;

/*******************************************************************************
 *
 * RGB_shared_palette_t
 *
 * DESCRIPTION:
 *  The palette which is shared with all the LEDs in a single instance.
 *
 ******************************************************************************/

typedef struct
{
  RGB_led_color_t slots[RGB_LED_PALETTE_SHARED_COUNT];
}
RGB_shared_palette_t;

/*******************************************************************************
 *
 * RGB_led_palette_t
 *
 * DESCRIPTION:
 *  The palette which is unique to each LED in the instance.
 *
 ******************************************************************************/

typedef struct
{
  RGB_led_color_t slots[RGB_LED_PALETTE_UNIQUE_COUNT];
}
RGB_led_palette_t;

/*******************************************************************************
 *
 * RGB_led_mode_on_ui_t
 *
 * DESCRIPTION:
 *  User-interface values for the ON mode.
 *
 * red
 *  Red color intensity.
 *
 * green
 *  Green color intensity.
 *
 * blue
 *  Blue color intensity.
 *
 * palette
 *  RGB output color as described by a palette color slot.
 *
 * use_palette
 *  Flag indicating if the palette value should be used (true), or if the
 *  32-bit RGB color should be used.
 *
 * NOTES:
 *  The red, green, and blue values were taken out of the rgb_led_color
 *  structure context to guard against Endianess in bit-shifting.
 *
 ******************************************************************************/

typedef union
{
  uint64_t all;
  struct
  {
    uint64_t red                          : 8;
    uint64_t green                        : 8;
    uint64_t blue                         : 8;
    uint64_t reserved24                   : 8;
    uint64_t palette                      : 4;
    uint64_t reserved36                   : 4;
    uint64_t use_palette                  : 1;
    uint64_t reserved41                   : 7;
    uint64_t reserved48                   : 16;
  };
}
RGB_led_mode_on_ui_t;

/*******************************************************************************
 *
 * RGB_led_mode_on_t
 *
 * DESCRIPTION:
 *  Internal working variables for the ON mode including the UI subset.
 *
 ******************************************************************************/

typedef union
{
  RGB_led_mode_on_ui_t ui;
}
RGB_led_mode_on_t;

/*******************************************************************************
 *
 * RGB_led_mode_pattern_ui_t
 *
 * DESCRIPTION:
 *  User-interface values for the PATTERN mode.
 *
 * phases
 *  Length of, or number of phases (4-bit palette colors), in the pattern value.
 *
 * divisor
 *  Divides the pattern phase rate by RGB_pattern_divisor_t.
 *
 * force_sync
 *  Forces the instance pattern step to reset to step-0, thus causing any new
 *  pattern(s) to start immediately. Otherwise, new pattern(s) will not begin
 *  until the instance pattern step reaches step-0.
 *
 * iterations
 *  Number of times to repeat the pattern. Value 0 indicates repeat forever.
 *
 * pattern
 *  Up to 12-phases defined by palette slot colors at 4-bits each. For example,
 *  a pattern that traverses from palette slots 0 -> 1 -> 2 -> 3 would be
 *  0x3210.
 *
 ******************************************************************************/

typedef union
{
  uint64_t all;
  struct
  {
    uint64_t phases                       : 4;
    uint64_t divisor                      : 3;
    uint64_t force_sync                   : 1;
    uint64_t iterations                   : 8;
    uint64_t pattern                      : 48;
  };
}
RGB_led_mode_pattern_ui_t;

/*******************************************************************************
 *
 * RGB_led_mode_pattern_t
 *
 * DESCRIPTION:
 *  Internal working variables for the PATTERN mode including the UI subset.
 *
 * working_pattern
 *  Pattern which originally gets set to the pattern value and then is shifted
 *  by 4-bits at each new phase in the LED pattern.
 *
 * phase_counter
 *  Keeps track of the number of pattern phases of the LED pattern which have
 *  passed.
 *
 * iteration_counter
 *  Keeps track of the number of times the pattern has repeated.
 *
 ******************************************************************************/

typedef struct
{
  RGB_led_mode_pattern_ui_t ui;
  uint64_t working_pattern;
  uint8_t phase_counter;
  uint8_t iteration_counter;
}
RGB_led_mode_pattern_t;

/*******************************************************************************
 *
 * RGB_led_mode_ramp_ui_t
 *
 * DESCRIPTION:
 *  User-interface values for the RAMP mode.
 *
 * palette_start
 *  Starting color for the ramp-up and hold value for the ramp_down_hold.
 *
 * palette_end
 *  Starting color for the ramp-down and hold value for the ramp_up_hold.
 *
 * iterations
 *  Number of times to repeat the ramp. Value 0 indicates repeat forever.
 *
 * start_delay_quanta
 *  Quanta to wait before starting the ramp-up. This delay is only done once on
 *  the first iteration of the ramp.
 *
 * ramp_up_quanta
 *  Quanta to ramp-up from the start to end colors.
 *
 * ramp_up_hold_quanta
 *  Quanta to hold the palette_end color after the ramp-up.
 *
 * ramp_down_quanta
 *  Quanta to ramp-down from the end to start colors.
 *
 * ramp_down_hold_quanta
 *  Quanta to hold the palette_start color after the ramp-down.
 *
 ******************************************************************************/

typedef union
{
  uint64_t all;
  struct
  {
    uint64_t palette_start                : 4;
    uint64_t palette_end                  : 4;
    uint64_t iterations                   : 8;
    uint64_t start_delay_quanta           : 8;
    uint64_t ramp_up_quanta               : 8;
    uint64_t ramp_up_hold_quanta          : 8;
    uint64_t ramp_down_quanta             : 8;
    uint64_t ramp_down_hold_quanta        : 8;
    uint64_t reserved56                   : 8;
  };
}
RGB_led_mode_ramp_ui_t;

/*******************************************************************************
 *
 * RGB_led_mode_ramp_t
 *
 * DESCRIPTION:
 *  Internal working variables for the RAMP mode including the UI subset.
 *
 * hres_start
 *  High resolution equivalent of the start color.
 *
 * hres_end
 *  High resolution equivalent of the end color.
 *
 * hres_color
 *  High resolution equivalent of the current output color.
 *
 * hres_step_up
 *  Interpolation step between the start and end colors during each ramp-up
 *  tick.
 *
 * hres_step_down
 *  Interpolation step between the end and start colors during each ramp-down
 *  tick.
 *
 * iteration_counter
 *  Keeps track of the number of times the ramp has repeated.
 *
 * tick_counter
 *  Keeps track of the number of times the ramp service has been called.
 *
 * quanta_counter
 *  Keeps track of the number of quanta (N ticks in a quanta) which have
 *  passed. This value is compared to the various ramp quanta timing values
 *  and is reset to 0 at the start of each new ramping state.
 *
 ******************************************************************************/

typedef struct
{
  RGB_led_mode_ramp_ui_t ui;
  RGB_led_color_hr_t hres_start;
  RGB_led_color_hr_t hres_end;
  RGB_led_color_hr_t hres_color;
  RGB_led_color_hr_t hres_step_up;
  RGB_led_color_hr_t hres_step_down;
  uint8_t iteration_counter;
  uint8_t tick_counter;
  uint8_t quanta_counter;
}
RGB_led_mode_ramp_t;

/*******************************************************************************
 *
 * RGB_led_mode_t
 *
 * DESCRIPTION:
 *  Union of the LED modes to save memory space - each LED can only be in
 *  one mode at a time.
 *
 ******************************************************************************/

typedef union
{
  RGB_led_mode_on_t on;
  RGB_led_mode_pattern_t pattern;
  RGB_led_mode_ramp_t ramp;
}
RGB_led_mode_t;

/*******************************************************************************
 *
 * RGB_led_mode_flags_t
 *
 * DESCRIPTION:
 *  Additional flags related to the LED mode.
 *
 * mode
 *  Current mode of the LED, RGB_led_mode_id_t.
 *
 * busy
 *  Set true when the LED is performing a finite animation and automatically set
 *  false when that animation finishes. The OFF, ON, and infinite patterns or
 *  ramps will keep this flag false.
 *
 * repeat_forever
 *  Indicates that the pattern or ramp should repeat forever.
 *
 * new_pattern
 *  Indicates that the LED in pattern mode has a new pattern and should wait
 *  for the first global pattern phase to activate.
 *
 * ramp_state
 *  State of the ramp state machine, RGB_ramp_state_t.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t mode                          : 3;
    uint8_t busy                          : 1;
    uint8_t repeat_forever                : 1;
    uint8_t new_pattern                   : 1;
    uint8_t ramp_state                    : 3;
  };
}
RGB_led_mode_flags_t;

/*******************************************************************************
 *
 * RGB_led_t
 *
 * DESCRIPTION:
 *  RGB LED settings and data variable collection.
 *
 * flags
 *  Mode flags.
 *
 * data
 *  Mode data variables.
 *
 * palette
 *  Unique customizable palette for the LED.
 *
 * output
 *  The final generated RGB output of the LED.
 *
 ******************************************************************************/

typedef struct
{
  RGB_led_mode_flags_t flags;
  RGB_led_mode_t data;
  RGB_led_palette_t palette;
  RGB_led_color_t output;
}
RGB_led_t;

/*******************************************************************************
 *
 * RGB_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 * halt_semaphore
 *  Provides a way to halt the service of pattern and ramp LEDs. This allows
 *  the user to prepare all the LEDs to their liking and then un-halt the
 *  instance and allow the LEDs to begin their animations at the same time.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t halt_semaphore                : RGB_SEMAPHORE_BIT_LENGTH;
  };
}
RGB_flags_t;

/*******************************************************************************
 *
 * RGB_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module flags.
 *
 * ticks_per_quanta
 *  A tick is defined as an entry into the service routine, which routine is
 *  meant to be called periodically. After every N-ticks, the quanta counter
 *  is incremented. When initialized, the value will be set to the default
 *  RGB_TICKS_PER_QUANTA_DEFAULT.
 *
 * pattern_step_counter
 *  Keeps track of the instance pattern step. Incremented at the start of a
 *  new quanta and reset to 0 after the 12th quanta.
 *
 * pattern_tick_counter
 *  Keeps track of the instance pattern tick counter. Every N-ticks will
 *  increment the pattern_step_counter.
 *
 * palette
 *  The shared color palette for all LEDs in the instance.
 *
 * led_count
 *  Number of RGB LEDs in the instance led_list.
 *
 * led_list
 *  List of RGB LEDs.
 *
 ******************************************************************************/

typedef struct
{
  RGB_flags_t flags;
  uint8_t ticks_per_quanta;
  uint8_t pattern_step_counter;
  uint8_t pattern_tick_counter;
  RGB_shared_palette_t palette;
  uint32_t led_count;
  RGB_led_t* led_list;
}
RGB_instance_t;

/*******************************************************************************
 *
 * RGB_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values.
 *
 * PARAMETERS:
 *  See RGB_instance_t.
 *
 ******************************************************************************/

void RGB_initialize(RGB_instance_t* instance,
                    RGB_led_t* led_list,
                    uint32_t led_count);

/*******************************************************************************
 *
 * RGB_set_ticks_per_quanta
 *
 * DESCRIPTION:
 *  Sets the instance ticks per quanta value. Decreasing the tick value will
 *  speed up the pattern and ramp animations, and increasing the tick value
 *  will slow the animations down.
 *
 * PARAMETERS:
 *  ticks
 *   Value ranging from 0-255. A value of 0 will cause the default value to
 *   be written.
 *
 ******************************************************************************/

void RGB_set_ticks_per_quanta(RGB_instance_t* instance, uint8_t ticks);

/*******************************************************************************
 *
 * RGB_halt_semaphore_increment
 *
 * DESCRIPTION:
 *  Increases the halt semaphore count by 1. The halt semaphore enables/
 *  disables the instance from servicing. This allows for a momentary hold
 *  while LEDs are synchronized so that the animations of all LEDs in the
 *  instance can all begin together. LED servicing will only run when the
 *  semaphore count is 0.
 *
 ******************************************************************************/

void RGB_halt_semaphore_increment(RGB_instance_t* instance);

/*******************************************************************************
 *
 * RGB_halt_semaphore_decrement
 *
 * DESCRIPTION:
 *  Decreases the halt semaphore count by 1. The halt semaphore enables/
 *  disables the instance from servicing. This allows for a momentary hold
 *  while LEDs are synchronized so that the animations of all LEDs in the
 *  instance can all begin together. LED servicing will only run when the
 *  semaphore count is 0.
 *
 ******************************************************************************/

void RGB_halt_semaphore_decrement(RGB_instance_t* instance);

/*******************************************************************************
 *
 * RGB_set_palette_slot_color
 *
 * DESCRIPTION:
 *  Sets the RGB color of a palette slot.
 *
 * PARAMETERS:
 *  led_index
 *   Index of the LED palette. This is not applicable and can be set as 0 if
 *   the palette index is within the shared instance palette.
 *
 *  palette_index
 *   Index of the palette slot. The index is analyzed to determine if the
 *   palette slot is within the shared or unique LED palette.
 *
 *  red, green, blue
 *   Color intensities.
 *
 ******************************************************************************/

void RGB_set_palette_slot_color(RGB_instance_t* instance,
                                uint32_t led_index,
                                uint8_t palette_index,
                                uint8_t red,
                                uint8_t green,
                                uint8_t blue);

/*******************************************************************************
 *
 * RGB_get_palette_slot_color
 *
 * DESCRIPTION:
 *  Gets the RGB color of a palette slot.
 *
 * PARAMETERS:
 *  led_index
 *   Index of the LED palette. This is not applicable and can be set as 0 if
 *   the palette index is within the shared instance palette.
 *
 *  palette_index
 *   Index of the palette slot. The index is analyzed to determine if the
 *   palette slot is within the shared or unique LED palette.
 *
 * RETURN:
 *  The RGB color of the palette slot.
 *
 ******************************************************************************/

RGB_led_color_t RGB_get_palette_slot_color(RGB_instance_t* instance,
                                            uint32_t led_index,
                                            uint8_t palette_index);

/*******************************************************************************
 *
 * RGB_set_mode_off
 *
 * DESCRIPTION:
 *  Sets the RGB LED into off mode, setting the output color to all 0's.
 *
 * PARAMETERS:
 *  led_index
 *   Index of the LED in the instance led_list.
 *
 ******************************************************************************/

void RGB_set_mode_off(RGB_instance_t* instance, uint32_t led_index);

/*******************************************************************************
 *
 * RGB_set_all_off
 *
 * DESCRIPTION:
 *  Sets all RGB LEDs into off mode, setting the output color to all 0's.
 *
 ******************************************************************************/

void RGB_set_all_off(RGB_instance_t* instance);

/*******************************************************************************
 *
 * RGB_set_mode_on
 *
 * DESCRIPTION:
 *  Sets the RGB LED into on mode.
 *
 * PARAMETERS:
 *  led_index
 *   Index of the LED in the instance led_list.
 *
 *  ui
 *   User-interface data subset.
 *
 ******************************************************************************/

void RGB_set_mode_on(RGB_instance_t* instance,
                     uint32_t led_index,
                     RGB_led_mode_on_ui_t* ui);

/*******************************************************************************
 *
 * RGB_set_mode_on_color
 *
 * DESCRIPTION:
 *  Sets the RGB LED into on mode based on RGB color.
 *
 * PARAMETERS:
 *  led_index
 *   Index of the LED in the instance led_list.
 *
 *  red, green, blue
 *   Color intensities.
 *
 ******************************************************************************/

void RGB_set_mode_on_color(RGB_instance_t* instance,
                           uint32_t led_index,
                           uint8_t red,
                           uint8_t green,
                           uint8_t blue);

/*******************************************************************************
 *
 * RGB_set_mode_on_palette
 *
 * DESCRIPTION:
 *  Sets the RGB LED into on mode based on palette color.
 *
 * PARAMETERS:
 *  led_index
 *   Index of the LED in the instance led_list.
 *
 *  palette_index
 *   Palette slot color.
 *
 ******************************************************************************/

void RGB_set_mode_on_palette(RGB_instance_t* instance,
                             uint32_t led_index,
                             uint8_t palette_index);

/*******************************************************************************
 *
 * RGB_set_mode_pattern
 *
 * DESCRIPTION:
 *  Sets the RGB LED into pattern mode.
 *
 * PARAMETERS:
 *  led_index
 *   Index of the LED in the instance led_list.
 *
 *  ui
 *   User-interface data subset.
 *
 ******************************************************************************/

void RGB_set_mode_pattern(RGB_instance_t* instance,
                          uint32_t led_index,
                          RGB_led_mode_pattern_ui_t* ui);

/*******************************************************************************
 *
 * RGB_set_mode_pattern_palette
 *
 * DESCRIPTION:
 *  Sets the RGB LED into pattern mode.
 *
 * PARAMETERS:
 *  led_index
 *   Index of the LED in the instance led_list.
 *
 *  iterations
 *   Number of times to repeat. Value 0 indicates repeat forever.
 *
 *  divisor
 *   Divides the pattern phase rate by RGB_pattern_divisor_t.
 *
 *  pattern_phases
 *   Length of, or number of phases (4-bit palette colors), in the pattern
 *   value.
 *
 *  palette_pattern
 *   Up to 12-phases defined by palette slot colors at 4-bits each. For example,
 *   a pattern that traverses from palette slots 0 -> 1 -> 2 -> 3 would be
 *   0x3210.
 *
 *  force_sync
 *   Forces the instance pattern step to reset to step-0, thus causing any new
 *   pattern(s) to start immediately. Otherwise, new pattern(s) will not begin
 *   until the instance pattern step reaches step-0.
 *
 ******************************************************************************/

void RGB_set_mode_pattern_palette(RGB_instance_t* instance,
                                  uint32_t led_index,
                                  uint32_t iterations,
                                  RGB_pattern_divisor_t divisor,
                                  uint8_t pattern_phases,
                                  uint64_t palette_pattern,
                                  bool force_sync);

/*******************************************************************************
 *
 * RGB_set_mode_ramp
 *
 * DESCRIPTION:
 *  Sets the RGB LED into ramping mode.
 *
 * PARAMETERS:
 *  led_index
 *   Index of the LED in the instance led_list.
 *
 *  ui
 *   User-interface data subset.
 *
 ******************************************************************************/

void RGB_set_mode_ramp(RGB_instance_t* instance,
                       uint32_t led_index,
                       RGB_led_mode_ramp_ui_t* ui);

/*******************************************************************************
 *
 * RGB_set_mode_ramp_color
 *
 * DESCRIPTION:
 *  Sets the RGB LED into ramping mode. The ramp is indicated by two RGB colors.
 *
 * PARAMETERS:
 *  led_index
 *   Index of the LED in the instance led_list.
 *
 *  iterations
 *   Number of times to repeat. Value 0 indicates repeat forever.
 *
 *  start_delay_quanta
 *   Quanta to wait before starting the ramp-up. This delay is only done once on
 *   the first iteration of the ramp.
 *
 *  ramp_up_quanta
 *   Quanta to ramp-up from the start to end colors.
 *
 *  ramp_up_hold_quanta
 *   Quanta to hold the palette_end color after the ramp-up.
 *
 *  ramp_down_quanta
 *   Quanta to ramp-down from the end to start colors.
 *
 *  ramp_down_hold_quanta
 *   Quanta to hold the palette_start color after the ramp-down.
 *
 *  *_start
 *   Ramp start color.
 *
 *  *_end
 *   Ramp end color.
 *
 * NOTES:
 *  The internal ramp data structure works with palette indices, not RGB color
 *  values. Hence, when this method is used it will automatically overwrite the
 *  last two palette slots with the start and end RGB colors specified.
 *
 ******************************************************************************/

void RGB_set_mode_ramp_color(RGB_instance_t* instance,
                             uint32_t led_index,
                             uint32_t iterations,
                             uint8_t start_delay_quanta,
                             uint8_t ramp_up_quanta,
                             uint8_t ramp_up_hold_quanta,
                             uint8_t ramp_down_quanta,
                             uint8_t ramp_down_hold_quanta,
                             uint8_t red_start,
                             uint8_t green_start,
                             uint8_t blue_start,
                             uint8_t red_end,
                             uint8_t green_end,
                             uint8_t blue_end);

/*******************************************************************************
 *
 * RGB_set_mode_ramp_palette
 *
 * DESCRIPTION:
 *  Sets the RGB LED into ramping mode. The ramp is indicated by palette slots.
 *
 * PARAMETERS:
 *  led_index
 *   Index of the LED in the instance led_list.
 *
 *  iterations
 *   Number of times to repeat. Value 0 indicates repeat forever.
 *
 *  start_delay_quanta
 *   Quanta to wait before starting the ramp-up. This delay is only done once on
 *   the first iteration of the ramp.
 *
 *  ramp_up_quanta
 *   Quanta to ramp-up from the start to end colors.
 *
 *  ramp_up_hold_quanta
 *   Quanta to hold the palette_end color after the ramp-up.
 *
 *  ramp_down_quanta
 *   Quanta to ramp-down from the end to start colors.
 *
 *  ramp_down_hold_quanta
 *   Quanta to hold the palette_start color after the ramp-down.
 *
 *  palette_index_start
 *   Palette index of the start color.
 *
 *  palette_index_end
 *   Palette index of the end color
 *
 ******************************************************************************/

void RGB_set_mode_ramp_palette(RGB_instance_t* instance,
                               uint32_t led_index,
                               uint32_t iterations,
                               uint8_t start_delay_quanta,
                               uint8_t ramp_up_quanta,
                               uint8_t ramp_up_hold_quanta,
                               uint8_t ramp_down_quanta,
                               uint8_t ramp_down_hold_quanta,
                               uint8_t palette_index_start,
                               uint8_t palette_index_end);

/*******************************************************************************
 *
 * RGB_service
 *
 * DESCRIPTION:
 *  Services the RGB pattern and ramp animations. Intended to be called
 *  periodically at a 1-ms interval. Every time this function is called the
 *  internal tick counter is incremented - which is the basis for advancing
 *  the ramp and pattern animations.
 *
 ******************************************************************************/

void RGB_service(RGB_instance_t* instance);

/*******************************************************************************
 *
 * RGB_service_pattern
 *
 * DESCRIPTION:
 *  Called from the service routine. Handles pattern-specific animations.
 *
 * PARAMETERS:
 *  led_index
 *   Index of the LED in the instance led_list.
 *
 * NOTES:
 *  Helper function primarily called from RGB_service. Exposed here for
 *  unit testing.
 *
 ******************************************************************************/

void RGB_service_pattern(RGB_instance_t* instance, uint32_t led_index);

/*******************************************************************************
 *
 * RGB_service_ramp
 *
 * DESCRIPTION:
 *  Called from the service routine.Handles ramp-specific animations.
 *
 * PARAMETERS:
 *  led_index
 *   Index of the LED in the instance led_list.
 *
 * NOTES:
 *  Helper function primarily called from RGB_service. Exposed here for
 *  unit testing.
 *
 ******************************************************************************/

void RGB_service_ramp(RGB_instance_t* instance, uint32_t led_index);

/*******************************************************************************
 *
 * RGB_pattern_force_sync
 *
 * DESCRIPTION:
 *  Forces the pattern phase step to restart at phase-0 on the next service
 *  tick, causing new patterns to start immediately.
 *
 * NOTES:
 *  Helper function primarily called from RGB_set_mode_pattern. Exposed here
 *  for unit testing.
 *
 ******************************************************************************/

void RGB_pattern_force_sync(RGB_instance_t* instance);

/*******************************************************************************
 *
 * RGB_pattern_tick_increment
 *
 * DESCRIPTION:
 *  Called from the service routine. Increments the pattern tick counter which,
 *  in-turn, may increment the phase counter.
 *
 * RETURN:
 *  True if a new pattern phase has been reached and patterns need to be
 *  serviced, else, false.
 *
 * NOTES:
 *  Helper function primarily called from RGB_service. Exposed here for
 *  unit testing.
 *
 ******************************************************************************/

bool RGB_pattern_tick_increment(RGB_instance_t* instance);

/*******************************************************************************
 *
 * RGB_ramp_update_output
 *
 * DESCRIPTION:
 *  Called from the ramp service routine. Translates the high-resolution ramp
 *  working colors to the final 8-bit output colors.
 *
 * PARAMETERS:
 *  led_index
 *   Index of the LED in the instance led_list.
 *
 * NOTES:
 *  Helper function primarily called from RGB_service_ramp. Exposed here for
 *  unit testing.
 *
 ******************************************************************************/

void RGB_ramp_update_output(RGB_instance_t* instance, uint32_t led_index);

#ifdef __cplusplus
}
#endif
#endif // RGB_J_H

/*******************************************************************************
 *
 *  Configures and drives animations for RGB LEDs. Requires proper
 *  initialization and the service routine to be called periodically.
 *
 ******************************************************************************/

#ifndef LED_ANIMATION_J_H
#define LED_ANIMATION_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *
 * LEDANIMATION_entry_flags_t
 *
 * DESCRIPTION:
 *  Animation flags for a single LED.
 *
 * active
 *  Set if the animation is running or is in queue to run.
 *
 * led_mode
 *  RGB LED mode (OFF, ON, PATTERN, RAMP).
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t active                        : 1;
    uint8_t led_mode                      : 2;
    uint8_t reserved3                     : 5;
  };
}
LEDANIMATION_entry_flags_t;

/*******************************************************************************
 *
 * LEDANIMATION_entry_t
 *
 * DESCRIPTION:
 *  A single animation entry for a single LED. The order of entries on the
 *  table indicate the animation priority. Other animation information is
 *  stored in this structure.
 *
 * animation_id
 *  The user should create a list of animation ID's. Then, several LEDs which
 *  need to work together to create an animation should use the same animation
 *  ID which is assigned to each different animation by the user.
 *
 * flags
 *  LED animation flags.
 *
 * rgb_ui_code
 *  The RGB UI code which is used by the RGB module to put an LED into OFF, ON,
 *  PATTERN, or RAMP modes.
 *
 ******************************************************************************/

typedef struct
{
  uint8_t animation_id;
  LEDANIMATION_entry_flags_t flags;
  uint64_t rgb_ui_code;
}
LEDANIMATION_entry_t;

/*******************************************************************************
 *
 * RGB_animation_table_t
 *
 * DESCRIPTION:
 *  Collection of LEDANIMATION_entry_t for a single LED which makes up an
 *  animation table with the higher priority animations listed first on the
 *  table.
 *
 * table
 *  Pointer to an array of user-defined LEDANIMATION_entry_t which have already
 *  been initialized with the animation IDs, LED mode, and RGB UI code.
 *
 * table_length
 *  Number of animations in the table. May have up to 255-entries ranging from
 *  0 to 254.
 *
 * active_index
 *  Index into the table of the currently highest priority animation which is
 *  active.
 *
 ******************************************************************************/

typedef struct
{
  LEDANIMATION_entry_t* table;
  uint8_t table_length;
  uint8_t active_index;
}
LEDANIMATION_led_table_t;

/*******************************************************************************
 *
 * LEDANIMATION_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t reserved0                     : 8;
  };
}
LEDANIMATION_flags_t;

/*******************************************************************************
 *
 * LEDANIMATION_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module flags.
 *
 * list
 *  List of LED animation tables. Each table index corresponds directly to
 *  the same index of an LED in the RGB instance. The animation list may be
 *  smaller than or equal to the RGB list, but it cannot be greater.
 *
 * list_length
 *  Number of tables in the list.
 *
 * rgb
 *  User-provided RGB instance which has already been initialized.
 *
 ******************************************************************************/

typedef struct
{
  LEDANIMATION_flags_t flags;
  LEDANIMATION_led_table_t* list;
  uint32_t list_length;
  RGB_instance_t* rgb;
}
LEDANIMATION_instance_t;

/*******************************************************************************
 *
 * LEDANIMATION_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values.
 *
 * PARAMETERS:
 *  See LEDANIMATION_instance_t.
 *
 ******************************************************************************/

void LEDANIMATION_initialize(LEDANIMATION_instance_t* instance,
                             LEDANIMATION_led_table_t* list,
                             uint32_t list_length,
                             RGB_instance_t* rgb);

/*******************************************************************************
 *
 * LEDANIMATION_service
 *
 * DESCRIPTION:
 *  Meant to be called periodically on a basis of around 100-milliseconds.
 *  Services the module instance - checks the status of the highest priority
 *  animation for each LED and de-activates the animation if the animation was
 *  finite and has completed. Will then look through the list for the next
 *  highest priority task. Should be called periodically on a basis of around
 *  100-milliseconds.
 *
 ******************************************************************************/

void LEDANIMATION_service(LEDANIMATION_instance_t* instance);

/*******************************************************************************
 *
 * LEDANIMATION_set
 *
 * DESCRIPTION:
 *  Traverses all LED tables and for each LED activates the selected animation.
 *  If the animation is highest priority then the new animation will either
 *  begin immediately or at the next pattern start cycle.
 *
 * PARAMETERS:
 *  animation_id
 *   The animation id to set active.
 *
 ******************************************************************************/

void LEDANIMATION_set(LEDANIMATION_instance_t* instance, uint8_t animation_id);

/*******************************************************************************
 *
 * LEDANIMATION_clear
 *
 * DESCRIPTION:
 *  Traverses all LED tables and for each LED stops the selected animation if
 *  it is currently running, else, de-activates the animation so it is not
 *  pending to run.
 *
 * PARAMETERS:
 *  animation_id
 *   The animation id to set de-activate.
 *
 ******************************************************************************/

void LEDANIMATION_clear(LEDANIMATION_instance_t* instance, uint8_t animation_id);

/*******************************************************************************
 *
 * LEDANIMATION_start
 *
 * DESCRIPTION:
 *  Helper function which starts a specified animation for a specified LED.
 *
 * PARAMETERS:
 *  led_index
 *   Index of the LED in the instance LED list.
 *
 *  animation_index
 *   Index of the animation in the LED animation table.
 *
 * NOTES:
 *  Helper function called internally. Exposed here for unit testing.
 *
 ******************************************************************************/

void LEDANIMATION_start(LEDANIMATION_instance_t* instance,
                        uint32_t led_index,
                        uint8_t animation_index);

/*******************************************************************************
 *
 * LEDANIMATION_is_finite
 *
 * DESCRIPTION:
 *  Helper function which determines if a given animation is finite.
 *
 * PARAMETERS:
 *  led_index
 *   Index of the LED in the instance LED list.
 *
 *  animation_index
 *   Index of the animation in the LED animation table.
 *
 * RETURN:
 *  True if the animation is finite (OFF, ON, PATTER or RAMP with infinite
 *  iterations), else, false.
 *
 * NOTES:
 *  Helper function called internally. Exposed here for unit testing.
 *
 ******************************************************************************/

bool LEDANIMATION_is_finite(LEDANIMATION_instance_t* instance,
                            uint32_t led_index,
                            uint8_t animation_index);

#ifdef __cplusplus
}
#endif
#endif // LED_ANIMATION_J_H

/*******************************************************************************
 *
 *  Simple settings storage on EEPROM media. The module uses a 4-byte header
 *  to wear-level the NVM ensure data integrity. Requires proper initialization
 *  and the service routine to be called repeatedly after a new task request
 *  until the task is completed.
 *
 ******************************************************************************/

#ifndef NVMBASIC_J_H
#define NVMBASIC_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *
 * NVMBASIC_data_meta_flags_t
 *
 * DESCRIPTION:
 *  Flags which are used for data status and NVM wear leveling.
 *
 * data_present
 *  Indicates that the associated block of NVM was used to store data.
 *
 * data_old
 *  Inidcates that this block of NVM data is outdated and that a newer version
 *  of the data exists in a later block.
 *
 ******************************************************************************/

typedef union
{
  uint16_t all;
  struct
  {
    uint16_t reserved0                    : 14;
    uint16_t data_present                 : 1;
    uint16_t data_old                     : 1;
  };
}
NVMBASIC_data_meta_flags_t;

/*******************************************************************************
 *
 * NVMBASIC_data_meta_t
 *
 * DESCRIPTION:
 *  Data structure which MUST be the first entry in the user data structure.
 *  Contains information which is used to help evenly wear NVM and ensure
 *  data integrity.
 *
 * crc16
 *  The CRC16 of the data starting immediately after the CRC bytes.
 *
 * flags
 *  See NVMBASIC_data_meta_flags_t.
 *
 ******************************************************************************/

typedef struct
{
  uint16_t crc16;
  NVMBASIC_data_meta_flags_t flags;
}
NVMBASIC_data_meta_t;

/*******************************************************************************
 *
 * NVMBASIC_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 * busy
 *  Set when the module is busy with a task and cleared when the task is
 *  completed.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t busy                          : 1;
    uint8_t reserved1                     : 3;
    uint8_t task_state                    : 3;
    uint8_t reserved7                     : 1;
  };
}
NVMBASIC_flags_t;

/*******************************************************************************
 *
 * NVMBASIC_pre_task_callback_t
 *
 * DESCRIPTION:
 *  Function template for a user-provided function which is called when a new
 *  task is accepted and initialized but before the task actually begins. This
 *  allows the user to perform any last additional configurations before the
 *  actual logic of the task begins.
 *
 * PARAMETERS:
 *  context
 *   User-provided context.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*NVMBASIC_pre_task_callback_t)(uint32_t);

/*******************************************************************************
 *
 * NVMBASIC_post_task_callback_t
 *
 * DESCRIPTION:
 *  Function template for a user-provided function which is called after a
 *  task completes.
 *
 * PARAMETERS:
 *  context
 *   User-provided context.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*NVMBASIC_post_task_callback_t)(uint32_t);

/*******************************************************************************
 *
 * NVMBASIC_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module flags.
 *
 * eeprom
 *  User-provided initialized instance of a EEPROM.
 *
 * bus_mutex
 *  User-provided initialized instance of a BUSMUTEX.
 *
 * bus_id
 *  The BUS ID associated with the BUSMUTEX instance.
 *
 * data_stable
 *  Pointer to a user buffer dedicated for holding the data to be written to
 *  EEPROM, and to maintain that data as a volatile copy of what is currently
 *  on EEPROM.
 *
 * data_working
 *  Pointer to the user data structure which is, presumably, being updated by
 *  the user code.
 *
 * data_length
 *  The length, in bytes, of the data structure. This should be the length of
 *  the data_stable and data_working buffers.
 *
 * memory_start
 *  The starting address, inclusive, in EEPROM memory that is dedicated to
 *  the data. The address must be word-aligned.
 *
 * memory_end
 *  The ending address, exclusive, in EEPROM memory that is dedicated to the
 *  data. The length from the start address to the end address must be evenly
 *  divisble by the data length.
 *
 * next_address
 *  The next EEPROM starting memory address where the data will be written.
 *
 * previous_address
 *  The last EEPROM starting memory address where the data was written.
 *
 * callback_context
 *  Context passed into the user pre/post operation callbacks.
 *
 * previous_meta_page_buffer
 *  Copy of the previous data structure, up to the first EEPROM page (i.e. if
 *  the data structue is larger than an EEPROM page, then the buffer will be
 *  only the first EEPROM page used by the previous data structure. If the
 *  data structure is smaller than an EEPROM page, then the entire previous
 *  data structure is stored in this buffer). Keeping a copy eliminates the
 *  need to re-read the old meta before doing a write to mark the data entry
 *  as old. We need to store up to an entire EEPROM page since any un-written
 *  bytes will be erased by the EEPROM.
 *
 * previous_meta_page_buffer_length
 *  The length, in bytes, of the previous_meta_page_buffer. This is set during
 *  initialization and is the smaller of the data structure length and EEPROM
 *  page length.
 *
 * *_task_*
 *  User-provided functions. See typedef comments.
 *
 ******************************************************************************/

typedef struct
{
  NVMBASIC_flags_t flags;
  EEPROM_instance_t* eeprom;
  BUSMUTEX_instance_t* bus_mutex;
  BUSMUTEX_bus_id_t bus_id;
  uint8_t* data_stable;
  uint8_t* data_working;
  uint32_t data_length;
  uint32_t memory_start;
  uint32_t memory_end;
  uint32_t next_address;
  uint32_t previous_address;
  uint32_t callback_context;
  uint8_t* previous_meta_page_buffer;
  uint32_t previous_meta_page_buffer_length;
  NVMBASIC_pre_task_callback_t pre_task_callback;
  NVMBASIC_post_task_callback_t post_task_callback;
}
NVMBASIC_instance_t;

/*******************************************************************************
 *
 * NVMBASIC_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values.
 *
 * PARAMETERS:
 *  See NVMBASIC_instance_t.
 *
 ******************************************************************************/

void NVMBASIC_initialize(NVMBASIC_instance_t* instance,
                         EEPROM_instance_t* eeprom,
                         BUSMUTEX_instance_t* bus_mutex,
                         BUSMUTEX_bus_id_t bus_id,
                         uint8_t* data_stable,
                         uint8_t* data_working,
                         uint32_t data_length,
                         uint32_t memory_start,
                         uint32_t memory_end,
                         uint8_t* previous_meta_page_buffer,
                         NVMBASIC_pre_task_callback_t pre_task_callback,
                         NVMBASIC_post_task_callback_t post_task_callback);

/*******************************************************************************
 *
 * NVMBASIC_blocked_restore
 *
 * DESCRIPTION:
 *  Blocking function which attempts to restore the latest saved version of the
 *  data from the EEPROM. This is a blocking function intended to be called
 *  during initialization.
 *
 * RETURN:
 *  True if the task request was performed and a restore point was found and
 *  restored, else, false.
 *
 ******************************************************************************/

bool NVMBASIC_blocked_restore(NVMBASIC_instance_t* instance);

/*******************************************************************************
 *
 * NVMBASIC_blocked_purge
 *
 * DESCRIPTION:
 *  Blocking function which erases the allocated memory block and resets the
 *  next write location to the beggining of the memory space.
 *
 * RETURN:
 *  True if the task request was performed, else, false.
 *
 ******************************************************************************/

bool NVMBASIC_blocked_purge(NVMBASIC_instance_t* instance);

/*******************************************************************************
 *
 * NVMBASIC_save
 *
 * DESCRIPTION:
 *  Captures and saves a copy of the working data in EEPROM.
 *
 * PARAMETERS:
 *  force
 *   Forces the save even if the stable and working data buffers are the same.
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool NVMBASIC_save(NVMBASIC_instance_t* instance, bool force);

/*******************************************************************************
 *
 * NVMBASIC_service
 *
 * DESCRIPTION:
 *  Services the task state machine. Must be called repeatedly until the task
 *  is completed.
 *
 * RETURN:
 *  False if there is an ongoing task which has not completed, else, true.
 *
 ******************************************************************************/

bool NVMBASIC_service(NVMBASIC_instance_t* instance);

/*******************************************************************************
 *
 * NVMBASIC_is_busy
 *
 * DESCRIPTION:
 *  Determines if an instance is currently busy with a task.
 *
 * RETURN:
 *  True if the instance is busy with a task, else, false.
 *
 ******************************************************************************/

bool NVMBASIC_is_busy(NVMBASIC_instance_t* instance);

#ifdef __cplusplus
}
#endif
#endif // NVMBASIC_J_H

/*******************************************************************************
 *
 *  Simple, and comparably tiny, replacement for stdio printf. This module
 *  parses standard variable types into their ASCII equivalents and places the
 *  characters on a user-provided Queue instance - it is expected that the user
 *  provides a character queue (element byte size of 1) and will transmit the
 *  queue via communication protocol to the terminal elsewhere.
 *
 ******************************************************************************/

#ifndef PRINT_J_H
#define PRINT_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Provide a shortened command interface for the Print module and an easy way
 * to enable/disable printing. (Disabling will completely remove the code from
 * the build.) The user should define PRINT_TERMINAL_QUEUE in their code as
 * their dedicated/default terminal output Queue.
 */

#ifdef PRINT_TERMINAL_QUEUE
#define PCHAR(Z)                PRINT_char(&PRINT_TERMINAL_QUEUE, Z)
#define PMSTR(Z, M)             PRINT_mstring(&PRINT_TERMINAL_QUEUE, Z, M)
#define PSTR(Z)                 PRINT_string(&PRINT_TERMINAL_QUEUE, Z)
#define PU32(Z)                 PRINT_uint32(&PRINT_TERMINAL_QUEUE, Z)
#define PI32(Z)                 PRINT_int32(&PRINT_TERMINAL_QUEUE, Z)
#define PH4(Z)                  PRINT_hex4(&PRINT_TERMINAL_QUEUE, Z)
#define PH8(Z)                  PRINT_hex8(&PRINT_TERMINAL_QUEUE, Z)
#define PH16(Z)                 PRINT_hex16(&PRINT_TERMINAL_QUEUE, Z)
#define PH32(Z)                 PRINT_hex32(&PRINT_TERMINAL_QUEUE, Z)
#define PB4(Z)                  PRINT_bin4(&PRINT_TERMINAL_QUEUE, Z)
#define PB8(Z)                  PRINT_bin8(&PRINT_TERMINAL_QUEUE, Z)
#define PB16(Z)                 PRINT_bin16(&PRINT_TERMINAL_QUEUE, Z)
#define PB32(Z)                 PRINT_bin32(&PRINT_TERMINAL_QUEUE, Z)
#define PNL()                   PRINT_newline(&PRINT_TERMINAL_QUEUE)
#define PCLEAR()                PRINT_clear(&PRINT_TERMINAL_QUEUE)
#else
#define PCHAR(Z)
#define PMSTR(Z, M)
#define PSTR(Z)
#define PU32(Z)
#define PI32(Z)
#define PH4(Z)
#define PH8(Z)
#define PH16(Z)
#define PH32(Z)
#define PB4(Z)
#define PB8(Z)
#define PB16(Z)
#define PB32(Z)
#define PNL()
#define PCLEAR()
#endif

/*******************************************************************************
 *
 * PRINT_char
 *
 * DESCRIPTION:
 *  Enqueues a single ASCII character.
 *
 * PARAMETERS:
 *  value
 *   ASCII characters to enqueue.
 *
 ******************************************************************************/

void PRINT_char(QUEUE_instance_t* instance, char value);

/*******************************************************************************
 *
 * PRINT_string
 *
 * DESCRIPTION:
 *  Enqueues an array of ASCII characters, a string, until a null character is
 *  reached.
 *
 * PARAMETERS:
 *  value
 *   ASCII characters to enqueue.
 *
 ******************************************************************************/

void PRINT_string(QUEUE_instance_t* instance, char* value);

/*******************************************************************************
 *
 * PRINT_mstring
 *
 * DESCRIPTION:
 *  Enqueues an array of ASCII characters, a string, until a null character or
 *  maximum length is reached.
 *
 * PARAMETERS:
 *  value
 *   ASCII characters to enqueue.
 *
 *  length
 *   Maximum number of characters to print if a null character is not reached.
 *
 ******************************************************************************/

void PRINT_mstring(QUEUE_instance_t* instance, char* value, uint32_t length);

/*******************************************************************************
 *
 * PRINT_uint32
 *
 * DESCRIPTION:
 *  Enqueues the ASCII representation of the provided uint32_t.
 *
 * PARAMETERS:
 *  value
 *   Value to translate to ASCII characters and enqueue.
 *
 ******************************************************************************/

void PRINT_uint32(QUEUE_instance_t* instance, uint32_t value);

/*******************************************************************************
 *
 * PRINT_int32
 *
 * DESCRIPTION:
 *  Enqueues the ASCII representation of the provided int32_t.
 *
 * PARAMETERS:
 *  value
 *   Value to translate to ASCII characters and enqueue.
 *
 ******************************************************************************/

void PRINT_int32(QUEUE_instance_t* instance, int32_t value);

/*******************************************************************************
 *
 * PRINT_uint64
 *
 * DESCRIPTION:
 *  Enqueues the ASCII representation of the provided uint64_t.
 *
 * PARAMETERS:
 *  value
 *   Value to translate to ASCII characters and enqueue.
 *
 ******************************************************************************/

void PRINT_uint64(QUEUE_instance_t* instance, uint64_t value);

/*******************************************************************************
 *
 * PRINT_int64
 *
 * DESCRIPTION:
 *  Enqueues the ASCII representation of the provided int64_t.
 *
 * PARAMETERS:
 *  value
 *   Value to translate to ASCII characters and enqueue.
 *
 ******************************************************************************/

void PRINT_int64(QUEUE_instance_t* instance, int64_t value);

/*******************************************************************************
 *
 * PRINT_hex4
 *
 * DESCRIPTION:
 *  Enqueues the ASCII representation of the provided uint8_t in hex-decimal
 *  format. The output will contain 1-characters.
 *
 * PARAMETERS:
 *  value
 *   Value to translate to ASCII character and enqueue.
 *
 ******************************************************************************/

void PRINT_hex4(QUEUE_instance_t* instance, uint8_t value);

/*******************************************************************************
 *
 * PRINT_hex8
 *
 * DESCRIPTION:
 *  Enqueues the ASCII representation of the provided uint8_t in hex-decimal
 *  format. The output will contain 2-characters.
 *
 * PARAMETERS:
 *  value
 *   Value to translate to ASCII characters and enqueue.
 *
 ******************************************************************************/

void PRINT_hex8(QUEUE_instance_t* instance, uint8_t value);

/*******************************************************************************
 *
 * PRINT_hex16
 *
 * DESCRIPTION:
 *  Enqueues the ASCII representation of the provided uint16_t in hex-decimal
 *  format. The output will contain 4-characters.
 *
 * PARAMETERS:
 *  value
 *   Value to translate to ASCII characters and enqueue.
 *
 ******************************************************************************/

void PRINT_hex16(QUEUE_instance_t* instance, uint16_t value);

/*******************************************************************************
 *
 * PRINT_hex32
 *
 * DESCRIPTION:
 *  Enqueues the ASCII representation of the provided uint32_t in hex-decimal
 *  format. The output will contain 8-characters.
 *
 * PARAMETERS:
 *  value
 *   Value to translate to ASCII characters and enqueue.
 *
 ******************************************************************************/

void PRINT_hex32(QUEUE_instance_t* instance, uint32_t value);

/*******************************************************************************
 *
 * PRINT_bin4
 *
 * DESCRIPTION:
 *  Enqueues the ASCII representation of the provided uint8_t in binary
 *  format. The output will contain 4-characters.
 *
 * PARAMETERS:
 *  value
 *   Value to translate to ASCII characters and enqueue.
 *
 ******************************************************************************/

void PRINT_bin4(QUEUE_instance_t* instance, uint8_t value);

/*******************************************************************************
 *
 * PRINT_bin8
 *
 * DESCRIPTION:
 *  Enqueues the ASCII representation of the provided uint8_t in binary
 *  format. The output will contain 8-characters.
 *
 * PARAMETERS:
 *  value
 *   Value to translate to ASCII characters and enqueue.
 *
 ******************************************************************************/

void PRINT_bin8(QUEUE_instance_t* instance, uint8_t value);

/*******************************************************************************
 *
 * PRINT_bin16
 *
 * DESCRIPTION:
 *  Enqueues the ASCII representation of the provided uint16_t in binary
 *  format. The output will contain 16-characters.
 *
 * PARAMETERS:
 *  value
 *   Value to translate to ASCII characters and enqueue.
 *
 ******************************************************************************/

void PRINT_bin16(QUEUE_instance_t* instance, uint16_t value);

/*******************************************************************************
 *
 * PRINT_bin32
 *
 * DESCRIPTION:
 *  Enqueues the ASCII representation of the provided uint32_t in binary
 *  format. The output will contain 32-characters.
 *
 * PARAMETERS:
 *  value
 *   Value to translate to ASCII characters and enqueue.
 *
 ******************************************************************************/

void PRINT_bin32(QUEUE_instance_t* instance, uint32_t value);

/*******************************************************************************
 *
 * PRINT_newline
 *
 * DESCRIPTION:
 *  Enqueues the carriage return and newline characters which produces a new
 *  line.
 *
 ******************************************************************************/

void PRINT_newline(QUEUE_instance_t* instance);

/*******************************************************************************
 *
 * PRINT_clear
 *
 * DESCRIPTION:
 *  Enqueues the special character sequence recognized by most terminal
 *  applications to clear the screen.
 *
 ******************************************************************************/

void PRINT_clear(QUEUE_instance_t* instance);

#ifdef __cplusplus
}
#endif
#endif // PRINT_J_H

/*******************************************************************************
 *  Rotary encoder/wheel with a A/B phase. Requires proper initialization and
 *  the service routine to be called periodically to detect changes in the A/B
 *  phases. Callback function is called when a rotation "click" is confirmed.
 *
 ******************************************************************************/

#ifndef ROTARYENCODER_J_H
#define ROTARYENCODER_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *
 * ROTARYENCODER_edge_trigger_t
 *
 * DESCRIPTION:
 *  The edge (rising, falling, or both) which will trigger the start of a new
 *  rotation tick.
 *
 ******************************************************************************/

typedef enum
{
  ROTARYENCODER_EDGE_TRIGGER_FALLING      = 0,
  ROTARYENCODER_EDGE_TRIGGER_RISING,
  ROTARYENCODER_EDGE_TRIGGER_BOTH
}
ROTARYENCODER_edge_trigger_t;

/*******************************************************************************
 *
 * ROTARYENCODER_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 * rotation_in_progress
 *  Set when a new rotation is detected. Cleared when the "click" is complete,
 *  that is, when A and B phases are the same.
 *
 * clockwise_rotation
 *  Set if the detected rotation is clockwise, else, cleared if counter-
 *  clockwise.
 *
 * debounced_a, debounced_b
 *  Set during an active rotation once each phase is debounced, and cleared
 *  at the "end" of the tick rotation. These are used to guard against false
 *  negatives that may occur by rotating the knob enough to trigger on phase
 *  and have it debounce, but then reverting to the previous tick location.
 *
 * last_a, last_b
 *  The last debounced phase A/B GPIO state.
 *
 * edge_trigger
 *  The type of edge which will qualify a for a new rotation tick. (I have
 *  found that encoders vary in their behavior. Some alternate between high
 *  and low while others will always pull back high or low after every tick.)
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t rotation_in_progress          : 1;
    uint8_t clockwise_rotation            : 1;
    uint8_t debounced_a                   : 1;
    uint8_t debounced_b                   : 1;
    uint8_t last_a                        : 1;
    uint8_t last_b                        : 1;
    uint8_t edge_trigger                  : 2;
  };
}
ROTARYENCODER_flags_t;

/*******************************************************************************
 *
 * ROTARYENCODER_hal_is_phase_a_set_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if phase 'a' of the rotary encoder is set.
 *
 * RETURN:
 *  True if phase 'a' is set, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*ROTARYENCODER_hal_is_phase_a_set_t)(void);

/*******************************************************************************
 *
 * ROTARYENCODER_hal_is_phase_b_set_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will check if phase 'b of the rotary encoder is set.
 *
 * RETURN:
 *  True if phase 'b' is set, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*ROTARYENCODER_hal_is_phase_b_set_t)(void);

/*******************************************************************************
 *
 * ROTARYENCODER_rotation_tick_callback_t
 *
 * DESCRIPTION:
 *  Function template for a user-provided function which is called when there is
 *  a successful clockwise (true) or counter-clockwise (false) rotation.
 *
 * PARAMETERS:
 *  clockwise
 *   True if the tick was clockwise, else, false for counter-clockwise.
 *
 ******************************************************************************/

typedef void (*ROTARYENCODER_rotation_tick_callback_t)(bool);

/*******************************************************************************
 *
 * ROTARYENCODER_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module flags.
 *
 * debounce_count
 *  The number of times, for both rotation directions, which the phase GPIO
 *  must be detected as stable before accepting a GPIO change.
 *
 * clockwise_debounce_counter
 *  Keeps track of the debounce count in the clockwise direction.
 *
 * counterclockwise_debounce_counter
 *  Keeps track of the debounce count in the counterclockwise direction.
 *
 * edge_trigger_check
 *  Internal helper function which is pointed to during initialization based
 *  upon the edge trigger type. This provides a more efficient check than
 *  repeated conditionals in each iteration.
 *
 * *_callback
 * *_hal_*
 *  User-provided functions. See typedef comments.
 *
 ******************************************************************************/

typedef struct
{
  ROTARYENCODER_flags_t flags;
  uint16_t debounce_count;
  uint16_t clockwise_debounce_counter;
  uint16_t counterclockwise_debounce_counter;
  bool (*edge_trigger_check)(bool edge);
  ROTARYENCODER_rotation_tick_callback_t rotation_tick_callback;
  ROTARYENCODER_hal_is_phase_a_set_t is_phase_a_set;
  ROTARYENCODER_hal_is_phase_b_set_t is_phase_b_set;
}
ROTARYENCODER_instance_t;

/*******************************************************************************
 *
 * ROTARYENCODER_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values.
 *
 * PARAMETERS:
 *  See ROTARYENCODER_instance_t.
 *
 ******************************************************************************/

void ROTARYENCODER_initialize(ROTARYENCODER_instance_t* instance,
                              uint16_t debounce_count,
                              ROTARYENCODER_edge_trigger_t edge_trigger,
                              ROTARYENCODER_rotation_tick_callback_t rotation_tick_callback,
                              ROTARYENCODER_hal_is_phase_a_set_t is_phase_a_set,
                              ROTARYENCODER_hal_is_phase_b_set_t is_phase_b_set);

/*******************************************************************************
 *
 * ROTARYENCODER_service
 *
 * DESCRIPTION:
 *  Services the task state machine.
 *
 ******************************************************************************/

void ROTARYENCODER_service(ROTARYENCODER_instance_t* instance);

#ifdef __cplusplus
}
#endif
#endif // ROTARYENCODER_J_H

/*******************************************************************************
 *
 *  Supports both bit-banged and SPI methods. Both methods require proper
 *  initialization and the service routine to be called repeatedly after a new
 *  task request until the task is completed.
 *
 *  A shift PISO is generally composed of three operation pins - shift/latch
 *  (SL), clock (CLK), and serial output (SO). SL is pulled low to activate
 *  the latch. A rising clock edge will cause the parallel input pins to be
 *  read. SL is then pulled high to activate shift mode. Each rising clock
 *  edge will shift the contents of each register to the next register in
 *  line. The last register in the apparatus will output to SO.
 *
 *  In the SPI method, the task takes place in two SPI transactions. The first
 *  triggers the latch and the second shifts the values out. The pins should
 *  be as follows:
 *
 *   MOSI - Latch(0)/Shift(1)
 *   MISO - Serial Output (Q)
 *   CLK  - CLK
 *
 *  Shift registers generally have a propagation delay of around 50nS or less.
 *  Hence, the application code should set the SPI frequency to around 20MHz or
 *  slower.
 *
 *  The nature of the shift register is that the first bit to enter the shift
 *  apparatus will actually be the "last" bit, or the farthest away bit, from
 *  the head where the MOSI and CLK are connected. The resulting bit order
 *  will differ depending on the endianness of the processor and the method
 *  used.
 *
 *  This module has been tested on both active low and active high MOSI/CLK
 *  configurations.
 *
 ******************************************************************************/

#ifndef SHIFTPISO_J_H
#define SHIFTPISO_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *
 * SHIFTPISO_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 * busy
 *  Set when the module is busy with a task and cleared when the task is
 *  completed.
 *
 * bit_banged
 *  Set if the instance has been initialized to use the bit-banging method.
 *
 * task_state
 *  State machine state.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t busy                          : 1;
    uint8_t bit_banged                    : 1;
    uint8_t reserved2                     : 2;
    uint8_t task_state                    : 3;
    uint8_t reserved7                     : 1;
  };
}
SHIFTPISO_flags_t;

/*******************************************************************************
 *
 * SHIFTPISO_error_flags_t
 *
 * DESCRIPTION:
 *  Module error flags.
 *
 * other
 *  All other types of errors.
 *
 * spi
 *  Errors relating to the SPI.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t other                         : 1;
    uint8_t spi                           : 1;
    uint8_t reserved2                     : 6;
  };
}
SHIFTPISO_error_flags_t;

/*******************************************************************************
 *
 * SHIFTPISO_pre_task_callback_t
 *
 * DESCRIPTION:
 *  Function template for a user-provided function which is called when a new
 *  task is accepted and initialized but before the task actually begins. This
 *  allows the user to perform any last additional configurations before the
 *  actual logic of the task begins.
 *
 * PARAMETERS:
 *  context
 *   User-provided context.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*SHIFTPISO_pre_task_callback_t)(uint32_t);

/*******************************************************************************
 *
 * SHIFTPISO_post_task_callback_t
 *
 * DESCRIPTION:
 *  Function template for a user-provided function which is called after a
 *  task completes.
 *
 * PARAMETERS:
 *  context
 *   User-provided context.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*SHIFTPISO_post_task_callback_t)(uint32_t);

/*******************************************************************************
 *
 * SHIFTPISO_hal_set_clock_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will set the shift/latch clock line high or low.
 *
 * PARAMETERS:
 *  value
 *   True to set high, false to set low.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SHIFTPISO_hal_set_clock_t)(bool);

/*******************************************************************************
 *
 * SHIFTPISO_hal_set_latch_shift_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will set the latch/shift line high or low.
 *
 * PARAMETERS:
 *  value
 *   True to activate latch, else, false to activate shift.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SHIFTPISO_hal_set_latch_shift_t)(bool);

/*******************************************************************************
 *
 * SHIFTPISO_hal_get_serial_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will get the serial line state.
 *
 * RETURN:
 *  True if the line is high, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*SHIFTPISO_hal_get_serial_t)(void);

/*******************************************************************************
 *
 * SHIFTPISO_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module flags.
 *
 * errors
 *  Errors which occurred in the last transaction.
 *
 * utimer
 *  User-provided initialized instance of a UTIMER.
 *
 * utimer_ticket
 *  Data structure used with the UTIMER instance.
 *
 * spi
 *  User-provided initialized instance of a SERSPI.
 *
 * bus_mutex
 *  User-provided initialized instance of a BUSMUTEX.
 *
 * bus_id
 *  The BUS ID associated with the BUSMUTEX instance.
 *
 * propagation_delay_us
 *  Microseconds required to delay between hardware operations in order to
 *  allow the internal circuits to stabilize.
 *
 * register_count
 *  Total number of shift registers (total channels, not the number of physical
 *  devices) in the apparatus.
 *
 * register_counter
 *  Keeps track of the which register is currently being serviced.
 *
 * byte_offset
 *  Keeps track of the byte offset in the serial buffer.
 *
 * bit_offset
 *  Keeps track of the bit offset in the serial buffer.
 *
 * serial_buffer_length
 *  Required length, in bytes, of the serial buffer.
 *
 * serial_buffer
 *  Buffer which stores the shift register values to be written.
 *
 * callback_context
 *  Context passed into the user pre/post task callbacks.
 *
 * service_handler
 *  Internal-use pointer to the active state machine service.
 *
 * *_task_*
 * *_hal_*
 *  User-provided functions. See typedef comments.
 *
 ******************************************************************************/

typedef struct
{
  SHIFTPISO_flags_t flags;
  SHIFTPISO_error_flags_t errors;
  UTIMER_instance_t* utimer;
  UTIMER_ticket_t utimer_ticket;
  SERSPI_instance_t* spi;
  BUSMUTEX_instance_t* bus_mutex;
  BUSMUTEX_bus_id_t bus_id;
  uint16_t propagation_delay_us;
  uint16_t register_count;
  uint16_t register_counter;
  uint16_t byte_offset;
  uint8_t bit_offset;
  uint16_t serial_buffer_length;
  uint8_t* serial_buffer;
  uint32_t callback_context;
  bool (*service_handler)(void*);
  SHIFTPISO_pre_task_callback_t pre_task_callback;
  SHIFTPISO_post_task_callback_t post_task_callback;
  SHIFTPISO_hal_set_clock_t set_clock;
  SHIFTPISO_hal_set_latch_shift_t set_latch_shift;
  SHIFTPISO_hal_get_serial_t get_serial;
}
SHIFTPISO_instance_t;

/*******************************************************************************
 *
 * SHIFTPISO_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values. SPI method.
 *
 * PARAMETERS:
 *  See SHIFTPISO_instance_t.
 *
 ******************************************************************************/

void SHIFTPISO_initialize(SHIFTPISO_instance_t* instance,
                          SERSPI_instance_t* spi,
                          BUSMUTEX_instance_t* bus_mutex,
                          BUSMUTEX_bus_id_t bus_id,
                          uint16_t register_count,
                          SHIFTPISO_pre_task_callback_t pre_task_callback,
                          SHIFTPISO_post_task_callback_t post_task_callback);

/*******************************************************************************
 *
 * SHIFTPISO_initialize_bb
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values. Bit-bang method.
 *
 * PARAMETERS:
 *  See SHIFTPISO_instance_t.
 *
 ******************************************************************************/

void SHIFTPISO_initialize_bb(SHIFTPISO_instance_t* instance,
                             UTIMER_instance_t* utimer,
                             uint16_t propagation_delay_us,
                             uint16_t register_count,
                             SHIFTPISO_pre_task_callback_t pre_task_callback,
                             SHIFTPISO_post_task_callback_t post_task_callback,
                             SHIFTPISO_hal_set_clock_t set_clock,
                             SHIFTPISO_hal_set_latch_shift_t set_latch_shift,
                             SHIFTPISO_hal_get_serial_t get_serial);

/*******************************************************************************
 *
 * SHIFTPISO_begin_new_read
 *
 * DESCRIPTION:
 *  Attempts to begin a new serial input task.
 *
 * PARAMETERS:
 *  serial_buffer
 *   Buffer which will hold the read data. Each bit represents a single
 *   register. The buffer should be large enough to accommodate the entirety of
 *   the shift apparatus.
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool SHIFTPISO_begin_new_read(SHIFTPISO_instance_t* instance, uint8_t* serial_buffer);

/*******************************************************************************
 *
 * SHIFTPISO_service
 *
 * DESCRIPTION:
 *  Services the task state machine. Must be called repeatedly until the task
 *  is completed.
 *
 * RETURN:
 *  False if there is an ongoing task which has not completed, else, true.
 *
 ******************************************************************************/

bool SHIFTPISO_service(SHIFTPISO_instance_t* instance);

/*******************************************************************************
 *
 * SHIFTPISO_is_busy
 *
 * DESCRIPTION:
 *  Determines if an instance is currently busy with a task.
 *
 * RETURN:
 *  True if the instance is busy with a task, else, false.
 *
 ******************************************************************************/

bool SHIFTPISO_is_busy(SHIFTPISO_instance_t* instance);

#ifdef __cplusplus
}
#endif
#endif // SHIFTPISO_J_H

/*******************************************************************************
 *
 *  Supports both bit-banged and SPI methods. Both methods require proper
 *  initialization and the service routine to be called repeatedly after a new
 *  task request until the task is completed.
 *
 *  A shift SIPO is generally composed of three operating pins - shift clock
 *  (SCLK), register clock (RCLK), and serial input (SI). The rising edge on the
 *  SCLK causes all the shift registers to transfer their bit-value to the next
 *  shift register in line. The first shift register takes up the value provided
 *  by SI. This process is done until all shift registers are loaded with the
 *  desired output. The RCLK is then pulsed to transfer the content of the shift
 *  registers into the data registers. So long as the output enable line is
 *  opened, the values on the data registers will be sent out through the
 *  parallel lines.
 *
 *  In the SPI method, the task takes place in a single SPI transaction, which
 *  loads the shift registers, followed by a RCLK toggle via GPIO. The pin-outs,
 *  for a simple approach, should be as follows:
 *
 *   MOSI - Serial Input
 *   CLK  - Shift Clock
 *   GPIO - Register Clock
 *   GND  - Output Enable
 *
 *  Shift registers generally have a propagation delay of around 50nS or less.
 *  Hence, the application code should set the SPI frequency to around 20MHz or
 *  slower.
 *
 *  The nature of the shift register is that the first bit to enter the shift
 *  apparatus will actually be the "last" bit, or the farthest away bit, from
 *  the head where the MOSI and CLK are connected. The resulting bit order
 *  will differ depending on the endianness of the processor and the method
 *  used.
 *
 *  This module has been tested on both active low and active high MOSI/CLK
 *  configurations.
 *
 ******************************************************************************/

#ifndef SHIFTSIPO_J_H
#define SHIFTSIPO_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *
 * SHIFTSIPO_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 * busy
 *  Set when the module is busy with a task and cleared when the task is
 *  completed.
 *
 * bit_banged
 *  Set if the instance has been initialized to use the bit-banging method.
 *
 * task_state
 *  State machine state.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t busy                          : 1;
    uint8_t bit_banged                    : 1;
    uint8_t reserved2                     : 2;
    uint8_t task_state                    : 3;
    uint8_t reserved7                     : 1;
  };
}
SHIFTSIPO_flags_t;

/*******************************************************************************
 *
 * SHIFTSIPO_error_flags_t
 *
 * DESCRIPTION:
 *  Module error flags.
 *
 * other
 *  All other types of errors.
 *
 * spi
 *  Errors relating to the SPI.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t other                         : 1;
    uint8_t spi                           : 1;
    uint8_t reserved2                     : 6;
  };
}
SHIFTSIPO_error_flags_t;

/*******************************************************************************
 *
 * SHIFTSIPO_pre_task_callback_t
 *
 * DESCRIPTION:
 *  Function template for a user-provided function which is called when a new
 *  task is accepted and initialized but before the task actually begins. This
 *  allows the user to perform any last additional configurations before the
 *  actual logic of the task begins.
 *
 * PARAMETERS:
 *  context
 *   User-provided context.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*SHIFTSIPO_pre_task_callback_t)(uint32_t);

/*******************************************************************************
 *
 * SHIFTSIPO_post_task_callback_t
 *
 * DESCRIPTION:
 *  Function template for a user-provided function which is called after a
 *  task completes.
 *
 * PARAMETERS:
 *  context
 *   User-provided context.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*SHIFTSIPO_post_task_callback_t)(uint32_t);

/*******************************************************************************
 *
 * SHIFTSIPO_hal_set_shift_clock_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will set the shift clock line high or low.
 *
 * PARAMETERS:
 *  value
 *   True to set high, false to set low.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SHIFTSIPO_hal_set_shift_clock_t)(bool);

/*******************************************************************************
 *
 * SHIFTSIPO_hal_set_register_clock_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will set the register clock line high or low.
 *
 * PARAMETERS:
 *  value
 *   True to set high, false to set low.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SHIFTSIPO_hal_set_register_clock_t)(bool);

/*******************************************************************************
 *
 * SHIFTSIPO_hal_set_serial_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will set the serial line high or low.
 *
 * PARAMETERS:
 *  value
 *   True to set high, false to set low.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*SHIFTSIPO_hal_set_serial_t)(bool);

/*******************************************************************************
 *
 * SHIFTSIPO_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module flags.
 *
 * errors
 *  Errors which occurred in the last transaction.
 *
 * utimer
 *  User-provided initialized instance of a UTIMER.
 *
 * utimer_ticket
 *  Data structure used with the UTIMER instance.
 *
 * spi
 *  User-provided initialized instance of a SERSPI.
 *
 * bus_mutex
 *  User-provided initialized instance of a BUSMUTEX.
 *
 * bus_id
 *  The BUS ID associated with the BUSMUTEX instance.
 *
 * propagation_delay_us
 *  Microseconds required to delay between hardware operations in order to
 *  allow the internal circuits to stabilize.
 *
 * register_count
 *  Total number of shift registers (total channels, not the number of physical
 *  devices) in the apparatus.
 *
 * register_counter
 *  Keeps track of the which register is currently being serviced.
 *
 * byte_offset
 *  Keeps track of the byte offset in the serial buffer.
 *
 * bit_offset
 *  Keeps track of the bit offset in the serial buffer.
 *
 * serial_buffer_length
 *  Required length, in bytes, of the serial buffer.
 *
 * serial_buffer
 *  Buffer which stores the shift register values to be written.
 *
 * callback_context
 *  Context passed into the user pre/post task callbacks.
 *
 * service_handler
 *  Internal-use pointer to the active state machine service.
 *
 * *_task_*
 * *_hal_*
 *  User-provided functions. See typedef comments.
 *
 ******************************************************************************/

typedef struct
{
  SHIFTSIPO_flags_t flags;
  SHIFTSIPO_error_flags_t errors;
  UTIMER_instance_t* utimer;
  UTIMER_ticket_t utimer_ticket;
  SERSPI_instance_t* spi;
  BUSMUTEX_instance_t* bus_mutex;
  BUSMUTEX_bus_id_t bus_id;
  uint16_t propagation_delay_us;
  uint16_t register_count;
  uint16_t register_counter;
  uint16_t byte_offset;
  uint8_t bit_offset;
  uint16_t serial_buffer_length;
  uint8_t* serial_buffer;
  uint32_t callback_context;
  bool (*service_handler)(void*);
  SHIFTSIPO_pre_task_callback_t pre_task_callback;
  SHIFTSIPO_post_task_callback_t post_task_callback;
  SHIFTSIPO_hal_set_shift_clock_t set_shift_clock;
  SHIFTSIPO_hal_set_register_clock_t set_register_clock;
  SHIFTSIPO_hal_set_serial_t set_serial;
}
SHIFTSIPO_instance_t;

/*******************************************************************************
 *
 * SHIFTSIPO_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values. SPI method.
 *
 * PARAMETERS:
 *  See SHIFTSIPO_instance_t.
 *
 ******************************************************************************/

void SHIFTSIPO_initialize(SHIFTSIPO_instance_t* instance,
                          UTIMER_instance_t* utimer,
                          SERSPI_instance_t* spi,
                          BUSMUTEX_instance_t* bus_mutex,
                          BUSMUTEX_bus_id_t bus_id,
                          uint16_t propagation_delay_us,
                          uint16_t register_count,
                          SHIFTSIPO_pre_task_callback_t pre_task_callback,
                          SHIFTSIPO_post_task_callback_t post_task_callback,
                          SHIFTSIPO_hal_set_register_clock_t set_register_clock);

/*******************************************************************************
 *
 * SHIFTSIPO_initialize_bb
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values. Bit-bang method.
 *
 * PARAMETERS:
 *  See SHIFTSIPO_instance_t.
 *
 ******************************************************************************/

void SHIFTSIPO_initialize_bb(SHIFTSIPO_instance_t* instance,
                             UTIMER_instance_t* utimer,
                             uint16_t propagation_delay_us,
                             uint16_t register_count,
                             SHIFTSIPO_pre_task_callback_t pre_task_callback,
                             SHIFTSIPO_post_task_callback_t post_task_callback,
                             SHIFTSIPO_hal_set_shift_clock_t set_shift_clock,
                             SHIFTSIPO_hal_set_register_clock_t set_register_clock,
                             SHIFTSIPO_hal_set_serial_t set_serial);

/*******************************************************************************
 *
 * SHIFTSIPO_begin_new_write
 *
 * DESCRIPTION:
 *  Attempts to begin a new serial output task.
 *
 * PARAMETERS:
 *  serial_buffer
 *   Buffer which contains the data to be written. Each bit represents a single
 *   register. The buffer should be large enough to accommodate the entirety of
 *   the shift apparatus.
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool SHIFTSIPO_begin_new_write(SHIFTSIPO_instance_t* instance, uint8_t* serial_buffer);

/*******************************************************************************
 *
 * SHIFTSIPO_service
 *
 * DESCRIPTION:
 *  Services the task state machine. Must be called repeatedly until the task
 *  is completed.
 *
 * RETURN:
 *  False if there is an ongoing task which has not completed, else, true.
 *
 ******************************************************************************/

bool SHIFTSIPO_service(SHIFTSIPO_instance_t* instance);

/*******************************************************************************
 *
 * SHIFTSIPO_is_busy
 *
 * DESCRIPTION:
 *  Determines if an instance is currently busy with a task.
 *
 * RETURN:
 *  True if the instance is busy with a task, else, false.
 *
 ******************************************************************************/

bool SHIFTSIPO_is_busy(SHIFTSIPO_instance_t* instance);

#ifdef __cplusplus
}
#endif
#endif // SHIFTSIPO_J_H

/*******************************************************************************
 *
 *  Terminal framework configured for 80-character width windows. The framework
 *  is composed of nodes, and each node is composed of a list of menu entries.
 *  Entries link to user handler functions, which can be sub-state machines or
 *  one-shot actions, or another menu node. The user is responsible for
 *  correctly linking nodes and providing function handlers when setting up the
 *  data structures. This module requires proper initialization and the service
 *  routine to be called repeatedly to process requests.
 *
 ******************************************************************************/

#ifndef TERMENU_J_H
#define TERMENU_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The maximum length of a description. When printed, the description string
 * will terminate at this length limit or when a NULL terminator is reached.
 */

#define TERMENU_DESCRIPTION_LENGTH_MAX    32U

/*******************************************************************************
 *
 * TERMENU_entry_handler_leaf_t
 *
 * DESCRIPTION:
 *  Function template for a user-provided function which is called as the
 *  handler for a leaf-type entry.
 *
 * PARAMETERS:
 *  context
 *   User-provided context.
 *
 * RETURN:
 *  False if there is an ongoing task which has not completed, else, true. If
 *  false is returned, the module state machine will continue to call the
 *  handler until true is returned. This allows the handler to implement its
 *  own sub-state machine if needed.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*TERMENU_entry_handler_leaf_t)(uint32_t);

/*******************************************************************************
 *
 * TERMENU_entry_handler_comment_t
 *
 * DESCRIPTION:
 *  Function template for a user-provided function which is called as the
 *  handler for a comment-type entry.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*TERMENU_entry_handler_comment_t)(void);

/*******************************************************************************
 *
 * TERMENU_entry_type_t
 *
 * DESCRIPTION:
 *  The various ways a menu entry can be interpreted. When a menu entry is
 *  selected, the way its handler is interpreted will depend on its type.
 *
 * LEAF
 *  Indicates that the handler is a user-provided function, either a sub-state
 *  machine or a one-shot action. The active menu will remain the same.
 *
 * NODE
 *  Indicates that the handler is a pointer to another menu node. The new
 *  menu becomes active and is printed.
 *
 * COMMENT
 *  Indicates that the entry is text which needs to be inserted. The handler
 *  is a pointer to a function which will be called when the entry is printed.
 *
 ******************************************************************************/

typedef enum
{
  TERMENU_ENTRY_TYPE_LEAF                 = 0,
  TERMENU_ENTRY_TYPE_NODE,
  TERMENU_ENTRY_TYPE_COMMENT
}
TERMENU_entry_type_t;

/*******************************************************************************
 *
 * TERMENU_entry_t
 *
 * DESCRIPTION:
 *  Single menu entry variables and handlers.
 *
 * type
 *  See TERMENU_entry_type_t.
 *
 * code
 *  Single character which indicates the value the user must enter to select
 *  the menu entry. The code should be unique for entry in a single menu. If
 *  more than one entries have the same code, only the first listed entry will
 *  be handled.
 *
 * description
 *  NULL-terminated string which describes the menu entry and/or what happens
 *  when it is selected.
 *
 * handler
 *  Pointer or function which will be triggered upon entry selection. It will
 *  be interpreted corresponding to the entry type.
 *
 * context
 *  Value which will be passed as an argument into the handler if the entry
 *  type is a leaf.
 *
 ******************************************************************************/

typedef struct
{
  TERMENU_entry_type_t type;
  char code;
  char* description;
  void* handler;
  uint32_t context;
}
TERMENU_entry_t;

/*******************************************************************************
 *
 * TERMENU_node_t
 *
 * DESCRIPTION:
 *  Collection of menu entries with an uplink to the parent node.
 *
 * entry_list
 *  List of menu entries.
 *
 * length
 *  The length, in number of entries, of the entry list.
 *
 * parent
 *  Parent node menu. The parent node will become active if the user presses
 *  <ESC>. If the parent is NULL, then the node is considered to be the root.
 *
 ******************************************************************************/

typedef struct
{
  TERMENU_entry_t* entry_list;
  uint8_t length;
  void* parent;
}
TERMENU_node_t;

/*******************************************************************************
 *
 * TERMENU_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 * busy
 *  Set when the module is busy with tasks and cleared when all tasks are
 *  completed. All tasks are considered completed when the Rx queue is emptied
 *  and the state machine state is idle.
 *
 * task_state
 *  State machine state.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t busy                          : 1;
    uint8_t reserved1                     : 3;
    uint8_t task_state                    : 2;
    uint8_t reserved6                     : 2;
  };
}
TERMENU_flags_t;

/*******************************************************************************
 *
 * TERMENU_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module flags.
 *
 * rx_queue
 *  User-provided initialized instance of a QUEUE which has been setup to hold
 *  received terminal bytes.
 *
 * tx_queue
 *  User-provided initialized instance of a QUEUE which has been setup to hold
 *  terminal bytes to be sent.
 *
 * active_node
 *  The node, or menu, which is currently displayed and listening for user
 *  input. When the module is initialized, this should normally be the top or
 *  root menu.
 *
 * password
 *  Pointer to a password string which is used to both protect terminal access
 *  as well as protect against electrical noise which might trigger false
 *  input data.
 *
 * password_length
 *  The length of the password string, not including the NULL-terminator. (Eg.
 *  The password "CAT" would have a length of 3.)
 *
 * password_offset
 *  Keeps track of the number of continuous characters entered by the user
 *  which match the password. When this value reaches the password length, the
 *  password is accepted and the terminal unlocked.
 *
 * task_entry_index
 *  Index into the active node entry list of the entry handler which is being
 *  processed.
 *
 ******************************************************************************/

typedef struct
{
  TERMENU_flags_t flags;
  QUEUE_instance_t* rx_queue;
  QUEUE_instance_t* tx_queue;
  TERMENU_node_t* active_node;
  char* password;
  uint8_t password_length;
  uint8_t password_offset;
  uint8_t task_entry_index;
}
TERMENU_instance_t;

/*******************************************************************************
 *
 * TERMENU_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values.
 *
 * PARAMETERS:
 *  See TERMENU_instance_t.
 *
 ******************************************************************************/

void TERMENU_initialize(TERMENU_instance_t* instance,
                        QUEUE_instance_t* rx_queue,
                        QUEUE_instance_t* tx_queue,
                        TERMENU_node_t* active_node,
                        char* password,
                        uint8_t password_length);

/*******************************************************************************
 *
 * TERMENU_service
 *
 * DESCRIPTION:
 *  Services the task state machine. Must be called repeatedly.
 *
 ******************************************************************************/

void TERMENU_service(TERMENU_instance_t* instance);

/*******************************************************************************
 *
 * TERMENU_is_busy
 *
 * DESCRIPTION:
 *  Determines if an instance is currently busy with a task.
 *
 * RETURN:
 *  True if the instance is busy with a task, else, false.
 *
 ******************************************************************************/

bool TERMENU_is_busy(TERMENU_instance_t* instance);

#ifdef __cplusplus
}
#endif
#endif // TERMENU_J_H

/*******************************************************************************
 *
 *  Terminal framework plugin for variable editing. A single instance consists
 *  of a list of variables which are displayed in table format. The variables
 *  are marked as RW for read/write or RO for read-only. When a RW variable is
 *  selected, the user may modify the variable value. Read and write hooks can
 *  be optionally used to handle read/write tasks which are more complex than
 *  simply writing to memory space. This module requires proper initialization
 *  and the service routine to be called repeatedly to process requests.
 *
 ******************************************************************************/

#ifndef TERVAR_J_H
#define TERVAR_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The maximum list length for a single node/instance. This length based on the
 * available alpha-numeric characters which will be used to number the entries.
 */

#define TERVAR_ENTRY_LIST_LENGTH_MAX      (10U + 26U + 26U)

/*
 * The maximum length of a description. When printed, the description string
 * will terminate at this length limit or when a NULL terminator is reached.
 */

#define TERVAR_DESCRIPTION_LENGTH_MAX     32U

/*
 * The maximum number of input characters for a variable edit. The longest
 * string should be a negative 31-bit integer which requires 11-characters.
 */

#define TERVAR_INPUT_LENGTH_MAX           11U

/*******************************************************************************
 *
 * TERVAR_hal_var_read_handler_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will provide special variable read handling if needed. The read value
 *  should be written to the input handler variable value.
 *
 * PARAMETERS:
 *  entry
 *   Pointer to the variable entry.
 *
 * RETURN:
 *  False if there is an ongoing task which has not completed, else, true. If
 *  false is returned, the terminal framework state machine will continue to
 *  call the handler until true is returned. This allows the handler to
 *  implement its own sub-state machine if needed.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef bool (*TERVAR_hal_var_read_handler_t)(void*);

/*******************************************************************************
 *
 * TERVAR_hal_var_write_handler_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will provide special variable write handling if needed. The value
 *  to be written will have been set as the variable value in the input handler
 *  previous to this function call.
 *
 * PARAMETERS:
 *  var_handle
 *   Pointer to the variable entry.
 *
 * RETURN:
 *  False if there is an ongoing task which has not completed, else, true. If
 *  false is returned, the terminal framework state machine will continue to
 *  call the handler until true is returned. This allows the handler to
 *  implement its own sub-state machine if needed.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef bool (*TERVAR_hal_var_write_handler_t)(void*);

/*******************************************************************************
 *
 * TERVAR_var_type_t
 *
 * DESCRIPTION:
 *  Lists the ways which a variable entry may be interpreted and handled when
 *  printed and edited.
 *
 ******************************************************************************/

typedef enum
{
  TERVAR_VAR_TYPE_UINT8                   = 0,
  TERVAR_VAR_TYPE_INT8,
  TERVAR_VAR_TYPE_UINT16,
  TERVAR_VAR_TYPE_INT16,
  TERVAR_VAR_TYPE_UINT32,
  TERVAR_VAR_TYPE_INT32,
  TERVAR_VAR_TYPE_FLOAT
}
TERVAR_var_type_t;

/*******************************************************************************
 *
 * TERVAR_var_flags_t
 *
 * DESCRIPTION:
 *  Variable node flags.
 *
 * read_only
 *  Indicates that the variable is read-only an cannot be edited.
 *
 * type
 *  See TERVAR_var_type_t.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t read_only                     : 1;
    uint8_t reserved1                     : 3;
    uint8_t type                          : 4;
  };
}
TERVAR_var_flags_t;

/*******************************************************************************
 *
 * TERVAR_input_buffer_t
 *
 * DESCRIPTION:
 *  Buffer which will be used to hold user input when editing a variable.
 *
 ******************************************************************************/

typedef struct
{
  uint8_t buffer[TERVAR_INPUT_LENGTH_MAX];
}
TERVAR_input_buffer_t;

/*******************************************************************************
 *
 * TERVAR_entry_t
 *
 * DESCRIPTION:
 *  Single variable entry variables and handlers.
 *
 *  variable
 *   Pointer to the variable to view/edit.
 *
 *  context
 *   Value which can provide additional information for the read and write
 *   handlers. For example, the register offset in I2C.
 *
 *  description
 *   NULL-terminated string which describes the variable.
 *
 *  flags
 *   See TERVAR_var_flags_t.
 *
 *  *_hal_*
 *   User-provided functions. See typedef comments.
 *
 ******************************************************************************/

typedef struct
{
  void* variable;
  uint32_t context;
  char* description;
  TERVAR_var_flags_t flags;
  TERVAR_hal_var_read_handler_t read_handler;
  TERVAR_hal_var_write_handler_t write_handler;
}
TERVAR_entry_t;

/*******************************************************************************
 *
 * TERVAR_node_t
 *
 * DESCRIPTION:
 *  Collection of variable edit entries.
 *
 * entry_list
 *  List of variable edit entries.
 *
 * length
 *  The length, in number of entries, of the entry list.
 *
 ******************************************************************************/

typedef struct
{
  TERVAR_entry_t* entry_list;
  uint8_t length;
}
TERVAR_node_t;

/*******************************************************************************
 *
 * TERVAR_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 * busy
 *  Set when the module is busy with tasks and cleared when all tasks are
 *  completed. All tasks are considered completed when the Rx queue is emptied
 *  and the state machine state is idle.
 *
 * task_state
 *  State machine state.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t busy                          : 1;
    uint8_t reserved1                     : 3;
    uint8_t task_state                    : 2;
    uint8_t reserved6                     : 2;
  };
}
TERVAR_flags_t;

/*******************************************************************************
 *
 * TERVAR_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module flags.
 *
 * rx_queue
 *  User-provided initialized instance of a QUEUE which has been setup to hold
 *  received terminal bytes.
 *
 * tx_queue
 *  User-provided initialized instance of a QUEUE which has been setup to hold
 *  terminal bytes to be sent.
 *
 * node
 *  Pointer to variable editor list.
 *
 * input_buffer
 *  Pointer to user-provided buffer for holding user input when editing a
 *  variable. Defined as a pointer to a buffer since several instances can
 *  share the same input buffer if used under a single instance of the
 *  framework.
 *
 * input_index
 *  Working index into the input buffer.
 *
 * task_entry_index
 *  Index into the active node entry list of the entry handler which is being
 *  processed.
 *
 ******************************************************************************/

typedef struct
{
  TERVAR_flags_t flags;
  QUEUE_instance_t* rx_queue;
  QUEUE_instance_t* tx_queue;
  TERVAR_node_t* node;
  TERVAR_input_buffer_t* input_buffer;
  uint8_t input_index;
  uint8_t task_entry_index;
}
TERVAR_instance_t;

/*******************************************************************************
 *
 * TERVAR_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values.
 *
 * PARAMETERS:
 *  See TERVAR_instance_t.
 *
 ******************************************************************************/

void TERVAR_initialize(TERVAR_instance_t* instance,
                       QUEUE_instance_t* rx_queue,
                       QUEUE_instance_t* tx_queue,
                       TERVAR_node_t* node,
                       TERVAR_input_buffer_t* input_buffer);

/*******************************************************************************
 *
 * TERVAR_service
 *
 * DESCRIPTION:
 *  Services the task state machine. Must be called repeatedly.
 *
 * RETURN:
 *  False if there is an ongoing task which has not completed, else, true.
 *
 ******************************************************************************/

bool TERVAR_service(TERVAR_instance_t* instance);

/*******************************************************************************
 *
 * TERVAR_is_busy
 *
 * DESCRIPTION:
 *  Determines if an instance is currently busy with a task.
 *
 * RETURN:
 *  True if the instance is busy with a task, else, false.
 *
 ******************************************************************************/

bool TERVAR_is_busy(TERVAR_instance_t* instance);

#ifdef __cplusplus
}
#endif
#endif // TERVAR_J_H

/*******************************************************************************
 *
 *  Collection of general purpose utilities (small helper functions).
 *
 ******************************************************************************/

#ifndef UTILITIES_J_H
#define UTILITIES_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Asserts can be turned on by defining the UTILITIES_ASSERT_ON macro in the
 * project build properties. When asserts are turned off, they will be replaced
 * with nothing.
 */

#if defined (UTILITIES_ASSERT_ON)
#define UTILS_ASSERT(Z)  UTILITIES_assert(Z)
#else
#define UTILS_ASSERT(Z)
#endif

/*
 * Shortcut for byte-swapping operation for non-pointer types.
 */

#define UTILS_BYTE_SWAP(Z)  UTILITIES_swap_byte_order((uint8_t*)(&Z), sizeof(Z))

/*
 * MIN and MAX math operations.
 */

#define UTILS_MIN(Y, Z)  ((Y < Z) ? Y : Z)
#define UTILS_MAX(Y, Z)  ((Y < Z) ? Z : Y)

/*******************************************************************************
 *
 * UTILITIES_assert
 *
 * DESCRIPTION:
 *  Asserts that a given argument is true. If it is not true, enters into an
 *  infinite loop. This function is meant to be used during debugging and
 *  turned off for production builds.
 *
 * PARAMETERS:
 *  assertion
 *   The assertion to be checked. User should normally pass in an inline
 *   comparison which returns a boolean result.
 *
 ******************************************************************************/

void UTILITIES_assert(bool assertion);

/*******************************************************************************
 *
 * UTILITIES_memclear
 *
 * DESCRIPTION:
 *  Lite alternative for memset in the string library which writes all 0's to
 *  a memory buffer.
 *
 * PARAMETERS:
 *  start_addr
 *   The starting address of the memory to clear.
 *
 *  length
 *   The number of bytes to clear.
 *
 ******************************************************************************/

void UTILITIES_memclear(void* start_addr, uint32_t length);

/*******************************************************************************
 *
 * UTILITIES_memset
 *
 * DESCRIPTION:
 *  Lite alternative for memset in the string library.
 *
 * PARAMETERS:
 *  start_addr
 *   The starting address of the memory to copy.
 *
 *  value
 *   Value to write for every byte.
 *
 *  length
 *   The number of bytes to copy.
 *
 ******************************************************************************/

void UTILITIES_memset(void* start_addr, uint8_t value, uint32_t length);

/*******************************************************************************
 *
 * UTILITIES_memcpy
 *
 * DESCRIPTION:
 *  Lite alternative for memcpy in the string library.
 *
 * PARAMETERS:
 *  dest_addr
 *   The starting destination address.
 *
 *  src_addr
 *   The starting source address of the memory to copy.
 *
 *  length
 *   The number of bytes to copy.
 *
 ******************************************************************************/

void UTILITIES_memcpy(void* dest_addr, void* src_addr, uint32_t length);

/*******************************************************************************
 *
 * UTILITIES_memcmp
 *
 * DESCRIPTION:
 *  Lite alternative for memcmp in the string library.
 *
 * PARAMETERS:
 *  a_addr
 *   The starting address of memory block a.
 *
 *  b_addr
 *   The starting address of memory block b.
 *
 *  length
 *   The number of bytes to compare.
 *
 * RETURN:
 *  0 if the memory blocks are equal, -1 if block a is less than block b,
 *  and 1 if block a is greater than block b.
 *
 ******************************************************************************/

int8_t UTILITIES_memcmp(void* a_addr, void* b_addr, uint32_t length);

/*******************************************************************************
 *
 * UTILITIES_strlen
 *
 * DESCRIPTION:
 *  Lite alternative for strlen in the string library. Counts the number of
 *  characters up to, but not including, the NULL terminator.
 *
 * PARAMETERS:
 *  str
 *   Input string.
 *
 * RETURN:
 *  The number of characters up to, but not including, the NULL terminator.
 *
 ******************************************************************************/

uint32_t UTILITIES_strlen(char* str);

/*******************************************************************************
 *
 * UTILITIES_strnlen
 *
 * DESCRIPTION:
 *  Lite alternative for strlen_s in the string library. Counts the number of
 *  characters up to, but not including, the NULL terminator or until a maximum
 *  length is reached.
 *
 * PARAMETERS:
 *  str
 *   Input string.
 *
 *  length
 *   Maximum characters to compare.
 *
 * RETURN:
 *  The number of characters up to, but not including, the NULL terminator or
 *  the maximum length if no NULL terminator is reached.
 *
 ******************************************************************************/

uint32_t UTILITIES_strnlen(char* str, uint32_t length);

/*******************************************************************************
 *
 * UTILITIES_strncpy
 *
 * DESCRIPTION:
 *  Lite alternative for strncpy in the string library. Copies up to (n - 1)
 *  bytes, or up to the source string NULL terminator, whichever comes first,
 *  from a source string to a destination string.
 *
 * PARAMETERS:
 *  dest_str
 *   The starting address of the destination string.
 *
 *  src_str
 *   The starting address of the source string.
 *
 *  n
 *   The maximum length of the destination string, including a space for the
 *   NULL terminator. This cannot be zero.
 *
 * RETURNS:
 *  The number of bytes written, including the NULL terminator.
 *
 ******************************************************************************/

uint32_t UTILITIES_strncpy(char* dest_str, char* src_str, uint32_t n);

/*******************************************************************************
 *
 * UTILITIES_strncmp
 *
 * DESCRIPTION:
 *  Lite alternative for strncmp in the string library.
 *
 * PARAMETERS:
 *  a_str
 *   The starting address of string a.
 *
 *  b_addr
 *   The starting address of string b.
 *
 *  length
 *   The maximum number of characters to compare in the case that a NULL-
 *   terminator is not detected in either string first.
 *
 * RETURN:
 *  0 if the strings are equal, -1 if string a is less than string b, and 1 if
 *  string a is greater than string b.
 *
 ******************************************************************************/

int8_t UTILITIES_strncmp(char* a_str, char* b_str, uint32_t length);

/*******************************************************************************
 *
 * UTILITIES_absolute_value
 *
 * DESCRIPTION:
 *  Calculates the absolute value of an input integer up to 63-bits with the
 *  MSB sign (0x7FFFFFFFFFFFFFFF = 9223372036854775807).
 *
 * PARAMETERS:
 *  value
 *   The value which is passed through the absolute value algorithm.
 *
 * RETURN:
 *  The absolute value of value.
 *
 ******************************************************************************/

uint64_t UTILITIES_absolute_value(int64_t value);

/*******************************************************************************
 *
 * UTILITIES_swap_byte_order
 *
 * DESCRIPTION:
 *  Swaps the byte order of an array of bytes. The array could represent
 *  anything from a 2-byte integer to an array of N elements.
 *
 * PARAMETERS:
 *  byte_ary
 *   Pointer to the byte array which will be byte order swapped.
 *
 *  length
 *   Length of the passed in array in bytes.
 *
 ******************************************************************************/

void UTILITIES_swap_byte_order(uint8_t* byte_ary, uint32_t length);

/*******************************************************************************
 *
 * UTILITIES_is_ascii_numeric
 *
 * DESCRIPTION:
 *  Determines if the provided character is a numeric ASCII value '0' - '9'.
 *
 * PARAMETERS:
 *  value
 *   The ASCII character to evaluate.
 *
 * RETURN:
 *  True if the character is ASCII numeric, else, false.
 *
 ******************************************************************************/

bool UTILITIES_is_ascii_numeric(char value);

/*******************************************************************************
 *
 * UTILITIES_is_ascii_alpha_lower
 *
 * DESCRIPTION:
 *  Determines if the provided character is a lower-case ASCII letter value
 *  'a' - 'z'.
 *
 * PARAMETERS:
 *  value
 *   The ASCII character to evaluate.
 *
 * RETURN:
 *  True if the character is ASCII lower-case letter, else, false.
 *
 ******************************************************************************/

bool UTILITIES_is_ascii_alpha_lower(char value);

/*******************************************************************************
 *
 * UTILITIES_is_ascii_alpha_upper
 *
 * DESCRIPTION:
 *  Determines if the provided character is a upper-case ASCII letter value
 *  'A' - 'Z'.
 *
 * PARAMETERS:
 *  value
 *   The ASCII character to evaluate.
 *
 * RETURN:
 *  True if the character is ASCII upper-case letter, else, false.
 *
 ******************************************************************************/

bool UTILITIES_is_ascii_alpha_upper(char value);

/*******************************************************************************
 *
 * UTILITIES_is_ascii_alpha_numeric
 *
 * DESCRIPTION:
 *  Determines if the provided character is an ASCII letter, upper or lower, or
 *  if it is a ASCII numeric value.
 *
 * PARAMETERS:
 *  value
 *   The ASCII character to evaluate.
 *
 * RETURN:
 *  True if the character is ASCII letter or numeric, else, false.
 *
 ******************************************************************************/

bool UTILITIES_is_ascii_alpha_numeric(char value);

/*******************************************************************************
 *
 * UTILITIES_is_ascii_hex_numeric
 *
 * DESCRIPTION:
 *  Determines if the provided character is a valid hex-decimal ASCII character,
 *  upper or lower case for letters A through F.
 *
 * PARAMETERS:
 *  value
 *   The ASCII character to evaluate.
 *
 * RETURN:
 *  True if the character is a valid ASCII hex-decimal value, else, false.
 *
 ******************************************************************************/

bool UTILITIES_is_ascii_hex_numeric(char value);

/*******************************************************************************
 *
 * UTILITIES_is_ascii_binary_numeric
 *
 * DESCRIPTION:
 *  Determines if the provided character is a valid binary ASCII character,
 *  either a '1' or a '0'.
 *
 * PARAMETERS:
 *  value
 *   The ASCII character to evaluate.
 *
 * RETURN:
 *  True if the character is a valid ASCII binary value, else, false.
 *
 ******************************************************************************/

bool UTILITIES_is_ascii_binary_numeric(char value);

/*******************************************************************************
 *
 * UTILITIES_parse_integer
 *
 * DESCRIPTION:
 *  Parses an integer, up to a signed 64-bit value, from a provided character
 *  string. The parse function supports decimal, hex-decimal, and binary input.
 *  Hex-decimal should be preceeded with 0x or 0X, and binary with a 0b or 0B.
 *  Non-applicable characters before or after the first detected numerical
 *  value or negative sign '-' will be ignored. In a string containing two
 *  integers separated by junk characters, only the first integer will be
 *  parsed.
 *
 * PARAMETERS:
 *  input
 *   Character string which contains the integer value in ASCII format. User
 *   may specify hex-decimal input by preceding the value with a 0x or 0X and
 *   binary input with 0b or 0B. The negative sign '-' will also be considered
 *   part of the valid integer string if found.
 *
 *  length
 *   Length, in bytes, of the input buffer. The input string will be evaluated
 *   until a NULL-terminator is detected or until this length is met.
 *
 * RETURN:
 *  The parsed integer value. The user can cast the result into the appropriate
 *  data type.
 *
 ******************************************************************************/

int64_t UTILITIES_parse_integer(char* input, uint8_t length);

/*******************************************************************************
 *
 * UTILITIES_crc16
 *
 * DESCRIPTION:
 *  Calculates the CRC16 (IBM version) of a provided buffer. The initial CRC
 *  is taken as the first parameter so that large buffers can be split up and
 *  sent through this function in chunks. For the first call, the user should
 *  start with a CRC value of 0.
 *
 * PARAMETERS:
 *  crc
 *   The CRC-seed (0) or on-going CRC value if this function is called multiple
 *   times for a large buffer split into smaller parts.
 *
 *  buffer
 *   Data buffer to send through the CRC algorithm.
 *
 *  length
 *   Length, in bytes, of the buffer.
 *
 * RETURN:
 *  Calculated CRC-16 value.
 *
 ******************************************************************************/

uint16_t UTILITIES_crc16(uint16_t crc, void* buffer, uint32_t length);

/*******************************************************************************
 *
 * UTILITIES_cidr_to_netmask
 *
 * DESCRIPTION:
 *  Converts the cidr (Classlesss Inter-Domain Routing) into the equivalent 32-
 *  bit netmask.
 *
 * PARAMETERS:
 *  cidr
 *   Classlesss Inter-Domain Routing is shorthand for the subnet mask which
 *   is represented by /n where n can be 0-31 (the LSB must not be used for
 *   the subnet (i.e. /32) since at least one address is needed for the IP).
 *
 * RETURN:
 *  32-bit subnet mask.
 *
 ******************************************************************************/

uint32_t UTILITIES_cidr_to_netmask(uint8_t cidr);

/*******************************************************************************
 *
 * UTILITIES_dummy_*
 *
 * DESCRIPTION:
 *  Collection of dummy functions. These functions are assigned to function
 *  pointers which the user had the option to populate but did not. The use of
 *  dummy functions takes the place of repetitive NULL checks.
 *
 *  The naming convention is _<returnType>_<argType0>_<argType1>_ ...
 *
 ******************************************************************************/

void UTILITIES_dummy_void_void(void);
void UTILITIES_dummy_void_bool(bool b);
bool UTILITIES_dummy_false_void(void);
bool UTILITIES_dummy_true_void(void);
bool UTILITIES_dummy_true_bool(bool b);
bool UTILITIES_dummy_false_voidp_u32(void* voidp, uint32_t u32) ;
uint32_t UTILITIES_dummy_u32_void(void);
void UTILITIES_dummy_void_u32(uint32_t u32);

#ifdef __cplusplus
}
#endif
#endif // UTILITIES_J_H

/*******************************************************************************
 *
 *  Website base templates, libraries, and other utilities.
 *
 ******************************************************************************/

#ifndef WEB_J_H
#define WEB_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Bootstrap v5.3 JavaScript.
 */

extern const uint8_t WEB_BOOTSTRAP_JS[];

/*
 * Bootstrap v5.3 CSS.
 */

extern const uint8_t WEB_BOOTSTRAP_CSS[];

#ifdef __cplusplus
}
#endif
#endif // WEB_J_H

/*******************************************************************************
 *
 *  WS2812 (GRB LEDs) protocol through SPI. Requires proper initialization and
 *  the service routine to be called repeatedly after a new task request until
 *  the task is completed.
 *
 *  MOSI - Data-In
 *
 *  Generally, most chips do not come equipped with the ability to handle the
 *  WS2812 protocol. Simply, the protocol requires a consecutive stream of
 *  24-bits per GRB LED with each bit consisting of a high and low voltage
 *  time. A bit-1 has a longer high time than a bit-0. Between each new write,
 *  the line is held low for a defined time.
 *
 *  The required times, based on documentation, are as follows:
 *
 *          _____
 *         |     |  T0L  |        T0H: (350 +- 150) nS
 *  0-Bit  | T0H |_______|        T0L: (800 +- 150) nS
 *
 *          _______
 *         |       | T1L |        T1H: (700 +- 150) nS
 *  1-Bit  |  T1H  |_____|        T1L: (600 +- 150) nS
 *
 *
 *         |    Treset   |        Treset: +50000 nS
 *  RESET  |_____________|
 *
 *
 *  However, these are the required times based on experimentation:
 *
 *          _____
 *         |     |  T0L  |        T0H: (65 < x < 500) nS
 *  0-Bit  | T0H |_______|        T0L: ((1250 - x) < y < (9000 - x)) nS
 *
 *          _______
 *         |       | T1L |        T1H: (625 < x < 1200) nS
 *  1-Bit  |  T1H  |_____|        T1L: ((1250 - x) < y < (9000 - x)) nS
 *
 *
 *         |    Treset   |        Treset: (z > 9000) nS
 *  RESET  |_____________|
 *
 *
 *  With the flexibility allowed from the experimental values, it is possible
 *  to run the protocol using a UART or SPI line. For 8-bit SPI, each GRB bit
 *  can be transmitted with 3-bits of SPI data. For example, in with an SPI
 *  frequency of 2.5MHz and period of 400nS, a GRB 0-bit would be indicated by
 *  0b100 and a GRB 1-bit would be indicated by 0b110.
 *
 *  Some chips idle the SPI line high during idle which, with this library,
 *  will cause issues with the required timeout between new writes. This can
 *  be worked around by adding an inverter IC and initializing the module to
 *  with the invert bits option.
 *
 ******************************************************************************/

#ifndef WS2812_J_H
#define WS2812_J_H

// Support C++ builds.

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The default timeout for chip select and unselect.
 */

#define WS2812_CHIP_SELECT_TIMEOUT_uS     100000U

/*
 * The number of bytes required to hold the 24-bit value of a single GRB LED.
 * A single GRB requires 9-bytes - 3-bits for each GRB bit at 24-bits equals
 * 72-bits or 9-bytes.
 */

#define WS2812_BYTES_PER_SINGLE_GRB       9U

/*******************************************************************************
 *
 * WS2812_flags_t
 *
 * DESCRIPTION:
 *  Module flags.
 *
 * busy
 *  Set when the module is busy with a task and cleared when the task is
 *  completed.
 *
 * dma_busy
 *  Set when the DMA is busy with a task and cleared when the DMA task is
 *  completed.
 *
 * task_state
 *  State machine state.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t busy                          : 1;
    uint8_t dma_busy                      : 1;
    uint8_t reserved2                     : 2;
    uint8_t task_state                    : 3;
    uint8_t reserved7                     : 1;
  };
}
WS2812_flags_t;

/*******************************************************************************
 *
 * WS2812_error_flags_t
 *
 * DESCRIPTION:
 *  Module error flags.
 *
 * timeout
 *  The maximum allowable time since the beginning of the transmit of a WS2812
 *  packet has passed.
 *
 * other
 *  All other types of errors.
 *
 ******************************************************************************/

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t timeout                       : 1;
    uint8_t other                         : 1;
    uint8_t reserved2                     : 6;
  };
}
WS2812_error_flags_t;

/*******************************************************************************
 *
 * WS2812_pre_task_callback_t
 *
 * DESCRIPTION:
 *  Function template for a user-provided function which is called when a new
 *  task is accepted and initialized but before the task actually begins. This
 *  allows the user to perform any last additional configurations before the
 *  actual logic of the task begins.
 *
 * PARAMETERS:
 *  context
 *   User-provided context.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*WS2812_pre_task_callback_t)(uint32_t);

/*******************************************************************************
 *
 * WS2812_post_task_callback_t
 *
 * DESCRIPTION:
 *  Function template for a user-provided function which is called after a
 *  task completes.
 *
 * PARAMETERS:
 *  context
 *   User-provided context.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef void (*WS2812_post_task_callback_t)(uint32_t);

/*******************************************************************************
 *
 * WS2812_hal_set_chip_select_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will enable or disable the device chip select. A boolean return is
 *  provided to allow more iteration time to select/unselect the chip. This
 *  function will continue to be called until true is returned.
 *
 *  A direct communication with WS2812 will likely not require a CS. However,
 *  if they share the SPI bus with other devices, an AND gate may be used along
 *  with CS to properly select which device is active.
 *
 * PARAMETERS:
 *  enable
 *   True to select the device, else, false.
 *
 * RETURN:
 *  True if the chip selection was completed, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - YES
 *
 ******************************************************************************/

typedef bool (*WS2812_hal_set_chip_select_t)(bool);

/*******************************************************************************
 *
 * WS2812_hal_configure_dma_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will configure and start the DMA transfer.
 *
 * PARAMETERS:
 *  src_addr
 *   Starting memory address of the data to be transferred.
 *
 *  src_length
 *   Number of bytes to transfer.
 *
 * RETURN:
 *  True if the configuration was successful and started, else, false.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef bool (*WS2812_hal_configure_dma_t)(void*, uint32_t);

/*******************************************************************************
 *
 * WS2812_hal_disable_dma_t
 *
 * DESCRIPTION:
 *  Hardware abstraction layer function template for a user-provided function
 *  which will disable the DMA.
 *
 * NOTES:
 *  Can be initialized as NULL - NO
 *
 ******************************************************************************/

typedef void (*WS2812_hal_disable_dma_t)(void);

/*******************************************************************************
 *
 * WS2812_instance_t
 *
 * DESCRIPTION:
 *  Instance data and function pointers.
 *
 * flags
 *  Module flags.
 *
 * errors
 *  Errors which occurred in the last transaction.
 *
 * utimer
 *  User-provided initialized instance of a UTIMER.
 *
 * spi
 *  User-provided initialized instance of a SERSPI.
 *
 * utimer_ticket
 *  Data structure used with the UTIMER instance.
 *
 * utimer_ticket_cs
 *  Additional utimer ticked specifically for chip select. This is needed to
 *  accomodate simultaneous timeouts.
 *
 * bus_mutex
 *  User-provided initialized instance of a BUSMUTEX.
 *
 * bus_id
 *  The BUS ID associated with the BUSMUTEX instance.
 *
 * src_buffer
 *  User-provided DMA compatible buffer.
 *
 * src_buffer_length
 *  Length of the source buffer in bytes. Each GRB LED will require 9-bytes.
 *
 * chip_select_timeout_us
 *  The time allowed for the chip select to complete before aborting. The value
 *  is initialized to the defined default, but can be directly modified by the
 *  user.
 *
 * dma_bytes_per_transfer
 *  The maximum number of bytes which can be sent in a single DMA transaction.
 *
 * dma_transfer_timeout_us
 *  The time allowed for the DMA transfer to complete before aborting.
 *
 * dma_transfer_counter
 *  The number of bytes which have been transferred by the DMA.
 *
 * dma_transfer_count
 *  The total number of bytes which need to be transferred by the DMA.
 *
 * dma_transfer_last_packet_length
 *  The length of the last data packet which will be transmitted by the DMA. It
 *  is calculated early on to avoid repetitive math during the task process.
 *
 * dma_src_buffer_offset
 *  Offset into the source buffer which is being transmitted by DMA.
 *
 * bit_code_0
 *  The SPI bits which represent a single 0-bit in the GRB color.
 *
 * bit_code_1
 *  The SPI bits which represent a single 1-bit in the GRB color.
 *
 * callback_context
 *  Context passed into the user pre/post task callbacks.
 *
 * *_task_*
 * *_hal_*
 *  User-provided functions. See typedef comments.
 *
 ******************************************************************************/

typedef struct
{
  volatile WS2812_flags_t flags;
  WS2812_error_flags_t errors;
  UTIMER_instance_t* utimer;
  SERSPI_instance_t* spi;
  UTIMER_ticket_t utimer_ticket;
  UTIMER_ticket_t utimer_ticket_cs;
  BUSMUTEX_instance_t* bus_mutex;
  BUSMUTEX_bus_id_t bus_id;
  uint8_t* src_buffer;
  uint32_t src_buffer_length;
  uint32_t chip_select_timeout_us;
  uint32_t dma_bytes_per_transfer;
  uint32_t dma_transfer_timeout_us;
  uint32_t dma_transfer_counter;
  uint32_t dma_transfer_count;
  uint32_t dma_transfer_last_packet_length;
  uint32_t dma_src_buffer_offset;
  uint8_t bit_code_0;
  uint8_t bit_code_1;
  uint32_t callback_context;
  WS2812_pre_task_callback_t pre_task_callback;
  WS2812_post_task_callback_t post_task_callback;
  WS2812_hal_set_chip_select_t set_chip_select;
  WS2812_hal_configure_dma_t configure_dma;
  WS2812_hal_disable_dma_t disable_dma;
}
WS2812_instance_t;

/*******************************************************************************
 *
 * WS2812_dma_transfer_complete_isr_handler
 *
 * DESCRIPTION:
 *  Handler for the DMA transfer complete interrupt. The user code must call
 *  this function from their DMA transfer complete ISR.
 *
 * IMPORTANT:
 *  The user MUST ensure that the DMA transmission complete interrupt is
 *  enabled and that this handler is called from that ISR.
 *
 ******************************************************************************/

void WS2812_dma_transfer_complete_isr_handler(WS2812_instance_t* instance);

/*******************************************************************************
 *
 * WS2812_initialize
 *
 * DESCRIPTION:
 *  Initializes a module instance, erasing all data structures and setting
 *  default values.
 *
 * PARAMETERS:
 *  See WS2812_instance_t.
 *
 ******************************************************************************/

void WS2812_initialize(WS2812_instance_t* instance,
                       UTIMER_instance_t* utimer,
                       SERSPI_instance_t* spi,
                       BUSMUTEX_instance_t* bus_mutex,
                       BUSMUTEX_bus_id_t bus_id,
                       uint8_t* src_buffer,
                       uint32_t src_buffer_length,
                       uint32_t dma_bytes_per_transfer,
                       uint32_t dma_transfer_timeout_us,
                       bool invert_bits,
                       WS2812_pre_task_callback_t pre_task_callback,
                       WS2812_post_task_callback_t post_task_callback,
                       WS2812_hal_set_chip_select_t set_chip_select,
                       WS2812_hal_configure_dma_t configure_dma,
                       WS2812_hal_disable_dma_t disable_dma);

/*******************************************************************************
 *
 * WS2812_begin_new_write
 *
 * DESCRIPTION:
 *  Attempts to begin a new WS2812 write task.
 *
 * RETURN:
 *  True if the task request was accepted, else, false.
 *
 ******************************************************************************/

bool WS2812_begin_new_write(WS2812_instance_t* instance);

/*******************************************************************************
 *
 * WS2812_service
 *
 * DESCRIPTION:
 *  Services the task state machine. Must be called repeatedly until the task
 *  is completed.
 *
 * RETURN:
 *  False if there is an ongoing task which has not completed, else, true.
 *
 ******************************************************************************/

bool WS2812_service(WS2812_instance_t* instance);

/*******************************************************************************
 *
 * WS2812_parse_rgb_instance
 *
 * DESCRIPTION:
 *  Parses RGB values from a user-provided RGB instance into the WS2812
 *  instance source buffer. The number of parsed RGB's will be the smaller of
 *  the WS2812 source buffer and the number of RGB LEDs.
 *
 * PARAMETERS:
 *  rgb
 *   Initialized RGB instance.
 *
 ******************************************************************************/

void WS2812_parse_rgb_instance(WS2812_instance_t* instance, RGB_instance_t* rgb);

/*******************************************************************************
 *
 * WS2812_parse_grb_array
 *
 * DESCRIPTION:
 *  Parses GRB into the WS2812 instance buffer. The number of parsed GRB's will
 *  be the smaller of the WS2812 source buffer and the number of GRB's.
 *
 * PARAMETERS:
 *  grb_array
 *   Pointer to a user-provided byte-array. The array NEEDs to be of a length
 *   divisible by 3. The first element is the Green intensity, the second the
 *   Red intensity, and the third the Blue intensity. This pattern is
 *   repeated for the duration of the array.
 *
 *  grb_array_length
 *   Total length of the array in bytes. This must be a value divisible by 3.
 *
 ******************************************************************************/

void WS2812_parse_grb_array(WS2812_instance_t* instance,
                            uint8_t* grb_array,
                            uint32_t grb_array_length);

/*******************************************************************************
 *
 * WS2812_is_busy
 *
 * DESCRIPTION:
 *  Determines if an instance is currently busy with a task.
 *
 * RETURN:
 *  True if the instance is busy with a task, else, false.
 *
 ******************************************************************************/

bool WS2812_is_busy(WS2812_instance_t* instance);

#ifdef __cplusplus
}
#endif
#endif // WS2812_J_H
