#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define SHM_NAME    "/my_counter_shm"
#define LOG_FILE    "app.log"
#define INIT_LOCK   "/tmp/my_app_init.lock"
#define LEADER_LOCK "/tmp/my_app_leader.lock"

typedef struct {
    pthread_mutex_t mutex;
    int counter;
} SharedData;

static SharedData *shared;
static char prog_path[512];

/* ---------- Лог: каждая запись открывает файл, пишет строку под flock ---------- */

void log_msg(const char* fmt, ...)
{
    char text[256];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(text, sizeof text, fmt, ap);
    va_end(ap);

    struct timespec ts;
    struct tm tm_now;
    char stamp[40], line[512];

    clock_gettime(CLOCK_REALTIME, &ts);
    localtime_r(&ts.tv_sec, &tm_now);
    snprintf(stamp, sizeof stamp, "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
             tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday,
             tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec,
             ts.tv_nsec / 1000000L);

    snprintf(line, sizeof line, "[%s] [PID=%d]: %s", stamp, (int)getpid(), text);
    

    int fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644);
    flock(fd, LOCK_EX);
    dprintf(fd, "%s \n", line);
    flock(fd, LOCK_UN);
    close(fd);
}

/* ---------- Разделяемая память и мьютекс ---------- */

static void lock_shared(void)
{
    if (pthread_mutex_lock(&shared->mutex) == EOWNERDEAD) {
        pthread_mutex_consistent(&shared->mutex);
        log_msg("previous owner of mutex died, mutex recovered");
    }
}

static void unlock_shared(void)
{
    pthread_mutex_unlock(&shared->mutex);
}

static void open_shared(void)
{
    /* Под блокировкой инициализации создаём память ровно один раз. */
    int lk = open(INIT_LOCK, O_CREAT | O_RDWR | O_CLOEXEC, 0666);
    flock(lk, LOCK_EX);

    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) { perror("shm_open"); exit(1); }

    struct stat st;
    fstat(fd, &st);
    int fresh = (st.st_size == 0);

    if (fresh && ftruncate(fd, sizeof(SharedData)) == -1) {
        perror("ftruncate"); exit(1);
    }

    shared = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE,
                  MAP_SHARED, fd, 0);
    if (shared == MAP_FAILED) { perror("mmap"); exit(1); }

    if (fresh) {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
        pthread_mutex_init(&shared->mutex, &attr);
        pthread_mutexattr_destroy(&attr);
        shared->counter = 0;
    }

    close(fd);
    flock(lk, LOCK_UN);
    close(lk);
}

/* ---------- Потоки ---------- */

static void *increment_thread(void *arg)
{
    (void)arg;
    struct timespec d = {0, 300000000L};   /* 300 мс */
    for (;;) {
        lock_shared();
        shared->counter++;
        unlock_shared();
        nanosleep(&d, NULL);
    }
    return NULL;
}

/* ---------- Копии ---------- */

static void run_copy1(void)
{
    log_msg("copy 1 started");
    lock_shared();
    shared->counter += 10;
    unlock_shared();
    log_msg("copy 1 finished");
}

static void run_copy2(void)
{
    log_msg("copy 2 started");
    lock_shared();
    shared->counter *= 2;
    int v = shared->counter;
    unlock_shared();
    log_msg("copy 2 doubled counter to %d", v);

    sleep(2);

    lock_shared();
    shared->counter /= 2;
    unlock_shared();
    log_msg("copy 2 finished");
}

static pid_t spawn_copy(const char *role)
{
    pid_t pid = fork();
    if (pid == 0) {
        execl("/proc/self/exe", prog_path, role, (char *)NULL);
        log_msg("exec failed: %s", strerror(errno));
        _exit(127);
    }
    if (pid == -1) {
        log_msg("fork failed: %s", strerror(errno));
        return 0;
    }
    return pid;
}

/* ----------------------- */

static void *leader_thread(void *arg)
{
    (void)arg;
    int lock_fd = open(LEADER_LOCK, O_CREAT | O_RDWR | O_CLOEXEC, 0666);
    int is_leader = 0;
    pid_t first = 0, second = 0;   /* 0 = копия не запущена */
    int tick = 0;

    for (;;) {
        sleep(1);
        tick++;

        if (!is_leader) {
            /* Если лидер завершился, его блокировка снимается, и мы можем стать лидером. */
            if (flock(lock_fd, LOCK_EX | LOCK_NB) == 0) {
                is_leader = 1;
                log_msg("became leader");
            } else {
                continue;
            }
        }

        lock_shared();
        int value = shared->counter;
        unlock_shared();
        log_msg("counter = %d", value);

        if (tick % 3 != 0) continue;

        if (first > 0 && waitpid(first, NULL, WNOHANG) == first)
            first = 0;
        if (second > 0 && waitpid(second, NULL, WNOHANG) == second)
            second = 0;

        if (first == 0 && second == 0) {
            first = spawn_copy("1");
            second = spawn_copy("2");
        } else {
            log_msg("previous copies have not finished yet, launch skipped");
        }
    }
    return NULL;
}


int Execute(int argc, char *argv[])
{
    strncpy(prog_path, argv[0], sizeof prog_path - 1);

    open_shared();

    if (argc > 1 && strcmp(argv[1], "1") == 0) { run_copy1(); return 0; }
    if (argc > 1 && strcmp(argv[1], "2") == 0) { run_copy2(); return 0; }

    log_msg("program started");

    pthread_t inc, lead;
    pthread_create(&inc, NULL, increment_thread, NULL);
    pthread_create(&lead, NULL, leader_thread, NULL);

    printf("Введите целое число, чтобы установить счётчик:\n");

    char line[64];
    while (fgets(line, sizeof line, stdin)) {
        char *end;
        errno = 0;
        long v = strtol(line, &end, 10);
        if (end == line || errno != 0 || (*end != '\n' && *end != '\0')) {
            printf("Нужно ввести целое число\n");
            continue;
        }
        lock_shared();
        shared->counter = (int)v;
        unlock_shared();
        printf("Счётчик установлен: %d\n", (int)v);
    }

    /* stdin закрыт: программа продолжает работать в фоне */
    for (;;) pause();
}
