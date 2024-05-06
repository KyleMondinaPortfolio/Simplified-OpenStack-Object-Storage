#ifndef PARTITION_MAP_H
#define PARTITION_MAP_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <cstdint>
#include <utility>
#include <sstream> 

class Range {
public:
    uint64_t start;
    uint64_t end;
    Range(int s, int e) : start(s), end(e) {}
    bool operator<(const Range& other) const {
        return start < other.start || (start == other.start && end < other.end);
    }
};

// A class representing a mapping of partition ranges to IP addresses of assigned machines. 
// This class manages the mapping and facilitates necessary updates when machines are added or removed, 
// as well as looking up the machine assigned to a given partition number.
class PartitionMap {
public:
    PartitionMap(const std::vector<std::string>& ipAddresses, Range partitionRange);
    std::string lookup(uint64_t partition);
    bool add(const std::string &ipAddress);
    bool remove(const std::string &ipAddress);
    std::string display();
    
private:
    std::map<std::vector<Range>, std::string> partitionMap;
    int machineCount;
};

std::vector<std::vector<Range>> split(const std::vector<Range>& ranges, int n);
std::vector<Range> apportion(std::vector<Range>& ranges, int n);
std::vector<Range> concatenateVectors(const std::vector<Range>& vec1, const std::vector<Range>& vec2);

#endif /* PARTITION_MAP_H */