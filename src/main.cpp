#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include "corner_stitches.hpp"

using namespace std;
bool cmp(Tile* a, Tile* b) {
    return a->get_id() < b->get_id();
}
int main(int argc, char* argv[]) {
	if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input> <output>" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    cout << "input file: " << argv[1] << "\noutput file: " << argv[2] << '\n';

    int outline_w, outline_h;
    int id, x, y;
    char op;
    vector<Tile*> tiles;

    ifstream testcase(inputFile);
    if (!testcase) {
        cerr << "Unable to open file input.txt";
        return 1;
    }

    testcase >> outline_w >> outline_h;
    Graph* graph = new Graph(outline_w, outline_h);
    // Read until the end of the file
    vector<pair<int,int>> ans;
    while (testcase >> op) {
        if(op != 'P'){
            testcase.unget();
            testcase >> id;
        }
        testcase >> x >> y; 
        if (op != 'P') { 
            // Tile creation
            int w, h; 
            testcase >> w >> h; 
            // cout << "Creating tile: " << x << ' ' << x+w << ' ' << y << ' ' << y + h<< '\n'; 
            graph->create_tile(id, x, y, w, h);
            //graph->output_drawing_text("draw" + std::to_string(block_tile_cnt++) + ".txt");
        } else { 
            // Point search
            // cout << "Finding point for: " << x << ' ' << y << '\n'; 
            Tile* result = graph->find_point(x, y);
            ans.push_back({result->get_x(), result->get_y()});
            //cout << "Result id: " << result->get_id() << " Is Block: " << (result->get_id() < 0) << '\n';
        } 
    }
    graph->output_drawing_text("draw.txt");
    vector<Tile*> block_tiles = graph->get_block_tiles();
    sort(block_tiles.begin(), block_tiles.end(), cmp);

    ofstream output(outputFile);
    output << graph->get_tiles().size() << '\n';
    for (Tile* tile: block_tiles) {
        vector<Tile*> nbs = graph->find_neighbors(tile, 0xf);
        int b_cnt = 0, s_cnt = 0;
        for (Tile* tile: nbs) {
            if (tile->get_id() > 0) b_cnt++;
            else s_cnt++;
        }
        output << tile->get_id() << ' ' << b_cnt << ' ' << s_cnt << '\n';
    }
    for (pair<int,int> p : ans) {
        output << p.first << ' ' << p.second << '\n';
    }
}
