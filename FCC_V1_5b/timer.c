

struct timer_descriptor       TIMER_0;
static struct timer_task TIMER_task1;
static volatile int TIMER_ready;
/**
 * Example of using TIMER.
 */
static void TIMER_task1_cb(const struct timer_task *const timer_task)
{
	TIMER_ready = 1;
}

void myTIMER_setup(uint32_t duration) {
	TIMER_task1.interval = duration;
	TIMER_task1.cb       = TIMER_task1_cb;
	TIMER_task1.mode     = TIMER_TASK_REPEAT;

	timer_add_task(&TIMER_0, &TIMER_task1);
}

void myTIMER_start() {
	TIMER_ready = 0;
	timer_start(&TIMER_0);
}

void myTIMER_stop() {
	// timer_remove_task(&TIMER_0, &TIMER_task1);
	timer_stop(&TIMER_0);
}

/**
 * \brief Timer initialization function
 *
 * Enables Timer peripheral, clocks and initializes Timer driver
 */
static void TIMER_0_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBA, RTC);
	_gclk_enable_channel(RTC_GCLK_ID, CONF_GCLK_RTC_SRC);
	timer_init(&TIMER_0, RTC, _rtc_get_timer());
}
