/******************************************************************************
 * @internal
 * @file        rCFD_report.h
 * @copyright   (C) 2022
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Daniel Queteschiner
 *****************************************************************************/

#ifndef RCFD_REPORT
#define RCFD_REPORT

#include <stdint.h>
#include <time.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#if !defined(RCFD_LIB_MPI)
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#else
#include <fcntl.h>
#endif
#endif
#include "rCFD_types.h"
#include "rCFD_macros.h"
#include "rCFD_memory.h"

static const char* Time_Unit_Names[] =
{
    "ns",
    "mus",
    "ms",
    "s"
};

static const char* Timer_Names[] =
{
    "(global)",
    "(init)",
    "(read_db)",
    "(read_shift)",
    "(run)",
    "(run_upd.)",
    "(run_shift)",
    "(run_swap)",
    "(run_bal.)",
    "(user)"
};

#if RP_NODE
static const char* Report_Category_Names[] =
{
    "(generic)",
    "(initialize)",
    "(create_shift)",
    "(write_shift)",
    "(write_fields)",
    "(write_recurrence)",
    "(write_topology)",
    "(read_topology)",
    "(read_database)",
    "(run_update)",
    "(convection)",
    "(diffusion)",
    "(balance)",
    "(run)"
    "(postprocess)",
    "(user)",
    "(unknown)"
};
#endif

static const char* Report_Priority_Names[] =
{
    "",
    "[debug]   ",
    "[info]    ",
    "[warning] ",
    "[error]   ",
    "[fatal]   ",
    "[unknown] "
};


Timer_type* unit_timer_create()
{
    Timer_type* new_timer = (Timer_type*)malloc(sizeof(Timer_type));
    if (new_timer == NULL) return NULL;
    new_timer->is_running = 0;
    new_timer->m_value = 0;
    new_timer->m_total_value = 0;
    return new_timer;
}
void unit_timer_destroy(Timer_type *t)  { free(t); }
void unit_timer_start(Timer_type *t)    { if (t && t->is_running == 0) { t->m_value =  CLOCKTIME();                                               t->is_running = 1;} }
void unit_timer_stop(Timer_type *t)     { if (t && t->is_running == 1) { t->m_value = (CLOCKTIME() - t->m_value); t->m_total_value += t->m_value; t->is_running = 0;} }
void unit_timer_restart(Timer_type *t)  { if (t)                       { t->m_value =  CLOCKTIME();               t->m_total_value = 0;           t->is_running = 1;} }
short unit_timer_is_running(Timer_type *t)  { return (t ? (t->is_running) : 0); }
ticks_t unit_timer_get_frame(Timer_type *t) { return (t ? ((t->is_running) ? (CLOCKTIME() - t->m_value) : (t->m_value)) : 0); }
ticks_t unit_timer_get_total(Timer_type *t) { return (t ? ((t->is_running) ? (t->m_total_value + CLOCKTIME() - t->m_value) : (t->m_total_value)) : 0); }


/*****************************************************************************/

/**
 * @brief Set the logging priority of the report module. Only messages of equal
 *        or higher priority will be reported
 * @param rcfd      pointer to rCFD instance
 * @param priority  priority level for report output
 */
void report_set_priority(rCFD_type* rcfd, int priority)
{
    rcfd->Report->priority = priority;
}

/**
 * @brief Toggle the console output of the report module
 * @param rcfd  pointer to rCFD instance
 * @param on    flag to turn on/off console output
 */
void report_toggle_console_output(rCFD_type* rcfd, int on)
{
    rcfd->Report->console_output_on = on;
}

/**
 * @brief Toggle the file output of the report module
 * @param rcfd  pointer to rCFD instance
 * @param on    flag to turn on/off file output
 */
void report_toggle_file_output(rCFD_type* rcfd, int on)
{
    rcfd->Report->file_output_on = on;
}

/**
 * @brief Toggle timing of the report module
 * @param rcfd  pointer to rCFD instance
 * @param on    flag to turn on/off timing
 */
void report_toggle_timing(rCFD_type* rcfd, int on)
{
    rcfd->Report->timing_on = on;
}

/**
 * @brief Set the time unit used by the report module
 * @param rcfd  pointer to rCFD instance
 * @param unit  time unit, one of @ref rcfd_timer_time_unit
 */
void report_set_time_unit(rCFD_type* rcfd, int unit)
{
    switch (unit){
    case RCFD_TIMER_NANOSEC:
        rcfd->Report->ticks_2_time_unit = 1e9;
        rcfd->Report->time_unit = RCFD_TIMER_NANOSEC;
        break;
    case RCFD_TIMER_MICROSEC:
        rcfd->Report->ticks_2_time_unit = 1e6;
        rcfd->Report->time_unit = RCFD_TIMER_MICROSEC;
        break;
    case RCFD_TIMER_MILLISEC:
        rcfd->Report->ticks_2_time_unit = 1e3;
        rcfd->Report->time_unit = RCFD_TIMER_MILLISEC;
        break;
    case RCFD_TIMER_SEC:
    default:
        rcfd->Report->ticks_2_time_unit = 1.0;
        rcfd->Report->time_unit = RCFD_TIMER_SEC;
        break;
    }
    rcfd->Report->ticks_2_time_unit /= CLOCKS_PER_SEC;
}

/**
 * @brief Get a human-readable time unit string
 * @param rcfd  pointer to rCFD instance
 * @return a human-readable time unit string
 */
const char* report_get_time_unit_name(rCFD_type* rcfd)
{
    return Time_Unit_Names[rcfd->Report->time_unit];
}

/**
 * @brief Close currently open output file and open given file
 * @param rcfd      pointer to rCFD instance
 * @param filename  base name of output file
 * @param mode      file access mode
 * @retval 0 new file was not opened
 * @retval 1 new file was opened
 */
int report_set_file(rCFD_type* rcfd, const char *filename, const char *mode)
{
#if RP_NODE
    if (rcfd->Report->file_is_open){
        fclose(rcfd->Report->report_file);
        rcfd->Report->file_is_open = 0;
    }

    if (filename){
        int numberpadding = 5;
        char *node_filename = (char*)malloc(strlen(filename) + 1 + numberpadding + 1);
        sprintf(node_filename, "%s_%d", filename, myid);

        rcfd->Report->report_file = fopen(node_filename, mode);

        free(node_filename);

        if (rcfd->Report->report_file){
            rcfd->Report->file_is_open = 1;
            return 1;
        }
        else{
            Message("Failed to open report file!\n");
        }
    }
#endif
    return 0;
}

/**
 * @brief Close currently open node0 output file and open given file on node0
 * @param rcfd      pointer to rCFD instance
 * @param filename  name of output file
 * @param mode      file access mode
 * @retval 0 new file was not opened
 * @retval 1 new file was opened or node is different from 0
 */
int report_set_file0(rCFD_type* rcfd, const char *filename, const char *mode)
{
#if RP_NODE
#if defined(RCFD_LIB_MPI)
    if (rcfd->Report->file0_is_open){
        MPI_File_close(&rcfd->Report->report_file0);
        rcfd->Report->file0_is_open = 0;
    }

    if (filename){
        if (MPI_File_open(rcfd->Parallel->world,
                          filename,
                          MPI_MODE_CREATE | MPI_MODE_WRONLY,
                          MPI_INFO_NULL,
                          &rcfd->Report->report_file0) == MPI_SUCCESS){
            rcfd->Report->file0_is_open = 1;
        }
        else{
            Message("[%d] Failed to open report file0!\n", myid);
            fflush(stdout);
        }
    }
#else
    if (rcfd->Report->file0_is_open){
        fclose(rcfd->Report->report_file0);
        rcfd->Report->file0_is_open = 0;
    }

    if (filename){
        rcfd->Report->report_file0 = fopen(filename, mode);

        if (rcfd->Report->report_file0){
            rcfd->Report->file0_is_open = 1;
        }
        else{
            Message("[%d] Failed to open report file0!\n", myid);
            fflush(stdout);
        }
    }
#endif

    return rcfd->Report->file0_is_open;
#endif
    return 0;
}

/**
 * @brief Check if output file is open
 * @param rcfd  pointer to rCFD instance
 * @retval 0 output file is not open
 * @retval 1 output file is open
 */
int report_is_open(rCFD_type* rcfd)
{
    return rcfd->Report->file_is_open;
}

/**
 * @brief Check if node0 output file is open
 * @param rcfd  pointer to rCFD instance
 * @retval 0 output file is not open
 * @retval 1 output file is open
 */
int report_is_open0(rCFD_type* rcfd)
{
    return rcfd->Report->file0_is_open;
}

/**
 * @brief Reset the number of reported messages
 * @param rcfd  pointer to rCFD instance
 */
void report_reset_number_of_reports(rCFD_type* rcfd)
{
    int i, j;
    for (i = 0; i < RCFD_REPORT_CATEGORY_MAX; ++i)
        for (j = 0; j < RCFD_REPORT_MAX; ++j)
            rcfd->Report->number_of_reports[i][j] = 0;
}

/*****************************************************************************/

/**
 * @brief Start/continue the given timer
 * @param rcfd  pointer to rCFD instance
 * @param unit  timer ID for the desired unit of code
 */
void report_start_timer(rCFD_type* rcfd, rcfd_timer_code_unit unit)
{
    if (!rcfd->Report->timing_on)
        return;
    if (rcfd->Report->timers[unit] == NULL)
        rcfd->Report->timers[unit] = unit_timer_create();
    unit_timer_start(rcfd->Report->timers[unit]);
}

/**
 * @brief Stop/pause the given timer
 * @param rcfd  pointer to rCFD instance
 * @param unit  timer ID for the desired unit of code
 */
void report_stop_timer(rCFD_type* rcfd, rcfd_timer_code_unit unit)
{
    if (!rcfd->Report->timing_on)
        return;
    if (rcfd->Report->timers[unit] == NULL)
        return;
    unit_timer_stop(rcfd->Report->timers[unit]);
}

/**
 * @brief Get the total time measured by the given timer
 * @param rcfd  pointer to rCFD instance
 * @param unit  timer ID for the desired unit of code
 * @return the total time measured by the given timer or -1 if timing is off or
 *         the unit timer has not been started
 */
double report_get_timer_total(rCFD_type* rcfd, rcfd_timer_code_unit unit)
{
    if (!rcfd->Report->timing_on)
        return -1.0;
    if (rcfd->Report->timers[unit] == NULL)
        return -1.0;
    return unit_timer_get_total(rcfd->Report->timers[unit]) * rcfd->Report->ticks_2_time_unit;
}

/*****************************************************************************/

/**
 * @brief Write formatted data from variable argument list to sized buffer
 * @param format    string that contains a format string
 * @param args      value identifying a variable arguments list initialized with va_start
 * @return the formatted string
 */
const char* vform(const char *format, va_list args)
{
    static char buffer[REPORT_BUFFER_SIZE] = {0};
    vsnprintf(buffer, REPORT_BUFFER_SIZE, format, args);
    return buffer;
}

/**
 * @brief Get a human-readable message priority string for the given priority
 * @param priority  message priority (cf. @ref rcfd_report_priority)
 * @return a human-readable message priority string ('[unknown]' if priority is not recognized)
 */
const char* report_get_priority_name(int priority)
{
    return Report_Priority_Names[(priority > RCFD_REPORT_UNKNOWN || priority < 0) ? RCFD_REPORT_UNKNOWN : priority];
}

/**
 * @brief Obtain exclusive lock for a given file (blocking)
 * @param file file to lock for writing
 * @return -1 upon error
 */
int report_lock_file(FILE *file)
{
#if !defined(RCFD_LIB_MPI)
#if defined(_WIN32) || defined(_WIN64)
    // TODO: Test
    /*
    int fd = _fileno(file);
    OVERLAPPED overlapped = {0};

    // wait for exclusive lock
    if(!LockFileEx(fd, LOCKFILE_EXCLUSIVE_LOCK, 0, MAXDWORD, MAXDWORD, &overlapped))
        return -1;
    */
#else
    int fd = fileno(file);      // file descriptor
    struct flock file_lock;

    file_lock.l_whence  = SEEK_SET;     // from the beginning of the file ...
    file_lock.l_start   = 0;            // with no further offset ...
    file_lock.l_len     = 0;            // lock the whole file
    file_lock.l_type    = F_WRLCK;      // exclusive lock

    return fcntl(fd, F_SETLKW, &file_lock); // wait for lock
#endif
#endif // RCFD_LIB_MPI
    return 0;
}

/**
 * @brief Release lock for a given file (blocking)
 * @param file file to unlock
 * @return -1 upon error
 */
int report_unlock_file(FILE *file)
{
#if !defined(RCFD_LIB_MPI)
#if defined(_WIN32) || defined(_WIN64)
    // TODO: Test
    /*
    int fd = _fileno(file);
    OVERLAPPED overlapped = {0};

    // wait for unlock
    if(!UnlockFileEx(fd, 0, MAXDWORD, MAXDWORD, &overlapped))
        return -1;
    */
#else
    int fd = fileno(file);      // file descriptor
    struct flock file_lock;

    file_lock.l_whence  = SEEK_SET;
    file_lock.l_start   = 0;
    file_lock.l_len     = 0;
    file_lock.l_type    = F_UNLCK;      // unlock

    return fcntl(fd, F_SETLKW, &file_lock); // wait for unlock
#endif
#endif // RCFD_LIB_MPI
    return 0;
}

/**
 * @brief Write a single message line to the node0 report output stream
 * @param rcfd      pointer to rCFD instance
 * @param priority  message priority (cf. @ref rcfd_report_priority)
 * @param message   string that contains the text to be written
 */
void report_write_line0(rCFD_type* rcfd, int priority, const char *message)
{
#if RP_NODE
    if (myid != 0) return;

    if (rcfd->Report->file_output_on && rcfd->Report->file0_is_open){
#if defined(RCFD_LIB_MPI)
        static char buffer[REPORT_BUFFER_SIZE] = {0};
        snprintf(buffer, REPORT_BUFFER_SIZE, "%s%s\n", report_get_priority_name(priority), message);
        MPI_File_write_shared(rcfd->Report->report_file0, buffer, strlen(buffer), MPI_CHAR, MPI_STATUS_IGNORE);
#else
        int lock_success = 0;
        int lock_required = (compute_node_count > 1) && ((!rcfd->Solver_Dict) || (!rcfd->Solver_Dict->multifile_output_on));

        if (lock_required)
            lock_success = report_lock_file(rcfd->Report->report_file0);

        if (lock_success != -1){
            fprintf(rcfd->Report->report_file0, "%s%s\n", report_get_priority_name(priority), message);
            if (rcfd->Report->fflush_on || priority <= RCFD_REPORT_DEBUG || lock_required) fflush(rcfd->Report->report_file0);

            if (lock_required)
                report_unlock_file(rcfd->Report->report_file0);
        }
#endif
    }

    if (rcfd->Report->console_output_on){
        Message0("%s%s\n", report_get_priority_name(priority), message);
        if (rcfd->Report->fflush_on || priority <= RCFD_REPORT_DEBUG) fflush(stdout);
    }
#endif
}

/**
 * @brief Write a single message line to the report output stream
 *
 * If per-process files have not been opened, the messages are rerouted to
 * the node0 output stream.
 * @param rcfd      pointer to rCFD instance
 * @param priority  message priority (cf. @ref rcfd_report_priority)
 * @param message   string that contains the text to be written
 */
void report_write_line(rCFD_type* rcfd, int priority, const char *message)
{
#if RP_NODE
    if (rcfd->Report->file_output_on){

        if (rcfd->Report->file_is_open){
            fprintf(rcfd->Report->report_file, "%s%s\n", report_get_priority_name(priority), message);
            if (rcfd->Report->fflush_on || priority <= RCFD_REPORT_DEBUG) fflush(rcfd->Report->report_file);
        }
        else if (rcfd->Report->file0_is_open){

#if defined(RCFD_LIB_MPI)
            static char buffer[REPORT_BUFFER_SIZE] = {0};
            snprintf(buffer, REPORT_BUFFER_SIZE, "%s%s\n", report_get_priority_name(priority), message);
            MPI_File_write_shared(rcfd->Report->report_file0, buffer, strlen(buffer), MPI_CHAR, MPI_STATUS_IGNORE);
#else
            int lock_success = report_lock_file(rcfd->Report->report_file0);

            if (lock_success != -1){
                fprintf(rcfd->Report->report_file0, "%s%s\n", report_get_priority_name(priority), message);
                fflush(rcfd->Report->report_file0);
                report_unlock_file(rcfd->Report->report_file0);
            }
            else{
                Message("[%d] failed to lock report file for writing\n", myid);
                if (rcfd->Report->fflush_on || priority <= RCFD_REPORT_DEBUG) fflush(stdout);
            }
#endif
        }
    }

    if (rcfd->Report->console_output_on){
        Message("%s%s\n", report_get_priority_name(priority), message);
        if (rcfd->Report->fflush_on || priority <= RCFD_REPORT_DEBUG) fflush(stdout);
    }
#endif
}

/*****************************************************************************/

/**
 * @brief Write a message to the report output stream, depending on the message priority
 * @param rcfd      pointer to rCFD instance
 * @param category  code section category, indicating message origin (cf. @ref rcfd_report_category)
 * @param priority  message priority determining visibility in output (cf. @ref rcfd_report_priority)
 * @param format    string that contains the text to be written; it may contain
 *                  format specifiers that are replaced by the values specified
 *                  in subsequent additional arguments and formatted as requested
 */
void report_log(rCFD_type* rcfd, int category, int priority, const char *format, ...)
{
    rcfd->Report->number_of_reports[category][priority]++;
    if (rcfd->Report->priority > priority) return;
    va_list args;
    va_start(args, format);
    report_write_line(rcfd, priority, vform(format, args));
    va_end(args);
}

/*****************************************************************************/

/**
 * @brief Write a message to the node0 report output stream, bypassing any priority check
 * @param rcfd      pointer to rCFD instance
 * @param format    string that contains the text to be written; it may contain
 *                  format specifiers that are replaced by the values specified
 *                  in subsequent additional arguments and formatted as requested
 */
void report_print0(rCFD_type* rcfd, const char *format, ...)
{
    if (myid != 0) return;
    rcfd->Report->number_of_reports[RCFD_REPORT_CATEGORY_NOTSET][RCFD_REPORT_TRACE]++;
    va_list args;
    va_start(args, format);
    report_write_line0(rcfd, RCFD_REPORT_TRACE, vform(format, args));
    va_end(args);
}

/**
 * @brief Write a message to the report output stream, bypassing any priority check
 * @param rcfd      pointer to rCFD instance
 * @param format    string that contains the text to be written; it may contain
 *                  format specifiers that are replaced by the values specified
 *                  in subsequent additional arguments and formatted as requested
 */
void report_print(rCFD_type* rcfd, const char *format, ...)
{
    rcfd->Report->number_of_reports[RCFD_REPORT_CATEGORY_NOTSET][RCFD_REPORT_TRACE]++;
    va_list args;
    va_start(args, format);
    report_write_line(rcfd, RCFD_REPORT_TRACE, vform(format, args));
    va_end(args);
}

/**
 * @brief Write a message to the node0 report output stream, if priority is equal to @ref RCFD_REPORT_TRACE
 * @param rcfd      pointer to rCFD instance
 * @param category  code section category, indicating message origin (cf. @ref rcfd_report_category)
 * @param format    string that contains the text to be written; it may contain
 *                  format specifiers that are replaced by the values specified
 *                  in subsequent additional arguments and formatted as requested
 */
void report_trace0(rCFD_type* rcfd, int category, const char *format, ...)
{
    if (myid != 0) return;
    rcfd->Report->number_of_reports[category][RCFD_REPORT_TRACE]++;
    if (rcfd->Report->priority > RCFD_REPORT_TRACE) return;
    va_list args;
    va_start(args, format);
    report_write_line0(rcfd, RCFD_REPORT_TRACE, vform(format, args));
    va_end(args);
}

/**
 * @brief Write a message to the report output stream, if priority is equal to @ref RCFD_REPORT_TRACE
 * @param rcfd      pointer to rCFD instance
 * @param category  code section category, indicating message origin (cf. @ref rcfd_report_category)
 * @param format    string that contains the text to be written; it may contain
 *                  format specifiers that are replaced by the values specified
 *                  in subsequent additional arguments and formatted as requested
 */
void report_trace(rCFD_type* rcfd, int category, const char *format, ...)
{
    rcfd->Report->number_of_reports[category][RCFD_REPORT_TRACE]++;
    if (rcfd->Report->priority > RCFD_REPORT_TRACE) return;
    va_list args;
    va_start(args, format);
    report_write_line(rcfd, RCFD_REPORT_TRACE, vform(format, args));
    va_end(args);
}

/**
 * @brief Write a message to the node0 report output stream, if priority is less or equal to @ref RCFD_REPORT_DEBUG
 * @param rcfd      pointer to rCFD instance
 * @param category  code section category, indicating message origin (cf. @ref rcfd_report_category)
 * @param format    string that contains the text to be written; it may contain
 *                  format specifiers that are replaced by the values specified
 *                  in subsequent additional arguments and formatted as requested
 */
void report_debug0(rCFD_type* rcfd, int category, const char *format, ...)
{
    if (myid != 0) return;
    rcfd->Report->number_of_reports[category][RCFD_REPORT_DEBUG]++;
    if (rcfd->Report->priority > RCFD_REPORT_DEBUG) return;
    va_list args;
    va_start(args, format);
    report_write_line0(rcfd, RCFD_REPORT_DEBUG, vform(format, args));
    va_end(args);
}

/**
 * @brief Write a message to the report output stream, if priority is less or equal to @ref RCFD_REPORT_DEBUG
 * @param rcfd      pointer to rCFD instance
 * @param category  code section category, indicating message origin (cf. @ref rcfd_report_category)
 * @param format    string that contains the text to be written; it may contain
 *                  format specifiers that are replaced by the values specified
 *                  in subsequent additional arguments and formatted as requested
 */
void report_debug(rCFD_type* rcfd, int category, const char *format, ...)
{
    rcfd->Report->number_of_reports[category][RCFD_REPORT_DEBUG]++;
    if (rcfd->Report->priority > RCFD_REPORT_DEBUG) return;
    va_list args;
    va_start(args, format);
    report_write_line(rcfd, RCFD_REPORT_DEBUG, vform(format, args));
    va_end(args);
}

/**
 * @brief Write a message to the node0 report output stream, if priority is less or equal to @ref RCFD_REPORT_INFO
 * @param rcfd      pointer to rCFD instance
 * @param category  code section category, indicating message origin (cf. @ref rcfd_report_category)
 * @param format    string that contains the text to be written; it may contain
 *                  format specifiers that are replaced by the values specified
 *                  in subsequent additional arguments and formatted as requested
 */
void report_info0(rCFD_type* rcfd, int category, const char* format, ...)
{
    if (myid != 0) return;
    rcfd->Report->number_of_reports[category][RCFD_REPORT_INFO]++;
    if (rcfd->Report->priority > RCFD_REPORT_INFO) return;
    va_list args;
    va_start(args, format);
    report_write_line0(rcfd, RCFD_REPORT_INFO, vform(format, args));
    va_end(args);
}

/**
 * @brief Write a message to the report output stream, if priority is less or equal to @ref RCFD_REPORT_INFO
 * @param rcfd      pointer to rCFD instance
 * @param category  code section category, indicating message origin (cf. @ref rcfd_report_category)
 * @param format    string that contains the text to be written; it may contain
 *                  format specifiers that are replaced by the values specified
 *                  in subsequent additional arguments and formatted as requested
 */
void report_info(rCFD_type* rcfd, int category, const char* format, ...)
{
    rcfd->Report->number_of_reports[category][RCFD_REPORT_INFO]++;
    if (rcfd->Report->priority > RCFD_REPORT_INFO) return;
    va_list args;
    va_start(args, format);
    report_write_line(rcfd, RCFD_REPORT_INFO, vform(format, args));
    va_end(args);
}

/**
 * @brief Write a message to the node0 report output stream, if priority is less or equal to @ref RCFD_REPORT_WARNING
 * @param rcfd      pointer to rCFD instance
 * @param category  code section category, indicating message origin (cf. @ref rcfd_report_category)
 * @param format    string that contains the text to be written; it may contain
 *                  format specifiers that are replaced by the values specified
 *                  in subsequent additional arguments and formatted as requested
 */
void report_warning0(rCFD_type* rcfd, int category, const char* format,...)
{
    if (myid != 0) return;
    rcfd->Report->number_of_reports[category][RCFD_REPORT_WARNING]++;
    if (rcfd->Report->priority > RCFD_REPORT_WARNING) return;
    va_list args;
    va_start(args, format);
    report_write_line0(rcfd, RCFD_REPORT_WARNING, vform(format, args));
    va_end(args);
}

/**
 * @brief Write a message to the report output stream, if priority is less or equal to @ref RCFD_REPORT_WARNING
 * @param rcfd      pointer to rCFD instance
 * @param category  code section category, indicating message origin (cf. @ref rcfd_report_category)
 * @param format    string that contains the text to be written; it may contain
 *                  format specifiers that are replaced by the values specified
 *                  in subsequent additional arguments and formatted as requested
 */
void report_warning(rCFD_type* rcfd, int category, const char* format,...)
{
    rcfd->Report->number_of_reports[category][RCFD_REPORT_WARNING]++;
    if (rcfd->Report->priority > RCFD_REPORT_WARNING) return;
    va_list args;
    va_start(args, format);
    report_write_line(rcfd, RCFD_REPORT_WARNING, vform(format, args));
    va_end(args);
}

/**
 * @brief Write a message to the node0 report output stream, if priority is less or equal to @ref RCFD_REPORT_ERROR
 * @param rcfd      pointer to rCFD instance
 * @param category  code section category, indicating message origin (cf. @ref rcfd_report_category)
 * @param format    string that contains the text to be written; it may contain
 *                  format specifiers that are replaced by the values specified
 *                  in subsequent additional arguments and formatted as requested
 */
void report_error0(rCFD_type* rcfd, int category, const char* format,...)
{
    if (myid != 0) return;
    rcfd->Report->number_of_reports[category][RCFD_REPORT_ERROR]++;
    if (rcfd->Report->priority > RCFD_REPORT_ERROR) return;
    va_list args;
    va_start(args, format);
    report_write_line0(rcfd, RCFD_REPORT_ERROR, vform(format, args));
    va_end(args);
}

/**
 * @brief Write a message to the report output stream, if priority is less or equal to @ref RCFD_REPORT_ERROR
 * @param rcfd      pointer to rCFD instance
 * @param category  code section category, indicating message origin (cf. @ref rcfd_report_category)
 * @param format    string that contains the text to be written; it may contain
 *                  format specifiers that are replaced by the values specified
 *                  in subsequent additional arguments and formatted as requested
 */
void report_error(rCFD_type* rcfd, int category, const char* format,...)
{
    rcfd->Report->number_of_reports[category][RCFD_REPORT_ERROR]++;
    if (rcfd->Report->priority > RCFD_REPORT_ERROR) return;
    va_list args;
    va_start(args, format);
    report_write_line(rcfd, RCFD_REPORT_ERROR, vform(format, args));
    va_end(args);
}

/**
 * @brief Write a message to the report output stream, if priority is less or equal to @ref RCFD_REPORT_FATAL
 * @param rcfd      pointer to rCFD instance
 * @param category  code section category, indicating message origin (cf. @ref rcfd_report_category)
 * @param format    string that contains the text to be written; it may contain
 *                  format specifiers that are replaced by the values specified
 *                  in subsequent additional arguments and formatted as requested
 */
void report_fatal(rCFD_type* rcfd, int category, const char* format,...)
{
    rcfd->Report->number_of_reports[category][RCFD_REPORT_FATAL]++;
    if (rcfd->Report->priority > RCFD_REPORT_FATAL) return;
    va_list args;
    va_start(args, format);
    report_write_line(rcfd, RCFD_REPORT_FATAL, vform(format, args));
    va_end(args);
}

/*****************************************************************************/

// Convenience functions for reporting errors while setting dict properties

void report_dict_invalid(rCFD_type *rcfd, const char *dict, const char *property)
{
    report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                  "Failed to set property '%s' in %s - invalid dictionary name",
                  property, dict);
}
void report_dict_invalid_property_type(rCFD_type *rcfd, const char *dict, const char *property, const char *property_type)
{
    report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                  "Failed to set property '%s' in %s - dictionary holds no %s properties",
                  property, dict, property_type);
}
void report_dict_access_denied(rCFD_type *rcfd, const char *dict, const char *property, const char *property_type)
{
    report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                  "Failed to set %s property '%s' in %s dict - access denied",
                  property_type, property, dict);
}
void report_dict_unknown_property(rCFD_type *rcfd, const char *dict, const char *property, const char *property_type)
{
    report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                  "Failed to set unknown %s property '%s' in %s dict",
                  property_type, property, dict);
}
void report_dict_invalid_layer(rCFD_type *rcfd, const char *dict, const char *property, const char *property_type, int layer)
{
    report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                  "Failed to set %s property '%s' in %s dict - invalid layer %d",
                  property_type, property, dict, layer);
}
void report_dict_invalid_phase(rCFD_type *rcfd, const char *dict, const char *property, const char *property_type, int phase)
{
    report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                  "Failed to set %s property '%s' in %s dict - invalid phase %d",
                  property_type, property, dict, phase);
}
void report_dict_invalid_data(rCFD_type *rcfd, const char *dict, const char *property, const char *property_type, int data)
{
    report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                  "Failed to set %s property '%s' in %s dict - invalid data index %d",
                  property_type, property, dict, data);
}
void report_dict_invalid_memory(rCFD_type *rcfd, const char *dict, const char *property, const char *property_type)
{
    report_error (rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                  "Failed to set %s property '%s' in %s dict - memory not allocated on proc %d",
                  property_type, property, dict, myid);
}
void report_dict_invalid_value_i(rCFD_type *rcfd, const char *dict, const char *property, int value)
{
    report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                  "Failed to set integer property '%s' in %s dict - invalid value %d",
                  property, dict, value);
}
void report_dict_invalid_value_r(rCFD_type *rcfd, const char *dict, const char *property, double value)
{
    report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                  "Failed to set real property '%s' in %s dict - invalid value %e",
                  property, dict, value);
}
void report_dict_invalid_value_r_3d(rCFD_type *rcfd, const char *dict, const char *property)
{
    report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                  "Failed to set vector property '%s' in %s dict - invalid value",
                  property, dict);
}
void report_dict_invalid_value_s(rCFD_type *rcfd, const char *dict, const char *property, const char *value)
{
    report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                  "Failed to set string property '%s' in %s dict - invalid value %s",
                  property, dict, value);
}

/*****************************************************************************/

/**
 * @brief Write timing information of the specified timer to the report output stream
 * @param rcfd  pointer to rCFD instance
 * @param unit  timer ID for the desired unit of code
 */
void report_timing(rCFD_type* rcfd, rcfd_timer_code_unit unit)
{
    if (!rcfd->Report->timing_on)
        return;
    if (rcfd->Report->timers[unit] == NULL)
        return;

    double frame_time = unit_timer_get_frame(rcfd->Report->timers[unit]) * rcfd->Report->ticks_2_time_unit;
    double total_time = unit_timer_get_total(rcfd->Report->timers[unit]) * rcfd->Report->ticks_2_time_unit;
    report_print(rcfd, "[%d] (step %ld) %-10s CPU time %.3e (%.3e)", myid, rcfd->Solver->global_run_counter, Timer_Names[unit], frame_time, total_time);
}

/*****************************************************************************/

/**
 * @brief Write a report summary and timing information to the report output stream
 * @param rcfd  pointer to rCFD instance
 */
void report_summary(rCFD_type* rcfd)
{
#if RP_NODE
    int i,j;
    int **number_of_reports_work = malloc_i_2d(RCFD_REPORT_CATEGORY_MAX, RCFD_REPORT_MAX);

    PRF_GISUM(&rcfd->Report->number_of_reports[0][0], RCFD_REPORT_CATEGORY_MAX*RCFD_REPORT_MAX, &number_of_reports_work[0][0]);

    free_i_2d(number_of_reports_work);

    report_print0(rcfd, "=== rCFD report summary ===");
    report_print0(rcfd, "category \\ priority | %-9s| %-9s| %-9s| %-9s| %-9s| %-9s| [generic]",
                        Report_Priority_Names[RCFD_REPORT_UNKNOWN], Report_Priority_Names[RCFD_REPORT_FATAL],
                        Report_Priority_Names[RCFD_REPORT_ERROR],   Report_Priority_Names[RCFD_REPORT_WARNING],
                        Report_Priority_Names[RCFD_REPORT_INFO],    Report_Priority_Names[RCFD_REPORT_DEBUG]);
    report_print0(rcfd, "--------------------+-----------+-----------+-----------+-----------+-----------+-----------+-----------");
    for (i = 0; i < RCFD_REPORT_CATEGORY_MAX; ++i){
        report_print0(rcfd, "%-19s | %9d | %9d | %9d | %9d | %9d | %9d | %9d", Report_Category_Names[i],
                            rcfd->Report->number_of_reports[i][RCFD_REPORT_UNKNOWN], rcfd->Report->number_of_reports[i][RCFD_REPORT_FATAL],
                            rcfd->Report->number_of_reports[i][RCFD_REPORT_ERROR],   rcfd->Report->number_of_reports[i][RCFD_REPORT_WARNING],
                            rcfd->Report->number_of_reports[i][RCFD_REPORT_INFO],    rcfd->Report->number_of_reports[i][RCFD_REPORT_DEBUG],
                            rcfd->Report->number_of_reports[i][RCFD_REPORT_TRACE]);
    }

    if (!rcfd->Report->timing_on) return;

    int64_t total_ticks[RCFD_TIMER_MAX];
    int64_t total_ticks_min[RCFD_TIMER_MAX];
    int64_t total_ticks_max[RCFD_TIMER_MAX];
    int64_t total_ticks_ave[RCFD_TIMER_MAX];
    int64_t *total_ticks_all[RCFD_TIMER_MAX] = {0};

    for (i = 0; i < RCFD_TIMER_MAX; ++i){
        total_ticks[i] = rcfd->Report->timers[i] ? unit_timer_get_total(rcfd->Report->timers[i]) : 0;
        total_ticks_min[i] = 0;
        total_ticks_max[i] = 0;
        total_ticks_ave[i] = 0;
        total_ticks_min[i] = PRF_GI64LOW1(total_ticks[i]);
        total_ticks_max[i] = PRF_GI64HIGH1(total_ticks[i]);
        total_ticks_ave[i] = PRF_GI64SUM1(total_ticks[i]);

        total_ticks_all[i] = (int64_t*)malloc(sizeof(int64_t)*compute_node_count);

        for (j = 0; j < compute_node_count; ++j)
            total_ticks_all[i][j] = 0;

        total_ticks_all[i][myid] = total_ticks[i];

        for (j = 0; j < compute_node_count; ++j)
            total_ticks_all[i][j] = PRF_GI64HIGH1(total_ticks_all[i][j]);
    }

    if (myid != 0) return;

    char buffer[REPORT_BUFFER_SIZE] = {0};
    int buffer_offset;

    report_print0(rcfd, "");
    report_print0(rcfd, "=== rCFD timing summary ===");

    buffer_offset = snprintf(buffer, REPORT_BUFFER_SIZE, "node \\ timer ");
    for (j = 0; j < RCFD_TIMER_MAX && buffer_offset < REPORT_BUFFER_SIZE; ++j){
        buffer_offset += snprintf(buffer+buffer_offset, REPORT_BUFFER_SIZE-buffer_offset, "| %-11s ", Timer_Names[j]);
    }
    report_print0(rcfd, buffer);

    buffer_offset = snprintf(buffer, REPORT_BUFFER_SIZE, "-------------");
    for (j = 0; j < RCFD_TIMER_MAX && buffer_offset < REPORT_BUFFER_SIZE; ++j){
        buffer_offset += snprintf(buffer+buffer_offset, REPORT_BUFFER_SIZE-buffer_offset, "+-------------");
    }
    report_print0(rcfd, buffer);

    // for each compute node write timings of all timing-units
    for ( i = 0; i < compute_node_count; ++i){
        buffer_offset = snprintf(buffer, REPORT_BUFFER_SIZE, "node %3d     ", i);
        for (j = 0; j < RCFD_TIMER_MAX && buffer_offset < REPORT_BUFFER_SIZE; ++j){
            if (rcfd->Report->timers[j])
                buffer_offset += snprintf(buffer+buffer_offset, REPORT_BUFFER_SIZE-buffer_offset, "|  %.3e  ", total_ticks_all[j][i]*rcfd->Report->ticks_2_time_unit);
            else
                buffer_offset += snprintf(buffer+buffer_offset, REPORT_BUFFER_SIZE-buffer_offset, "|      -      ");
        }
        report_print0(rcfd, buffer);
    }

    // write min timing for all timing units
    buffer_offset = snprintf(buffer, REPORT_BUFFER_SIZE, "node min     ");
    for (j = 0; j < RCFD_TIMER_MAX && buffer_offset < REPORT_BUFFER_SIZE; ++j){
        if (rcfd->Report->timers[j])
            buffer_offset += snprintf(buffer+buffer_offset, REPORT_BUFFER_SIZE-buffer_offset, "|  %.3e  ", total_ticks_min[j]*rcfd->Report->ticks_2_time_unit);
        else
            buffer_offset += snprintf(buffer+buffer_offset, REPORT_BUFFER_SIZE-buffer_offset, "|      -      ");
    }
    report_print0(rcfd, buffer);

    // write max timing for all timing units
    buffer_offset = snprintf(buffer, REPORT_BUFFER_SIZE, "node max     ");
    for (j = 0; j < RCFD_TIMER_MAX && buffer_offset < REPORT_BUFFER_SIZE; ++j){
        if (rcfd->Report->timers[j])
            buffer_offset += snprintf(buffer+buffer_offset, REPORT_BUFFER_SIZE-buffer_offset, "|  %.3e  ", total_ticks_max[j]*rcfd->Report->ticks_2_time_unit);
        else
            buffer_offset += snprintf(buffer+buffer_offset, REPORT_BUFFER_SIZE-buffer_offset, "|      -      ");
    }
    report_print0(rcfd, buffer);

    // write ave timing for all timing units
    buffer_offset = snprintf(buffer, REPORT_BUFFER_SIZE, "node ave     ");
    for (j = 0; j < RCFD_TIMER_MAX && buffer_offset < REPORT_BUFFER_SIZE; ++j){
        if (rcfd->Report->timers[j])
            buffer_offset += snprintf(buffer+buffer_offset, REPORT_BUFFER_SIZE-buffer_offset, "|  %.3e  ", total_ticks_ave[j]/compute_node_count*rcfd->Report->ticks_2_time_unit);
        else
            buffer_offset += snprintf(buffer+buffer_offset, REPORT_BUFFER_SIZE-buffer_offset, "|      -      ");
    }
    report_print0(rcfd, buffer);

    for (i = 0; i < RCFD_TIMER_MAX; ++i){
        free(total_ticks_all[i]);
    }
#endif
}

/*****************************************************************************/

#if defined(RCFD_LIB_TEST_MODULES)
int test_report(rCFD_type* rcfd)
{
    int success = 0;
#if RP_NODE
    int i;
    int ret_val;
    int64_t value = 0;
    int64_t value_prev = 1;
    int64_t value_prev_prev = 0;
    int64_t niterations = 0;
    const char *nodefilename = "report_nodelog";
    const char *summaryfilename = "report_summary";

    Message0("\nStarting rCFD test 'report'\n");

    report_set_time_unit(rcfd, RCFD_TIMER_SEC);
    report_reset_number_of_reports(rcfd);
    //report_set_time_unit(cfd, RCFD_TIMER_MILLISEC);
    //report_set_time_unit(cfd, RCFD_TIMER_MICROSEC);

    ret_val = report_set_file(rcfd, nodefilename, "w");
    if(ret_val == 0){
        Message("[%d] New file '%s' was not opened\n", myid, nodefilename);
        success |= 1;
    }
    else if(ret_val == 1){
        Message("[%d] New file '%s' was opened\n", myid, nodefilename);
    }
    else{
        Message("[%d] Unknown return value when opening '%s'\n", myid, nodefilename);
        success |= 2;
    }

    ret_val = report_set_file0(rcfd, summaryfilename, "a");
    if(ret_val == 0){
        Message("[%d] New file '%s' was not opened\n", myid, summaryfilename);
        success |= 4;
    }
    else if(ret_val == 1){
        Message("[%d] New file '%s' was opened\n", myid, summaryfilename);
    }
    else{
        Message("[%d] Unknown return value when opening '%s'\n", myid, summaryfilename);
        success |= 8;
    }

    report_toggle_file_output(rcfd, 1);
    report_toggle_timing(rcfd, 1);

    // test timing
    PRF_GSYNC();
    report_start_timer(rcfd, RCFD_TIMER_GLOBAL);
    report_start_timer(rcfd, RCFD_TIMER_RUN);

    do{
        value = 0;
        value_prev = 1;
        value_prev_prev = 0;
        ++niterations;

        for (i = 0; i < 200; ++i){
            value = value_prev + value_prev_prev;
            value_prev_prev = value_prev;
            value_prev = value;
        }
    }while(unit_timer_get_frame(rcfd->Report->timers[RCFD_TIMER_RUN]) < 100000*(myid+1));

    report_stop_timer(rcfd, RCFD_TIMER_RUN);
    report_timing(rcfd, RCFD_TIMER_RUN);

    Message("[%d] 1st loop: iterations %ld\n", myid, niterations);

    value = 0;
    value_prev = 1;
    value_prev_prev = 0;
    niterations = 0;

    report_start_timer(rcfd, RCFD_TIMER_RUN);

    do{
        value = 0;
        value_prev = 1;
        value_prev_prev = 0;
        ++niterations;

        for (i = 0; i < 200; ++i){
            value = value_prev + value_prev_prev;
            value_prev_prev = value_prev;
            value_prev = value;
        }
    }while(unit_timer_get_frame(rcfd->Report->timers[RCFD_TIMER_RUN]) < 100000*(myid+1));

    report_stop_timer(rcfd, RCFD_TIMER_RUN);
    report_timing(rcfd, RCFD_TIMER_RUN);

    report_stop_timer(rcfd, RCFD_TIMER_GLOBAL);
    report_timing(rcfd, RCFD_TIMER_GLOBAL);

    PRF_GSYNC();

    Message("[%d] 2nd loop: iterations %ld\n", myid, niterations);

    // test reporting
    report_summary(rcfd);
    // TODO add tests for reporting functions in single and multi file mode

#endif
    return success;
}
#endif // RCFD_LIB_TEST_MODULES

#endif


