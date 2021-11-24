#include "udevmonitor.hpp"
#include "registry.hpp"
#include "ambe3kregistry.hpp"

#include <thread>
#include <iostream>
#include <cstring>

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
    fd_set fds;
    struct timeval tv {};
    int ret;

    while (true) {
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 10;
        tv.tv_usec = 0;

        ret = select(fd+1, &fds, NULL, NULL, &tv);
        if (ret > 0 && FD_ISSET(fd, &fds)) {
            udev_device* dev = udev_monitor_receive_device(monitor);
            if (dev) {
                std::string action = udev_device_get_action(dev);
                std::string node = udev_device_get_devnode(dev);
                Registration* match = Registry::get()->findByNode(node);
                if (action == "remove") {
                    if (match && match->device) {
                        std::cerr << "unregistering device " << node << " due to udev remove event\n";
                        CodecServer::Registry::get()->unregisterDevice(match->device);
                        match->device = nullptr;
                    }
                } else if (action == "add") {
                    if (match && !match->device) {
                        CodecServer::Registry::get()->loadDeviceFromConfig(match->config);
                    }
                }

                /* free dev */
                udev_device_unref(dev);
            }
        }
    }

    udev_unref(udev);
}