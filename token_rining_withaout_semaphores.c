#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>

#define READ 0
#define WRITE 1
#define MAX 50

int main(int argc, char** argv) {
	// If the argument doesnt exist or is beyond limits, then set to 1.
	int k = (argc > 1) && (atoi(argv[1]) > 0 && atoi(argv[1]) <= MAX) ? atoi(argv[1]) : 1;
	// File descriptors for pipes.
	int fd1[2], fd2[2];
	// Child read and write IO
	int c1_wr, ck_rd;
	int status;
	
	char payload[MAX];
	printf("Please enter a message to pass around: ");
	fgets(payload, 50, stdin);
	printf("\nYou entered: %sI will now pass it around!\n", payload);

	int ppid = getpid();
	printf("Parent with PID %d.\n", ppid);
	sleep(3);

	// Create pipe to write to child.
	if (pipe(fd1) < 0) {
		perror("Plumbing problem.");
		exit(1);
	}

	// Get the write for the first child.
	c1_wr = dup(fd1[WRITE]);
	write(c1_wr, &payload, MAX);
	printf("Spawning %d processes.\n", k);
	for (int i = 0; i < k; i++) {
		sleep(2);
		// Make pipe for communication between children.
		if (pipe(fd2) < 0) {
			perror("Plumbing problem.");
			exit(1);
		}

		fflush(stdout); // necessary?

		pid_t pid = fork();
		if (pid < 0) {
			perror("Fork failed.");
			exit(1);
		} else if (pid == 0) { // child function
			// Close write of input pipe.
			close(fd1[WRITE]);
			// Close read of output pipe.
			close(fd2[READ]);
			// Read in.
			char msg[MAX];
			read(fd1[READ], &msg, MAX);
			printf("Child %d with PID %d got the message: %s", i, getpid(), msg);
			write(fd2[WRITE], &msg, MAX);
			printf("Child %d with PID %d sending message forward.\n", i, getpid());
			close(fd1[READ]);
			close(fd2[WRITE]);
			wait(&status);
			exit(0);
		}
		// Close pipes and move to where next pipe points.
		close(fd1[READ]);
		close(fd1[WRITE]);
		fd1[READ] = fd2[READ];
		fd1[WRITE] = fd2[WRITE];
		printf("%d\n", getpid());
		wait (&pid);
	}

	ck_rd = fd2[READ];
	close(fd2[WRITE]);
	close(c1_wr);

	char msg2[MAX];
	read(ck_rd, &msg2, MAX);
	close(ck_rd);
	printf("Parent %d has message: %s", getpid(), msg2);

	return 0;
}
