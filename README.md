# dag-object

`dag-object` is a modern, header-only C++20 library for representing and manipulating Directed Acyclic Graphs (DAGs) in your applications. It is designed for high performance, ease of use, and maximum flexibility, making it ideal for dependency management, scheduling, data processing, and more.

## Features

- **Header-only**: No build or link step required—just include and use.
- **C++20**: Leverages modern language features for safety and expressiveness.
- **Generic**: Supports custom node and edge types.
- **Efficient**: Optimized for fast graph operations.
- **Simple API**: Intuitive interface for creating, modifying, and querying DAGs.
- **Cycle Detection**: Ensures graph remains acyclic.
- **Topological Sort**: Easily obtain valid processing order.

## Getting Started

### 1. Installation

Simply add the `dag.hpp` header to your project:

Optionally use fetch on cmake:

```cmake
FetchContent_Declare(
  dag_object
  GIT_REPOSITORY https://github.com/RySah/dag-object.git
  GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(dag_object)
```

No additional dependencies or build steps are required.

### 2. Basic Usage

Create a DAG container, add nodes and edges, and perform queries:

```cpp
#include <iostream>

using namespace dag;

// Use a StaticDAG with max 10 nodes and 20 edges
StaticDAG<int, 10, 20> dg;

// Add some nodes (data = arbitrary integers)
auto a = dg.addNode(1);
auto b = dg.addNode(2);
auto c = dg.addNode(3);
auto d = dg.addNode(4);
auto e = dg.addNode(5);

// Add edges
dg.addEdge(a, b);  // 1 -> 2
dg.addEdge(a, c);  // 1 -> 3
dg.addEdge(b, d);  // 2 -> 4
dg.addEdge(c, d);  // 3 -> 4
dg.addEdge(d, e);  // 4 -> 5

// Attempt to create a cycle
if (!dg.addEdge(e, a)) {
    std::cout << "Cycle detected when trying to add edge 5 -> 1\n";
}

// Topological sort
auto order = dg.topologicalSort();
std::cout << "Topological order of nodes:\n";
for (auto idx : order) {
    std::cout << dg.nodes[idx].data << " ";
}
std::cout << "\n";

// Transitive reduction
auto reduced = dg.transitivelyReducePerNode();
std::cout << "Transitive reduction edges:\n";
for (size_t i = 0; i < reduced.size(); ++i) {
    for (auto j : reduced[i]) {
        std::cout << dg.nodes[i].data << " -> " << dg.nodes[j].data << "\n";
    }
}

// Reachability check
std::cout << "Is 1 reachable to 5? "
    << (dg.reachable(a, e) ? "Yes" : "No") << "\n";
std::cout << "Is 3 reachable to 2? "
    << (dg.reachable(c, b) ? "Yes" : "No") << "\n";

std::cout << "\nUse on graphviz to get a graphical view, e.g. `dot -Tpng test.dot -o mydag.png`:\n\n";

exportToDot(dg, std::cout);
```

### 3. Custom Types

You can use your own types for nodes and edges by specifying template parameters:

```cpp
dag::StaticDAG<T, MaxNodes, MaxEdges>
dag::DynamicDAG<T>
```

## API Overview

### Types

- `dag::DAG<NodeType>`: Main graph type, parameterized by your node type.

### Key Functions

- `bool reachable(DagIndexType from, DagIndexType target, ReachableFn fn = [](DagIndexType, DagIndexType, const Edge&) { return true; })`: Returns if a node can point to another (indirectly or directly)
- `DagIndexType addNode(const T& data)`: Adds a node and returns an ID/index.
- `T* addEdge(DagIndexType from, DagIndexType to, DagEdgeFlags flags = 0)`: Adds a edge and returns an pointer to value stored with `from`.
- `std::vector<DagIndexType> topologicalSort(ReachableFn edgeFilter = nullptr) const`: Returns a vector of indexes by which represents the sorted graph.
- `[collection]<std::vector<DagIndexType>> transitivelyReducePerNode(ReachableFn edgeFilter = nullptr) const`: Returns routes for each node, by which points to another.

## How It Works

- Internally, the DAG is represented using adjacency lists for fast traversal.
- All edge additions are checked for cycles; attempts to introduce cycles are rejected.
- Topological sorting is performed using Kahn's algorithm for efficiency.

## Requirements

- C++20 compiler (tested with MSVC, GCC, Clang)
- No external dependencies

## License

Distributed under the MIT License.

## Contributing

Pull requests and issues are welcome!

---

For more details, see the header file documentation and code comments.
