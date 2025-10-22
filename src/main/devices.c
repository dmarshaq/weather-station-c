#include "main/devices.h"

#include "core/log.h"

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

static Devices_Info *dev_info;


int devices_init(Devices_Info *devices_info) {
    // Save pointer internally since it is used in other functions after init.
    dev_info = devices_info;

    // Set devices length to zero in the beginning.
    dev_info->devices_length = 0;



    // Open csv for output of data.
    // @Todo: Move it to a different part of code, it should execute only if USB we are writting to is found.
    srand(time(NULL));
    FILE *file = fopen("data.csv", "a");

    if (file == NULL) {
        return -1;
    }

    dev_info->csv_output = file;

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
            LOG_WARNING("Added device '%s' already exists, skipping.", name);
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
        .state = OFFLINE,
        .fd = -1,
    };

    // Save name.
    strcpy(dev_info->devices[dev_info->devices_length].name, name);
    
    // Save path.
    strcpy(dev_info->devices[dev_info->devices_length].path, path);


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
            LOG_INFO("Detected removable USB drive: '%s'.", path);
            devices_add(entry->d_name, DEVICE_USB_DRIVE, path);
        }
    }

    closedir(d);
}

void devices_detect_i2c() {
    // @Incomplete: Write implementation.
}

void devices_detect_arduino() {
    // @Incomplete: Write implementation.
}

int devices_detect() {
    devices_detect_usb();
    devices_detect_i2c();
    devices_detect_arduino();


    return 0;
}

int devices_connect() {

    return 0;
}

int devices_collect_data(Data_Point *data_point) {
    if (data_point == NULL) {
        return -1;
    }

    // Used to approximate values
    // https://weatherspark.com/y/20372/Average-Weather-in-Buffalo-New-York-United-States-Year-Round

    data_point->temperature = 17.8f + ((float)rand() / (float)RAND_MAX * (26.1f - 17.8f));
    data_point->humidity = 0.0f + ((float)rand() / (float)RAND_MAX * (31.0f - 0.0f));
    data_point->wind_speed = 2.68f + ((float)rand() / (float)RAND_MAX * (5.32f - 2.68f));
    data_point->wind_direction = 0.0f + ((float)rand() / (float)RAND_MAX * (360.0f - 0.0f));
    data_point->pressure = 979.35f + ((float)rand() / (float)RAND_MAX * (1048.75f - 979.35f));
    data_point->precipitation = 30.48f + ((float)rand() / (float)RAND_MAX * (73.66f - 30.48f));
    data_point->uv_index = 0.0f + ((float)rand() / (float)RAND_MAX * (15.0f - 0.0f));

    time_t t = time(NULL);
    struct tm *local_time = localtime(&t);
    if (local_time != NULL) {
        data_point->timestamp = *local_time;
    }

    return 0;
}

void readSerial(int serial_port, Data_Point* current_data_point){
    //256 bit buffer to read from serial port

    /* Byte codes
    0x01 - Temperature
    0x02 - Humidity
    0x03 - Wind speed
    0x04 - Wind direction
    0x05 - Pressure
    0x06 - Precipitation
    0x07 - UV Index
    */

    //tcflush(serial_port, TCIFLUSH);
    unsigned char read_buf[8];
    memset(read_buf, 0x00, sizeof(read_buf));
    
    //If error opening serial port, return
    if(serial_port < 0){
        LOG_ERROR("Error opening serial port");
        return;
    }
    //Create termios struct
    //Helps control serial communication with flags
    struct termios tty;
    //Clear memory
    memset(&tty, 0, sizeof(tty));
    //Get serial port attributes and store them in tty variable
    if(tcgetattr(serial_port, &tty) != 0){
        LOG_ERROR("Error getting termios attributes");
        close(serial_port);
        return;
    }
    //Set baud rate (Must match Serial.begin number in Arduino)
    cfsetispeed(&tty, B9600);
    //Output speed only required if writing to serial port
    //cfsetospeed(&tty, B9600);
    
    //8 bits, no parity (extra error correction bits), 1 stop bit (8n1)
    //&= - Bitwise operator where x &= y is equivalent to x = x & y
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8; //8 bits per byte

    //No flow control
    tty.c_cflag &= ~CRTSCTS;

    //Turn on read and ignore ctrl lines
    tty.c_cflag |= CREAD | CLOCAL;

    //Make raw
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // No software flow control

    // Set timeout
    tty.c_cc[VMIN] = 1; //1 bytes minimum to read
    tty.c_cc[VTIME] = 5; // 5 second read timeout

    // Save settings
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        LOG_ERROR("Error setting termios attributes");
        close(serial_port);
        return;
    }

    //Get to start of message
    char a = 0x00;
    while(a != '\n'){
        read(serial_port, &a, 1);
    }

    // Read data (8 bytes)
    int total_read = 0;
    int bytes_to_read = 8;
    while (total_read < bytes_to_read) {
        int n = read(serial_port, read_buf + total_read, bytes_to_read - total_read);
        if (n > 0) {
            total_read += n;
        } else if (n == 0) {
            // Timeout or no data
            break;
        } else {
            LOG_ERROR("read error");
            break;
        }
    }
    /*
    if (num_bytes < 0) {
        LOG_ERROR("Error reading from serial port");
        close(serial_port);
        return read_buf;
    }
    */

    //close(serial_port);

    /* Byte codes
    0x01 - Temperature
    0x02 - Humidity
    0x03 - Wind speed
    0x04 - Wind direction
    0x05 - Pressure
    0x06 - Precipitation
    0x07 - UV Index
    */
    /*
    for(int i = 0; i < sizeof(read_buf) - 1; i++){
        printf("%#x ", read_buf[i]);//*(read_buf + i));
    }
    */
    switch(read_buf[0]){
        case 0x01:
            current_data_point->temperature = *((float*)(read_buf + 1));
            break;
        case 0x02:
            current_data_point->humidity = *((float*)(read_buf + 1));
            break;
        case 0x03:
            current_data_point->wind_speed = *((float*)(read_buf + 1));
            break;
        case 0x04:
            current_data_point->wind_direction = *((float*)(read_buf + 1));
            break;
        case 0x05:
            current_data_point->pressure = *((float*)(read_buf + 1));
            break;
        case 0x06:
            current_data_point->precipitation = *((float*)(read_buf + 1));
            break;
        case 0x07:
            current_data_point->uv_index = *((float*)(read_buf + 1));
            break;
    }
    return;
}