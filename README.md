# Simplified OpenStack Object Storage (SOSS)
An implementation of a simplified OpenStack object storage using C++. SOOS stores and retrieves files on a 
distributed file system with scale and provides software-defined-storage (SDS), which is scalable, durable, available, manageable, flexible, adaptable, open, economic, and eventually consistent.

## SOSS Features
- proxy process communicates with external clients, and accepts download, list, upload, (stat,) and delete operations
- routing process finds storage location using consistent hashing
- storage process stores objects and metadata
- consistency process ensures data integrity and availability by finding failed drives or corrupted data, and replicating objects to a preset number of cipies, by default, 2
