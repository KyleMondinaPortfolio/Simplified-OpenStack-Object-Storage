# Simplified OpenStack Object Storage (SOSS)
An implementation of a simplified OpenStack object storage using C++. SOOS stores and retrieves files on a 
distributed file system with scale and provides software-defined-storage (SDS), which is scalable, durable, available, manageable, flexible, adaptable, open, economic, and eventually consistent. Programming assignement of Santa Clara University's Cloud Computing Class.

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
