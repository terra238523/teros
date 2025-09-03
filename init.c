#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/wait.h>
#include <ctype.h>
#include <linux/limits.h>

#define MNT "/mnt"

// --- Helpers ---
static void trim(char *s) {
    if (!s) return;
    size_t len = strlen(s);
    while (len > 0 && (s[len-1] == '\n' || isspace((unsigned char)s[len-1]))) s[--len] = '\0';
    size_t start = 0;
    while (s[start] && isspace((unsigned char)s[start])) start++;
    if (start > 0) memmove(s, s + start, strlen(s + start) + 1);
}

static int split_argv(char *line, char *argv[], int max_args) {
    int argc = 0;
    trim(line);
    char *p = line;
    while (*p && argc < max_args - 1) {
        while (*p && isspace((unsigned char)*p)) p++;
        if (!*p) break;
        argv[argc++] = p;
        while (*p && !isspace((unsigned char)*p)) p++;
        if (*p) { *p = '\0'; p++; }
    }
    argv[argc] = NULL;
    return argc;
}

static int has_slash(const char *s) { return s && strchr(s, '/'); }

static void list_files(void) {
    DIR *d = opendir(MNT);
    if (!d) { printf("Cannot open %s: %s\n", MNT, strerror(errno)); return; }
    struct dirent *de;
    while ((de = readdir(d))) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
        printf("%s\n", de->d_name);
    }
    closedir(d);
}

static void cat_file(char *filename) {
    trim(filename);
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/%s", MNT, filename);
    FILE *f = fopen(path, "r");
    if (!f) { printf("File not found: %s (%s)\n", filename, strerror(errno)); return; }
    char buf[512];
    while (fgets(buf, sizeof(buf), f)) fputs(buf, stdout);
    fclose(f);
}

static void save_file(char *filename) {
    trim(filename);
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/%s", MNT, filename);
    FILE *f = fopen(path, "w");
    if (!f) { printf("Cannot create file: %s (%s)\n", filename, strerror(errno)); return; }
    printf("Enter content. End with a single line containing only '.'\n");
    char line[512];
    for (;;) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) {
    // don't exit init if fgets returns NULL due to non-blocking fd or EINTR
    clearerr(stdin);
    continue;
}
        trim(line);
        if (strcmp(line, ".") == 0) break;
        fputs(line, f);
        fputc('\n', f);
    }
    fclose(f);
}

static int try_exec_with_paths(char *argv[]) {
    if (has_slash(argv[0])) {
        execv(argv[0], argv);
        return -1;
    }
    const char *paths[] = { "/bin", "/sbin", NULL };
    char candidate[PATH_MAX];
    for (int i = 0; paths[i]; i++) {
        snprintf(candidate, sizeof(candidate), "%s/%s", paths[i], argv[0]);
        execv(candidate, argv);
    }
    return -1;
}

static void print_help(void) {
    puts("Built-ins: help, echo <text>, ls, cat <file>, save <file>, pwd, cd <dir>, exit");
    puts("External programs: type program name or ./prog / absolute paths");
    puts("Searched in: /bin, /sbin");
}

int main(void) {
    // Ensure folders exist
    mkdir(MNT, 0755);
    mkdir("/bin", 0755);
    mkdir("/sbin", 0755);

    printf("Welcome to TerOS\nType 'help' for commands.\n");

    char line[512], *argv[64];

    for (;;) {
        char cwd[PATH_MAX];
        if (!getcwd(cwd, sizeof(cwd))) strncpy(cwd, "?", sizeof(cwd));
        printf("%s> ", cwd);
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) break;
        trim(line);
        if (line[0] == '\0') continue;

        int argc = split_argv(line, argv, 64);
        if (argc == 0) continue;

        // ---- Builtins ----
        if (strcmp(argv[0], "exit") == 0) {
            printf("Use reboot/poweroff — 'exit' disabled in init.\n");
            continue; }
        else if (strcmp(argv[0], "help") == 0) { print_help(); continue; }
        else if (strcmp(argv[0], "echo") == 0) { for (int i = 1; i < argc; i++) { fputs(argv[i], stdout); if (i+1<argc) fputc(' ', stdout); } fputc('\n', stdout); continue; }
        else if (strcmp(argv[0], "ls") == 0) { list_files(); continue; }
        else if (strcmp(argv[0], "cat") == 0) { if (argc < 2) { printf("Usage: cat <filename>\n"); continue; } cat_file(argv[1]); continue; }
        else if (strcmp(argv[0], "save") == 0) { if (argc < 2) { printf("Usage: save <filename>\n"); continue; } save_file(argv[1]); continue; }
        else if (strcmp(argv[0], "pwd") == 0) { puts(cwd); continue; }
        else if (strcmp(argv[0], "cd") == 0) { const char *target = (argc>=2)?argv[1]:"/"; if (chdir(target)==-1) printf("cd: %s: %s\n", target, strerror(errno)); continue; }
        else if (strcmp(argv[0], "run") == 0) { printf("What are you running from?\n"); continue; }

        // ---- External programs ----
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            continue;
        }
        if (pid == 0) { // child
            if (try_exec_with_paths(argv) == -1) {
                fprintf(stderr, "exec failed: %s\n", strerror(errno));
                _exit(127); // crucial: never return to init shell
            }
        } else { // parent
            int status;
            while (1) {
                pid_t w = waitpid(pid, &status, 0);
                if (w == -1) {
                    if (errno == EINTR) continue;
                    perror("waitpid failed");
                    break;
                }
                break;
            }
        }
    }

    return 0;
}
