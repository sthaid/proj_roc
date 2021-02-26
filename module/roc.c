#include <linux/module.h>
#include <linux/io.h>
#include <linux/kthread.h>
#include <linux/delay.h>

//
// defines
//

//
// typedefs
//

#define GPIO_BASE_ADDR  (0xfe000000 + 0x200000)
#define TIMER_BASE_ADDR (0xfe000000 + 0x3000)

//
// variables
//

volatile unsigned int *timer_regs;
unsigned int           timer_initial_value;
unsigned int           timer_last_value;

volatile unsigned int *gpio_regs;

struct task_struct    *monitor_thread_id;

bool                   roc_proc_exit_request;
bool                   roc_proc_exitted;
uint64_t               roc_counter;
int                    roc_max_duration;

//
// prototypes
//

// this is the new kernel API
extern int run_offline_cpu(int cpu, void (*proc)(void));

int roc_init(void);
void roc_exit(void);
int monitor_thread(void *cx);
uint64_t roc_time(void);
uint64_t roc_delay(int duration);
void roc_proc(void);

//
// module info
//
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Steven Haid");
MODULE_VERSION("1.0.0");

module_init(roc_init);
module_exit(roc_exit);

// -----------------  MODULE INIT & EXIT  -------------------------------------

int roc_init(void)
{
    int rc;

    // init timer access
    timer_regs = ioremap(TIMER_BASE_ADDR, 0x1000);
    timer_initial_value = timer_last_value = timer_regs[1];
    printk("timer_regs = %px\n", timer_regs);
    printk("timer_initial_value = %d\n", timer_initial_value);

    // init gpio access
    gpio_regs = ioremap(GPIO_BASE_ADDR, 0x1000);

    // run roc_proc on cpu 3
    printk("calling run_offline_cpu(3)\n");
    rc = run_offline_cpu(3, roc_proc);
    if (rc != 0) {
        printk("ERROR run_offline_cpu rc=%d\n", rc);
        iounmap(timer_regs);
        iounmap(gpio_regs);
        return rc;
    }

    // create monitor thread, this monitors the progress of roc_proc()
    monitor_thread_id = kthread_create(monitor_thread, NULL, "roc_monitor_thread");
    wake_up_process(monitor_thread_id);

    // return success
    printk("roc_init complete\n");
    return 0;
}

void roc_exit(void)
{
    // cause roc_proc to return
    roc_proc_exit_request = true;
    while (roc_proc_exitted == false) {
        msleep(100);
    }
    msleep(100);

    // stop the monitor thread
    kthread_stop(monitor_thread_id);

    // unmap 
    iounmap(timer_regs);
    iounmap(gpio_regs);

    // done
    printk("roc_exit complete\n");
}

// -----------------  MONITOR THREAD  ----------------------------------------

int monitor_thread(void *cx)
{
    uint64_t roc_counter_last = 0;

    printk("monitor_thread starting\n");

    while (kthread_should_stop() == false) {
        // sleep 1 sec
        msleep(1000);

        // print roc_proc info
        printk("roc_counter = %lld (%d.%03d million)  roc_max_duration = %d\n",
               roc_counter-roc_counter_last, 
               (int)(roc_counter-roc_counter_last) / 1000000,
               ((int)(roc_counter-roc_counter_last) % 1000000) / 1000,
               roc_max_duration);
        roc_counter_last = roc_counter;
    }

    printk("monitor_thread terminating\n");
    return 0;
}

// -----------------  ROC PROC & SUPPORT ROUTINES ----------------------------

// the following code runs on the offline cpu
// 
// three test cases are provided

#define TEST 2

#if TEST == 1
// test:   tight loop reading gpio input register
// result: 37 million reads per sec
void roc_proc(void)
{
    unsigned char i = 0;

    // only read gpio
    while (true) {
        gpio_regs[13];

        if (++i == 0) {
            barrier();
            roc_counter += 256;
            if (roc_proc_exit_request) {
                break;
            }
        }
    }

    // ack that this routine is exitting
    roc_proc_exitted = true;
}
#endif

#if TEST == 2
// test:   read gpio input register, 
//         delay to next timer tick,
//         keep track of the max duration of the gpio read + delay
// result: 1.04 million loops per sec, max_duration=2 us
void roc_proc(void)
{
    unsigned char i = 0;
    uint64_t time_start, time_end;

    time_start = roc_time();
    while (true) {
        gpio_regs[13];
        time_end = roc_delay(0);

        if (time_end-time_start > roc_max_duration) {
            roc_max_duration = time_end-time_start;
        }
        time_start = time_end;

        if (++i == 0) {
            barrier();
            roc_counter += 256;
            if (roc_proc_exit_request) {
                break;
            }
        }
    }

    // ack that this routine is exitting
    roc_proc_exitted = true;
}
#endif

#if TEST == 3
// test:   read gpio input register, 
//         keep track of the max duration of the gpio read
// result: 4.8 million/sec  max_duration=2 us
void roc_proc(void)
{
    unsigned char i = 0;
    uint64_t time_start, time_end;

    time_start = roc_time();
    while (true) {
        gpio_regs[13];
        time_end = roc_time();

        if (time_end-time_start > roc_max_duration) {
            roc_max_duration = time_end-time_start;
        }
        time_start = time_end;

        if (++i == 0) {
            barrier();
            roc_counter += 256;
            if (roc_proc_exit_request) {
                break;
            }
        }
    }

    // ack that this routine is exitting
    roc_proc_exitted = true;
}
#endif

// - - - - - - - - -  TIME & DELAY SUPPORT - - - - - - - - - 

// returns time in us since the module was loaded
uint64_t roc_time(void)
{
    unsigned int value;
    static uint64_t high_part;

    value = timer_regs[1];

    if (value < timer_last_value) {
        high_part += ((uint64_t)1 << 32);
    }
    timer_last_value = value;

    return high_part + value - timer_initial_value;
}

// delays for duration us, and returns the time at the end of the delay
uint64_t roc_delay(int duration)
{
    uint64_t start, now;

    start = roc_time();
    while (true) {
        if ((now=roc_time()) > start + duration) {
            break;
        }
    }

    return now;
}
