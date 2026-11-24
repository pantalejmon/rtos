/*
 * main.c — etap 1: bare-metal start.
 *
 * Jeszcze bez zadań: scheduler kooperacyjny z dokumentu 01 (krok 3).
 * Każda czynność to funkcja, która sprawdza swój czas i od razu wraca.
 * Gdy w etapie 2–3 powstaną zadania, każda z tych funkcji stanie się
 * osobnym zadaniem z własną pętlą while(1) i os_delay().
 */
#include <stdio.h>

#include "bsp.h"
#include "os.h"
#include "port.h"

typedef struct {
    void (*fn)(void);
    uint32_t period;
    uint32_t last;
} job_t;

static void led_job(void)
{
    bsp_led_toggle();
}

static void heartbeat_job(void)
{
    static uint32_t n;
    printf("tick %lu, heartbeat %lu\n", (unsigned long)os_ticks(), (unsigned long)++n);
}

static void echo_job(void)
{
    int c = bsp_console_getc();
    if (c >= 0)
        printf("odebrano: '%c' (0x%02x)\n", (c >= 0x20 && c < 0x7f) ? c : '.', c);
}

static job_t jobs[] = {
    { led_job,        500, 0 },
    { heartbeat_job, 1000, 0 },
    { echo_job,         1, 0 },
};

#define N_JOBS (sizeof jobs / sizeof jobs[0])

int main(void)
{
    bsp_init();
    port_init(bsp_core_clock_hz(), OS_TICK_HZ);

    printf("\n=== rtos — etap 1: bare-metal start ===\n");
    printf("plytka: %s, rdzen: %lu MHz, tick: %u Hz\n",
           bsp_board_name(), (unsigned long)(bsp_core_clock_hz() / 1000000u), OS_TICK_HZ);

    for (;;) {
        uint32_t now = os_ticks();
        for (unsigned i = 0; i < N_JOBS; i++) {
            if (now - jobs[i].last >= jobs[i].period) {
                jobs[i].last += jobs[i].period;
                jobs[i].fn();
            }
        }
        port_wait_for_interrupt();      /* śpij do następnego ticka zamiast kręcić się w pętli */
    }
}
