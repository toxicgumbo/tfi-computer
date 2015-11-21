#include "platform.h"
#include "util.h"
#include "decoder.h"
#include "scheduler.h"
#include "config.h"
#include "table.h"
#include "adc.h"
#include <libopencm3/stm32/gpio.h>

struct decoder d;

int main() {
  decoder_init(&d);
  platform_init(&d, NULL);
  while (1) {
    adc_process();
    if (d.needs_decoding) {
      d.decode(&d);

      if (decoder_valid(&d)) {
        float adv = interpolate_table_twoaxis(config.timing, d.rpm, 
            config.adc[ADC_MAP].processed_value);
        for (unsigned int e = 0; e < config.num_events; ++e) {
          switch(config.events[e].type) {
            case IGNITION_EVENT:
              schedule_ignition_event(&config.events[e], &d, (degrees_t)adv, 2000);
              break;
            case FUEL_EVENT:
              schedule_fuel_event(&config.events[e], &d, 0);
              break;
            case ADC_EVENT:
              schedule_adc_event(&config.events[e], &d);
              break;
          }

        }
      }
    }
  }

  return 0;
}

