#include "udevmonitor.hpp"

#include <thread>
#include <iostream>

using namespace Ambe3K::Udev;

Monitor::Monitor() {
    std::thread thread([ this ] { run(); });
    thread.detach();
}

void Monitor::run() {
    udev* udev = udev_new();
    if (!udev) {
        std::cerr << "WARNING: could not create udev context\n";
        return;
    }

    udev_monitor* monitor = udev_monitor_new_from_netlink(udev, "udev");
    if (!monitor) {
        std::cerr << "WARNING: could not create udev monitor\n";
        udev_unref(udev);
        return;
    }

    udev_monitor_filter_add_match_subsystem_devtype(monitor, "tty", NULL);
    udev_monitor_enable_receiving(monitor);

    int fd = udev_monitor_get_fd(monitor);
    while (1) {
        fd_set fds;
        struct timeval tv;
        int ret;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        ret = select(fd+1, &fds, NULL, NULL, &tv);
        if (ret > 0 && FD_ISSET(fd, &fds)) {
            udev_device* dev = udev_monitor_receive_device(monitor);
            if (dev) {
                printf("I: ACTION=%s\n", udev_device_get_action(dev));
                printf("I: DEVNAME=%s\n", udev_device_get_sysname(dev));
                printf("I: DEVPATH=%s\n", udev_device_get_devpath(dev));
                printf("I: SUBSYSTEM=%s\n", udev_device_get_subsystem(dev));
                printf("I: DEVTYPE=%s\n", udev_device_get_devtype(dev));
                printf("---\n");

                /* free dev */
                udev_device_unref(dev);
            }
        }
    }

    udev_unref(udev);
}