#include <iostream>
#include "dag-object.h"  // Your StaticDAG/DynamicDAG header

int main() {
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

    return 0;
}
