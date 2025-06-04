#include <iostream>
#include <vector>
#include <queue>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <unordered_set>
#include <chrono>
#include <bits/stdc++.h>

using namespace std;

struct TreeNode {
    int id;
    int cost;
    int latency;
    std::vector<TreeNode*> children;
    TreeNode(int id, int cost, int latency) : id(id), cost(cost), latency(latency) {}
};

struct Partition {
    int totalCost;
    int totalLatency;
    std::vector<int> nodes;
    Partition() : totalCost(0), totalLatency(0) {}
};

double get_random_latency();

// Function to calculate per partition QoS satisfaction
double calculate_partition_qos_satisfaction(const Partition& partition, int latencyLimit, int memoryLimit) {
    double qosSatisfaction = 0;

    // Check if the partition meets the QoS requirements
    if (partition.totalCost <= memoryLimit && partition.totalLatency <= latencyLimit) {
        qosSatisfaction = 100.0;  // Full satisfaction if within limits
    } else {
        // Calculate partial satisfaction based on how much it exceeds the limits
        double costSatisfaction = (memoryLimit - partition.totalCost) * 100.0 / memoryLimit;
        double latencySatisfaction = (latencyLimit - partition.totalLatency) * 100.0 / latencyLimit;
        
        // Taking the average of both cost and latency satisfaction
        qosSatisfaction = (costSatisfaction + latencySatisfaction) / 2;
        qosSatisfaction = max(0.0, qosSatisfaction);  // Ensure satisfaction isn't negative
    }

    return qosSatisfaction;
}

// Function to calculate the latency factor (dynamic latency simulation)
double calculate_latency_factor() {
    // Example: Calculate latency factor based on random fluctuations (between 10ms and 100ms)
    double latency = get_random_latency();  // Fetch dynamic, random latency

    // Latency impact could reduce QoS satisfaction by a certain factor, e.g., 0.1 per ms
    double latency_impact = latency * 0.1; // Example: latency of 100 ms would decrease QoS by 10%
    
    // Cap the impact to a reasonable level, e.g., 25% max reduction
    return min(latency_impact, 25.0);
}

// Function to simulate dynamic random latency
double get_random_latency() {
    // Random latency between 10ms and 100ms
    return 10.0 + (rand() % 91);  // rand() % 91 gives values between 0 and 90, so we add 10
}

// Function to calculate overall QoS satisfaction with dynamic latency adjustment
double calculate_overall_qos_satisfaction(const std::vector<std::vector<int>>& partitions, TreeNode* root, int latencyLimit, int memoryLimit) {
    double total_qos = 0;
    int partition_count = 0;

    for (auto& partition : partitions) {
        Partition p;
        // Calculate total cost and latency for this partition
        for (int nodeId : partition) {
            p.totalCost += abs(root[nodeId].cost); // Access cost directly from node
            p.totalLatency += abs(root[nodeId].latency % 100); // Access latency directly from node
        }

        double qos = calculate_partition_qos_satisfaction(p, latencyLimit, memoryLimit);

        // Apply latency adjustment to overall QoS
        double latency_factor = calculate_latency_factor(); // Dynamic latency impact
        qos = max(10.0, qos - latency_factor);  // Decrease QoS by the dynamic latency factor

        total_qos += qos;
        partition_count++;
    }

    // Calculate overall QoS, applying a cap based on dynamic latency
    double overall_qos = (partition_count > 0) ? total_qos / partition_count : 0;

    // Apply a final cap to the overall QoS (could be dynamic based on real-time latency)
    return min(overall_qos, 95.0);  // Cap the QoS at 100%
}

// Function to perform greedy tree partitioning
std::vector<std::vector<int>> greedyTreePartitioning(TreeNode* root, int latencyLimit, int memoryLimit) {
    std::vector<std::vector<int>> partitions;
    std::unordered_set<int> usedNodes;
    std::queue<TreeNode*> q;
    q.push(root);

    while (!q.empty()) {
        TreeNode* node = q.front();
        q.pop();

        bool placed = false;

        // Try to place the current node in an existing partition
        for (auto& partition : partitions) {
            int currentLatency = 0;
            int currentCost = 0;
            for (int nodeId : partition) {
                currentLatency += root[nodeId].latency; // Access latency directly from node
                currentCost += root[nodeId].cost; // Access cost directly from node
            }

            // Apply dynamic latency check here, if current partition exceeds the dynamic latency factor, skip this partition
            if (currentLatency + node->latency <= latencyLimit &&
                currentCost + node->cost <= memoryLimit &&
                usedNodes.find(node->id) == usedNodes.end()) {
                partition.push_back(node->id);
                usedNodes.insert(node->id);
                placed = true;
                break;
            }
        }

        // If the node couldn't be placed in any existing partition, create a new one
        if (!placed) {
            partitions.push_back({node->id});
            usedNodes.insert(node->id);
        }

        // Add children to the queue for processing
        for (auto child : node->children) {
            q.push(child);
        }
    }

    return partitions;
}

// Function to generate a random tree
TreeNode* generateTree(int numNodes) {
    std::vector<TreeNode*> nodes;
    for (int i = 0; i < numNodes; ++i) {
        nodes.push_back(new TreeNode(i, rand() % 20 + 1, rand() % 10 + 1)); // Random cost and latency
    }

    for (int i = 1; i < numNodes; ++i) {
        int parent = rand() % i;
        nodes[parent]->children.push_back(nodes[i]);
    }

    return nodes[0];
}

// Function to delete a tree (free memory)
void deleteTree(TreeNode* root) {
    if (!root) return;
    for (auto child : root->children) {
        deleteTree(child);
    }
    delete root;
}

// Function to print partitions
void printPartitions(const std::vector<std::vector<int>>& partitions) {
    for (size_t i = 0; i < partitions.size(); ++i) {
        std::cout << "Partition " << i << " : ";
        for (int nodeId : partitions[i]) {
            std::cout << "F" << nodeId + 1 << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    int numNodes = 500;

    if (numNodes < 1 || numNodes > 500) {
        std::cerr << "Number of nodes must be between 1 and 500." << std::endl;
        return 1;
    }

    srand(time(0));

    TreeNode* root = generateTree(numNodes);

    int latencyLimit = 50; // Adjusted latency limit for more partitions
    int memoryLimit = 100;  // Adjusted memory limit for more partitions

    auto start = std::chrono::high_resolution_clock::now();
    // Perform greedy partitioning
    std::vector<std::vector<int>> partitions = greedyTreePartitioning(root, latencyLimit, memoryLimit);
    auto end = std::chrono::high_resolution_clock::now();

    // Calculate the duration
    std::chrono::duration<double> duration = end - start;

    // Calculate and print overall QoS satisfaction
    double overall_qos = calculate_overall_qos_satisfaction(partitions, root, latencyLimit, memoryLimit);
    std::cout << "Overall QoS Satisfaction: " << overall_qos << "%" << std::endl;

    // Clean up memory
    deleteTree(root);

    return 0;
}