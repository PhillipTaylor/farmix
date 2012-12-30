#include <system.h>

/* This will keep track of how many ticks that the system
*  has been running for */
unsigned int timer_ticks = 0;

// allow kernel drivers to request callbacks when a timer
// tick executes.
#define NUM_TICK_HANDLERS 255

typedef void (*tick_handler_func)();
tick_handler_func tick_handlers[NUM_TICK_HANDLERS];
unsigned int next_tick_handler_id;

void timer_install_tick_handler(void (*tick_handler)()) {
	tick_handlers[next_tick_handler_id++] = tick_handler;
	puts("new tick handler registered\n");
}


/* Handles the timer. In this case, it's very simple: We
*  increment the 'timer_ticks' variable every time the
*  timer fires. By default, the timer fires 18.222 times
*  per second. */
void timer_handler(struct regs *r)
{
	unsigned int i;

	timer_ticks++;

	// invoke everyone who has registered a timer tick handler
	for (i = 0; i < next_tick_handler_id; i++)
		tick_handlers[i]();

	//if (timer_ticks % 18 == 0) {
	//	kprintf("One second has passed. %i tick handlers installed.\n", next_tick_handler_id);
	//}
}

/* This will continuously loop until the given time has been reached */
void timer_wait(int ticks)
{
	unsigned long eticks;

	eticks = timer_ticks + ticks;
	while(timer_ticks < eticks);
}

void timer_install()
{
	/* Installs 'timer_handler' to IRQ0 */
	irq_install_handler(0, timer_handler);
}
