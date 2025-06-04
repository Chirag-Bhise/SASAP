#include <iostream>
#include <vector>
#include <queue>
#include <climits>
#include <cstdlib> // for rand()
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

// Declaration of findTreeNode function
TreeNode* findTreeNode(TreeNode* root, int id);

// Declaration of printCompositeFunctions function
void printCompositeFunctions(const std::vector<std::unordered_set<int>>& compositeFunctions, TreeNode* root);

// Declaration of dfs function
void dfs(TreeNode* node, int latencyLimit, std::unordered_set<int>& currentPartition, std::unordered_set<TreeNode*>& visited);

// Function to calculate QoS satisfaction for a partition
double calculatePartitionQoS(int totalCost, int totalLatency, int costLimit, int latencyLimit) {
    double costSatisfaction = (costLimit - totalCost) * 100.0 / costLimit;
    double latencySatisfaction = (latencyLimit - totalLatency) * 100.0 / latencyLimit;

    // Ensure the satisfaction is between 0 and 100%
    costSatisfaction = std::max(0.0, costSatisfaction);
    latencySatisfaction = std::max(0.0, latencySatisfaction);

    return (costSatisfaction + latencySatisfaction) / 2;
}

// Function to perform bicriteria approximation and partition tree nodes
void bicriteriaApproximation(TreeNode* root, int latencyLimit, int costLimit, std::vector<std::unordered_set<int>>& compositeFunctions, double& overallQoS) {
    std::unordered_set<TreeNode*> visited;

    // Perform DFS to assign nodes to composite functions
    for (TreeNode* node : root->children) {
        if (visited.find(node) == visited.end()) {
            std::unordered_set<int> currentPartition;
            int totalCost = 0;
            int totalLatency = 0;

            dfs(node, latencyLimit, currentPartition, visited);

            if (!currentPartition.empty()) {
                // Calculate total cost and latency for this partition
                for (int id : currentPartition) {
                    totalCost += node->cost;
                    totalLatency += node->latency;
                }

                // Calculate QoS satisfaction for the partition
                double partitionQoS = calculatePartitionQoS(totalCost, totalLatency, costLimit, latencyLimit);
                overallQoS += partitionQoS;

                // Add partition to composite functions
                compositeFunctions.push_back(std::move(currentPartition));
            }
        }
    }
}

// DFS to assign nodes to composite functions
void dfs(TreeNode* node, int latencyLimit, std::unordered_set<int>& currentPartition, std::unordered_set<TreeNode*>& visited) {
    visited.insert(node);
    currentPartition.insert(node->id);

    for (TreeNode* child : node->children) {
        if (visited.find(child) == visited.end() && node->latency + child->latency <= latencyLimit) {
            dfs(child, latencyLimit - node->latency, currentPartition, visited);
        }
    }
}

// Function to print composite functions in a hierarchical manner
void printCompositeFunctions(const std::vector<std::unordered_set<int>>& compositeFunctions, TreeNode* root) {
    std::cout << "Partitions (Composite Functions):" << std::endl;
    int partitionNum = 0;

    for (const auto& partition : compositeFunctions) {
        partitionNum++;

        // Print composite function header
        std::cout << "Partition " << partitionNum << " :";

        // Collect nodes in composite function in a hierarchical manner
        std::queue<int> nodeQueue;
        std::unordered_set<int> visited;

        // Push all nodes of the current partition into the queue
        for (int id : partition) {
            nodeQueue.push(id);
            visited.insert(id);
        }

        // BFS to collect nodes in a hierarchical order
        while (!nodeQueue.empty()) {
            int currentId = nodeQueue.front();
            nodeQueue.pop();

            std::cout << " " << currentId;

            TreeNode* currentNode = findTreeNode(root, currentId);
            if (currentNode) {
                for (TreeNode* child : currentNode->children) {
                    if (visited.find(child->id) == visited.end()) {
                        visited.insert(child->id);
                        nodeQueue.push(child->id);
                    }
                }
            }
        }

        std::cout << std::endl;
    }
}

// Helper function to find a TreeNode by its ID in the tree
TreeNode* findTreeNode(TreeNode* root, int id) {
    if (!root) return nullptr;
    if (root->id == id) return root;

    for (TreeNode* child : root->children) {
        TreeNode* found = findTreeNode(child, id);
        if (found) return found;
    }

    return nullptr;
}

int main() {
    int N = 500;
    if (N < 1 || N > 500) {
        std::cerr << "Number of nodes must be between 1 and 500." << std::endl;
        return 1;
    }

    // Seed for random number generation
    srand(time(0));

    // Create tree structure with random costs and latencies
    std::vector<TreeNode*> nodes(N);
    for (int i = 0; i < N; ++i) {
        int cost = rand() % 50 + 1;   // Random cost between 1 and 50
        int latency = rand() % 10 + 1; // Random latency between 1 and 10
        nodes[i] = new TreeNode(i, cost, latency);
    }

    // Create random tree structure
    for (int i = 1; i < N; ++i) {
        int parent = rand() % i;
        nodes[parent]->children.push_back(nodes[i]);
    }

    // Root node is nodes[0]
    TreeNode* root = nodes[0];

    // Latency limit (example value)
    int latencyLimit = 20;
    // Cost limit (example value)
    int costLimit = 100;

    // Vector to store composite functions (partitions)
    std::vector<std::unordered_set<int>> compositeFunctions;
    double overallQoS = 0.0;

    auto start = std::chrono::high_resolution_clock::now();
    // Perform bicriteria approximation
    bicriteriaApproximation(root, latencyLimit, costLimit, compositeFunctions, overallQoS);
    auto end = std::chrono::high_resolution_clock::now();

    // Calculate the duration
    std::chrono::duration<double> duration = end - start;

    // Calculate and print overall QoS satisfaction
    int numPartitions = compositeFunctions.size();
    overallQoS = (numPartitions > 0) ? overallQoS / numPartitions : 0.0;
    std::cout << "Overall QoS Satisfaction: " << overallQoS << "%" << std::endl;

    // Print composite functions
    printCompositeFunctions(compositeFunctions, root);

    // Clean up memory
    for (int i = 0; i < N; ++i) {
        delete nodes[i];
    }

    return 0;
}
