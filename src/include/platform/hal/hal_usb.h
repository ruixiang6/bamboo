#ifndef HAL_USB_H_
#define HAL_USB_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define USB_NOT_CONFIGURED		0
#define USB_CONFIGURED 			1
#define USB_HOST_APP_READY		2

/*******************************************************************************
 * Data reception event handler function Prototype.
 */
typedef void (*hal_usb_rx_cb_handler_t)( uint32_t rx_count, uint32_t rx_err_status );
typedef void (*hal_usb_tx_cb_handler_t)( void );


void hal_usb_init(void);
void hal_usb_deinit(void);
bool_t hal_usb_tx(uint8_t* buf, uint32_t length, hal_usb_tx_cb_handler_t func);
bool_t hal_usb_rx(uint8_t* buf, uint32_t length, hal_usb_rx_cb_handler_t func);
bool_t hal_usb_tx_done(void);
uint8_t hal_usb_config_state(void);
  
#ifdef __cplusplus
}
#endif

#endif /* HAL_USB_H_*/
