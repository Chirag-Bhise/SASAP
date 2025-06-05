**Serverless Application Partitioning Research :**
This repository contains the source code and experimental implementation for research conducted in the field of serverless computing, specifically focusing on application partitioning strategies. A novel algorithm named Security Aware Serverless Application Partitioning (SASAP) has been proposed, alongside implementations of existing approaches for comparative analysis.

**Overview :**
Serverless computing allows developers to build and run applications without managing infrastructure. However, partitioning applications securely and efficiently remains a challenge. This work explores and evaluates multiple partitioning strategies, with a focus on balancing security constraints and performance optimization.

**Implemented Algorithms :**
The following algorithms are implemented and available in this repository:

SASAP (Security Aware Serverless Application Partitioning) – A novel algorithm proposed in this research for partitioning serverless applications while considering security policies and minimizing inter-function communication.

GrTP (Greedy Tree Partitioning Approach) – A heuristic method that greedily partitions a tree-structured workflow.

LRTP (Left Right Tree Traversal) – A tree traversal-based method aiming at effective workload distribution.

BiFPTAS (Bicriteria Fully Polynomial Time Approximation Scheme) – A bicriteria approximation scheme that balances two key metrics such as cost and latency.

**Compiling & Running :**
All programs are written in C++ and can be compiled and executed using a standard g++ environment, using the command : '
g++ filename.cpp -o output_file' to compile and './output_file' to run.

**Empirical Analysis :**
In addition to the C++ implementations, the repository includes a Jupyter Notebook named 'Illustrations.ipynb that contains illustrative graphs and plots.It is the analysis based on empirical data collected from running both the basic and modified versions of the source code. A comparative evaluation of the implemented algorithms has been done. This notebook is useful for understanding the performance and security trade-offs among different approaches through visualizations and data summaries.
