#include "at32f435_437_clock.h"
#include "usart.h"
#include "psram.h"

int main(void)
{
  system_clock_config();
  usart_print_init(115200);

  ly68l6400_init();
  uint16_t id = ly68l6400_readID();
  printf("%04X\n", id);
  ly68l6400_xip_mode_init();

  uint8_t *pArray = (uint8_t *)QSPI2_MEM_BASE;
  for (int i = 0; i < 50; i++)
  {
    pArray[i] = i * 2;
  }
  for (int i = 0; i < 50; i++)
  {
    printf("pArray[%d] = %d\n", i, pArray[i]);
  }

  while (1)
  {
  }
}
