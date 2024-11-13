#include<vector>
#include<utility>
#include<unordered_map>
#include<fstream>
#include<string>
using namespace std;

class Tile {
    public:
    int id;         // Block index (0 if space tile)
    int id_vec;     // Index in vector
    Tile* tr;       // Topmost-right tile
    Tile* rt;       // Rightmost-top tile
    Tile* lb;       // Leftmost-bottom tile
    Tile* bl;       // Bottommost-left tile
    int x, y;       // Bottom-left corner coordinates
    int w, h;     

    Tile(int index, int index_vec, int x, int y, int w, int h)
        : id(index), id_vec(index_vec),
          tr(nullptr), rt(nullptr), lb(nullptr), bl(nullptr), 
          x(x), y(y), w(w), h(h) {};
    void set_id(int i) {this->id = i;};
    void set_id_vec(int i) {this->id_vec = i;};
    void set_width (int w) {this->w = w;};
    void set_height(int h) {this->h = h;};
    void set_x(int x) {this->x = x;};
    void set_y(int y) {this->y = y;};
    int get_id(){return this->id;};
    int get_x() {return this->x;};
    int get_y() {return this->y;};
    int get_w() {return this->w;};
    int get_h() {return this->h;};
    friend ostream& operator<<(ostream& os, const Tile& tile);
};

class Graph {
    public:
    Graph(int outline_w, int outline_h);
    void output_drawing_text(string output_file);
    void create_tile (int id, int x, int y, int w, int h);
    void remove_tile (Tile *tile);
    Tile* find_point (int x, int y);
    vector<Tile*> find_neighbors (Tile *tile, int op);

    vector<Tile*> get_tiles() {return this->tiles;};
    vector<Tile*> get_block_tiles() {return this->block_tiles;};
    int get_outline_width  () {return this->outline_w;};
    int get_outline_height () {return this->outline_h;};

    private:
    int outline_w, outline_h;
    int space_cnt;

    vector<Tile*> tiles;
    vector<Tile*> block_tiles;


    pair<Tile*, Tile*> split_tile_h (Tile *tile, Tile *block_tile, int a, int b);
    pair<Tile*, pair<Tile*, Tile*>> split_tile_v (Tile *tile, Tile *block_tile, int a, int b);
    bool search_area (int x, int y, int w, int h);
    void free_tile (Tile *tile);
    Tile* __create_tile (int id, int x, int y, int w, int h);
    void remove_from_list (Tile *tile);
    void remove_space_tile (Tile *tile);
    void append_to_list (Tile *tile);
    int  mergeable (Tile *a, Tile *b, bool allow_block=false);
    Tile* merge_tile (Tile *a, Tile *b, bool allow_block=false);
};
