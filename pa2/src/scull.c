#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <pthread.h>

#include "scull.h"

#define CDEV_NAME "/dev/scull"

/* Quantum command line option */
static int g_quantum;

static void usage(const char *cmd)
{
	printf("Usage: %s <command>\n"
	       "Commands:\n"
	       "  R          Reset quantum\n"
	       "  S <int>    Set quantum\n"
	       "  T <int>    Tell quantum\n"
	       "  G          Get quantum\n"
	       "  Q          Query quantum\n"
	       "  X <int>    Exchange quantum\n"
	       "  H <int>    Shift quantum\n"
	       "  h          Print this message\n",
	       cmd);
}

typedef int cmd_t;

static cmd_t parse_arguments(int argc, const char **argv)
{
	cmd_t cmd;

	if (argc < 2) {
		fprintf(stderr, "%s: Invalid number of arguments\n", argv[0]);
		cmd = -1;
		goto ret;
	}

	/* Parse command and optional int argument */
	cmd = argv[1][0];
	switch (cmd) {
	case 'S':
	case 'T':
	case 'H':
	case 'X':
		if (argc < 3) {
			fprintf(stderr, "%s: Missing quantum\n", argv[0]);
			cmd = -1;
			break;
		}
		g_quantum = atoi(argv[2]);
		break;
	case 'R':
	case 'G':
	case 'Q':
	case 'h':
		break;
	case 'i':
		break;
	case 'p':
		break;
	case 't':
		break;
	default:
		fprintf(stderr, "%s: Invalid command\n", argv[0]);
		cmd = -1;
	}

ret:
	if (cmd < 0 || cmd == 'h') {
		usage(argv[0]);
		exit((cmd == 'h')? EXIT_SUCCESS : EXIT_FAILURE);
	}
	return cmd;
}

/* function ran in case t when pthread_create is called*/
void* tfunction(int *fd)
{
	struct task_info *t_info = malloc(sizeof(struct task_info));
	int ret;
	for(int j = 0; j < 2; j++) {	
		/*Set t_info with call to driver*/
		ret = ioctl(*fd, SCULL_IOCIQUANTUM, t_info);
		/* Print task_info values */
		printf("state %ld, stack %p, cpu %d, prio %d, sprio %d, nprio %d, rtprio %d, pid %ld, tgid %ld, nv %lu, niv %lu\n", t_info->state, t_info->stack, t_info->cpu, t_info->prio,
			t_info->static_prio, t_info->normal_prio, t_info->rt_priority, t_info->pid, t_info->tgid, t_info->nvcsw, t_info->nivcsw);	
	}
	/* deallocate memory */
	free(t_info);
	return NULL;
}

static int do_op(int fd, cmd_t cmd)
{
	/* declare variables to be used in case statements */
	int ret, q;
	struct task_info *t_info = malloc(sizeof(struct task_info));
	pid_t pid = NULL;
	pthread_t ptid1, ptid2, ptid3, ptid4;

	switch (cmd) {
	case 'R':
		ret = ioctl(fd, SCULL_IOCRESET);
		if (ret == 0)
			printf("Quantum reset\n");
		break;
	case 'Q':
		q = ioctl(fd, SCULL_IOCQQUANTUM);
		printf("Quantum: %d\n", q);
		ret = 0;
		break;
	case 'G':
		ret = ioctl(fd, SCULL_IOCGQUANTUM, &q);
		if (ret == 0)
			printf("Quantum: %d\n", q);
		break;
	case 'T':
		ret = ioctl(fd, SCULL_IOCTQUANTUM, g_quantum);
		if (ret == 0)
			printf("Quantum set\n");
		break;
	case 'S':
		q = g_quantum;
		ret = ioctl(fd, SCULL_IOCSQUANTUM, &q);
		if (ret == 0)
			printf("Quantum set\n");
		break;
	case 'X':
		q = g_quantum;
		ret = ioctl(fd, SCULL_IOCXQUANTUM, &q);
		if (ret == 0)
			printf("Quantum exchanged, old quantum: %d\n", q);
		break;
	case 'H':
		q = ioctl(fd, SCULL_IOCHQUANTUM, g_quantum);
		printf("Quantum shifted, old quantum: %d\n", q);
		ret = 0;
		break;
	case 'i':
		ret = ioctl(fd, SCULL_IOCIQUANTUM, t_info);
		printf("state %ld, stack %p, cpu %d, prio %d, sprio %d, nprio %d, rtprio %d, pid %ld, tgid %ld, nv %lu, niv %lu\n", t_info->state, t_info->stack, t_info->cpu, t_info->prio,
			t_info->static_prio, t_info->normal_prio, t_info->rt_priority, t_info->pid, t_info->tgid, t_info->nvcsw, t_info->nivcsw);
		break;
	case 'p':
		/* Create 4 child processes */
		for(int i = 0; i < 4; i++) {
			pid = fork();
			/* check to see if process is child */
			if(pid == 0) {
				for(int j = 0; j < 2; j++) {
					/*Set t_info with call to driver*/
					ret = ioctl(fd, SCULL_IOCIQUANTUM, t_info);
					/* Print task_info values */
					printf("state %ld, stack %p, cpu %d, prio %d, sprio %d, nprio %d, rtprio %d, pid %ld, tgid %ld, nv %lu, niv %lu\n", t_info->state, t_info->stack, t_info->cpu,
				    	   	t_info->prio, t_info->static_prio, t_info->normal_prio, t_info->rt_priority, t_info->pid, t_info->tgid, t_info->nvcsw, t_info->nivcsw);
				}
				exit(0);
			}
		}
		/* wait until all processes are done */
		for(int i = 0; i < 4; i++) {
			wait(NULL);
		}
		break;
	case 't':
		/* create 4 threads with separate pid */
		pthread_create(&ptid1, NULL, &tfunction, &fd);
		pthread_create(&ptid2, NULL, &tfunction, &fd);
		pthread_create(&ptid3, NULL, &tfunction, &fd);
		pthread_create(&ptid4, NULL, &tfunction, &fd);

		/* join threads */
		pthread_join(ptid1, NULL);
		pthread_join(ptid2, NULL);
		pthread_join(ptid3, NULL);
		pthread_join(ptid4, NULL);

		ret = 0;
		break;
	default:
		/* Should never occur */
		abort();
		ret = -1; /* Keep the compiler happy */
	}

	if (ret != 0)
		perror("ioctl");
	return ret;
}

int main(int argc, const char **argv)
{
	int fd, ret;
	cmd_t cmd;

	cmd = parse_arguments(argc, argv);

	fd = open(CDEV_NAME, O_RDONLY);
	if (fd < 0) {
		perror("cdev open");
		return EXIT_FAILURE;
	}

	printf("Device (%s) opened\n", CDEV_NAME);

	ret = do_op(fd, cmd);

	if (close(fd) != 0) {
		perror("cdev close");
		return EXIT_FAILURE;
	}

	printf("Device (%s) closed\n", CDEV_NAME);

	return (ret != 0)? EXIT_FAILURE : EXIT_SUCCESS;
}
