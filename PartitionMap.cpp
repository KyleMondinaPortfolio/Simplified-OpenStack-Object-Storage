#include "PartitionMap.h"

PartitionMap::PartitionMap(const std::vector<std::string>& ipAddresses, Range partitionRange) {
    machineCount = ipAddresses.size();

    // Split the partitionsRange by machine count
    std::vector<Range> vectorPartitions;
    vectorPartitions.push_back(partitionRange);
    std::vector<std::vector<Range>> splitRanges = split(vectorPartitions, machineCount);

    // Initialize the partitionMap
    auto it = ipAddresses.begin();
    for (const auto& range : splitRanges) {
        // Pair each IP address with its corresponding range vector
        partitionMap[range] = *it;
        ++it; // Move to the next IP address
    }

    display();
}

std::string PartitionMap::lookup(uint64_t partition) {
    for (const auto &pair: partitionMap) {
        for (const auto &range: pair.first) {
            if (partition >= range.start && partition <= range.end) {
                return pair.second;
            }
        }
    }

    std::cout << "partition not found in PartitionMap" << std::endl;
    return "";
}

bool PartitionMap::add(const std::string &ipAddress) {
    std::map<std::vector<Range>, std::string> newPartitionMap;
    std::vector<Range> allApportionedRanges;

    for (auto &pair: partitionMap) {
        std::vector<Range> ranges = pair.first; 
        std::vector<Range> apportionedRanges = apportion(ranges, machineCount + 1); // Take 1/nth of ranges and put into apportionedRanges. Modify ranges to contain only the remaining partitions.
        newPartitionMap.insert(std::make_pair(ranges,pair.second)); // ranges has been modified by apportion
        allApportionedRanges.insert(allApportionedRanges.end(), apportionedRanges.begin(), apportionedRanges.end());
    }

    newPartitionMap.insert(std::make_pair(allApportionedRanges,ipAddress));
    partitionMap = newPartitionMap;

    machineCount += 1;

    std::cout << "New PartitionMap after addition: " << std::endl;
    display();
    return true;
}

bool PartitionMap::remove(const std::string &ipAddress) {

    if (machineCount <= 2) {
        std::cout << "Error: You need at least two machines in your database! Deletion rejected!"  << std::endl;
        return false;
    }

    bool found = false;
    for (auto it = partitionMap.begin(); it != partitionMap.end(); ++it) {
        if (ipAddress == it->second) {
            found = true;
            break;
        }
    }

    if (!found) {
        std::cout << "Error: " << ipAddress << " not found in your database. Deletion rejected!" << std::endl;
        return false;
    }

    std::vector<Range> rangesToReallocate;
    std::map<std::vector<Range>, std::string> tmpPartitionMap;
    for (auto it = partitionMap.begin(); it != partitionMap.end(); ++it){
        if (it->second == ipAddress){
            // extract the paritions of the hardrive to be deleted
            rangesToReallocate = it-> first;
        } else {
            // transfer the element to tmpRange
            std::pair<std::vector<Range>, std::string> element = std::make_pair(it->first, it->second);
            tmpPartitionMap.insert(element);
        }
    }

    std::vector<std::vector<Range>> splittedRanges = split(rangesToReallocate, machineCount -1);
    std::map<std::vector<Range>, std::string> newPartitionMap;

    auto it1 = splittedRanges.begin();
    auto it2 = tmpPartitionMap.begin();
    for (; it1 != splittedRanges.end() && it2 != tmpPartitionMap.end(); ++it1, ++it2) {
        const std::vector<Range>& vec1 = *it1;
        const std::vector<Range>& vec2 = it2->first;
        std::vector<Range> concatenatedVec = concatenateVectors(vec1, vec2);
        newPartitionMap.insert(std::make_pair(concatenatedVec, it2->second));
    }

    partitionMap = newPartitionMap;
    machineCount -= 1;

    std::cout << "New PartitionMap after deletion:" << std::endl;
    display();
    return true;

}

std::string PartitionMap::display() {
    std::ostringstream oss;
    oss << "PartitionMap:\n";
    for (const auto& pair : partitionMap) {
        oss << "IP Address: " << pair.second << ", Ranges: [";
        for (const auto& range : pair.first) {
            oss << "(" << range.start << ", " << range.end << ") ";
        }
        oss << "]\n";
    }
    std::cout << oss.str() << std::endl;
    return oss.str();
}

// Utility Functions

std::vector<std::vector<Range>> split(const std::vector<Range>& ranges, int n) {
    std::vector<std::vector<Range>> result(n);

    for (const auto& range : ranges) {
        int partitionSize = (range.end - range.start + 1) / n;
        int remainder = (range.end - range.start + 1) % n;
        int currentStart = range.start;

        for (int i = 0; i < n; ++i) {
            int currentEnd = currentStart + partitionSize - 1;
            if (remainder > 0) {
                currentEnd++;
                remainder--;
            }
            if (currentStart <= range.end) {
                result[i].push_back(Range(currentStart, currentEnd));
                currentStart = currentEnd + 1;
            }
        }
    }

    return result;
}

std::vector<Range> apportion(std::vector<Range>& ranges, int n) {
    std::vector<Range> extractedRanges;

    for (auto it = ranges.begin(); it != ranges.end(); ) {
        int partitionSize = (it->end - it->start + 1) / n;
        int remainder = (it->end - it->start + 1) % n;

        // If there is only one element: do nothing
        if (it->start == it->end){
            break;
        }

        else if (partitionSize == 0) {
            // Just grab the last part
            extractedRanges.push_back(Range(it->end, it->end));
            it->start = it->start;
            it->end = it->end-1;
            continue;
        }

        int tmpRangeStart = it->start;
        int tmpRangeEnd = it->end;

        if (partitionSize < remainder) {
            it->start = tmpRangeStart;
            it->end = tmpRangeEnd - remainder;

            extractedRanges.push_back(Range(tmpRangeEnd - remainder + 1, tmpRangeEnd));

        } else {
            it->start = tmpRangeStart;
            it->end = tmpRangeEnd - (partitionSize + remainder);

            extractedRanges.push_back(Range(tmpRangeEnd - (partitionSize + remainder) + 1, tmpRangeEnd));
        }

        ++it;
    }

    return extractedRanges;
}

std::vector<Range> concatenateVectors(const std::vector<Range>& vec1, const std::vector<Range>& vec2) {
    std::vector<Range> concatenated;
    concatenated.reserve(vec1.size() + vec2.size());
    concatenated.insert(concatenated.end(), vec1.begin(), vec1.end());
    concatenated.insert(concatenated.end(), vec2.begin(), vec2.end());
    return concatenated;
}





