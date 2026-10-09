#include "zephyr/sys/printk.h"
#include <zephyr/init.h>
#include <zephyr/kernel.h>

static int Scratch_init(void) {

  printk("Board Initialized\n");
  return 0;
}

SYS_INIT(Scratch_init, APPLICATION, 0)
