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

// This is is where heart of the game is implemented.

#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

#include "gamelogic.hxx"
#include "save.hxx"

using namespace std;


#include<iostream>// DEBUG CODE
namespace
{
int rand_coord(int top)
{
  // TODO: There's probably some better random function out there
  return rand() % top;
}
} // namespace


/// A square patch of water, which may or may not contain a mine
class Patch
{
public:
  Patch();

  /// Initialization: set a mine
  void mine();

  bool mined() const throw () { return m_mined; }
  bool revealed() const throw () { return m_revealed; }
  int near_mines() const throw () { return m_nearmines; }
  int near_hiddenmines() const throw () { return m_near_hiddenmines; }
  int near_unknown() const throw () { return m_near_unknown; }

  void reveal() throw () { m_revealed = true; }

  /// Initialization: mark the fact that a mine has been set in a nearby Patch
  void set_nearby_mine();

  /// Adjust to revelation of nearby Patch (mined or not, depending on argument)
  void reveal_nearby(bool is_mined);

  /// Should the state of all nearby Patches now be obvious to the user?
  bool obvious() const throw ();

private:
  int m_nearmines;
  int m_near_hiddenmines;
  /// Nearby unrevealed patches (not correct for border patches, but who cares)
  int m_near_unknown;
  bool m_mined;
  bool m_revealed;
};


Patch::Patch() :
  m_nearmines(0),
  m_near_hiddenmines(0),
  m_near_unknown(8),
  m_mined(false),
  m_revealed(false)
{
}

void Patch::mine()
{
  assert(!mined());
  m_mined = true;
  assert(mined());
}

void Patch::set_nearby_mine()
{
  ++m_nearmines;
  ++m_near_hiddenmines;
  assert(m_nearmines <= 8);
  assert(m_near_hiddenmines <= m_nearmines);
}

void Patch::reveal_nearby(bool is_mined)
{
  m_near_hiddenmines -= is_mined;
  --m_near_unknown;

  assert(m_near_hiddenmines >= 0);
  assert(m_near_unknown >= 0);
  assert(m_near_hiddenmines <= m_near_unknown);
}

bool Patch::obvious() const throw ()
{
  return near_unknown() && revealed() && !mined() &&
    (!near_hiddenmines() || near_hiddenmines()==near_unknown());
}


namespace
{

/// Functor to apply to neighbouring Patches when initializing a mine
struct set_nearby_mine
{
  void operator()(Coords, Patch &p) const { p.set_nearby_mine(); }
};

/// Functor: note that nearby patch has been revealed, update counters
class reveal_nearby
{
public:
  explicit reveal_nearby(bool mined) : m_mine(mined) {}
  void operator()(Coords, Patch &p) const { p.reveal_nearby(m_mine); }

private:
  bool m_mine;

  reveal_nearby();
};

/// Functor: add element to set if it satisfies given condition functor
template<typename COND> class set_add : private COND
{
public:
  explicit set_add(set<Coords> &worklist) : m_worklist(worklist) {}
  void operator()(Coords c, const Patch &p) const
  {
    if (COND::operator()(p)) m_worklist.insert(c);
  }
private:
  set<Coords> &m_worklist;
};

/// Condition functor: we're not fully done with Patch yet?
class UnfinishedPatch
{
public:
  bool operator()(const Patch &p) const throw () { return p.near_unknown(); }
};

/// Condition functor: Patch has not been revelealed yet?
class UnrevealedPatch
{
public:
  bool operator()(const Patch &p) const throw () { return !p.revealed(); }
};

/// Condition functor: Patch has become "obvious"?
class ObviousPatch
{
public:
  bool operator()(const Patch &p) const throw () { return p.obvious(); }
};

struct CoordsPair
{
  bool operator()(const pair<Coords,Coords> &lhs,
      const pair<Coords,Coords> &rhs) const throw ()
  {
    return (lhs.first < rhs.first) ||
      (!(rhs.first < lhs.first) && (lhs.second < rhs.second));
  }
};


bool are_neighbours(Coords a, Coords b)
{
  return abs(a.row-b.row) <= 1 && abs(a.col-b.col) <= 1;
}

} // namespace


Lake::Lake(int _rows, int _cols, int mines) :
  m_patches(0),
  m_rows(_rows),
  m_cols(_cols),
  m_intelligence(1),
  m_patches_to_go(m_rows*m_cols),
  m_moves(0)
{
  init_field();

  while (mines)
  {
    const int row = rand_coord(m_rows), col = rand_coord(m_cols);
    mines -= place_mine_at(row,col);
  }
}


Lake::Lake(const char buffer[]) :
  m_patches(0),
  m_rows(0),
  m_cols(0),
  m_intelligence(0),
  m_patches_to_go(0),
  m_moves(0)
{
  initialize_encoding();

  const char *here = read_header(buffer);

  m_rows = read_int("rows",here);
  m_cols = read_int("cols",here);
  m_moves = read_int("move",here);
  m_intelligence = read_int("intl",here);
  m_patches_to_go = m_rows * m_cols;

  init_field();

  here = skip_whitespace(here);

  const int padding = linepadding(m_cols);

  // Read data block: mine placement & revealed fields
  for (int r=0; r<m_rows; ++r)
  {
    for (int c = 0; c < m_cols; c += patchesperchar)
    {
      unsigned int x = extract_char(here);
      for (int i = 0; i < patchesperchar; ++i)
      {
	if (x & 2) place_mine_at(r,c+i);
	if (x & 1) reveal_patch(r,c+i);
	x >>= 2;
      }
    }
    here = read_eol(here, padding);
  }
  read_terminator(here);
}


Lake::~Lake() throw ()
{
  delete [] m_patches;
}


void Lake::init_field()
{
  assert(m_rows > 0);
  assert(m_cols > 0);
  assert(m_moves >= 0);
  assert(m_intelligence >= 0);

  m_patches = new Patch[arraysize()];
  for (int b=1; b<border; ++b)
  {
    border_row(-b);
    border_row(m_rows+b-1);
    border_col(-b);
    border_col(m_cols+b-1);
  }
}

int Lake::savesize() const throw ()
{
  return m_rows * ((m_cols+patchesperchar-1)/patchesperchar+3) + 100;
}


int Lake::save(char buf[]) const
{
  initialize_encoding();
  char *here = write_header(buf);
  here = write_int("rows",here,m_rows);
  here = write_int("cols",here,m_cols);
  here = write_int("move",here,m_moves);
  here = write_int("intl",here,m_intelligence);
  here = write_newline(here);

  // Padding at end of line required by base64
  const int padding = linepadding(m_cols);

  // Write mines & revealed fields
  for (int r = 0; r < m_rows; ++r)
  {
    for (int c = 0; c < m_cols; c += patchesperchar)
    {
      unsigned int x = 0;
      for (int i = patchesperchar-1; i >= 0; --i)
      {
	x <<= 2;
	if (c+i < m_cols && at(r,c+i).mined()) x |= 2;
	if (c+i < m_cols && at(r,c+i).revealed()) x |= 1;
      }
      *here++ = produce_char(x);
    }
    here = write_eol(here, padding);
  }
  terminate(here);

  return here - buf;
}


bool Lake::place_mine_at(int row, int col)
{
  Patch &p = at(row,col);
  if (p.mined()) return false;

  p.mine();
  --m_patches_to_go;
  for_neighbours(row,col,set_nearby_mine());
  return true;
}

const Patch &Lake::at(int row, int col) const
{
  check_pos(row, col);
  const int idx = index_for(row, col);
  check_index(idx);
  return m_patches[idx];
}


Patch &Lake::at(int row, int col)
{
  check_pos(row, col);
  const int idx = index_for(row, col);
  check_index(idx);
  return m_patches[idx];
}

void Lake::probe(int row, int col, set<Coords> &changes, bool as_mine)
{
  assert(m_patches_to_go >= 0);

  Patch &p = at(row,col);
  if (!p.revealed())
  {
    ++m_moves;
    const Coords pos(row,col);
    if (p.mined() != as_mine)
    {
      reveal_patch(row,col);
      throw Boom(pos, m_moves, p.mined());
    }
    set<Coords> worklist;
    worklist.insert(pos);
    propagate(worklist, changes);
    assert(m_patches_to_go >= 0);
  }
}

char Lake::status_at(int row, int col) const
{
  const Patch &p = at(row,col);
  return p.revealed() ? (p.mined() ? '*' : ('0'+p.near_mines())) : '^';
}

void Lake::reveal_patch(int row, int col)
{
  Patch &p = at(row,col);
  if (!p.revealed())
  {
    p.reveal();
    for_neighbours(row,col,reveal_nearby(p.mined()));
    if (row >= 0 && row < m_rows && col >= 0 && col < m_cols && !p.mined())
      --m_patches_to_go;
  }
}

void Lake::border_row(int r)
{
  for (int c=m_cols+border-1; c>=-border; --c) reveal_patch(r,c);
}

void Lake::border_col(int c)
{
  for (int r=m_rows-1; r>=0; --r) reveal_patch(r,c);
}

void Lake::propagate(set<Coords> &work, set<Coords> &changes)
{
  while (!work.empty())
  {
    set<Coords> next, area;

    /* Reveal any patches from working set with no nearby mines, and their
     * neighbours.  This part is what all Minesweeper implementations do.
     */
    for (set<Coords>::const_iterator i = work.begin(); i != work.end(); ++i)
    {
      const int row = i->row, col = i->col;
      if (row >= 0 && row < m_rows && col >= 0 && col < m_cols)
      {
        Patch &p = at(row,col);
        if (!p.revealed())
        {
          reveal_patch(row,col);
	  changes.insert(Coords(row,col));
          for_zone<2,true>(row,col,set_add<UnfinishedPatch>(area));
        }
        if (m_intelligence > 0 && p.obvious())
          for_neighbours(row,col,set_add<UnrevealedPatch>(next));
      }
    }

    /* Recognize revealed Patches whose unexplored neighbours must obviously be
     * either all clear or all mines, and reveal their neighbours.
     */
    if (m_intelligence > 1)
      for (set<Coords>::const_iterator i = area.begin(); i != area.end(); ++i)
        for_zone<1,true>(i->row,i->col,set_add<ObviousPatch>(next));

    /* Recognize cases where two patches' sets of nearby unrevealed patches
     * overlap, such that one of the two difference sets can be concluded to be
     * mine-free and the other have only mined patches.  One of the two
     * difference sets may be empty.
     */
    if (m_intelligence > 2)
    {
      // TODO: TODO: TODO:
    }

    work.swap(next);
  }
}


int Lake::index_for(int row, int col) const throw ()
{
  return (row+border)*(m_cols+2*border) + col + border;
}


int Lake::arraysize() const throw ()
{
  return (m_rows+2*border)*(m_cols+2*border);
}


void Lake::check_index(int i) const
{
  assert(i >= 0);
  assert(i < arraysize());
}


void Lake::check_pos(int row, int col) const
{
  assert(row >= -border);
  assert(row < m_rows+border);
  assert(col >= -border);
  assert(col < m_cols+border);
}

