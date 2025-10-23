#include "main/devices.h"

#include "core/log.h"
#include "core/str.h"

#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <libgen.h>
#include <sys/stat.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>

static Devices_Info *dev_info;


int devices_init(Devices_Info *devices_info) {
    // Save pointer internally since it is used in other functions after init.
    dev_info = devices_info;

    // Set devices length to zero in the beginning.
    dev_info->devices_length = 0;

    srand(time(NULL));



    return 0;
}

// Helper function to add new devices that the program is connected to.
void devices_add(char *name, Device_Type type, char *path) {
    if (dev_info->devices_length >= MAX_DEVICES) {
        LOG_ERROR("Cannot add more devices, max devices reached.");
        return;
    }

    for (int i = 0; i < dev_info->devices_length; i++) {
        if (strcmp(name, dev_info->devices[i].name) == 0) {
            dev_info->devices[i].state = ONLINE;
            return;
        }
    }

    if (strlen(name) >= 32) {
        LOG_ERROR("Cannot add device '%s', name is longer than 32 bytes.", name);
        return;
    }

    if (strlen(path) >= 128) {
        LOG_ERROR("Cannot add device '%s', path is longer than 128 bytes.", name);
        return;
    }

    dev_info->devices[dev_info->devices_length] = (Device) {
        .type = type,
        .state = ONLINE,
    };

    // Save name.
    strcpy(dev_info->devices[dev_info->devices_length].name, name);
    
    // Save path.
    strcpy(dev_info->devices[dev_info->devices_length].path, path);

    switch(dev_info->devices[dev_info->devices_length].type) {
        case DEVICE_ARDUINO:
            int serial_port = open(dev_info->devices[dev_info->devices_length].path, O_RDWR);
            tcflush(serial_port, TCIFLUSH);
            dev_info->devices[dev_info->devices_length].serial_port = serial_port;
            break;
    }

    dev_info->devices_length++;
}

int is_removable_usb(const char *devname) {
    char path[PATH_MAX];
    char buf[128];

    // Construct path to removable file
    snprintf(path, sizeof(path), "/sys/block/%s/removable", devname);
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    if (!fgets(buf, sizeof(buf), f)) {
        fclose(f);
        return 0;
    }
    fclose(f);

    // Check if device is marked removable
    if (buf[0] != '1') return 0;

    // Resolve the real path of the device
    snprintf(path, sizeof(path), "/sys/block/%s", devname);
    char realpath_buf[PATH_MAX];
    if (!realpath(path, realpath_buf)) return 0;

    // Walk up the directory tree to see if it's connected via USB
    char *p = realpath_buf;
    while (strcmp(p, "/sys") != 0) {
        snprintf(path, sizeof(path), "%s/subsystem", p);
        ssize_t len = readlink(path, buf, sizeof(buf)-1);
        if (len > 0) {
            buf[len] = '\0';
            if (strstr(buf, "usb")) {
                return 1;
            }
        }

        // Go one level up
        p = dirname(p);
    }

    return 0;
}


void devices_detect_usb() {
    // For example: Detecting if any USB's are connected.
    DIR *d = opendir("/sys/block");
    struct dirent *entry;

    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        if (is_removable_usb(entry->d_name)) {
            char path[128] = "/dev/";
            strcat(path, entry->d_name);

            // LOG_INFO("Detected removable USB drive: '%s'.", path);
            // LOG_INFO("Assuming there exists 1st partition on usb: '%s1'.", path);

            size_t l = strlen(path);

            if (l > 126) {
                LOG_ERROR("Detected usb couldn't be added, path buffer overflow, when accessing partition 1.");
            }

            path[l] = '1';
            path[l + 1] = '\0';

            devices_add(entry->d_name, DEVICE_USB_DRIVE, path);
        }
    }

    closedir(d);
}

int is_mounted_usb(Device *usb, char *mount_path_buffer, size_t mount_path_buffer_size) {
    FILE *fp = fopen("/proc/mounts", "r");
    if (!fp) {
        LOG_ERROR("Couldn't mount usb: couldn't open to read '/proc/mounts' file.");
        return 0;
    }

    char dev[256], mount[256], rest[512];
    int found = 0;

    // Read through mounted usb devices, see if device name matches, if it does, set found to 1 and break.
    while (fscanf(fp, "%255s %255s %511[^\n]\n", dev, mount, rest) == 3) {
        // printf("Comparing %s to %s.\n", dev, usb->path);
        if (strcmp(dev, usb->path) == 0) {
            strncpy(mount_path_buffer, mount, mount_path_buffer_size - 1);
            mount_path_buffer[mount_path_buffer_size - 1] = '\0';
            found = 1;
            break;
        }
    }

    fclose(fp);

    return found;
}

int mount_usb_device(Device *usb, const char *target) {
    // using cmd command to do it since it is more robust and easier.
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s && mount %s %s", target, usb->path, target);
    return system(cmd);
}

void devices_ensure_usb_connected(Device *usb) {
    char mount_path[256];

    if (is_mounted_usb(usb, mount_path, 256) == 0) {
        // Mount usb here.
        // strrchar will find the last occurence of '/' and return the pointer to it, example: from "dev/sdb1" we will get "/sdb1".
        snprintf(mount_path, sizeof(mount_path), "/mnt/usb_%s", strrchr(usb->path, '/') + 1);

        LOG_INFO("Mounting '%s' to '%s'.", usb->name, mount_path);
        if (mount_usb_device(usb, mount_path) == 0)
            LOG_INFO("Successfully mounted '%s'.", usb->name);
    }
    
    if (dev_info->csv_output == NULL) {
        char output_path[512];

        strcpy(output_path, mount_path);
        strcpy(output_path + strlen(mount_path), "/data.csv");

        // Open csv for output of data.
        FILE *file = fopen(output_path, "a");

        if (file == NULL) {
            LOG_ERROR("Couldn't open file to output data on the mounted usb at '%s'.", output_path);
            perror("error: ");
            return;
        }

        dev_info->csv_output = file;
    }
    
    
}

void devices_detect_i2c() {
    // @Incomplete: Write implementation.
}

void devices_detect_arduino() {
    DIR *d = opendir("/dev");
    if(!d) return;

    struct dirent *entry;
     while ((entry = readdir(d)) != NULL) {
        if(entry->d_name[0] == '.') continue;
        if(strncmp(entry->d_name, "ttyACM", 6) == 0 || strncmp(entry->d_name, "ttyUSB", 6) == 0) {
            char path[128];
            snprintf(path, sizeof(path), "/dev/%s", entry->d_name);
            LOG_INFO("Arduino: '%s'", path);
            devices_add(entry->d_name, DEVICE_ARDUINO, path);
        }
     }
     closedir(d);
}

void devices_reset_state() {
    for (int i = 0; i < dev_info->devices_length; i++) {
        dev_info->devices[i].state = OFFLINE;
    }
}

#define USB_FEATURE_OFF

int devices_detect() {
    devices_reset_state();

    devices_detect_usb();
    devices_detect_i2c();
    devices_detect_arduino();

#ifndef USB_FEATURE_OFF
    Device *usb = NULL;
    for (int i = 0; i < dev_info->devices_length; i++) {
        if (dev_info->devices[i].type == DEVICE_USB_DRIVE) {
            usb = dev_info->devices + i;
            break;
        }
    }
    
    if (usb != NULL) {
        devices_ensure_usb_connected(usb);
    }
#else
    if (dev_info->csv_output == NULL) {
        // Open csv for output of data.
        FILE *file = fopen("data.csv", "a");

        if (file == NULL) {
            LOG_ERROR("Couldn't open file to output data on the mounted usb at 'data.csv'.");
            perror("Error");
            return 0;
        }

        dev_info->csv_output = file;
    }
#endif

    



    return 0;
}

int devices_collect_data(Data_Point *data_point) {
    if (data_point == NULL) {
        return -1;
    }

    // Used to approximate values
    // https://weatherspark.com/y/20372/Average-Weather-in-Buffalo-New-York-United-States-Year-Round

    // data_point->temperature = 17.8f + ((float)rand() / (float)RAND_MAX * (26.1f - 17.8f));
    // data_point->humidity = 0.0f + ((float)rand() / (float)RAND_MAX * (31.0f - 0.0f));
    // data_point->wind_speed = 2.68f + ((float)rand() / (float)RAND_MAX * (5.32f - 2.68f));
    // data_point->wind_direction = 0.0f + ((float)rand() / (float)RAND_MAX * (360.0f - 0.0f));
    // data_point->pressure = 979.35f + ((float)rand() / (float)RAND_MAX * (1048.75f - 979.35f));
    // data_point->precipitation = 30.48f + ((float)rand() / (float)RAND_MAX * (73.66f - 30.48f));
    // data_point->uv_index = 0.0f + ((float)rand() / (float)RAND_MAX * (15.0f - 0.0f));


    Device *arduino = NULL;
    for (int i = 0; i < dev_info->devices_length; i++) {
        if (dev_info->devices[i].type == DEVICE_ARDUINO) {
            arduino = dev_info->devices + i;
            if (arduino->serial_port > 0) {
                read_serial(arduino->serial_port, data_point);
            }
            break;
        }
    }


    time_t t = time(NULL);
    struct tm *local_time = localtime(&t);
    if (local_time != NULL) {
        data_point->timestamp = *local_time;
    }



    return 0;
}

void read_serial(int fd, Data_Point* current_data_point) {

	/* Byte codes
	   0x01 - Temperature
	   0x02 - Humidity
	   0x03 - Wind speed
	   0x04 - Wind direction
	   0x05 - Pressure
	   0x06 - Precipitation
	   0x07 - UV Index
	   */

	//Create termios struct
	//Helps control serial communication with flags
	struct termios tty = {0};

	//Get serial port attributes and store them in tty variable
	if(tcgetattr(fd, &tty) != 0){
		LOG_ERROR("Couldn't get termios attributes.");
		close(fd);
		return;
	}
	// Output speed only required if writing to serial port
	cfsetospeed(&tty, B9600);
	// Set baud rate (Must match Serial.begin number in Arduino)
	cfsetispeed(&tty, B9600);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;  // 8-bit chars
	tty.c_iflag &= ~IGNBRK;                      // disable break processing
	tty.c_lflag = 0;                             // no signaling chars, no echo
	tty.c_oflag = 0;                             // no remapping, no delays
	tty.c_cc[VMIN]  = 1;                         // read blocks until 1 byte
	tty.c_cc[VTIME] = 1;                         // 0.1 sec timeout

	tty.c_iflag &= ~(IXON | IXOFF | IXANY);      // shut off xon/xoff ctrl
	tty.c_cflag |= (CLOCAL | CREAD);             // ignore modem controls, enable read
	tty.c_cflag &= ~(PARENB | PARODD);           // no parity
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;                     // no hardware flow control

	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		perror("tcsetattr");
		return;
	}

	char buffer[64];

	ssize_t len = 64;

	char *ptr = buffer;
	ssize_t ret = 0;
	while((ret = read(fd, ptr, len)) != 0) {
		if (ret == -1) {
			if (errno == EINTR)
				continue;

			fprintf(stderr, "Couldn't read arduino serial port.");
			break;
		}

		ptr += ret;
		len -= ret;
	}

	char *start;
	char *end;

	for (size_t i = 0; i < ptr - buffer; i++) {
		if (buffer[i] == 0xAA) {
			start = buffer + i;
			for (size_t j = i; j < ptr - buffer; j++) {
				if (buffer[j] == 0xBB) {
					end = buffer + j + 1;
					goto loop_exit;
				}
			}
			return;
		}
	}
	return;

loop_exit:

    LOG_INFO("Received: %d bytes from the arduino, identified payload of size: %d bytes.", ptr - buffer, end - start);

	/* Byte codes
	   0x01 - Temperature
	   0x02 - Humidity
	   0x03 - Wind speed
	   0x04 - Wind direction
	   0x05 - Pressure
	   0x06 - Precipitation
	   0x07 - UV Index
	   */

	// Because first and last bytes are 0xAA and 0xBB we exclude them.
	for (size_t i = 1; i < end - start - 1; i++) {
		printf("0x%02X: ", start[i]);
		switch(start[i]) {
			case 0x01:
				printf("Temperature: %s\n", start + i + 1);
                current_data_point->temperature = str_parse_float(CSTR(start + i + 1));
				break;
			case 0x02:
				printf("Humidity: %s\n", start + i + 1);
                current_data_point->humidity = str_parse_float(CSTR(start + i + 1));
				break;
			case 0x03:
				printf("Wind speed: %s\n", start + i + 1);
                current_data_point->wind_speed = str_parse_float(CSTR(start + i + 1));
				break;
			case 0x04:
				printf("Wind direction: %s\n", start + i + 1);
                current_data_point->wind_direction = str_parse_float(CSTR(start + i + 1));
				break;
			case 0x05:
				printf("Pressure: %s\n", start + i + 1);
                current_data_point->pressure = str_parse_float(CSTR(start + i + 1));
				break;
			case 0x06:
				printf("Precipitation: %s\n", start + i + 1);
                current_data_point->precipitation = str_parse_float(CSTR(start + i + 1));
				break;
			case 0x07:
				printf("UV Index: %s\n", start + i + 1);
                current_data_point->uv_index = str_parse_float(CSTR(start + i + 1));
				break;
		}
		i += strlen(start + i + 1) + 1;
	}

    return;
}
