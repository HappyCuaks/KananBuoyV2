# NanoStation Loco M5 Configuration Guide

## Overview
This guide provides step-by-step instructions to configure the NanoStation Loco M5 to connect to your Wi-Fi network.

## Steps

### 1. Access NanoStation Web Interface
1. Connect your PC to the NanoStation via an Ethernet cable. We used the bridge that comes with the LocoM5. Connecting the PC to the LAN port, and the Loco M5 to the PoE port.
2. Set your PC’s IP to a static address:
    1. Go to Control Panel\Network and Internet\Network Connections
    2. Right click on the new Ethernet icon that appeared due to connecting the PC to the bridge > Properties
    3. Under "Networking" Tab, find item "Internet Protocol Version 4 (TCP/IPv4)" and click on "Properties" while having the item selected.
    4. Give the PC a static address by setting option "Use the following IP address:":
        - **IP Address**: `192.168.1.32`
        - **Subnet Mask**: `255.255.255.1`
        - **Default Gateway**: `192.168.1.1`
    5. Push "OK" button when done.
3. Make sure that only the Ethernet network is enabled. 
4. Open a web browser and navigate to `http://192.168.1.20`.
5. Log in with the default credentials:
   - **Username**: `ubnt`
   - **Password**: `ubnt`

### 2. Change IP Address (Optional)
1. Navigate to the `Network` tab.
2. Configure the following settings:
   - **IP Address**: `192.168.1.20` 
   - **Gateway IP**: `192.168.1.1`
   - **Netmask**: `255.255.255.0`
3. Click `Change` and then `Apply`.

### 3. Configure Wireless Settings
1. Go to the `Wireless` tab.
2. Set `Wireless Mode` to `Station`.
3. Click `Select` to scan for available networks.
4. Choose your Wi-Fi network.
5. Enter your Wi-Fi password in the `WPA Preshared Key` field.
6. Ensure the `SSID` matches your Wi-Fi network name.
7. Click `Change` and then `Apply`.
   - **Note**: The NanoStation may ask you to change its password. Please make sure it is written somewhere on the NanoStation casing.

### 4. Finalize Configuration
1. Reset your PC’s IP settings to obtain an address automatically via DHCP.
2. Disconnect Ethernet connection from PC.
3. Connect PC to Wi-Fi network
4. Open a web browser and navigate to `http://192.168.1.20`.
3. Verify the connection and signal strength in the NanoStation web interface.

## Additional Tips
- **Firmware**: Ensure the NanoStation firmware is up to date. Download the latest firmware from the [Ubiquiti website](https://www.ui.com/download/).
- **Security**: Change the default login credentials to enhance security.
- **Placement**: Position the NanoStation for optimal Wi-Fi signal strength.