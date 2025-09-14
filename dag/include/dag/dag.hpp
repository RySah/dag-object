#pragma once

#include <array>
#include <stdexcept>
#include <vector>
#include <queue>
#include <functional>
#include <string>
#include <ostream>

// Hopefully can bring some memory/size optimizations at compile time.
// USE CASE:
// - If the "subject" uses an limited amount of nodes and doesnt require a large type like size_t to represent it per Node and Edge.

#ifndef DAG_INDEX_TYPE
#define DAG_INDEX_TYPE size_t
#endif

#ifndef DAG_EDGE_FLAGS_TYPE
#define DAG_EDGE_FLAGS_TYPE uint32_t
#endif

using DagIndexType = DAG_INDEX_TYPE;
using DagEdgeFlags = DAG_EDGE_FLAGS_TYPE;

namespace dag
{

    template<typename T, size_t MaxNodes, size_t MaxEdges>
    struct StaticDAG {
        using data_type = T;

        static constexpr DagIndexType npos = -1;

        struct Node {
            using data_type = data_type;
            T data;
            DagIndexType firstEdge = npos;
        };

        struct Edge {
            DagIndexType to;
            DagIndexType next;
            DagEdgeFlags flags; // optional metadata
        };

        std::array<Node, MaxNodes> nodes{};
        std::array<Edge, MaxEdges> edges{};
        DagIndexType edgeCount = 0;
        DagIndexType nodeCount = 0;
        const char* lastError = nullptr;

        // === Customizable reachability function ===
        using ReachableFn = std::function<bool(DagIndexType from, DagIndexType to, const Edge& edge)>;

        void clear() {
            lastError = nullptr;
            edgeCount = 0;
            nodeCount = 0;
        }

        bool reachable(DagIndexType from, DagIndexType target,
            ReachableFn edgeFilter = [](DagIndexType, DagIndexType, const Edge&) { return true; }) const
        {
            if (from >= nodeCount || target >= nodeCount) return false;
            std::vector<bool> seen(MaxNodes, false);
            return dfsReachable(from, target, seen, edgeFilter);
        }

        DagIndexType addNode(const T& data) {
            if (nodeCount >= MaxNodes) { lastError = "Node pool full"; return npos; }
            nodes[nodeCount] = { data, npos };
            return nodeCount++;
        }

        T* addEdge(DagIndexType from, DagIndexType to, DagEdgeFlags flags = 0) {
            if (from >= nodeCount || to >= nodeCount) { lastError = "Invalid node index"; return nullptr; }
            if (reachable(to, from)) { lastError = "Cycle detected"; return nullptr; }
            if (edgeCount >= MaxEdges) { lastError = "Edge pool full"; return nullptr; }

            edges[edgeCount] = { to, nodes[from].firstEdge, flags };
            nodes[from].firstEdge = edgeCount++;
            return &nodes[from].data;
        }

        std::vector<DagIndexType> topologicalSort(ReachableFn edgeFilter = nullptr) const {
            std::vector<DagIndexType> indegree(nodeCount, 0);

            for (DagIndexType i = 0; i < nodeCount; i++) {
                for (DagIndexType e = nodes[i].firstEdge; e != npos; e = edges[e].next) {
                    if (!edgeFilter || edgeFilter(i, edges[e].to, edges[e])) {
                        indegree[edges[e].to]++;
                    }
                }
            }

            std::queue<DagIndexType> q;
            for (DagIndexType i = 0; i < nodeCount; i++) if (indegree[i] == 0) q.push(i);

            std::vector<DagIndexType> order;
            while (!q.empty()) {
                auto n = q.front(); q.pop();
                order.push_back(n);
                for (DagIndexType e = nodes[n].firstEdge; e != npos; e = edges[e].next) {
                    auto to = edges[e].to;
                    if (!edgeFilter || edgeFilter(n, to, edges[e])) {
                        if (--indegree[to] == 0) q.push(to);
                    }
                }
            }

            return order;
        }

        std::array<std::vector<DagIndexType>, MaxNodes> transitivelyReducePerNode(ReachableFn edgeFilter = nullptr) const {
            std::array<std::vector<DagIndexType>, MaxNodes> reducedEdges;
            auto order = topologicalSort(edgeFilter);

            for (auto u : order) {
                for (DagIndexType e = nodes[u].firstEdge; e != npos; e = edges[e].next) {
                    auto v = edges[e].to;
                    // Custom reachability: skip the direct edge (u, v)
                    std::vector<bool> seen(MaxNodes, false);
                    bool altPath = false;
                    for (DagIndexType f = nodes[u].firstEdge; f != npos; f = edges[f].next) {
                        if (f == e) continue; // skip direct edge
                        if (dfsReachable(edges[f].to, v, seen, edgeFilter)) {
                            altPath = true;
                            break;
                        }
                    }
                    if (!altPath) {
                        reducedEdges[u].push_back(v);
                    }
                }
            }
            return reducedEdges;
        }

    private:
        void dfsMark(DagIndexType node, std::vector<bool>& seen) const {
            if (seen[node]) return;
            seen[node] = true;
            for (DagIndexType e = nodes[node].firstEdge; e != npos; e = edges[e].next) {
                dfsMark(edges[e].to, seen);
            }
        }

        bool dfsReachable(DagIndexType current, DagIndexType target, std::vector<bool>& seen, ReachableFn edgeFilter) const {
            if (current == target) return true;
            if (seen[current]) return false;
            seen[current] = true;

            for (DagIndexType e = nodes[current].firstEdge; e != npos; e = edges[e].next) {
                if (edgeFilter(current, edges[e].to, edges[e]) && dfsReachable(edges[e].to, target, seen, edgeFilter))
                    return true;
            }
            return false;
        }
    };

    template<size_t MaxNodes, size_t MaxEdges>
    struct StaticDAG<void, MaxNodes, MaxEdges> {
        using data_type = void;

        static constexpr DagIndexType npos = -1;

        struct Node {
            using data_type = data_type;
            DagIndexType firstEdge = npos;
        };

        struct Edge {
            DagIndexType to;
            DagIndexType next;
            DagEdgeFlags flags; // optional metadata
        };

        std::array<Node, MaxNodes> nodes{};
        std::array<Edge, MaxEdges> edges{};
        DagIndexType edgeCount = 0;
        DagIndexType nodeCount = 0;
        const char* lastError = nullptr;

        // === Customizable reachability function ===
        using ReachableFn = std::function<bool(DagIndexType from, DagIndexType to, const Edge& edge)>;

        void clear() {
            lastError = nullptr;
            edgeCount = 0;
            nodeCount = 0;
        }

        bool reachable(DagIndexType from, DagIndexType target,
            ReachableFn edgeFilter = [](DagIndexType, DagIndexType, const Edge&) { return true; }) const
        {
            if (from >= nodeCount || target >= nodeCount) return false;
            std::vector<bool> seen(MaxNodes, false);
            return dfsReachable(from, target, seen, edgeFilter);
        }

        DagIndexType addNode() {
            if (nodeCount >= MaxNodes) { lastError = "Node pool full"; return npos; }
            nodes[nodeCount] = { npos };
            return nodeCount++;
        }

        void addEdge(DagIndexType from, DagIndexType to, DagEdgeFlags flags = 0) {
            if (from >= nodeCount || to >= nodeCount) { lastError = "Invalid node index"; return nullptr; }
            if (reachable(to, from)) { lastError = "Cycle detected"; return nullptr; }
            if (edgeCount >= MaxEdges) { lastError = "Edge pool full"; return nullptr; }

            edges[edgeCount] = { to, nodes[from].firstEdge, flags };
            nodes[from].firstEdge = edgeCount++;
        }

        std::vector<DagIndexType> topologicalSort(ReachableFn edgeFilter = nullptr) const {
            std::vector<DagIndexType> indegree(nodeCount, 0);

            for (DagIndexType i = 0; i < nodeCount; i++) {
                for (DagIndexType e = nodes[i].firstEdge; e != npos; e = edges[e].next) {
                    if (!edgeFilter || edgeFilter(i, edges[e].to, edges[e])) {
                        indegree[edges[e].to]++;
                    }
                }
            }

            std::queue<DagIndexType> q;
            for (DagIndexType i = 0; i < nodeCount; i++) if (indegree[i] == 0) q.push(i);

            std::vector<DagIndexType> order;
            while (!q.empty()) {
                auto n = q.front(); q.pop();
                order.push_back(n);
                for (DagIndexType e = nodes[n].firstEdge; e != npos; e = edges[e].next) {
                    auto to = edges[e].to;
                    if (!edgeFilter || edgeFilter(n, to, edges[e])) {
                        if (--indegree[to] == 0) q.push(to);
                    }
                }
            }

            return order;
        }

        std::array<std::vector<DagIndexType>, MaxNodes> transitivelyReducePerNode(ReachableFn edgeFilter = nullptr) const {
            std::array<std::vector<DagIndexType>, MaxNodes> reducedEdges;
            auto order = topologicalSort(edgeFilter);

            for (auto u : order) {
                for (DagIndexType e = nodes[u].firstEdge; e != npos; e = edges[e].next) {
                    auto v = edges[e].to;
                    // Custom reachability: skip the direct edge (u, v)
                    std::vector<bool> seen(MaxNodes, false);
                    bool altPath = false;
                    for (DagIndexType f = nodes[u].firstEdge; f != npos; f = edges[f].next) {
                        if (f == e) continue; // skip direct edge
                        if (dfsReachable(edges[f].to, v, seen, edgeFilter)) {
                            altPath = true;
                            break;
                        }
                    }
                    if (!altPath) {
                        reducedEdges[u].push_back(v);
                    }
                }
            }
            return reducedEdges;
        }

    private:
        void dfsMark(DagIndexType node, std::vector<bool>& seen) const {
            if (seen[node]) return;
            seen[node] = true;
            for (DagIndexType e = nodes[node].firstEdge; e != npos; e = edges[e].next) {
                dfsMark(edges[e].to, seen);
            }
        }

        bool dfsReachable(DagIndexType current, DagIndexType target, std::vector<bool>& seen, ReachableFn edgeFilter) const {
            if (current == target) return true;
            if (seen[current]) return false;
            seen[current] = true;

            for (DagIndexType e = nodes[current].firstEdge; e != npos; e = edges[e].next) {
                if (edgeFilter(current, edges[e].to, edges[e]) && dfsReachable(edges[e].to, target, seen, edgeFilter))
                    return true;
            }
            return false;
        }
    };

    template<typename T>
    struct DynamicDAG {
        static constexpr DagIndexType npos = -1;

        struct Node {
            T data;
            std::vector<std::pair<DagIndexType, DagEdgeFlags>> edges; // target + optional flags
        };

        std::vector<Node> nodes;
        const char* lastError = nullptr;

        using ReachableFn = std::function<bool(DagIndexType from, DagIndexType to, uint32_t flags)>;

        void clear() {
            lastError = nullptr;
            nodes.clear();
        }

        DagIndexType addNode(const T& data) {
            nodes.push_back({ data, {} });
            return nodes.size() - 1;
        }

        bool addEdge(DagIndexType from, DagIndexType to, DagEdgeFlags flags = 0) {
            if (from >= nodes.size() || to >= nodes.size()) { lastError = "Invalid node index"; return false; }
            if (reachable(to, from)) { lastError = "Cycle detected"; return false; }
            nodes[from].edges.emplace_back(to, flags);
            return true;
        }

        bool reachable(DagIndexType from, DagIndexType target, ReachableFn edgeFilter = nullptr) const {
            if (from >= nodes.size() || target >= nodes.size()) return false;
            std::vector<bool> seen(nodes.size(), false);
            return dfsReachable(from, target, seen, edgeFilter);
        }

        std::vector<DagIndexType> topologicalSort(ReachableFn edgeFilter = nullptr) const {
            std::vector<DagIndexType> indegree(nodes.size(), 0);
            for (DagIndexType i = 0; i < nodes.size(); i++) {
                for (auto& [to, flags] : nodes[i].edges) {
                    if (!edgeFilter || edgeFilter(i, to, flags)) indegree[to]++;
                }
            }

            std::queue<DagIndexType> q;
            for (DagIndexType i = 0; i < nodes.size(); i++) if (indegree[i] == 0) q.push(i);

            std::vector<DagIndexType> order;
            while (!q.empty()) {
                auto n = q.front(); q.pop();
                order.push_back(n);
                for (auto& [to, flags] : nodes[n].edges) {
                    if (!edgeFilter || edgeFilter(n, to, flags)) {
                        if (--indegree[to] == 0) q.push(to);
                    }
                }
            }

            return order;
        }

        std::vector<std::vector<DagIndexType>> transitivelyReducePerNode(ReachableFn edgeFilter = nullptr) const {
            std::vector<std::vector<DagIndexType>> reducedEdges(nodes.size());

            auto order = topologicalSort(edgeFilter);
            for (auto u : order) {
                for (size_t i = 0; i < nodes[u].edges.size(); ++i) {
                    auto v = nodes[u].edges[i].first;
                    // Custom reachability: skip the direct edge (u, v)
                    std::vector<bool> seen(nodes.size(), false);
                    bool altPath = false;
                    for (size_t j = 0; j < nodes[u].edges.size(); ++j) {
                        if (i == j) continue; // skip direct edge
                        if (dfsReachable(nodes[u].edges[j].first, v, seen, edgeFilter)) {
                            altPath = true;
                            break;
                        }
                    }
                    if (!altPath) {
                        reducedEdges[u].push_back(v);
                    }
                }
            }
            return reducedEdges;
        }

    private:
        void dfsMark(DagIndexType current, std::vector<bool>& seen) const {
            if (seen[current]) return;
            seen[current] = true;
            for (auto& [to, _] : nodes[current].edges) {
                dfsMark(to, seen);
            }
        }

        bool dfsReachable(DagIndexType current, DagIndexType target, std::vector<bool>& seen, ReachableFn edgeFilter) const {
            if (current == target) return true;
            if (seen[current]) return false;
            seen[current] = true;

            for (auto& [to, flags] : nodes[current].edges) {
                if (!edgeFilter || edgeFilter(current, to, flags)) {
                    if (dfsReachable(to, target, seen, edgeFilter)) return true;
                }
            }
            return false;
        }
    };

    template<>
    struct DynamicDAG<void> {
        static constexpr DagIndexType npos = -1;

        struct Node {
            std::vector<std::pair<DagIndexType, DagEdgeFlags>> edges; // target + optional flags
        };

        std::vector<Node> nodes;
        const char* lastError = nullptr;

        using ReachableFn = std::function<bool(DagIndexType from, DagIndexType to, uint32_t flags)>;

        void clear() {
            lastError = nullptr;
            nodes.clear();
        }

        DagIndexType addNode() {
            nodes.push_back({ {} });
            return nodes.size() - 1;
        }

        bool addEdge(DagIndexType from, DagIndexType to, DagEdgeFlags flags = 0) {
            if (from >= nodes.size() || to >= nodes.size()) { lastError = "Invalid node index"; return false; }
            if (reachable(to, from)) { lastError = "Cycle detected"; return false; }
            nodes[from].edges.emplace_back(to, flags);
            return true;
        }

        bool reachable(DagIndexType from, DagIndexType target, ReachableFn edgeFilter = nullptr) const {
            if (from >= nodes.size() || target >= nodes.size()) return false;
            std::vector<bool> seen(nodes.size(), false);
            return dfsReachable(from, target, seen, edgeFilter);
        }

        std::vector<DagIndexType> topologicalSort(ReachableFn edgeFilter = nullptr) const {
            std::vector<DagIndexType> indegree(nodes.size(), 0);
            for (DagIndexType i = 0; i < nodes.size(); i++) {
                for (auto& [to, flags] : nodes[i].edges) {
                    if (!edgeFilter || edgeFilter(i, to, flags)) indegree[to]++;
                }
            }

            std::queue<DagIndexType> q;
            for (DagIndexType i = 0; i < nodes.size(); i++) if (indegree[i] == 0) q.push(i);

            std::vector<DagIndexType> order;
            while (!q.empty()) {
                auto n = q.front(); q.pop();
                order.push_back(n);
                for (auto& [to, flags] : nodes[n].edges) {
                    if (!edgeFilter || edgeFilter(n, to, flags)) {
                        if (--indegree[to] == 0) q.push(to);
                    }
                }
            }

            return order;
        }

        std::vector<std::vector<DagIndexType>> transitivelyReducePerNode(ReachableFn edgeFilter = nullptr) const {
            std::vector<std::vector<DagIndexType>> reducedEdges(nodes.size());

            auto order = topologicalSort(edgeFilter);
            for (auto u : order) {
                for (size_t i = 0; i < nodes[u].edges.size(); ++i) {
                    auto v = nodes[u].edges[i].first;
                    // Custom reachability: skip the direct edge (u, v)
                    std::vector<bool> seen(nodes.size(), false);
                    bool altPath = false;
                    for (size_t j = 0; j < nodes[u].edges.size(); ++j) {
                        if (i == j) continue; // skip direct edge
                        if (dfsReachable(nodes[u].edges[j].first, v, seen, edgeFilter)) {
                            altPath = true;
                            break;
                        }
                    }
                    if (!altPath) {
                        reducedEdges[u].push_back(v);
                    }
                }
            }
            return reducedEdges;
        }

    private:
        void dfsMark(DagIndexType current, std::vector<bool>& seen) const {
            if (seen[current]) return;
            seen[current] = true;
            for (auto& [to, _] : nodes[current].edges) {
                dfsMark(to, seen);
            }
        }

        bool dfsReachable(DagIndexType current, DagIndexType target, std::vector<bool>& seen, ReachableFn edgeFilter) const {
            if (current == target) return true;
            if (seen[current]) return false;
            seen[current] = true;

            for (auto& [to, flags] : nodes[current].edges) {
                if (!edgeFilter || edgeFilter(current, to, flags)) {
                    if (dfsReachable(to, target, seen, edgeFilter)) return true;
                }
            }
            return false;
        }
    };

    template<typename DAG>
    void exportToDot(
        const DAG& dag,
        std::ostream& output,
        std::function<std::string(DagIndexType)> nodeLabel = nullptr,
        std::function<bool(DagIndexType, DagIndexType)> edgeFilter = nullptr
    ) {
        output << "digraph DAG {\n";

        // Default node label function
        if (!nodeLabel) {
            nodeLabel = [&](DagIndexType i) {
                // Attempt to access `data` for label
                if constexpr (requires { dag.nodes[i].data; }) {
                    return std::to_string(dag.nodes[i].data);
                }
                else {
                    return std::to_string(i);
                }
                };
        }

        // Iterate nodes and edges
        if constexpr (requires { typename DAG::Node; DAG::npos; }) {
            // StaticDAG
            for (size_t i = 0; i < dag.nodeCount; ++i) {
                for (DagIndexType e = dag.nodes[i].firstEdge; e != DAG::npos; e = dag.edges[e].next) {
                    auto to = dag.edges[e].to;
                    if (!edgeFilter || edgeFilter(i, to)) {
                        output << "    " << nodeLabel(i) << " -> " << nodeLabel(to) << ";\n";
                    }
                }
            }
        }
        else {
            // DynamicDAG
            for (size_t i = 0; i < dag.nodes.size(); ++i) {
                for (auto& [to, _] : dag.nodes[i].edges) {
                    if (!edgeFilter || edgeFilter(i, to)) {
                        output << "    " << nodeLabel(i) << " -> " << nodeLabel(to) << ";\n";
                    }
                }
            }
        }

        output << "}\n";
    }



} // namespace dag