/*
 * os_config.h — parametry jądra ustawiane w czasie kompilacji.
 */
#ifndef OS_CONFIG_H
#define OS_CONFIG_H

#define OS_TICK_HZ            1000u   /* tick co 1 ms */

/* Etap 2+ */
#define OS_MAX_TASKS          8u
#define OS_MAX_PRIORITIES     32u     /* bitmapa gotowych zadań mieści się w jednym słowie → CLZ */
#define OS_IDLE_STACK_WORDS   128u

#endif /* OS_CONFIG_H */
