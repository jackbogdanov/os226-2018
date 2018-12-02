#pragma once

#include <stdbool.h>

bool irq_save(void);

void irq_restore(bool v);
