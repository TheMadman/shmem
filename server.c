#include <sys/mman.h>
#include <spawn.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

extern char **environ;

void print_error(const char *file, int line)
{
	fprintf(stderr, "%s:%d: Error: %s\n", file, line, strerror(errno));
}

#define exit_error() print_error(__FILE__, __LINE__), exit(EXIT_FAILURE)

int main()
{
	int sockets[2] = { 0 };
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0)
		exit_error();
	fcntl(sockets[0], F_SETFD, fcntl(sockets[0], F_GETFD) | FD_CLOEXEC);
	fcntl(sockets[1], F_SETFD, fcntl(sockets[1], F_GETFD) | FD_CLOEXEC);

	posix_spawnattr_t attr;
	posix_spawn_file_actions_t fa;
	int pid;

	char *const client[] = {
		"./client",
		NULL,
	};

	if (posix_spawnattr_init(&attr) != 0)
		exit_error();
	if (posix_spawn_file_actions_init(&fa) != 0)
		exit_error();
	if (posix_spawn_file_actions_adddup2(&fa, sockets[1], 3) != 0)
		exit_error();
	if (posix_spawnp(&pid, client[0], &fa, &attr, client, environ) < 0)
		exit_error();

	close(sockets[1]);

	char data = 0;
	int shmem = -1;

	struct iovec io[] = {
		{
			.iov_base = &data,
			.iov_len = sizeof(data),
		}
	};

	union {
		char buf[CMSG_SPACE(sizeof(int))];
		struct cmsghdr align;
	} u;

	struct msghdr hdr = {
		.msg_iov = io,
		.msg_iovlen = 1,
		.msg_control = &u,
		.msg_controllen = sizeof(u),
	};

	ssize_t received = recvmsg(sockets[0], &hdr, 0);
	if (received < 0)
		exit_error();

	memcpy(&shmem, CMSG_DATA(CMSG_FIRSTHDR(&hdr)), sizeof(shmem));

	int *shm = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shmem, 0);
	if (shm == MAP_FAILED)
		exit_error();

	printf("*shm: %d\n", *shm);
	unlink("/tmp/my-test-socket");
}
