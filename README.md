# Simplified OpenStack Object Storage (SOSS)
An implementation of a simplified OpenStack object storage using C++. SOOS stores and retrieves files on a 
distributed file system with scale and provides software-defined-storage (SDS), which is scalable, durable, available, manageable, flexible, adaptable, open, economic, and eventually consistent. 

This project was completed as part of COEN 241 - Cloud Computing at Santa Clara University under the guidance of Professor Ming-Hwa Wang.

## SOSS Features
- proxy process communicates with external clients, and accepts download, list, upload, (stat,) and delete operations
- routing process finds storage location using consistent hashing
- storage process stores objects and metadata
- consistency process ensures data integrity and availability by finding failed drives or corrupted data, and replicating objects to a preset number of cipies, by default, 2

The proxy process is implemented using client-server network programming. Server process is first run using parition power and list of avialble disks as command-lind arguments. The server will find an available port automatically and display the port number, then the client can run using either the server's IP address or the server's machine name and the port number displayed by the server as command-line argument

```
$ server 16 10.16.9.3 10.16.8.4 10.16.8.6
Port number = 9999
$ client 10.16.8.2 9999
```
The routing process assumes any object consists of a unique user name and non unique object (i.e file) name in the form of ` user/object `  and hashed by md5sum <<< `<string>` A fix partition power is given. The algorithm always store object to the least-used drive. A device lookup table contains number-of-replica rows and number-of-partition columns, each entry is the drive to store the given object


## Supported Commands
- `download <user/object>` - display the content of `<user/object>`
- `list <user>` - display the `<user>`'s objects/files in `ls –lrt` format
- `upload <user/object>` - display which disks the `<user/object>` is saved
- `delete <user/object>` - display a confirmation or error message
- `add <disk>` - display new partitions with all `<user/object>` within
- `remove <disk>` - display where old partitions went to
- `clean` - clear all disks (i.e., all `/tmp/<login>` directories)

The `md5` hashing takes the `<user/object>` input as a string, and generates a 128-bit hashed value in hex. You divide the hashed value into `2^partition power` partitions and assign partitions roughly equally to all disks you have. Assume that each machine only has one disk drive, so we use the machine name for the disk. We also assume all objects are text files.

