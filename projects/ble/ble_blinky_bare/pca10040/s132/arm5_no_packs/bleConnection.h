#define UART_TX_BUF_SIZE        256
#define UART_RX_BUF_SIZE        1


#define CENTRAL_LINK_COUNT              0                                           /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT           1                                           /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define ADVERTISING_LED_PIN             BSP_LED_0_MASK                              /**< Is on when device is advertising. */
#define CONNECTED_LED_PIN               BSP_LED_1_MASK                              /**< Is on when device has connected. */
