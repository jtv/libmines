/*
This file is part of libmines.

Copyright (C) 2005-2022, Jeroen T. Vermeulen.

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

/** @addtogroup CXXAPI Native C++ API to libmines
 */
//@{

#include <set>

/// Coordinates of a patch of sea
struct Coords
{
  /// Location's "y coordinate"
  int row;
  /// Location's "x coordinate"
  int col;
  /// Coordinates pair of square at given row and column
  Coords(int Row, int Col) : row(Row), col(Col) {}
  /// Total-order comparison for sorting purposes
  /** Compares locations of two squares.  Although it's not essential, this is
   * done in the same way that addresses of elements in a two-dimensional C
   * array compare.
   */
  bool operator<(const Coords &rhs) const throw ()
  	{ return (rhs.row < row) || (rhs.row==row && rhs.col<col); }
};


/// Exception thrown when a mine is hit
struct Boom
{
  /// Coordinates of field touched in fatal move
  Coords position;
  /// Number of moves made, including the fatal one
  int moves;
  /// Was the field in question actually mined?
  bool mined;
  /// Compose "you die" information to be thrown as exception
  Boom(Coords pos, int mov, bool mine) : position(pos), moves(mov), mined(mine)
	{}
};


class Patch;

/// The "minefield."  This is where it all happens.
/** The Lake is a rectangle of square Patches, each of which may or may not have
 * a mine in it.  In the internal representation, the Lake is surrounded by a
 * border of clear Patches, which are not shown.  This means there are fewer
 * special cases in the algorithm for border Patches.  They do complicate the
 * array indexing and such, but all that is nicely hidden here.
 *
 * Remember to initialize the randomizer by calling srand() with some random
 * input before starting a game, or you'll always get the same configuration.
 */
class Lake
{
public:
  /// Start new game
  Lake(int rows, int cols, int mines);
  /// Start game from saved game state
  explicit Lake(const char[]);

  ~Lake() throw ();

  /// Maximum intelligence level available
  static int max_intelligence() throw () { return 2; }

  /// Change intelligence level
  void set_intelligence(int i) throw () { m_intelligence = i; }

  /// Mark given patch as being either clear or mined
  /** The specified amount of "intelligence" is recursively applied in revealing
   * surrounding patches whose state becomes obvious.  Boom is thrown if the
   * given patch is actually mined.
   * @param row "y coordinate" of field
   * @param col "x coordinate" of field
   * @param changes will receive a list of all patches revealed by this move
   * @param as_mine indicates whether user thinks this patch is mined
   */
  void probe(int row, int col, std::set<Coords> &changes, bool as_mine=false);

  /// Number of unmined patches still to be revealed
  int to_go() const throw () { return m_patches_to_go; }

  /// Number of moves made
  int moves() const throw () { return m_moves; }

  /// Status representation of patch at given coordinates
  /** Returns '^' for unexplored water; '*' for a known mine; or the single
   * textual digit representing the number of nearby mines.
   *
   * Coordinates are zero-based.  Patches "just outside" the playing field (i.e.
   * having coordinates of -1 or the exact number of rows/columns respectively)
   * may be safely queried.
   */
  char status_at(int row, int col) const;

  /// Number of rows making up this Lake
  const int rows() const throw () { return m_rows; }
  /// Number of columns making up this Lake
  const int cols() const throw () { return m_cols; }

  /// Maximum number of bytes required to save this game
  int savesize() const throw ();

  /// Write game state (in ASCII) to memory buffer
  /** The available buffer space must be at least rows*cols+100 bytes large.
   */
  int save(char buf[]) const;

private:
  enum { border = 3 };
  void init_field();
  Patch &at(int row, int col);
  const Patch &at(int row, int col) const;

  bool place_mine_at(int row, int col);

  /// Apply functor f to a square of Patches centered at (row,col)
  /** The INCLUDECENTER template argument determines whether the central patch
   * should be included in this square, or whether it should be skipped.
   */
  template<int RADIUS, bool INCLUDECENTER, typename FUNCT>
  void for_zone(int row, int col, FUNCT f)
  {
    const int top = std::max(-border, row-RADIUS),
	      bottom = std::min(row+RADIUS+1, m_rows+border),
	      left = std::max(-border, col-RADIUS),
	      right = std::min(col+RADIUS+1, m_cols+border);

    for (int r = top; r < bottom; ++r) for (int c = left; c < right; ++c)
      if (INCLUDECENTER || r!=row || c!=col)
        f(Coords(r,c),at(r,c));
  }

  template<typename FUNCT> void for_neighbours(int row, int col, FUNCT f)
	{ return for_zone<1,false>(row,col,f); }

  void reveal_patch(int row, int col);

  /// Initialize a row of patches in the lake's border
  void border_row(int);
  /// Initialize a row of patches in the lake's border
  void border_col(int);

  /// Recursively reveal any patches whose status becomes or has become obvious
  /** This is where the intelligence level is applied in order to expose patches
   * that become obvious.
   *
   * Intelligence may currently be 0 ("don't reveal anything except at the
   * user's request"); 1 ("only reveal patches with zero neighbouring mines and
   * their immediate neighbours"); or 2 ("reveal any mines that have had all
   * their surrounding mines revealed, or have as many surrounding unexplored
   * mines as they have surrounding unexplored fields").  Most minesweeper
   * implementations implement level 1.
   *
   * Higher intelligence levels are accepted, but do not instill any greater
   * intelligence than is implemented.  More levels will be added in the future.
   */
  void propagate(std::set<Coords> &work, std::set<Coords> &changes);

  int index_for(int row, int col) const throw ();
  int arraysize() const throw ();
  void check_index(int) const;
  void check_pos(int row, int col) const;

  Patch *m_patches;
  int m_rows, m_cols;
  int m_intelligence;
  int m_patches_to_go;
  int m_moves;

  Lake();
  Lake(const Lake &);
  const Lake &operator=(const Lake &);
};

//@}

