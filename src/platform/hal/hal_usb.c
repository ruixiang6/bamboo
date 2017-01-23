#include <platform.h>
#include <mss_gpio.h>
#include <mss_usb_std_def.h>
#include <mss_usb_device.h>
#include <mss_usb_device_cdc.h>

extern mss_usbd_user_descr_cb_t user_descriptors_cb;

static volatile bool_t usb_tx_state = PLAT_TRUE;
static volatile uint8_t usb_config_state = USB_NOT_CONFIGURED;
static hal_usb_rx_cb_handler_t usb_rx_fun_cb = PLAT_NULL;
static hal_usb_tx_cb_handler_t usb_tx_fun_cb = PLAT_NULL;
/*******************************************************************************
* Private function declarations.
*/
static void usb_init_cb(void);
static void usb_release_cb(void);
static uint8_t usb_tx_cb(uint8_t status);
static uint8_t usb_rx_cb(uint8_t status, uint32_t rx_count);
static void usb_notification_cb(uint8_t** buf_p, uint32_t* len_p);

static uint8_t usb_process_request_cb(mss_usbd_setup_pkt_t* setup_pkt,
                               uint8_t** buf,
                               uint32_t* length);
static uint8_t usb_cep_tx_cb(uint8_t status);
static uint8_t usb_cep_rx_cb(uint8_t status);

static mss_usbd_cdc_app_cb_t cdc_app_cb = 
{
	usb_init_cb,
    usb_release_cb,
    usb_process_request_cb,
    usb_tx_cb,
    usb_rx_cb,
    usb_notification_cb,
    usb_cep_tx_cb,
    usb_cep_rx_cb
};

/*******************************************************************************
 * CDC VCP line coding paramenters definitions
 */
#define MSS_USB_VCP_110_BAUD                            110u
#define MSS_USB_VCP_300_BAUD                            300u
#define MSS_USB_VCP_1200_BAUD                           1200u
#define MSS_USB_VCP_2400_BAUD                           2400u
#define MSS_USB_VCP_4800_BAUD                           4800u
#define MSS_USB_VCP_9600_BAUD                           9600u
#define MSS_USB_VCP_19200_BAUD                          19200u
#define MSS_USB_VCP_38400_BAUD                          38400u
#define MSS_USB_VCP_57600_BAUD                          57600u
#define MSS_USB_VCP_115200_BAUD                         115200u
#define MSS_USB_VCP_230400_BAUD                         230400u
#define MSS_USB_VCP_460800_BAUD                         460800u
#define MSS_USB_VCP_921600_BAUD                         921600u

#define MSS_USB_VCP_ONE_STOP_BIT                        ((uint8_t) 0x00)
#define MSS_USB_VCP_ONEHALF_STOP_BIT                    ((uint8_t) 0x01)
#define MSS_USB_VCP_TWO_STOP_BITS                       ((uint8_t) 0x02)

#define MSS_USB_VCP_NO_PARITY                           ((uint8_t) 0x00)
#define MSS_USB_VCP_ODD_PARITY                          ((uint8_t) 0x01)
#define MSS_USB_VCP_EVEN_PARITY                         ((uint8_t) 0x02)
#define MSS_USB_VCP_MARK_PARITY                         ((uint8_t) 0x03)
#define MSS_USB_VCP_SPACE_PARITY                        ((uint8_t) 0x04)

#define MSS_USB_VCP_DATA_5_BITS                         ((uint8_t) 0x05)
#define MSS_USB_VCP_DATA_6_BITS                         ((uint8_t) 0x06)
#define MSS_USB_VCP_DATA_7_BITS                         ((uint8_t) 0x07)
#define MSS_USB_VCP_DATA_8_BITS                         ((uint8_t) 0x08)
#define MSS_USB_VCP_DATA_16_BITS                        ((uint8_t) 0x10)

typedef struct vcp_linecoding_params_t {
    uint32_t bitrate;
    uint8_t  format;
    uint8_t  paritytype;
    uint8_t  datatype;
} vcp_linecoding_params_t;

const vcp_linecoding_params_t g_linecoding_params = {
    MSS_USB_VCP_9600_BAUD,
    MSS_USB_VCP_ONE_STOP_BIT,
    MSS_USB_VCP_NO_PARITY,
    MSS_USB_VCP_DATA_8_BITS
};

void hal_usb_init(void)
{
  	MSS_GPIO_config(MSS_GPIO_30 , MSS_GPIO_OUTPUT_MODE);

    /*Keep USB PHY out of Reset*/
    MSS_GPIO_set_output(MSS_GPIO_30 , 1);

	/* Assign call-back function handler structure needed by USB Device Core driver */
    MSS_USBD_set_descr_cb_handler(&user_descriptors_cb);

    /*Assign call-back function handler structure needed by USBD-CDC driver.*/
    MSS_USBD_CDC_init(&cdc_app_cb, MSS_USB_DEVICE_HS);

    /*Initialize USBD driver driver */
    MSS_USBD_init(MSS_USB_DEVICE_HS);
}

void hal_usb_deinit(void)
{
  	MSS_GPIO_config(MSS_GPIO_30 , MSS_GPIO_OUTPUT_MODE);

    /*Keep USB PHY out of Reset*/
    MSS_GPIO_set_output(MSS_GPIO_30 , 0);
}

bool_t hal_usb_tx(uint8_t* buf, uint32_t length, hal_usb_tx_cb_handler_t func)
{
	if((usb_config_state >= USB_CONFIGURED) 
		&& (buf != (uint8_t*)0)
		&& (func != PLAT_NULL))
    {
        usb_tx_state = PLAT_FALSE;
		usb_tx_fun_cb = func;
		//hal_uart_send_char(UART_DEBUG, 'A');
        MSS_USBD_CDC_tx(buf, length);
        return PLAT_TRUE;
    }
    else
    {
        return PLAT_FALSE;
    }
}

bool_t hal_usb_rx(uint8_t* buf, uint32_t length, hal_usb_rx_cb_handler_t func)
{
	if((usb_config_state >= USB_CONFIGURED) 
		&& (func != PLAT_NULL) 
		&& (buf != PLAT_NULL))
    {
        usb_rx_fun_cb = func;
        MSS_USBD_CDC_rx_prepare(buf, length);
        return PLAT_TRUE;
    }
    else
    {
        return PLAT_FALSE;
    }
}

bool_t hal_usb_tx_done(void)
{
	return usb_tx_state;
}

uint8_t hal_usb_config_state(void)
{
	return usb_config_state;
}

static void usb_init_cb(void)
{
	usb_tx_state = PLAT_TRUE;	
	usb_config_state = USB_CONFIGURED;
}

static void usb_release_cb(void)
{
	usb_config_state = USB_NOT_CONFIGURED;
}

static uint8_t usb_tx_cb(uint8_t status)
{
	/*Ignore underrun error*/
	if(status < 2)
    {
        usb_tx_state = PLAT_TRUE;
		if (usb_tx_fun_cb)
		{
			(* usb_tx_fun_cb)();
		}
		//hal_uart_send_char(UART_DEBUG, 'S');
    }
    else
    {
        DBG_ASSERT(0);
    }
	
    return PLAT_TRUE;	
}

static uint8_t usb_rx_cb(uint8_t status, uint32_t rx_count)
{
	/*Call the Handler function provided by user with hal_usb_rx() function*/
	if (usb_rx_fun_cb)
	{
		(* usb_rx_fun_cb)(rx_count, status);
		return PLAT_TRUE;
	}
	else
	{
		return PLAT_FALSE;
	}    
}

static void usb_notification_cb(uint8_t** buf_p, uint32_t* len_p)
{
	*buf_p = (uint8_t*)0;
    *len_p = 0;
}

static uint8_t usb_process_request_cb(mss_usbd_setup_pkt_t* setup_pkt,
                               uint8_t** buf,
                               uint32_t* length)
{
	uint8_t result = USB_FAIL;
#if 1	
  	DBG_TRACE("setup_pkt reqtypr=%x,req=%x\r\n", setup_pkt->request_type, setup_pkt->request);

	switch(setup_pkt->request)
    {
	case USB_CDC_SET_LINE_CODING:
        *buf = (uint8_t*)&g_linecoding_params;
        *length = 7u;
        result = USB_SUCCESS;
        break;

    case USB_CDC_GET_LINE_CODING:
        *buf = (uint8_t*)&g_linecoding_params;
        *length = 7u;
		result = USB_SUCCESS;
    break;

    case USB_CDC_SET_CONTROL_LINE_STATE:       
        DBG_TRACE("vcp_serial_state = %x\r\n", (uint8_t)setup_pkt->value);
		result = USB_SUCCESS;
    break;

	default:
        result = USB_FAIL;
    break;
	}
#endif	
	return USB_SUCCESS;
}

static uint8_t usb_cep_tx_cb(uint8_t status)
{
	return 1;
}

static uint8_t usb_cep_rx_cb(uint8_t status)
{
	static bool_t detect_host = 0;

	if(detect_host == 0)
    {
        usb_config_state = USB_HOST_APP_READY;
        detect_host = 1;
    }
	
	return 1;
}