/* 

   Source Code on Security Aware Serverless Application Partitioning (SASAP) algorithm. 
   This algorithm takes into account, the security perspective 
   for communicating data amongst different functions invocated. 

*/

#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <openssl/evp.h>
#include <openssl/aes.h>

using namespace std;

// Define a structure for tree nodes with a secure computation flag
struct TreeNode {
    int id;                       // Unique identifier for the node
    int cost;                     // Cost associated with the node
    int latency;                  // Latency associated with the node
    bool secureComputation;       // Flag to indicate if secure computation is required
    vector<TreeNode*> children;   // Vector of pointers to child nodes
    TreeNode(int id, int cost, int latency, bool secure = false) 
        : id(id), cost(cost), latency(latency), secureComputation(secure) {}
};

// Define a structure for partitions of nodes
struct Partition {
    int totalCost;                // Total cost of nodes in the partition
    int totalLatency;             // Total latency of nodes in the partition
    vector<int> nodes;            // Vector of node identifiers belonging to this partition
    bool hasSecureNode;           // Flag to indicate if the partition contains secure nodes
};

// Define a structure to store inter-linkages between partitions
struct Linkage {
    int fromNode;
    int toNode;
};

// Function to create a new partition containing a single node
Partition createPartition(TreeNode* node) {
    Partition partition = { node->cost, node->latency, {node->id}, node->secureComputation };
    return partition;
}

// Function to partition the tree nodes based on latency and memory limits
vector<Partition> improvedTreePartitioning(TreeNode* root, int latencyLimit, int memoryLimit, vector<Linkage>& linkages) {
    vector<Partition> partitions;           // Vector to store all partitions
    unordered_set<int> usedNodes;           // Set to track used node identifiers
    queue<TreeNode*> q;                     // Queue for breadth-first traversal
    q.push(root);                           // Start with the root node

    // Traverse through the tree nodes
    while (!q.empty()) {
        TreeNode* node = q.front();
        q.pop();

        bool placed = false;

        // Try to place the current node in an existing partition
        for (auto& partition : partitions) {
            if (partition.totalLatency + node->latency <= latencyLimit &&
                partition.totalCost + node->cost <= memoryLimit &&
                usedNodes.find(node->id) == usedNodes.end() &&
                (!node->secureComputation || partition.hasSecureNode)) {
                // Update the partition with the current node
                partition.totalLatency += node->latency;
                partition.totalCost += node->cost;
                partition.nodes.push_back(node->id);
                usedNodes.insert(node->id);
                if (node->secureComputation) {
                    partition.hasSecureNode = true;
                }
                placed = true;
                break;
            }
        }

        // If the node couldn't be placed in any existing partition, create a new partition
        if (!placed) {
            Partition new_partition = createPartition(node);
            partitions.push_back(new_partition);
            usedNodes.insert(node->id);
        }

        // Enqueue all children of the current node for further processing
        for (auto child : node->children) {
            if (usedNodes.find(child->id) == usedNodes.end()) {
                linkages.push_back({node->id, child->id});  // Store inter-linkage
                q.push(child);
            }
        }
    }

    return partitions;   // Return all partitions created
}

// Function to generate a random tree structure with given number of nodes and secure nodes
TreeNode* generateTree(int numNodes, int secureNodeCount) {
    vector<TreeNode*> nodes;   // Vector to store all tree nodes
    vector<int> secureNodes;   // Vector to store indices of secure nodes

    // Create nodes with random cost and latency
    for (int i = 0; i < numNodes; ++i) {
        bool secure = (i < secureNodeCount); // Assign secure computation flag to the first secureNodeCount nodes
        nodes.push_back(new TreeNode(i, rand() % 20 + 1, rand() % 10 + 1, secure));
        if (secure) {
            secureNodes.push_back(i);
        }
    }

    // Establish parent-child relationships to form a tree structure
    for (int i = 1; i < numNodes; ++i) {
        int parent = rand() % i;     // Randomly select a parent node index
        nodes[parent]->children.push_back(nodes[i]);  // Add current node as a child of the selected parent
    }

    return nodes[0];   // Return the root node of the generated tree
}

// Function to recursively delete all nodes of the tree to free memory
void deleteTree(TreeNode* root) {
    if (!root) return;
    for (auto child : root->children) {
        deleteTree(child);
    }
    delete root;
}

// Function to print all partitions and their respective node IDs
void printPartitions(const vector<Partition>& partitions) {
    for (size_t i = 0; i < partitions.size(); ++i) {
        cout << "Composite Function " << i + 1 << " : ";
        for (int nodeId : partitions[i].nodes) {
            cout << nodeId << " ";
        }
        cout << (partitions[i].hasSecureNode ? " (Contains Secure Nodes)" : "") << endl;
    }
}

// Function to print inter-linkages between partitions
void printLinkages(const vector<Linkage>& linkages) {
    cout << "Inter-Linkages between partitions:" << endl;
    for (const auto& linkage : linkages) {
        cout << "Node " << linkage.fromNode << " -> Node " << linkage.toNode << endl;
    }
}

// Function to encrypt data (dummy encryption for simulation)
string encryptData(const string& data) {
    return "encrypted(" + data + ")";
}

// Function to decrypt data (dummy decryption for simulation)
string decryptData(const string& data) {
    if (data.find("encrypted(") == 0 && data.back() == ')') {
        return data.substr(10, data.size() - 11);  // Strip "encrypted(" and ")"
    }
    return data;
}

// Function to simulate partition deployment on vCPUs
void deployPartitions(const vector<Partition>& partitions, const vector<Linkage>& linkages, int numVCPUs) {
    vector<thread> vCPUs(numVCPUs);
    mutex mtx;
    int cpu = 0;

    auto executePartition = [&mtx](const Partition& partition, int cpu) {
        for (int nodeId : partition.nodes) {
            this_thread::sleep_for(chrono::milliseconds(100));
            lock_guard<mutex> lock(mtx);
            cout << "Executing node " << nodeId << " on partition on CPU " << cpu << endl;
        }
    };

    auto secureCommunicate = [&mtx](const Linkage& linkage) {
        this_thread::sleep_for(chrono::milliseconds(100)); // Simulate communication delay
        lock_guard<mutex> lock(mtx);
        string data = "Data from node " + to_string(linkage.fromNode) + " to node " + to_string(linkage.toNode);
        string encryptedData = encryptData(data);
        string decryptedData = decryptData(encryptedData);
        cout << "Secure communication: " << encryptedData << " -> " << decryptedData << endl;
    };

    for (int i = 0; i < partitions.size(); ++i) {
        if (i < numVCPUs) {
            vCPUs[i] = thread(executePartition, partitions[i], cpu);
            cpu = (cpu + 1) % numVCPUs;
        } else {
            vCPUs[i % numVCPUs].join();
            vCPUs[i % numVCPUs] = thread(executePartition, partitions[i], cpu);
            cpu = (cpu + 1) % numVCPUs;
        }
    }

    for (auto& vCPU : vCPUs) {
        if (vCPU.joinable()) {
            vCPU.join();
        }
    }

    for (const auto& linkage : linkages) {
        thread(secureCommunicate, linkage).join();
    }
}

// Main function to execute the program
int main(int argc, char* argv[]) {
    // Validate command line arguments
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <number_of_nodes> <number_of_vcpus> <number_of_secure_nodes>" << endl;
        return 1;
    }

    int numNodes = stoi(argv[1]);    // Number of nodes in the tree
    int numVCPUs = stoi(argv[2]);    // Number of vCPUs
    int secureNodeCount = stoi(argv[3]);  // Number of nodes requiring secure computation

    if (numNodes < 1 || numNodes > 500 || numVCPUs < 1 || secureNodeCount < 0 || secureNodeCount > numNodes) {
        cerr << "Number of nodes must be between 1 and 500, number of vCPUs must be at least 1, and number of secure nodes must be between 0 and number of nodes." << endl;
        return 1;
    }

    srand(time(0));   // Seed the random number generator

    TreeNode* root = generateTree(numNodes, secureNodeCount);   // Generate a random tree with specified number of nodes

    int latencyLimit = 50;    // Adjusted latency limit for more partitions
    int memoryLimit = 100;    // Adjusted memory limit for more partitions

    vector<Linkage> linkages;    // Vector to store inter-linkages
    auto start = chrono::high_resolution_clock::now();
    vector<Partition> partitions = improvedTreePartitioning(root, latencyLimit, memoryLimit, linkages);   // Partition the tree nodes
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    // Recalculate total costs and latencies for all partitions
    for (auto& partition : partitions) {
        partition.totalCost = 0;
        partition.totalLatency = 0;
        for (int nodeId : partition.nodes) {
            partition.totalCost += abs(root[nodeId].cost % 100 );        // Sum up the costs of nodes in the partition
            partition.totalLatency += abs(root[nodeId].latency % 100);  // Sum up the latencies of nodes in the partition
        }
    }

    // Find the best partition based on total cost and total latency
    Partition best_partition = partitions[0];
    for (const auto& partition : partitions) {
        if (partition.totalCost < best_partition.totalCost ||
            (partition.totalCost == best_partition.totalCost && partition.totalLatency < best_partition.totalLatency)) {
            best_partition = partition;
        }
    }

    // Print the best partition and its details
    cout << "Partitions (Composite Functions): " << endl;
    printPartitions(partitions);

    // Print inter-linkages between partitions
    printLinkages(linkages);

    cout << "Execution time: " << duration.count() << " seconds." << endl;

    // Simulate deployment on vCPUs and secure communication between partitions
    deployPartitions(partitions, linkages, numVCPUs);

    deleteTree(root);   // Delete the tree nodes to free memory

    return 0;   // Exit the program
}