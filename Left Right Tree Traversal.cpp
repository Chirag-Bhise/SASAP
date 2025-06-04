#include <iostream>
#include <vector>
#include <stack>
#include <tuple>
#include <climits>
#include <unordered_set>
#include <chrono>
#include <bits/stdc++.h>

using namespace std;

struct TreeNode {
    int id;
    int cost;
    int latency;  // Added latency for each node
    std::vector<TreeNode*> children;
    TreeNode(int id, int cost, int latency) : id(id), cost(cost), latency(latency) {}
    ~TreeNode() {
        for (TreeNode* child : children) {
            delete child;
        }
        children.clear();
    }
};

// Recursive Tree Partitioning Algorithm to compute minimum cost
int recursiveTreePartition(TreeNode* node, std::vector<std::vector<int>>& dp) {
    if (!node) return 0;
    if (node->children.empty()) {
        dp[node->id][0] = node->cost;
        return node->cost;
    }

    int minCost = node->cost;
    for (auto child : node->children) {
        minCost += recursiveTreePartition(child, dp);
    }

    dp[node->id][0] = minCost;
    return minCost;
}

// Left-Right (Hybrid) Tree Traversal Procedure
std::vector<std::pair<int, int>> LeftRightTreeTraversal(TreeNode* root) {
    std::vector<std::pair<int, int>> traversalResult;
    std::stack<std::tuple<TreeNode*, int, int>> stack;

    if (root) {
        stack.push(std::make_tuple(root, -1, 0)); // (-1 indicates root is being visited)
    }

    while (!stack.empty()) {
        auto [node, parent_index, index] = stack.top();
        stack.pop();

        // Process node
        traversalResult.push_back(std::make_pair(node->id, node->cost));

        // Push children onto the stack in reverse order (right-to-left traversal)
        for (int i = node->children.size() - 1; i >= 0; --i) {
            stack.push(std::make_tuple(node->children[i], index, i));
        }
    }

    return traversalResult;
}

// QoS Satisfaction Calculation based on cost and latency limits
double calculate_partition_qos_satisfaction(int totalCost, int totalLatency, int costLimit, int latencyLimit) {
    double qosSatisfaction = 0.0;

    if (totalCost <= costLimit && totalLatency <= latencyLimit) {
        qosSatisfaction = 100.0;  // Full satisfaction if within limits
    } else {
        // Partial satisfaction
        double costSatisfaction = (costLimit - totalCost) * 100.0 / costLimit;
        double latencySatisfaction = (latencyLimit - totalLatency) * 100.0 / latencyLimit;
        
        qosSatisfaction = (costSatisfaction + latencySatisfaction) / 2;
        qosSatisfaction = max(0.0, qosSatisfaction);  // Ensure satisfaction isn't negative
    }

    return qosSatisfaction;
}

// Dynamic Latency Adjustment (random fluctuation simulation)
double calculate_latency_factor() {
    // Example: Calculate latency factor based on random fluctuations (between 10ms and 100ms)
    double latency = 10.0 + (rand() % 91);  // Random latency between 10 and 100ms
    double latencyImpact = latency * 0.1;  // 0.1% decrease per ms latency

    return min(latencyImpact, 25.0);  // Cap the impact at 25%
}

// Function to print composite functions and overall QoS
void printCompositeFunctions(TreeNode* root, const std::vector<std::pair<int, int>>& traversalResult, int costLimit, int latencyLimit) {
    std::cout << "Partitions (Composite Functions):" << std::endl;

    // Collect composite functions using DFS
    std::vector<std::unordered_set<int>> compositeFunctions(traversalResult.size());
    std::unordered_set<int> printed;
    
    double totalQoS = 0.0;  // Total QoS satisfaction accumulator
    int partitionCount = 0;

    for (auto [id, cost] : traversalResult) {
        if (printed.find(id) == printed.end()) {
            std::cout << "Partition " << id << " ";

            // DFS to collect all nodes in the composite function
            std::stack<TreeNode*> stack;
            stack.push(root);

            int totalCost = 0, totalLatency = 0;
            while (!stack.empty()) {
                TreeNode* node = stack.top();
                stack.pop();

                if (node->id == id) {
                    totalCost += node->cost;
                    totalLatency += node->latency;

                    for (TreeNode* child : node->children) {
                        stack.push(child);
                        compositeFunctions[id].insert(child->id);
                        totalCost += child->cost;
                        totalLatency += child->latency;
                    }
                    break;
                }

                for (TreeNode* child : node->children) {
                    stack.push(child);
                }
            }

            // Calculate QoS satisfaction for this partition
            double qos = calculate_partition_qos_satisfaction(totalCost, totalLatency, costLimit, latencyLimit);
            std::cout << "QoS Satisfaction: " << qos << "%" << std::endl;

            // Accumulate the QoS satisfaction
            totalQoS += qos;
            partitionCount++;

            // Print all nodes in the composite function
            for (int node_id : compositeFunctions[id]) {
                std::cout << node_id << " ";
                printed.insert(node_id);
            }

            std::cout << std::endl;
        }
    }

    // Calculate and display the overall QoS satisfaction
    double overallQoS = (partitionCount > 0) ? (totalQoS / partitionCount) : 0.0;
    std::cout << "Overall QoS Satisfaction: " << overallQoS << "%" << std::endl;
}

int main() {
    int N = 500;
    if (N < 1 || N > 500) {
        std::cerr << "Number of nodes must be between 1 and 500." << std::endl;
        return 1;
    }

    srand(time(0));

    // Example tree structure creation with N nodes
    std::vector<TreeNode*> nodes(N);
    for (int i = 0; i < N; ++i) {
        nodes[i] = new TreeNode(i, rand() % 100, rand() % 50 + 1);  // Random cost and latency
    }

    // Create tree structure (example: simple binary tree for demonstration)
    for (int i = 1; i < N; ++i) {
        int parent = rand() % i;  // Randomly select parent node
        nodes[parent]->children.push_back(nodes[i]);
    }

    // Root node is nodes[0]
    TreeNode* root = nodes[0];

    // Set limits for QoS calculation
    int latencyLimit = 50;  // Maximum allowed latency
    int costLimit = 100;    // Maximum allowed cost

    auto start = std::chrono::high_resolution_clock::now();
    // Perform left-right (hybrid) tree traversal
    std::vector<std::pair<int, int>> traversalResult = LeftRightTreeTraversal(root);

    // Compute minimum cost after partitioning
    std::vector<std::vector<int>> dp(N, std::vector<int>(1, INT_MAX));
    int result = recursiveTreePartition(root, dp);

    // Display partitions (composite functions)
    printCompositeFunctions(root, traversalResult, costLimit, latencyLimit);

    // Clean up memory
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    return 0;
}
