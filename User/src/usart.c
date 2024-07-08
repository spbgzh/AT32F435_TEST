#include "usart.h"
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

/* support printf function, usemicrolib is unnecessary */
#if (__ARMCC_VERSION > 6000000)
__asm(".global __use_no_semihosting\n\t");
void _sys_exit(int x)
{
  x = x;
}
/* __use_no_semihosting was requested, but _ttywrch was */
void _ttywrch(int ch)
{
  ch = ch;
}
FILE __stdout;
#else
#ifdef __CC_ARM
#pragma import(__use_no_semihosting)
struct __FILE
{
  int handle;
};
FILE __stdout;
void _sys_exit(int x)
{
  x = x;
}
/* __use_no_semihosting was requested, but _ttywrch was */
void _ttywrch(int ch)
{
  ch = ch;
}
#endif
#endif

#if defined(__GNUC__) && !defined(__clang__)
#define GETCHAR_PROTOTYPE int __io_getchar(void)
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define GETCHAR_PROTOTYPE int fgetc(FILE *f)
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

/**
 * @brief  retargets the c library printf function to the usart.
 * @param  none
 * @retval none
 */
PUTCHAR_PROTOTYPE
{
  while (usart_flag_get(USART1, USART_TDBE_FLAG) == RESET)
    ;
  usart_data_transmit(USART1, ch);
  return ch;
}

GETCHAR_PROTOTYPE
{
  return 0;
}

#if defined(__GNUC__) && !defined(__clang__)
__attribute__((weak)) int _write(int file, char *ptr, int len)
{
  (void)file;
  int DataIdx;
  for (DataIdx = 0; DataIdx < len; DataIdx++)
  {
    __io_putchar(*ptr++);
  }
  return len;
}
#endif

__attribute__((weak)) int _read(int file, char *ptr, int len)
{
  (void)file;
  int DataIdx;
  for (DataIdx = 0; DataIdx < len; DataIdx++)
  {
    *ptr++ = __io_getchar();
  }
  return len;
}

__attribute__((weak)) int _isatty(int fd)
{
  if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
    return 1;

  errno = EBADF;
  return 0;
}

__attribute__((weak)) int _close(int fd)
{
  if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
    return 0;

  errno = EBADF;
  return -1;
}

__attribute__((weak)) int _lseek(int fd, int ptr, int dir)
{
  (void)fd;
  (void)ptr;
  (void)dir;

  errno = EBADF;
  return -1;
}

__attribute__((weak)) int _fstat(int fd, struct stat *st)
{
  if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
  {
    st->st_mode = S_IFCHR;
    return 0;
  }

  errno = EBADF;
  return 0;
}

__attribute__((weak)) int _getpid(void)
{
  errno = ENOSYS;
  return -1;
}

__attribute__((weak)) int _kill(pid_t pid, int sig)
{
  (void)pid;
  (void)sig;
  errno = ENOSYS;
  return -1;
}

struct Receiver_Struct Receiver;

static void usart_print_dma_init(void)
{

  dma_init_type dma_init_struct;
  crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);
  dmamux_enable(DMA1, TRUE);
  dma_reset(DMA1_CHANNEL4);
  dma_init_struct.buffer_size = 0;
  dma_init_struct.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
  dma_init_struct.memory_base_addr = (uint32_t)0;
  dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
  dma_init_struct.memory_inc_enable = TRUE;
  dma_init_struct.peripheral_base_addr = (uint32_t)&USART1->dt;
  dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
  dma_init_struct.peripheral_inc_enable = FALSE;
  dma_init_struct.priority = DMA_PRIORITY_MEDIUM;
  dma_init_struct.loop_mode_enable = FALSE;
  dma_init(DMA1_CHANNEL4, &dma_init_struct);

  dma_reset(DMA1_CHANNEL5);
  dma_init_struct.peripheral_base_addr = (uint32_t)&USART1->dt;
  dma_init_struct.memory_base_addr = (uint32_t)Receiver.rxbuf;
  dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
  dma_init_struct.buffer_size = USART_RECEVIE_LEN;
  dma_init(DMA1_CHANNEL5, &dma_init_struct);

  dmamux_init(DMA1MUX_CHANNEL5, DMAMUX_DMAREQ_ID_USART1_RX);
  dmamux_init(DMA1MUX_CHANNEL4, DMAMUX_DMAREQ_ID_USART1_TX);
  usart_dma_receiver_enable(USART1, TRUE);
  dma_channel_enable(DMA1_CHANNEL5, TRUE);
}

static void usartdmasend(uint8_t *data, uint16_t len)
{
  DMA1_CHANNEL4->dtcnt = len;
  DMA1_CHANNEL4->maddr = (uint32_t)data;
  usart_dma_transmitter_enable(USART1, TRUE);
  dma_channel_enable(DMA1_CHANNEL4, TRUE);
  while (dma_flag_get(DMA1_FDT4_FLAG) == RESET)
    ;
  dma_flag_clear(DMA1_FDT4_FLAG);
  dma_channel_enable(DMA1_CHANNEL4, FALSE);
  usart_dma_transmitter_enable(USART1, FALSE);
}

static void usartdmarecv(uint8_t *data, uint16_t len)
{
  dma_flag_clear(DMA1_FDT5_FLAG);
  dma_channel_enable(DMA1_CHANNEL5, FALSE);
  usart_dma_receiver_enable(USART1, FALSE);
  DMA1_CHANNEL5->dtcnt = len;
  DMA1_CHANNEL5->maddr = (uint32_t)data;
  usart_dma_receiver_enable(USART1, TRUE);
  dma_channel_enable(DMA1_CHANNEL5, TRUE);
}

void usart_print_init(uint32_t baudrate)
{
  gpio_init_type gpio_init_struct;

#if defined(__GNUC__) && !defined(__clang__)
  setvbuf(stdout, NULL, _IONBF, 0);
#endif

  /* enable the uart and gpio clock */
  crm_periph_clock_enable(CRM_USART1_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);

  gpio_default_para_init(&gpio_init_struct);

  /* configure the uart tx pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = GPIO_PINS_9 | GPIO_PINS_10;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOA, &gpio_init_struct);

  gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE9, GPIO_MUX_7);
  gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE10, GPIO_MUX_7);

  usart_print_dma_init();

  nvic_irq_enable(USART1_IRQn, 0, 0);

  /*Configure UART param*/
  usart_init(USART1, baudrate, USART_DATA_8BITS, USART_STOP_1_BIT);
  usart_hardware_flow_control_set(USART1, USART_HARDWARE_FLOW_NONE);
  usart_parity_selection_config(USART1, USART_PARITY_NONE);
  usart_transmitter_enable(USART1, TRUE);
  usart_receiver_enable(USART1, TRUE);

  usart_interrupt_enable(USART1, USART_IDLE_INT, TRUE);
  usart_enable(USART1, TRUE);
}

void USART1_IRQHandler(void)
{
  uint8_t clear;
  if (usart_flag_get(USART1, USART_IDLEF_FLAG) != RESET)
  {
    clear = USART1->sts;
    clear = USART1->dt;
    clear &= 0;
    Receiver.state = 1;

    Receiver.len = USART_RECEVIE_LEN - dma_data_number_get(DMA1_CHANNEL5);

    // handle rx data
    usartdmasend(Receiver.rxbuf, Receiver.len);

    Receiver.len = 0;
    Receiver.state = 0;
    usartdmarecv(Receiver.rxbuf, USART_RECEVIE_LEN);
  }
}