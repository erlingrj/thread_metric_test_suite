/**************************************************************************/ 
/*                                                                        */ 
/*            Copyright (c) 1996-2016 by Express Logic Inc.               */ 
/*                                                                        */ 
/*  This Original Work may be modified, distributed, or otherwise used in */ 
/*  any manner with no obligations other than the following:              */ 
/*                                                                        */ 
/*    1. This legend must be retained in its entirety in any source code  */ 
/*       copies of this Work.                                             */ 
/*                                                                        */ 
/*    2. This software may not be used in the development of an operating */
/*       system product.                                                  */ 
/*                                                                        */  
/*  This Original Work is hereby provided on an "AS IS" BASIS and WITHOUT */ 
/*  WARRANTY, either express or implied, including, without limitation,   */ 
/*  the warranties of NON-INFRINGEMENT, MERCHANTABILITY or FITNESS FOR A  */ 
/*  PARTICULAR PURPOSE. THE ENTIRE RISK AS TO THE QUALITY OF this         */ 
/*  ORIGINAL WORK IS WITH the user.                                       */ 
/*                                                                        */ 
/*  Express Logic, Inc. reserves the right to modify this software        */ 
/*  without notice.                                                       */ 
/*                                                                        */ 
/*  Express Logic, Inc.                     info@expresslogic.com         */
/*  11423 West Bernardo Court               http://www.expresslogic.com   */
/*  San Diego, CA  92127                                                  */
/*                                                                        */
/**************************************************************************/

/**************************************************************************/
/**************************************************************************/
/**                                                                       */ 
/** Thread-Metric Component                                               */
/**                                                                       */
/**   Porting Layer (Must be completed with RTOS specifics)               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/* Include necessary files.  */

#include    <stdlib.h>
#include    <zephyr/kernel.h>
#include    <zephyr/shell/shell.h>
#include    <zephyr/app_memory/app_memdomain.h>
#include    <zephyr/sys/libc-hooks.h>
#include    <zephyr/arch/arm/aarch32/cortex_m/cmsis.h>
#include    "tm_api.h"

LOG_MODULE_REGISTER(thread_metric);

static void tm_thread_task(intptr_t exinf);

K_THREAD_DEFINE(TM_TASK_0, 512/*STACKSIZE*/, tm_thread_task, 0, NULL, NULL, 1, K_USER, -1);
K_THREAD_DEFINE(TM_TASK_1, 512/*STACKSIZE*/, tm_thread_task, 1, NULL, NULL, 1, K_USER, -1);
K_THREAD_DEFINE(TM_TASK_2, 512/*STACKSIZE*/, tm_thread_task, 2, NULL, NULL, 1, K_USER, -1);
K_THREAD_DEFINE(TM_TASK_3, 512/*STACKSIZE*/, tm_thread_task, 3, NULL, NULL, 1, K_USER, -1);
K_THREAD_DEFINE(TM_TASK_4, 512/*STACKSIZE*/, tm_thread_task, 4, NULL, NULL, 1, K_USER, -1);
K_THREAD_DEFINE(TM_TASK_5, 512/*STACKSIZE*/, tm_thread_task, 5, NULL, NULL, 1, K_USER, -1);

K_MSGQ_DEFINE(TM_MSGQ, 16, 1, 4);

K_SEM_DEFINE(TM_SEM, 1, 1);

typedef struct {
    k_tid_t   taskid;
    void      *entry_function;
} thread_t;

K_APP_DMEM(part_tm) static thread_t thread_tab[] = {
	{TM_TASK_0}, {TM_TASK_1}, {TM_TASK_2}, {TM_TASK_3}, {TM_TASK_4}, {TM_TASK_5}
};

static void
tm_thread_task(intptr_t exinf)
{
	((void (*)(void)) thread_tab[exinf].entry_function)();
}


/* This function called from main performs basic RTOS initialization, 
   calls the test initialization function, and then starts the RTOS function.  */
void  tm_initialize(void (*test_initialization_function)(void))
{
	(test_initialization_function)();
}


/* This function takes a thread ID and priority and attempts to create the
   file in the underlying RTOS.  Valid priorities range from 1 through 31, 
   where 1 is the highest priority and 31 is the lowest. If successful, 
   the function should return TM_SUCCESS. Otherwise, TM_ERROR should be returned.   */
int  tm_thread_create(int thread_id, int priority, void (*entry_function)(void))
{
	thread_tab[thread_id].entry_function = entry_function;
    k_thread_priority_set(thread_tab[thread_id].taskid, priority);
    k_thread_start(thread_tab[thread_id].taskid);
	tm_thread_suspend(thread_id);
	return TM_SUCCESS;
}


/* This function resumes the specified thread.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_thread_resume(int thread_id)
{
    k_thread_resume(thread_tab[thread_id].taskid);
	return TM_SUCCESS;
}


/* This function suspends the specified thread.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_thread_suspend(int thread_id)
{
    k_thread_suspend(thread_tab[thread_id].taskid);
	return TM_SUCCESS;
}


/* This function relinquishes to other ready threads at the same
   priority.  */
void tm_thread_relinquish(void)
{
    k_yield();
}


/* This function suspends the specified thread for the specified number
   of seconds.  If successful, the function should return TM_SUCCESS. 
   Otherwise, TM_ERROR should be returned.  */
void tm_thread_sleep(int seconds)
{
    k_sleep(K_MSEC(seconds * 1000U));
}


/* This function creates the specified queue.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_queue_create(int queue_id)
{
	return TM_SUCCESS;
}


/* This function sends a 16-byte message to the specified queue.  If successful, 
   the function should return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_queue_send(int queue_id, unsigned long *message_ptr)
{
    k_msgq_put(&TM_MSGQ, message_ptr, K_FOREVER);
	return TM_SUCCESS;
}


/* This function receives a 16-byte message from the specified queue.  If successful, 
   the function should return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_queue_receive(int queue_id, unsigned long *message_ptr)
{
    k_msgq_get(&TM_MSGQ, message_ptr, K_FOREVER);
	return TM_SUCCESS;
}


/* This function creates the specified semaphore.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_semaphore_create(int semaphore_id)
{
	return TM_SUCCESS;
}


/* This function gets the specified semaphore.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_semaphore_get(int semaphore_id)
{
    if (k_sem_take(&TM_SEM, K_NO_WAIT) == 0) {
		return TM_SUCCESS;
    }
	return TM_ERROR;
}


/* This function waits the specified semaphore.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_semaphore_wait(int semaphore_id)
{
    if (k_sem_take(&TM_SEM, K_FOREVER) == 0) {
		return TM_SUCCESS;
    }
	return TM_ERROR;
}


/* This function puts the specified semaphore.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_semaphore_put(int semaphore_id)
{
    k_sem_give(&TM_SEM);
	return TM_SUCCESS;
}


/* This function puts the specified semaphore.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_semaphore_put_from_isr(int semaphore_id)
{
    k_sem_give(&TM_SEM);
	return TM_SUCCESS;
}

#ifdef INTERRUPT_TEST
void tm_interrupt_raise(void)
{
    // NVIC->STIR = SWI3_EGU3_IRQn;
}

void
tm_interrupt_handler()
{
	((void (*)(void)) test_interrupt_handler)();
}
#endif

#if defined(TEST_COOPERATIVE)
    #include "tm_cooperative_scheduling_test.c"
#elif defined(TEST_MESSAGE)
    #include "tm_message_processing_test.c"
#endif
void
tm_main_task(intptr_t exinf)
{
	printf("Thread-Metric Test Suite starts.\n");
    _main();
    k_thread_abort(k_current_get());
}

K_THREAD_DEFINE(TM_MAIN, 512/*STACKSIZE*/, tm_main_task, NULL, NULL, NULL, CONFIG_MAIN_THREAD_PRIORITY, K_USER, 0);
