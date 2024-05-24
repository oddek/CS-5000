#!/bin/bash

set -e

sudo systemctl restart isc-dhcp-server
sudo systemctl status isc-dhcp-server


