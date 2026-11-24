/*
 * os.h — publiczne API jądra.
 *
 * Etap 1 (bare-metal start) — zaimplementowane:
 *   os_tick_isr(), os_ticks()
 *
 * Kolejne etapy dopisują resztę. Docelowe API (z docs/01-od-petli-do-rtosa.md)
 * jest niżej w komentarzu, żeby było widać, dokąd zmierzamy — odkomentowuj
 * deklaracje w miarę implementacji, wtedy kompilator pilnuje zgodności.
 */
#ifndef OS_H
#define OS_H

#include <stdint.h>

#include "os_config.h"

/* ---- Etap 1: czas ------------------------------------------------------ */

/* Wołane z SysTick_Handler co 1/OS_TICK_HZ s. Tylko dla portu. */
void os_tick_isr(void);

/* Liczba ticków od startu. Przepełnia się po ~49 dniach — porównuj różnicą:
 *   if (os_ticks() - start >= timeout) ...   (działa też przez przepełnienie) */
uint32_t os_ticks(void);

/* ---- Etap 2–5: plan API ------------------------------------------------ */
#if 0
typedef struct tcb tcb_t;
typedef void (*os_task_fn)(void *arg);

#define OS_WAIT_FOREVER 0xFFFFFFFFu

/* Etap 2–3: zadania i scheduler */
int  os_task_create(tcb_t *tcb, os_task_fn fn, void *arg, uint8_t prio,
                    uint32_t *stack, uint32_t stack_words);
void os_start(void);                          /* nie wraca */
void os_delay(uint32_t ticks);                /* zadanie → Blocked do ticka */
void os_yield(void);

/* Etap 4: synchronizacja */
typedef struct os_sem   os_sem_t;
typedef struct os_mutex os_mutex_t;           /* z dziedziczeniem priorytetu */
typedef struct os_queue os_queue_t;

void os_sem_init(os_sem_t *s, uint32_t initial);
int  os_sem_take(os_sem_t *s, uint32_t timeout);
void os_sem_give(os_sem_t *s);                /* wolno wołać z przerwania */

void os_mutex_init(os_mutex_t *m);
int  os_mutex_lock(os_mutex_t *m, uint32_t timeout);   /* NIGDY z przerwania */
void os_mutex_unlock(os_mutex_t *m);

void os_queue_init(os_queue_t *q, void *buf, uint32_t item_size, uint32_t capacity);
int  os_queue_send(os_queue_t *q, const void *item, uint32_t timeout);
int  os_queue_send_isr(os_queue_t *q, const void *item);   /* nie blokuje */
int  os_queue_receive(os_queue_t *q, void *item, uint32_t timeout);
#endif

#endif /* OS_H */
