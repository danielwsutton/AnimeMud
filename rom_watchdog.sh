#!/bin/bash

# Check if rom is running.
if ! pidof rom > /dev/null; then
    echo "$(date): ROM was not running, retarting..." >> ~/rom_watchdog.log
    cd /home/ubuntu/animemud/src
    nohup ./rom > ~/rom_output.log 2>&1 &
fi