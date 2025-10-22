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
    DIR *d = opendir("/sys/bus/i2c/devices");
    struct dirent *entry;

    if (d) {
        while ((entry = readdir(d)) != NULL) {
            if (entry->d_name[0] == '.') continue;
            if (strncmp(entry->d_name, "i2c-", 4) == 0) continue;

            char *dash = strchr(entry->d_name, '-');
            if (!dash) continue;

            int bus = atoi(entry->d_name);
            char devpath[128];
            snprintf(devpath, sizeof(devpath), "/dev/i2c-%d", bus);

            struct stat st;
            if (stat(devpath, &st) != 0) continue;

            char name[64] = {0};
            char namepath[PATH_MAX];
            snprintf(namepath, sizeof(namepath), "/sys/bus/i2c/devices/%s/name", entry->d_name);

            FILE *f = fopen(namepath, "r");
            if (f) {
                if (fgets(name, sizeof(name), f)) {
                    size_t n = strcspn(name, "\r\n");
                    name[n] = '\0';
                }
                fclose(f);
            }
            if (name[0] == '\0') {
                strncpy(name, entry->d_name, sizeof(name) - 1);
                name[sizeof(name) - 1] = '\0';
            }

            devices_add(name, DEVICE_I2C_SENSOR, devpath);
        }
        closedir(d);
    }

    d = opendir("/dev");
    if (d) {
        while ((entry = readdir(d)) != NULL) {
            if (strncmp(entry->d_name, "i2c-", 4) != 0) continue;

            char path[128] = "/dev/";
            if (strlen(path) + strlen(entry->d_name) >= sizeof(path)) continue;
            strcat(path, entry->d_name);

            devices_add(entry->d_name, DEVICE_I2C_SENSOR, path);
        }
        closedir(d);
    }
}

void devices_detect_arduino() {
    // @Incomplete: Write implementation.
}

void devices_reset_state() {
    for (int i = 0; i < dev_info->devices_length; i++) {
        dev_info->devices[i].state = OFFLINE;
    }
}

int devices_detect() {
    devices_reset_state();

    devices_detect_usb();
    devices_detect_i2c();
    devices_detect_arduino();

    // Just for test searching other usb.
    Device *usb = NULL;
    for (int i = 0; i < dev_info->devices_length; i++) {
        if (strcmp(dev_info->devices[i].name, "sdb") != 0) {
            usb = dev_info->devices + i;
            break;
        }
    }
    
    if (usb != NULL) {
        devices_ensure_usb_connected(usb);
    }

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
