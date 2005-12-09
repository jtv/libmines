#include <cassert>
#include <set>
#include <vector>

#include "gamelogic.hxx"

using namespace std;


namespace
{
int rand_coord(int top)
{
  // TODO: Actually randomize
  return rand() % top;
}
}


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
  assert(m_nearmines == m_near_hiddenmines);
  assert(m_nearmines <= 8);
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


/// Is Patch a candidate for "superset detection" based on newly revealed Patch?
/** This looks for the case where the unexplored area around the Patch is a
 * proper superset of the one of a neighbour that was just revealed.  If it is,
 * and if the number of unexplored mines near the current one is either equal to
 * that near the newly revealed one, or the difference is equal to the size
 * difference of the unexplored areas, then the unexplored area near the Patch
 * is either all clear or all mined, respectively.
 */
class SuperSet
{
  set<Coords> &m_worklist;
  const Patch &m_revealed;
public:
  SuperSet(set<Coords> &worklist, const Patch &newly_revealed) :
    m_worklist(worklist),
    m_revealed(newly_revealed)
  {
    assert(newly_revealed.revealed());
  }

  void operator()(Coords c, const Patch &p) const
  {
    if (p.revealed())
    {
      const int areadiff = p.near_unknown() - m_revealed.near_unknown();
      if (areadiff > 0 &&
	  (p.near_hiddenmines() == m_revealed.near_hiddenmines() ||
	   p.near_hiddenmines() == areadiff))
      m_worklist.insert(c);
    }
  }
};
} // namespace


Lake::Lake(int rows, int cols, int mines, int intelligence) :
  m_patches(0),
  m_rows(rows),
  m_cols(cols),
  m_intelligence(intelligence),
  m_patches_to_go(rows*cols-mines),
  m_moves(0)
{
  assert(rows > 0);
  assert(cols > 0);
  assert(mines > 0);
  assert(mines <= rows*cols);

  m_patches = new Patch[arraysize()];

  while (mines)
  {
    const int row = rand_coord(m_rows), col = rand_coord(m_cols);
    if (!at(row,col).mined())
    {
      at(row,col).mine();
      for_neighbours(row,col,set_nearby_mine());
      --mines;
    }
  }

  for (int b=1; b<border; ++b)
  {
    border_row(-b);
    border_row(m_rows+b-1);
    border_col(-b);
    border_col(m_cols+b-1);
  }
}


Lake::~Lake() throw ()
{
  delete [] m_patches;
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
    if (p.mined() != as_mine) throw Boom(pos, m_moves, p.mined());
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

void Lake::reveal_patch(int row, int col, set<Coords> &changes)
{
  Patch &p = at(row,col);
  if (!p.revealed())
  {
    p.reveal();
    changes.insert(Coords(row,col));
    for_neighbours(row,col,reveal_nearby(p.mined()));
  }
}

void Lake::border_row(int r)
{
  set<Coords> changes;
  for (int c=m_cols+border-1; c>=-border; --c) reveal_patch(r,c,changes);
}

void Lake::border_col(int c)
{
  set<Coords> changes;
  for (int r=m_rows-1; r>=0; --r) reveal_patch(r,c,changes);
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
          reveal_patch(row,col,changes);
          if (!p.mined()) --m_patches_to_go;
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

    /* Recognize the case where two explored patches A and B have sets of
     * unexplored neighbours U such that 
     *  1. U(A) is a proper subset of U(B), and 
     *  2. the numbers of unexplored nearby mines M satisfy either:
     *   (a) M(B) - M(A) = 0 (in which case U(B)\U(A) is all clear) or
     *   (b) M(B) - M(A) = |U(B)\U(A)|  (in which case U(B)\U(A) is all mines).
     *
     * At the moment I don't see a clever way of implementing this with mere
     * local scalar comparisons.  Unless we count use of bitfields; the full set
     * of neighbours for a Patch fits seductively well into a byte.  But that
     * would introduce unpleasant complexity.
     */
    if (m_intelligence > 2)
    {
      set<Coords> cand;

      // First step: filter out the possible cases
      for (set<Coords>::const_iterator i = area.begin(); i != area.end(); ++i)
      {
	const Patch &p = at(i->row, i->col);
        if (p.revealed()) for_neighbours(i->row, i->col, SuperSet(cand,p));
      }
      
      // TODO: Iterate through candidates to find the *real* superset cases
      for (set<Coords>::const_iterator i = cand.begin(); i != cand.end(); ++i)
      {
      }
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

