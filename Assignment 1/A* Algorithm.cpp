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
 
// Position struct representing a cell in the grid with coordinates (x, y).
// movement cost (g), heuristic value (h), and a pointer to the parent position.
struct Position {
    int x, y, g, h;
    shared_ptr<Position> parent;
    bool has_backdoor_key;
 
    Position(int x, int y, int g, int h, bool has_key = false, shared_ptr<Position> parent = nullptr)
        : x(x), y(y), g(g), h(h), has_backdoor_key(has_key), parent(parent) {}
 
    int f() const { return g + h; }    // Total cost function (f = g + h)
 
    bool operator==(const Position& other) const { return x == other.x && y == other.y; }
};
 
//Manhattan distance calculation between two points (x1, y1) and (x2, y2)
int manhattan_distance(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}
 
//Variable initialization for the Keymaker's coordinates
int key_x, key_y;
 
// Function to check if a position (x, y) is dangerous, allowing Neo to ignore Agent Smith's perception zones if he can see them
bool is_dangerous(int x, int y, const unordered_set<pair<int, int>, hash_pair>& dangers,
                  const unordered_set<pair<int, int>, hash_pair>& agent_smith_positions,
                  bool has_backdoor_key, int perception_range) {
    // If Neo has the backdoor key, ignore Agent Smith perception zones
    if (has_backdoor_key) {
        auto it = dangers.find({x, y});
        if (it != dangers.end() && it->second == 'A') {
            return false;  // Ignore danger if it's Agent Smith
        }
    }
 
    // If Neo can see Agent Smith, allow movement through his perception cells
    if (!agent_smith_positions.empty()) {
        // Check if the current position is an Agent Smith's perception cell
        if (dangers.count({x, y}) > 0) {
            return false;  // Allow passing through Agent Smith's cell if he can see at least one
        }
    }
 
    // Return true if there is a danger in the area
    return dangers.count({x, y}) > 0;
}
 
// Get valid neighboring positions from the current position, avoiding dangers and respecting grid bounds
vector<shared_ptr<Position>> get_neighbors(const shared_ptr<Position>& pos,
                                           const unordered_set<pair<int, int>, hash_pair>& dangers,
                                           const unordered_set<pair<int, int>, hash_pair>& agent_smith_positions,
                                           int perception_range) {
    vector<shared_ptr<Position>> neighbors;
    vector<pair<int, int>> directions = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};  // Restrict to adjacent moves only
    for (auto [dx, dy] : directions) {
        int nx = pos->x + dx, ny = pos->y + dy;
        // Ensure the new position is within grid bounds
        if (nx >= 0 && nx < 9 && ny >= 0 && ny < 9) {  // Ensure bounds
            // Check if the position is dangerous
            if (!is_dangerous(nx, ny, dangers, agent_smith_positions, pos->has_backdoor_key, perception_range)) {
                // Create a neighbor position and add it to the list
                shared_ptr<Position> neighbor = make_shared<Position>(nx, ny, pos->g + 1,
                                                                      manhattan_distance(nx, ny, key_x, key_y), pos->has_backdoor_key, pos);
                neighbors.push_back(neighbor);
            }
        }
    }
    return neighbors;
}
 
// Reconstructs and outputs the path from the start to the target position
void reconstruct_path(shared_ptr<Position> end_pos) {
    vector<pair<int, int>> path;
    for (shared_ptr<Position> p = end_pos; p; p = p->parent) {
        path.emplace_back(p->x, p->y);
    }
    reverse(path.begin(), path.end());
    cout << "e " << path.size() - 1 << endl; //Outputting the Path Size
}
 
// A* search algorithm implementation to find the path from the start to the key position
void a_star_search(int variant) {
    unordered_set<pair<int, int>, hash_pair> dangers;
    unordered_map<pair<int, int>, shared_ptr<Position>, hash_pair> open_set;
    unordered_set<pair<int, int>, hash_pair> closed_list;
 
    auto cmp = [](const shared_ptr<Position>& a, const shared_ptr<Position>& b) {
        if (a->f() == b->f()) {
            return manhattan_distance(a->x, a->y, key_x, key_y) > manhattan_distance(b->x, b->y, key_x, key_y);
        }
        return a->f() > b->f();
    };
 
    priority_queue<shared_ptr<Position>, vector<shared_ptr<Position>>, decltype(cmp)> open_list(cmp);
    shared_ptr<Position> start = make_shared<Position>(0, 0, 0, manhattan_distance(0, 0, key_x, key_y));
    open_list.push(start);
    open_set[{0, 0}] = start;
    unordered_set<pair<int, int>, hash_pair> agent_smith_positions; // Track positions of Agent Smiths
 
    // Main A* loop
    while (!open_list.empty()) {
        shared_ptr<Position> current = open_list.top();
        open_list.pop();
 
        // Output the current position in the path
        if (current->x != key_x || current->y != key_y) {
            cout << "m " << current->x << " " << current->y << endl;
        }
 
        // Check if the goal is reached
        if (current->x == key_x && current->y == key_y) {
            reconstruct_path(current);
            return;
        }
 
        //Mark current node as visited
        closed_list.insert({current->x, current->y});
 
        //Reading dangerous positions and items
        int n;
        cin >> n;
        dangers.clear();
        for (int i = 0; i < n; ++i) {
            int x, y;
            char type;
            cin >> x >> y >> type;
 
            if (type == 'B') {
                current->has_backdoor_key = true; // Collecting backdoor key
            }
 
            if (type == 'A') {
                agent_smith_positions.insert({x, y}); // Add Agent Smith position
            } else if (type == 'S' || type == 'P') {
                dangers.insert({x, y}); // Other dangers
            }
        }
 
        // Determining perception range based on variant
        int perception_range = (variant == 1) ? 1 : 2;
 
        // Exploring neighboring positions
        for (auto& neighbor : get_neighbors(current, dangers, agent_smith_positions, perception_range)) {
            if (closed_list.count({neighbor->x, neighbor->y})) {
                continue; // Skip already visited positions
            }
 
            // Calculate tentative g value for neighbor
            int tentative_g = current->g + 1;
            neighbor->h = manhattan_distance(neighbor->x, neighbor->y, key_x, key_y);
 
            // Update the open set if the neighbor position is new or has a better g score
            auto it = open_set.find({neighbor->x, neighbor->y});
            if (it == open_set.end() || tentative_g < it->second->g) {
                neighbor->g = tentative_g;
                neighbor->parent = current;
 
                if (it == open_set.end()) {
                    open_list.push(neighbor);
                    open_set[{neighbor->x, neighbor->y}] = neighbor;
                } else {
                    open_set[{neighbor->x, neighbor->y}] = neighbor;
                }
            }
        }
    }
    cout << "e -1" << endl;     // If no path is found, output failure.
}
 
int main() {
    int variant;
    cin >> variant >> key_x >> key_y; // Read variant and key position
    a_star_search(variant); // Run A* search with the given variant
    return 0;
}