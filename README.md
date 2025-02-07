# KananBuoyV2
Kanan Buoy v2 brings a new approach using a videovigilance camera plus a Wireless NanoStation

## Bill of Materials (BOM) for Prototype

| **Component**                   | **Quantity** | **Price** (per unit) | **Total Price** | **Consumption** |
|---------------------------------|--------------|----------------------|-----------------|-----------------|
| Reolink Camera RLC-81PA         | 1            | 120                  | $XXX.XX         | 12W             |
| TL-SF1005P                      | 1            | 33.7                 | $XXX.XX         | 7W              |
| Battery 48V                     | 1            | 100                  | $XXX.XX         | N/A             |
| Ethernet Cable 30 meters        | 1            | 30                   | $XXX.XX         | N/A             |
| Ethernet Cable for Raspberry Pi | 1            | 5                    | $XXX.XX         | N/A             |
| Raspberry Pi 4 Model B          | 1            | 60                   | $XXX.XX         | 3.5W            |
| Raspberry Pi PoE+ Hat           | 1            | 30                   | $XXX.XX         | 4W              |
| Ethernet Cable for Loco M5 Station | 1         | 5                    | $XXX.XX         | N/A             |
| Loco M5 Station                 | 1            | 78.48                | $XXX.XX         | 6.5W            |
| CN101A                          | 1            | 14                   | $XXX.XX         | N/A             |
| 12V Battery 20000mAh            | 1            | 40                   | $XXX.XX         | N/A             |
| Ubiquity Instant                | 1            | 40                   | $XXX.XX         | N/A             |


## Description of script
Can you make a script in C++ that takes the images from he webcam Reolink RLC-81PA IP using the command http://192.168.1.55/cgi-bin/api.cgi?cmd=Snap&channel=0&rs=Takito&user=admin&password=jumanji3&width=3840&height=2160? . This script will forever run in a pc and will ask the camera to only take X pictures per day. Make it so the snap_times can be read every time before a picture is taken from an external file. You can choose the best format for this config file. Look into the command for all the paramenters that can be configured from the camera snapshot. This file and code will handle different cameras we must read from. Therefore, this file and this program needs to take into account that multiple cameras can be connected to the network and each of can have a different configuration.
I am imagining that the code will have a sw unit reading the config file 3 snap_times per day, that can be configured as well in this config file. Default hardcoded values will be at 9, 13 and 18. If these refreshing snap_times are given in the file, take the new values. Every time we reach one of this refreshing snap_times, this sw unit must read from the config file. This file will contain the characteristics of every camera connected to the system. Which means that every refresh should check if new cameras are added or removed. 
When a camera is added, this sw filesystem unit will create an camera object that will contain the camera information and at what time we want the carema to take pictures. The amount per pictures and time of this pictures per day is different across cameras and each camera can have different amount of pics per day.
Every time each one of these cameras takes a picture, they will send the information to another sw storage unit. This unit will be in charge of actively running a process in the background that listens to all the active cameras and receives images from them through queues or other interprocess communication channel. Then, this process will be in charge of being aware of which cameras are added and which ones are removed for every refresh period. For every camera added, the sw storage unit will create a new folder with the camera ID. Whenever this camera sends an image to the sw storage unit, the picture will be saved into the camera's specific folder with the name of the camera that took the picture and the snap_times stamp.

### Config file
The config file is called "config.cfg" and has the next format:

{
  "refresh_times": ["09:00", "13:00", "18:00"],
  "cameras": [
    {
      "id": "camera1",
      "ip": "192.168.1.55",
      "user": "admin",
      "password": "jumanji3",
      "resolution": {"width": 3840, "height": 2160},
      "snap_times": ["08:00", "12:00", "16:00"],
      "pictures_per_day": 3
    }
    // Add more camera configurations here
  ]
}

### Main Script
This script will implement main. Main creates the object StorageDaemon and runs it.

### Storage Class
This class will implement a storage handling process that is always listening if someone wants to send an image to it. During runtime, multiple processes will send images to the StorageDaemon. This process needs to storage these images into different folders depending on which process is sending the image. If a process sends an image, then the StorageDaemon needs to put the image into the <camera 1 id> folder and name it <camera 1 id>_snap_timestamp.

### Camera Class
Constructor receives and must store the next parameters:
+ "id": "camera1",
+ "ip": "192.168.1.55",
+ "user": "admin",
+ "password": "jumanji3",
+ "resolution": {"width": 3840, "height": 2160},
+ "snap_times": ["08:00", "12:00", "16:00"],
+ "pictures_per_day": 3
These parameters will be saved in the class as private and accessed through getters and setters. To start running the object, the camera will connect as a client to the storage process that is currently running. If uncapable to connect, which means the storage object is not properly running nor listening to incoming connections, throw an exception. If connection to the storage process is working, The class then will have to initialize as many interruptions as "snap_times" are given. Every time the pc time matches a value from the "snap_times", the camera object must take a snapshot. This is done by interacting with webcam Reolink RLC-81PA IP. To capture a snapshot from your Reolink RLC-81PA camera using a URL, you can use the following parameters within the URL structure:

- **cmd**: This should be set to `Snap` to indicate that a snapshot is being requested.
- **channel**: The camera channel, typically `0` for a single camera setup.
- **rs**: A random string to ensure the URL request is unique.
- **user**: The username for accessing the camera.
- **password**: The password for accessing the camera.
- **width** and **height**: Optional parameters to specify the desired resolution of the snapshot.

Here is an example URL that includes these parameters:

```arduino
http://192.168.1.55/cgi-bin/api.cgi?cmd=Snap&channel=0&rs=Takito&user=admin&password=jumanji3&width=3840&height=2160
```
This should trigger a snapshot in the Reolink RLC-81PA and the response should be an image with the characteristics given to the constructor. Once this image is received, the camera object must send its own "id" to the storage class using the previously opened port. After sending the id, the camera object must send the image received from the Reolink RLC-81PA.
A stop method should also be implemented, waiting for all threads to finish and clossing any ongoing processes.

### ConfigReader Class
ConfigReader class needs to have a constructor that goes through the file once. This constructor calls a private function that goes through the whole json config file. Inside this function the class reads the json file "config.cfg". From this file, the function will store in the private attibute of the class the "refresh_snap_times" and return a list called "activeCameras" made up of "Camera" objects. These camera objects will contain the next parameters:
+ "id": "camera1",
+ "ip": "192.168.1.55",
+ "user": "admin",
+ "password": "jumanji3",
+ "resolution": {"width": 3840, "height": 2160},
+ "snap_times": ["08:00", "12:00", "16:00"],
+ "pictures_per_day": 3

The config file is called "config.json" and it contains a json with the next format:
{
  "refresh_times": ["09:00", "13:00", "18:00"],
  "cameras": [
    {
      "id": "camera1",
      "ip": "192.168.1.55",
      "user": "admin",
      "password": "jumanji3",
      "resolution": {"width": 3840, "height": 2160},
      "snap_times": ["08:00", "12:00", "16:00"],
      "pictures_per_day": 3
    }
    // Add more camera configurations here
  ]
}

To run this object we must use a start method. This method must initialize as many interruptions as "refresh_times" are given. Every time the pc time matches a value from the "refresh_times", the camera object must read the config file. If the characteristics of a camera have changed, please use the getters and the setters provided by the Camera Class. In case a camera has been removed, please stop the camera object from running and close object related to that camera id. If a camera is added, create a new object and insert it into the list of "Camera" objects.
A stop method must also be implemented, this method should stop the refreshing events.

### Main Class

Finally, this class implements the main. This main, first of all, creates and runs a storageHandler object. Afterwards, it creates a ConfigReader object and starts running it. The main will then wait for esc keyboard input and ask the user if it wants to terminate the session with a (Y/N) command. If the users chooses N, then the scipt must resume execution until esc is pressed again. If user chooses Y, the main must stop the Config reader object from running. Once this is done, main should finally stop object storageHandle and make sure all resources have been freed.