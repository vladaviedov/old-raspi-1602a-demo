#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <i2c/smbus.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define I2C_PATH "/dev/i2c-%d"
#define I2C_ADDR 0x27

#define EN 0b00000100
#define RW 0b00000010
#define RS 0b00000001

#define DELAY 500000

#define BACKLIGHT 0x08
#define TWO_LINE 0x08
#define DISPLAY_ON 0x04
#define CURSOR_ON 0x02
#define BLINK_ON 0x01
#define ENTRY_LEFT 0x02
#define MOVE_BIT 0x10
#define DISPLAY_MOVE 0x08

#define CLEAR_DISPLAY 0x01
#define RETURN_HOME 0x02
#define ENTRYMODE_SET 0x04
#define DISPLAY_CONTROL 0x08
#define FUNCTION_SET 0x20

int find_device(void);
void send_char(int fd, char ch);
void init(int fd);
void i2c_write(int fd, uint8_t data);
void i2c_split(int fd, uint8_t data);
void i2c_split_rs(int fd, uint8_t data);

int main() {
	int fd = find_device();
	init(fd);
	printf("init done\n");
	
	time_t tm = time(NULL);
	char *t = ctime(&tm);
	int ctr = 0;
	while (1) {
		if (*t == '\0') {
			time_t ntm = time(NULL);
			t = ctime(&ntm);
		}
		if (ctr > 16) {
			i2c_split(fd, DISPLAY_MOVE | MOVE_BIT);
		}
		send_char(fd, *t++);
		ctr++;
		usleep(DELAY);
	}

	return 0;
}

int find_device(void) {
	int fd;
	for (int i = 0; i < 4; i++) {
		char bus[32];
		snprintf(bus, 32, I2C_PATH, i);
		printf("trying bus %s\n", bus);

		if ((fd = open(bus, O_RDWR)) < 0) {
			printf("failed\n");
			continue;
		}
		if (ioctl(fd, I2C_SLAVE, I2C_ADDR) < 0) {
			printf("failed\n");
			continue;
		}
		if (i2c_smbus_read_byte(fd) < 0) {
			printf("failed\n");
			continue;
		}

		break;
	}
	
	if (fd < 0)
		exit(1);
	return fd;
}

void send_char(int fd, char ch) {
	printf("writing char %c\n", ch);
	i2c_split_rs(fd, ch);
}

void init(int fd) {
	i2c_split(fd, RETURN_HOME);
	usleep(2000);
	i2c_split(fd, CLEAR_DISPLAY);
	usleep(2000);
	i2c_split(fd, FUNCTION_SET);
}

void i2c_write(int fd, uint8_t data) {
	i2c_smbus_write_byte(fd, data | BACKLIGHT);
	i2c_smbus_write_byte(fd, data | EN | BACKLIGHT);
	usleep(50);
	i2c_smbus_write_byte(fd, (data & ~EN) | BACKLIGHT);
	usleep(50);
}

void i2c_split(int fd, uint8_t data) {
	uint8_t top = (data & 0xF0);
	uint8_t bottom = ((data << 4) & 0xF0);
	i2c_write(fd, top);
	i2c_write(fd, bottom);
}

void i2c_split_rs(int fd, uint8_t data) {
	uint8_t top = (data & 0xF0) | RS;
	uint8_t bottom = ((data << 4) & 0xF0) | RS;
	i2c_write(fd, top);
	i2c_write(fd, bottom);
}
