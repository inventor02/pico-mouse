#include <stdint.h>

#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"

#include "config.h"

#define FLG_LMB 0b001
#define FLG_RMB 0b010
#define FLG_MMB 0b100

static volatile uint8_t flags = 0x00;

void send_hid_report()
{
  if (!tud_hid_ready()) return;

  tud_hid_mouse_report(REPORT_ID_MOUSE, flags, 0, 0, 0, 0);
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, const uint8_t *buffer, uint16_t buffer_size) {}

uint16_t tud_hid_get_report_cb(uint8_t inst, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t buffer_size) 
{  
  return 0;
}

void gpio_button_down(uint gpio) 
{
  switch (gpio)
  {
    case PIN_LMB:
      flags = flags | FLG_LMB;
      break;
    case PIN_MMB:
      flags = flags | FLG_MMB;
      break;
    case PIN_RMB:
      flags = flags | FLG_RMB;
      break;
    default:
  }
}

void gpio_button_up(uint gpio) 
{
  switch (gpio)
  {
    case PIN_LMB:
      flags = flags & ~FLG_LMB;
      break;
    case PIN_MMB:
      flags = flags & ~FLG_MMB;
      break;
    case PIN_RMB:
      flags = flags & ~FLG_RMB;
      break;
    default:
  }
}

void gpio_cb(uint gpio, uint32_t event_mask)
{
  printf("%d GPIO CB\n", gpio);

  if ((event_mask & GPIO_IRQ_EDGE_FALL) == GPIO_IRQ_EDGE_FALL)
  {
    gpio_button_down(gpio);
  }
  else if ((event_mask & GPIO_IRQ_EDGE_RISE) == GPIO_IRQ_EDGE_RISE)
  {
    gpio_button_up(gpio);
  }
}

void init_pin(uint gpio)
{
  gpio_init(gpio);
  gpio_pull_up(gpio);
  gpio_set_irq_enabled_with_callback(gpio, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &gpio_cb);
}

void main()
{
  stdio_init_all();

  board_init();
  tusb_init();

  init_pin(PIN_LMB);
  init_pin(PIN_MMB);
  init_pin(PIN_RMB);

  for (;;) 
  {
    printf("flags: 0x%x\n", flags);
    tud_task();
    send_hid_report();
    sleep_ms(100);
  }
}