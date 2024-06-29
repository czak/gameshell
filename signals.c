#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/signalfd.h>

#include "log.h"

static struct {
	int sfd;
	void (*on_child)(uint32_t pid, int32_t code);
} signals;

void signals_init(void (*on_child)(uint32_t pid, int32_t code))
{
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	signals.sfd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
	signals.on_child = on_child;
}

int signals_get_fd()
{
	return signals.sfd;
}

void signals_dispatch()
{
	struct signalfd_siginfo si;
	int n = read(signals.sfd, &si, sizeof(si));

	if (n > 0) {
		if (signals.on_child)
			signals.on_child(si.ssi_pid, si.ssi_code);
	}
	else if (n < 0 && errno != EWOULDBLOCK) {
		LOG("Failed to read signalfd");
	}
}
