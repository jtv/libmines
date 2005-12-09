/* C API/ABI to C++ Minesweeper implementation
 */
#ifdef __cplusplus
extern "C"
{
#endif

typedef void Minefield;

/** @brief Create minefield of given size.  Clean up with mines_close() later!
 */
Minefield *mines_init(int rows, int cols, int mines, int intelligence);

/** @brief Clean up minefield created with mines_init()
 */
void mines_close(Minefield *);

/** @brief User states that patch at given coordinates is clear or a mine
 * @return Boolean: correctness of guess (or -1 on error)
 */
int mines_probe(Minefield *, int row, int col, int minedP);

/** Number of moves made
 */
int mines_moves(const Minefield *);

/** @brief Status of patch at given coordinates
 *  @return '^' for unexplored water, '*' for a known mine, or a textual digit
 * indicating the number of nearby mines
 */
char mines_at(const Minefield *, int row, int col);

/** @brief Number of unmined fields still left to be uncovered
 */
int mines_togo(const Minefield *);

#ifdef __cplusplus
}
#endif

