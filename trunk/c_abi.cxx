#include <set>

#include "gamelogic.hxx"
#include "c_abi.h"

using namespace std;

namespace
{
Lake *castback(Minefield *f)
{
  return static_cast<Lake *>(f);
}

const Lake *castback(const Minefield *f)
{
  return static_cast<const Lake *>(f);
}
} // namespace

extern "C"
{
Minefield *mines_init(int rows, int cols, int mines, int intelligence)
{
  return new Lake(rows, cols, mines, intelligence);
}


void mines_close(Minefield *f)
{
  delete castback(f);
}

int mines_probe(Minefield *f, int row, int col, int minedP)
{
  try
  {
    set<Coords> changes;
    castback(f)->probe(row,col,changes,minedP);
  }
  catch (const Boom &)
  {
    return 0;
  }
  catch (const exception &)
  {
    return -1;
  }
  return 1;
}


int mines_moves(const Minefield *f)
{
  return castback(f)->moves();
}

char mines_at(const Minefield *f, int row, int col)
{
  return castback(f)->status_at(row,col);
}

int mines_togo(const Minefield *f)
{
  return castback(f)->to_go();
}

} // extern "C"

