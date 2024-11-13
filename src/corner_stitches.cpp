#include<iostream>
#include<fstream>
#include<string>
#include"corner_stitches.hpp"
using namespace std;
void Graph::output_drawing_text(string output_file) {
    cout << "draw file: " << output_file << '\n';
    ofstream output(output_file);
    output << this->tiles.size() << '\n';
    output << this->outline_w << ' ' << this->outline_h << '\n';
    for(auto tile : this->tiles) {
        output << tile->id << ' ';
        output << tile->x << ' ' << tile->y << ' ' << tile->w << ' ' << tile->h << '\n';
    }
}
ostream& operator<<(ostream& os, const Tile& tile) {
    os << (tile.id <= 0 ? "Space" : "Block") << ", ID: " << tile.id
       << ", x0: " << tile.x << ", x1: " << tile.x + tile.w
       << ", y0: " << tile.y << ", y1: " << tile.y + tile.h << "\n";
    auto print_neighbor = [&os](const string& name, Tile* neighbor) {
        if (neighbor) {
            os << name << ": ("
               << (neighbor->id < 0 ? "Space" : "Block") << ", ID: " << neighbor->id
               << ", x0: " << neighbor->x << ", x1: " << neighbor->x + neighbor->w
               << ", y0: " << neighbor->y << ", y1: " << neighbor->y + neighbor->h << ")\n";
        }
    };
    print_neighbor("\ttr", tile.tr);
    print_neighbor("\trt", tile.rt);
    print_neighbor("\tlb", tile.lb);
    print_neighbor("\tbl", tile.bl);
    return os;
}

Graph::Graph(int outline_w, int outline_h)
     : outline_w(outline_w), outline_h(outline_h), space_cnt(0) {
    
    Tile *tile = this->__create_tile(0, 0, 0, outline_w, outline_h);
    this->append_to_list(tile);
};

inline bool in_range(int x0, int x1, int y0, int y1, int x, int y){
    return (x >= x0 && x <= x1) && (y >= y0 && y <= y1);
}
Tile* Graph::find_point(int x, int y){
    if (x < 0 || x >= this->outline_w || y < 0 || y >= this->outline_h) return nullptr;
    int st = 0;
    Tile *cur = this->tiles[st];
    int bottom = cur->y, top   = cur->y + cur->h - 1, 
        left   = cur->x, right = cur->x + cur->w - 1;
    // iterate through vertically and horizontally alternatively
    bool horizontal = 0;
    for(;!in_range(left, right, bottom, top, x, y);){
        bottom = cur->y, top   = cur->y + cur->h - 1, 
        left   = cur->x, right = cur->x + cur->w - 1;
        if(horizontal){
            // point is to the right of current tile
            if (x > right) {
                cur = cur->tr;
            }
            // point is to the left of current tile
            else if(x < left) {
                cur = cur->bl;
            } else {
                horizontal = !horizontal;
            }
        }
        else {
            // point is below current tile
            if (y < bottom) {
                cur = cur->lb;
            }
            // point is above current tile
            else if(y > top) {
                cur = cur->rt;
            } else {
                horizontal = !horizontal;
            }
        }
    }
    return cur;
}
vector<Tile*> Graph::find_neighbors (Tile *tile, int op=0xF) {
    // op is a bit mast <1111> corresponds to four edges <lrbt>
    vector<Tile*> nbs;
    Tile *cur;
    // left
    for (cur = tile->bl; cur && op & 1; cur = cur->rt) {
        if (cur->y >= tile->y+tile->h) break;
        nbs.push_back(cur);
    }
    // right
    for (cur = tile->tr; cur && op & 2; cur = cur->lb) {
        if (cur->y + cur->h <= tile->y) break;
        nbs.push_back(cur);
    }
    // bottom
    for (cur = tile->lb; cur && op & 4; cur = cur->tr) {
        if (cur->x >= tile->x+tile->w) break;
        nbs.push_back(cur);
    }
    // top
    for (cur = tile->rt; cur && op & 8; cur = cur->bl) {
        if (cur->x + cur->w <= tile->x) break;
        nbs.push_back(cur);
    }
    return nbs;
}
pair<Tile*, Tile*> Graph::split_tile_h (Tile *tile, Tile *block_tile, int a, int b) {
    int bottom = tile->y, top   = tile->y + tile->h, 
        left   = tile->x, right = tile->x + tile->w;
    Tile *top_tile = tile, *bottom_tile = nullptr;
    if (bottom < a && top > a) {
        // Set tile's new shape
        top_tile->set_height(top - a);
        top_tile->set_y(a);
        bottom_tile = this->__create_tile(0, left, bottom, tile->w, a-bottom);
        this->append_to_list(bottom_tile);
        // Set pointers
        bottom_tile->rt = top_tile;
        bottom_tile->bl = top_tile->bl;
        bottom_tile->lb = top_tile->lb;
        top_tile->lb = bottom_tile;
        
        // Get bottom tile's tr by going down from top tiles's tr
        Tile *cur = top_tile->tr;
        while (cur && cur->y >= bottom_tile->y + bottom_tile->h) {
            cur = cur->lb;
        }
        bottom_tile->tr = cur;
        // Get top tile's bl by going up from bottom tile's bl
        cur = bottom_tile->bl;
        while (cur && cur->y + cur->h <= top_tile->y) {
            cur = cur->rt;
        }
        top_tile->bl = cur;

        // Update neighbors' pointers
        // Right neighbors
        cur = bottom_tile->tr;
        while (cur && cur->y >= bottom_tile->y) {
            cur->bl = bottom_tile;
            cur = cur->lb;
        }
        // Left neighbors
        cur = bottom_tile->bl;
        while (cur && cur->y + cur->h <= bottom_tile->y + bottom_tile->h) {
            cur->tr = bottom_tile;
            cur = cur->rt;
        }
        // Bottom neighbors
        cur = bottom_tile->lb;
        while (cur && cur->x + cur->w <= bottom_tile->x + bottom_tile->w) {
            cur->rt = bottom_tile;
            cur = cur->tr;
        }
    }
    return {top_tile, bottom_tile};
}

pair<Tile*, pair<Tile*, Tile*>> Graph::split_tile_v (Tile *tile, Tile *block_tile, int a, int b) {
    int bottom = tile->y, top   = tile->y + tile->h, 
        left   = tile->x, right = tile->x + tile->w;
    Tile *left_tile = nullptr, *mid_tile = nullptr, *right_tile = nullptr;
    bool split_left = left < a, split_right = right > b;
    mid_tile = this->__create_tile(0, a, bottom, b-a, top-bottom);
    // Update pointers
    if (split_right) {
        right_tile = this->__create_tile(0, b, bottom, right-b, top-bottom);
        right_tile->bl = mid_tile;
        right_tile->tr = tile->tr;
        right_tile->rt = tile->rt;
        mid_tile->tr = right_tile;
    } else {
        mid_tile->tr = tile->tr;
        mid_tile->rt = tile->rt;
    }
    if (split_left) {
        left_tile = this->__create_tile(0, tile->x, tile->y, a-tile->x, tile->h);
        left_tile->tr = mid_tile;
        left_tile->bl = tile->bl;
        left_tile->lb = tile->lb;
        mid_tile->bl = left_tile;
    } else {
        mid_tile->bl = tile->bl;
        mid_tile->lb = tile->lb;
    }
    // Update neighbor's pointer
    vector<Tile*> left_nbs, right_nbs, top_nbs, bottom_nbs; 
    left_nbs   = this->find_neighbors(tile, 1);
    right_nbs  = this->find_neighbors(tile, 2);
    bottom_nbs = this->find_neighbors(tile, 4);
    top_nbs    = this->find_neighbors(tile, 8);
    // left
    Tile *to_set = split_left ? left_tile : mid_tile;
    for(Tile *iter: left_nbs) {
        if (iter->y + iter->h <= to_set->y + to_set->h)
            iter->tr = to_set;
    }
    // right
    to_set = split_right ? right_tile : mid_tile;
    for(Tile *iter: right_nbs) {
        if (iter->y >= to_set->y)
            iter->bl = to_set;
    }
    // bottom
    bool first_0 = true, first_1 = true;
    for(Tile *iter: bottom_nbs) {
        int iter_right = iter->x + iter->w;
        if (split_left && iter_right <= left_tile->x + left_tile->w) {
            iter->rt = left_tile;
        } else if (iter_right > mid_tile->x && iter_right <= mid_tile->x + mid_tile->w) {
            iter->rt = mid_tile;
        } else if (split_right && iter_right > right_tile->x && iter_right <= right_tile->x + right_tile->w) {
            iter->rt = right_tile;
        }
        if (iter_right > mid_tile->x && first_0) {
            mid_tile->lb = iter;
            first_0 = false;
        }
        if (split_right && iter_right > right_tile->x && first_1) {
            right_tile->lb = iter;
            first_1 = false;
        }
    }
    // top
    first_0 = first_1 = true;
    for(Tile *iter: top_nbs) {
        if (split_right && iter->x >= right_tile->x) {
            iter->lb = right_tile;
        } else if (iter->x >= mid_tile->x && iter->x < mid_tile->x + mid_tile->w) {
            iter->lb = mid_tile;
        } else if (split_left && iter->x >= left_tile->x) {
            iter->lb = left_tile;
        }
        if (first_0 && iter->x < mid_tile->x + mid_tile->w) {
            mid_tile->rt = iter;
            first_0 = false;
        }
        if (split_left && first_1 && iter->x < left_tile->x + left_tile->w) {
            left_tile->rt = iter;
            first_1 = false;
        }
    }
    this->append_to_list(mid_tile);
    if (split_left) {
        this->append_to_list(left_tile);
        if (this->mergeable(left_tile, left_tile->rt)) {
            Tile *left_up_tile = this->merge_tile(left_tile, left_tile->rt);
            if (this->mergeable(left_up_tile, left_up_tile->lb)) {
                this->merge_tile(left_up_tile, left_up_tile->lb);
            }
        } else if (this->mergeable(left_tile, left_tile->lb)) {
            this->merge_tile(left_tile, left_tile->lb);
        }
    }
    if (split_right) {
        this->append_to_list(right_tile);
        if (this->mergeable(right_tile, right_tile->rt)) {
            Tile *right_up_tile = this->merge_tile(right_tile, right_tile->rt);
            if (this->mergeable(right_up_tile, right_up_tile->lb)) {
                this->merge_tile(right_up_tile, right_up_tile->lb);
            }
        } else if (this->mergeable(right_tile, right_tile->lb)) {
            this->merge_tile(right_tile, right_tile->lb);
        }
    }
    this->remove_space_tile(tile);
    return {mid_tile, {left_tile, right_tile}};
}
bool Graph::search_area(int x, int y, int w, int h){
    // traverse downward to check if space tiles' right bound is
    // (1) smaller than area's right bound => block tile exists
    // (2) larger than area's right bound => good
    Tile *upper_left_tile = this->find_point(x, y+h-1);
    Tile* tile = upper_left_tile;
    for (;;) {
        while (tile->x + tile->w <= x) {
            tile = tile->tr;
        }
        // For space tile, check right bound
        if (tile->id < 0 && (tile->x + tile->w < x + w)) return true;
        // For block tile, check the point is inside or
        // on the edge of the block tile
        if (tile->id >= 0 && (tile->y < y)) return true;

        tile = tile->lb;
        if (!tile || tile->y < y) break;
    }

    return false;
}
Tile* Graph::__create_tile(int id, int x, int y, int w, int h) {
    Tile *tile = new Tile(id, -1, x, y, w, h);
    return tile;
}
void Graph::create_tile(int id, int x, int y, int w, int h) {
    // Check if space is occupied by block
    if (this->search_area(x, y, w, h)) return;
    // if not, insert block tile
    Tile *new_block_tile = this->__create_tile(id, x, y, w, h);
    this->append_to_list(new_block_tile);
    Tile *top_tile, *bottom_tile,
         *top_top_tile, *top_bottom_tile,
         *bottom_top_tile, *bottom_bottom_tile;
    // (1) Find space tile containing top edge of inserting tile
    //     There should be only one space tile due to maximal horizontal strip property
    // (2) Split this space tile vertically by line y = tile->top 
    top_tile = this->find_point(x, y+h-1);
    auto p = this->split_tile_h(top_tile, new_block_tile, y+h, -1);
    top_top_tile    = p.first;
    top_bottom_tile = p.second;
    
    // (3) Same operation for bottom edge of inserting tile
    bottom_tile = this->find_point(x, y);
    p = this->split_tile_h(bottom_tile, new_block_tile, y, -1);
    bottom_top_tile    = p.first;
    bottom_bottom_tile = p.second;
    // (4) Traverse along vertical neighbors in left(right) edges
    //     and split them horizontally
    Tile *cur, *mid = nullptr;
    cur = (top_bottom_tile) ? top_bottom_tile : top_top_tile;

    while(cur != bottom_top_tile->lb){
        while (cur->x + cur->w <= new_block_tile->x) {
            cur = cur->tr;
        }
        auto tmp = this->split_tile_v(cur, new_block_tile, x, x+w);
        Tile *tmp_mid = tmp.first;
        if (mid) {
            mid = this->merge_tile(tmp_mid, mid);
        } else {
            mid = tmp_mid;
        }
        // go down
        cur = cur->lb;
    }
    this->merge_tile(mid, new_block_tile, true);
    return;
}
void Graph::free_tile(Tile *tile) {
    delete tile;
    tile = nullptr;
}
void Graph::remove_from_list(Tile *tile) {
    // This function will delete tile in vector
    // and release memory allocation
    
    // swap this tile to back of vector
    int id = tile->id;
    int id_0 = tile->id_vec, id_1 = this->tiles.size() - 1;
    this->tiles[id_1]->set_id_vec(id_0);
    swap(this->tiles[id_0], this->tiles[id_1]);
    this->tiles.pop_back();
    if (tile->id < 0) {
        this->space_cnt--;
        if (this->tiles[id_0]->id < 0) {
            this->tiles[id_0]->set_id(id);
        }
    }
}
void Graph:: append_to_list(Tile* tile) {
    tile->set_id_vec(this->tiles.size());
    tile->id = (tile->id > 0 ? tile->id : -(++this->space_cnt));
    this->tiles.push_back(tile);
    if (tile->id > 0) {
        this->block_tiles.push_back(tile);
    }
}
void Graph::remove_space_tile(Tile *tile) {
    this->remove_from_list(tile);
    this->free_tile(tile);
}
void Graph::remove_tile(Tile *tile) {
    //vector<Tile*> right_nbs = this->find_neighbors(tile, 2);
    //this->__free_tile();
}
int Graph::mergeable (Tile *src, Tile *dest, bool allow_block) {
    if (!src || !dest) return 0;
    if (!allow_block && (src->id > 0 || dest->id > 0)) return 0;
    //if (src->id > 0 || dest->id > 0) return 0;
    bool align_v = src->w == dest->w && src->x == dest->x, 
         align_h = src->h == dest->h && src->y == dest->y,
         src_to_right  = src->x + src->w >  dest->x + dest->w,
         dest_to_right = src->x + src->w <= dest->x + dest->w,
         src_above     = src->y + src->h >  dest->y + dest->h,
         dest_above    = src->y + src->h <= dest->y + dest->h;
    if (align_v &&       align_h) return 5;
    if (align_v &&     src_above) return 1;
    if (align_v &&    dest_above) return 2;
    if (align_h &&  src_to_right) return 3;
    if (align_h && dest_to_right) return 4;
    return 0;
}
Tile* Graph::merge_tile(Tile *src, Tile *dest, bool allow_block) {
    if (!src || !dest) return nullptr;

    int m = this->mergeable(src, dest, allow_block);
    if (!m) return nullptr;
    vector<Tile*> left_nbs, right_nbs, bottom_nbs, top_nbs;
    left_nbs   = this->find_neighbors(src, 1);
    right_nbs  = this->find_neighbors(src, 2);
    bottom_nbs = this->find_neighbors(src, 4);
    top_nbs    = this->find_neighbors(src, 8);
    switch (m) {
        case 1: 
            dest->set_height(src->y + src->h - dest->y);
            dest->rt = src->rt;
            dest->tr = src->tr;
            break;
        case 2: 
            dest->set_height(dest->y + dest->h - src->y);
            dest->set_y(src->y);
            dest->bl = src->bl;
            dest->lb = src->lb;
            break;
        case 3: 
            dest->set_width(src->x + src->w - dest->x);
            dest->rt = src->rt;
            dest->tr = src->tr;
            break;
        case 4: 
            dest->set_width(dest->x + dest->w - src->x);
            dest->set_x(src->x);
            dest->bl = src->bl;
            dest->lb = src->lb;
        case 5: 
            dest->rt = src->rt;
            dest->tr = src->tr;
            dest->bl = src->bl;
            dest->lb = src->lb;
            break;
    }
    
    if (m == 1 || m == 2 || m == 4 || m == 5) {
        for (Tile *iter: left_nbs) {
            if (iter->y + iter->h <= src->y + src->h) {
                iter->tr = dest;
            }
        }
    }
    if (m == 1 || m == 3 || m == 4 || m == 5) {
        for (Tile *iter: top_nbs) {
            if (iter->x >= src->x) {
                iter->lb = dest;
            }
        }
    }
    if (m == 1 || m == 2 || m == 3 || m == 5) {
        for (Tile *iter: right_nbs) {
            if (iter->y >= src->y) {
                iter->bl = dest;
            }
        }
    }
    if (m == 2 || m == 3 || m == 4 || m == 5) {
        for (Tile *iter: bottom_nbs) {
            if (iter->x + iter->w <= src->x + src->w) {
                iter->rt = dest;
            }
        }
    }

    this->remove_space_tile(src);
    return dest;
}
