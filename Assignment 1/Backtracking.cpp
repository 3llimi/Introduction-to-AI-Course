#include <bits/stdc++.h>
using namespace std;
 
// Hash Function for hashing pairs
struct hash_pair {
    template <class T1, class T2>
    size_t operator()(const pair<T1, T2>& p) const {
        auto hash1 = hash<T1>{}(p.first);
        auto hash2 = hash<T2>{}(p.second);
        return hash1 ^ (hash2 << 1);
    }
};
 
// Position struct representing a cell in the grid
struct Position {
    int x, y, g, h; // g is the cost from the start, h is the heuristic
    shared_ptr<Position> parent;
    bool has_backdoor_key;
 
    Position(int x, int y, int g, int h, bool has_key = false, shared_ptr<Position> parent = nullptr)
        : x(x), y(y), g(g), h(h), has_backdoor_key(has_key), parent(parent) {}
 
    int f() const { return g + h; } // f(n) = g(n) + h(n)
 
    bool operator==(const Position& other) const { return x == other.x && y == other.y; }
};
 
// Manhattan distance calculation between two points
int manhattan_distance(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}
 
// Variable initialization for the Keymaker's coordinates
int key_x, key_y;
 
// Function to check if a position (x, y) is dangerous
bool is_dangerous(int x, int y, const unordered_set<pair<int, int>, hash_pair>& dangers,
                  const unordered_set<pair<int, int>, hash_pair>& agent_smith_positions, bool has_backdoor_key) {
    // If Neo has the backdoor key, he can ignore Agent Smith's perception zones
    if (has_backdoor_key) {
        return dangers.count({x, y}) > 0 && agent_smith_positions.count({x, y}) > 0;
    }
    // Dangerous if it's in the dangers set and not an Agent Smith's position
    return dangers.count({x, y}) > 0 || agent_smith_positions.count({x, y}) > 0;
}
 
// Get valid neighboring positions from the current position, avoiding dangers and respecting grid bounds
vector<shared_ptr<Position>> get_neighbors(const shared_ptr<Position>& pos,
                                           const unordered_set<pair<int, int>, hash_pair>& dangers,
                                           const unordered_set<pair<int, int>, hash_pair>& agent_smith_positions) {
    vector<shared_ptr<Position>> neighbors;
    vector<pair<int, int>> directions = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}; // Adjacent moves only
 
    for (auto [dx, dy] : directions) {
        int nx = pos->x + dx, ny = pos->y + dy;
 
        // Ensure the new position is within grid bounds
        if (nx >= 0 && nx < 9 && ny >= 0 && ny < 9) {
            // Check if the position is dangerous
            if (!is_dangerous(nx, ny, dangers, agent_smith_positions, pos->has_backdoor_key)) {
                // Heuristic based on distance to the Keymaker
                int h = manhattan_distance(nx, ny, key_x, key_y);
                shared_ptr<Position> neighbor = make_shared<Position>(nx, ny, pos->g + 1, h, pos->has_backdoor_key, pos);
                neighbors.push_back(neighbor);
            }
        }
    }
 
    // Prioritize based on y-coordinate proximity to Keymaker's column
    sort(neighbors.begin(), neighbors.end(), [](const shared_ptr<Position>& a, const shared_ptr<Position>& b) {
        // prioritize by vertical alignment (same column as Keymaker)
        if (a->y == key_y && b->y != key_y) {
            return true;
        }
        if (b->y == key_y && a->y != key_y) {
            return false;
        }
        return a->f() < b->f();
    });
 
    return neighbors;
}
 
// Function to reconstruct and output the path from start to end position
void reconstruct_path(shared_ptr<Position> end_pos) {
    vector<pair<int, int>> path;
    for (shared_ptr<Position> p = end_pos; p; p = p->parent) {
        path.emplace_back(p->x, p->y);
    }
    reverse(path.begin(), path.end());
    cout << "e " << path.size() - 1 << endl; // Print the number of moves
}
 
// Backtracking search to find the path to the Keymaker
bool backtracking_search(shared_ptr<Position> current,
                         unordered_set<pair<int, int>, hash_pair>& dangers,
                         unordered_set<pair<int, int>, hash_pair>& agent_smith_positions,
                         unordered_set<pair<int, int>, hash_pair>& visited) {
    // Check if the Keymaker was reached
    if (current->x == key_x && current->y == key_y) {
        reconstruct_path(current); // Output the path
        return true; // Terminate the search
    }
 
    // Mark the current position as visited
    visited.insert({current->x, current->y});
    cout << "m " << current->x << " " << current->y << endl;  // Output the move
 
    // Get neighbors and check for danger
    auto neighbors = get_neighbors(current, dangers, agent_smith_positions);
 
    // If no valid neighbors are available, terminate with e -1
    if (neighbors.empty()) {
        cout << "e -1" << endl; // No valid moves left
        return false; // Path not found
    }
 
    for (auto& neighbor : neighbors) {
        if (visited.count({neighbor->x, neighbor->y}) == 0) { // Process unvisited neighbors
            int n;
            cin >> n; // Read number of dangers
            for (int i = 0; i < n; ++i) {
                int x, y;
                char type;
                cin >> x >> y >> type; // Read danger position and type
 
                // Check if Neo picks up the backdoor key
                if (current->x == x && current->y == y && type == 'B') {
                    neighbor->has_backdoor_key = true; // Neo picks up the backdoor key
                }
 
                // Track Agent Smith and other dangers based on type
                if (type == 'A') {
                    agent_smith_positions.insert({x, y});
                } else if (type == 'S' || type == 'P') {
                    dangers.insert({x, y});
                }
            }
 
            // Recursively search from the neighbor position
            if (backtracking_search(neighbor, dangers, agent_smith_positions, visited)) {
                return true;  // If the path to the Keymaker is found, return true
            }
        }
    }
 
    // Unmark current position if path not found from this position
    visited.erase({current->x, current->y});
    return false; // Path not found
}
 
int main() {
    int variant;
    cin >> variant >> key_x >> key_y; // Read variant and Keymaker's position
 
    unordered_set<pair<int, int>, hash_pair> dangers; // Set of dangerous positions
    unordered_set<pair<int, int>, hash_pair> agent_smith_positions; // Set of Agent Smith positions
    unordered_set<pair<int, int>, hash_pair> visited; // Set of visited positions
    // Start from initial position (0, 0) with g = 0, and heuristic to Keymaker
    shared_ptr<Position> start = make_shared<Position>(0, 0, 0, manhattan_distance(0, 0, key_x, key_y));
 
    // Start the backtracking search and output -1 if Keymaker is unreachable
    if (!backtracking_search(start, dangers, agent_smith_positions, visited)) {
        cout << "e -1" << endl; // Output if Keymaker is unreachable
    }
 
    return 0;
}