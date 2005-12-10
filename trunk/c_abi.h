/*
This file is part of libmines.

Copyright (C) 2005, Jeroen T. Vermeulen <jtv@xs4all.nl>

libmines is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

libmines is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
libmines; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA  02111-1307  USA
*/

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
