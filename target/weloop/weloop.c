#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "app_button.h"
#include "nrf51_bitfields.h"
#include "app_trace.h"
#include "softdevice_handler.h"
#include "../../timer.h"
#include "../../target.h"
#include "../../spi.h"
#include "../../ext_ram.h"
#include "../../ext_flash.h"
#include "../../mlcd.h"
#include "../../buttons.h"
#include "../../battery.h"
#include "../../accel.h"
#include "../../rtc.h"
#include "../../ossw.h"

#define DEAD_BEEF                        0xDEADBEEF                                 /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#ifdef OSSW_DEBUG
		#include "app_uart.h"
#endif

///**@brief Callback function for asserts in the SoftDevice.
// *
// * @details This function will be called in case of an assert in the SoftDevice.
// *
// * @warning This handler is an example only and does not fit a final product. You need to analyze
// *          how your product is supposed to react in case of Assert.
// * @warning On assert from the SoftDevice, the system can only recover on reset.
// *
// * @param[in] line_num   Line number of the failing ASSERT call.
// * @param[in] file_name  File name of the failing ASSERT call.
// */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

#ifdef OSSW_DEBUG
void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}

void init_uart(void) {
    uint32_t err_code;
		const app_uart_comm_params_t comm_params =
      {
          RX_PIN_NUMBER,
          TX_PIN_NUMBER,
          0xFF,
          0xFF,
          APP_UART_FLOW_CONTROL_DISABLED,
          false,
          UART_BAUDRATE_BAUDRATE_Baud230400
      };

    APP_UART_FIFO_INIT(&comm_params,
                         1,
                         128,
                         uart_error_handle,
                         APP_IRQ_PRIORITY_LOW,
                         err_code);

    APP_ERROR_CHECK(err_code);
	}
#endif

	
/** @file
* @brief Software controlled SPI Master driver.
*
*
* @defgroup lib_driver_spi_master Software controlled SPI Master driver
* @{
* @ingroup nrf_drivers
* @brief Software controlled SPI Master driver.
*
* Supported features:
* - Operate two SPI masters independently or in parallel.
* - Transmit and Receive given size of data through SPI.
* - configure each SPI module separately through @ref spi_master_init.
*/

/**
 *  SPI master operating frequency
 */
typedef enum
{
    Freq_125Kbps = 0,        /*!< drive SClk with frequency 125Kbps */
    Freq_250Kbps,            /*!< drive SClk with frequency 250Kbps */
    Freq_500Kbps,            /*!< drive SClk with frequency 500Kbps */
    Freq_1Mbps,              /*!< drive SClk with frequency 1Mbps */
    Freq_2Mbps,              /*!< drive SClk with frequency 2Mbps */
    Freq_4Mbps,              /*!< drive SClk with frequency 4Mbps */
    Freq_8Mbps               /*!< drive SClk with frequency 8Mbps */
} SPIFrequency_t;

/**
 *  SPI master module number
 */
typedef enum
{
    SPI0 = 0,               /*!< SPI module 0 */
    SPI1                    /*!< SPI module 1 */
} SPIModuleNumber;

/**
 *  SPI mode
 */
typedef enum
{
    //------------------------Clock polarity 0, Clock starts with level 0-------------------------------------------
    SPI_MODE0 = 0,          /*!< Sample data at rising edge of clock and shift serial data at falling edge */
    SPI_MODE1,              /*!< sample data at falling edge of clock and shift serial data at rising edge */
    //------------------------Clock polarity 1, Clock starts with level 1-------------------------------------------
    SPI_MODE2,              /*!< sample data at falling edge of clock and shift serial data at rising edge */
    SPI_MODE3               /*!< Sample data at rising edge of clock and shift serial data at falling edge */
} SPIMode;

uint32_t * p_spi0_base_address;
uint32_t * p_spi1_base_address;

/**
 * @brief Function for initializing given SPI master with given configuration.
 *
 * After initializing the given SPI master with given configuration, this function also test if the
 * SPI slave is responding with the configurations by transmitting few test bytes. If the slave did not
 * respond then error is returned and contents of the rx_data are invalid.
 *
 * @param module_number SPI master number (SPIModuleNumber) to initialize.
 * @param mode SPI master mode (mode 0, 1, 2 or 3 from SPIMode)
 * @param lsb_first true if lsb is first bit to shift in/out as serial data on MISO/MOSI pins.
 * @return
 * @retval pointer to direct physical address of the requested SPI module if init was successful
 * @retval 0, if either init failed or slave did not respond to the test transfer
 */
static uint32_t* spi_master_init(SPIModuleNumber module_number, SPIMode mode, bool lsb_first)
{
    uint32_t config_mode;

    NRF_SPI_Type *spi_base_address = (SPI0 == module_number)? NRF_SPI0 : (NRF_SPI_Type *)NRF_SPI1;

    if(SPI0 == module_number)
    {
        /* Configure GPIO pins used for pselsck, pselmosi, pselmiso and pselss for SPI0 */
        nrf_gpio_cfg_output(SPI0_SCK);
        nrf_gpio_cfg_output(SPI0_MOSI);
        nrf_gpio_cfg_input(SPI0_MISO, NRF_GPIO_PIN_NOPULL);
        nrf_gpio_cfg_output(SPI0_SS0);
        nrf_gpio_cfg_output(SPI0_SS1);

        /* Configure pins, frequency and mode */
        spi_base_address->PSELSCK  = SPI0_SCK;
        spi_base_address->PSELMOSI = SPI0_MOSI;
        spi_base_address->PSELMISO = SPI0_MISO;
        nrf_gpio_pin_set(SPI0_SS0); /* disable Set slave select (inactive high) */
        nrf_gpio_pin_set(SPI0_SS1); /* disable Set slave select (inactive high) */
    }
    else
    {
        // Configure GPIO pins used for pselsck, pselmosi, pselmiso and pselss for SPI1
        nrf_gpio_cfg_output(SPI1_SCK);
        nrf_gpio_cfg_output(SPI1_MOSI);
        nrf_gpio_cfg_input(SPI1_MISO, NRF_GPIO_PIN_NOPULL);
        nrf_gpio_cfg_output(SPI1_SS0);

        // Configure pins, frequency and mode 
        spi_base_address->PSELSCK  = SPI1_SCK;
        spi_base_address->PSELMOSI = SPI1_MOSI;
        spi_base_address->PSELMISO = SPI1_MISO;
        nrf_gpio_pin_clear(SPI1_SS0);         // disable Set slave select (inactive low)
    }

    spi_base_address->FREQUENCY = (uint32_t) SPI_OPERATING_FREQUENCY;

    /*lint -e845 -save // A zero has been given as right argument to operator '!'" */
    /** @snippet [SPI Select mode] */
    switch (mode )
    {
       
        case SPI_MODE0:
            config_mode = (SPI_CONFIG_CPHA_Leading << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos);
            break;
        case SPI_MODE1:
            config_mode = (SPI_CONFIG_CPHA_Trailing << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos);
            break;
        case SPI_MODE2:
            config_mode = (SPI_CONFIG_CPHA_Leading << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveLow << SPI_CONFIG_CPOL_Pos);
            break;
        case SPI_MODE3:
            config_mode = (SPI_CONFIG_CPHA_Trailing << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveLow << SPI_CONFIG_CPOL_Pos);
            break;
        default:
            config_mode = 0;
            break;
    
    }
    /** @snippet [SPI Select mode] */
    /*lint -restore */

    /*lint -e845 -save // A zero has been given as right argument to operator '!'" */
    /** @snippet [SPI Select endianess] */
    if (lsb_first)
    {
        spi_base_address->CONFIG = (config_mode | (SPI_CONFIG_ORDER_LsbFirst << SPI_CONFIG_ORDER_Pos));
    }
    else
    {
        spi_base_address->CONFIG = (config_mode | (SPI_CONFIG_ORDER_MsbFirst << SPI_CONFIG_ORDER_Pos));
    }
    /** @snippet [SPI Select endianess] */
    /*lint -restore */

    spi_base_address->EVENTS_READY = 0U;

    /* Enable */
    spi_base_address->ENABLE = (SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos);

    return (uint32_t *)spi_base_address;
}

void spi_init(void)
{
    p_spi0_base_address = spi_master_init(SPI0, SPI_MODE0, false);
    p_spi1_base_address = spi_master_init(SPI1, SPI_MODE0, false);
}

/**@brief Function for application main entry.
 */
int main(void)
{
	
#ifdef OSSW_DEBUG
		init_uart();
#endif
	
	  spi_init();
	  ext_ram_init();
	
    nrf_gpio_cfg_output(LCD_ENABLE);
    nrf_gpio_cfg_output(LCD_BACKLIGHT);
    nrf_gpio_cfg_output(LCD_VOLTAGE_REG);
    nrf_gpio_pin_clear(LCD_ENABLE);
    nrf_gpio_pin_clear(LCD_BACKLIGHT);
    nrf_gpio_pin_clear(LCD_VOLTAGE_REG);
	
    mlcd_init();
    mlcd_power_on();
	
		accel_init();
	
    // Initialize.
    timer_init();
	  rtc_timer_init();
		buttons_init();
	  battery_init();

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, NULL);
	
		ossw_main();
}

void __aeabi_assert(const char * a, const char * b, int c){
}
