#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

const int sock = 3;

void print_error(const char *file, int line)
{
	fprintf(stderr, "%s:%d: Error: %s\n", file, line, strerror(errno));
}

#define exit_error() print_error(__FILE__, __LINE__), exit(EXIT_FAILURE)

int main()
{
	static char data = 0;

	int shmem = shm_open("/my-test-shmem", O_RDWR | O_CREAT | O_EXCL, 0600);
	if (shmem < 0)
		exit_error();

	if (ftruncate(shmem, (off_t)sizeof(int)) < 0)
		exit_error();

	int *mem = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shmem, 0);
	if (mem == MAP_FAILED)
		exit_error();

	shm_unlink("/my-test-shmem");

	*mem = 6;

	struct iovec io[] = {
		{
			.iov_base = &data,
			.iov_len = sizeof(data),
		}
	};

	union {
		char buf[CMSG_SPACE(sizeof(shmem))];
		struct cmsghdr align;
	} u;

	struct msghdr hdr = {
		.msg_iov = io,
		.msg_iovlen = 1,
		.msg_control = &u,
		.msg_controllen = sizeof(u),
	};

	struct cmsghdr *cmsg = CMSG_FIRSTHDR(&hdr);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(shmem));
	memcpy(CMSG_DATA(cmsg), &shmem, sizeof(shmem));

	sendmsg(sock, &hdr, 0);
}
