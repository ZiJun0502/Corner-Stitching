# Corner Stitching
An implementation of the algorithm from the paper: [Corner Stitching: An Efficient Data-Structure for VLSI Layout Editors](https://ieeexplore.ieee.org/document/1270061).

## How to Run
1. Clone the project:
   ```bash
   git clone <repository-url>
2. Compile the code by running:

```bash
make
```
3. Run the program with:

```bash
./corner_stitches input_filename output_filename
```
Example input and output file formats can be found in the testcase and output directories.

The Python script `draw_block_layout.py` can be used to visualize the output of a test case. Running a test case will generate a text file named `draw.txt`, which contains the data for visualization. To create the visualization image, run:
```bash
python draw_block_layout.py draw.png
```