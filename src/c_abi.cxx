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
Minefield *mines_init(int rows, int cols, int mines)
{
  return new Lake(rows, cols, mines);
}


Minefield *mines_load(const char buffer[])
{
  return new Lake(buffer);
}


void mines_close(Minefield *f)
{
  delete castback(f);
}


int mines_savesize(const Minefield *f)
{
  return castback(f)->savesize();
}


int mines_save(Minefield *f, char buffer[])
{
  return castback(f)->save(buffer);
}


int mines_max_intelligence()
{
  return Lake::max_intelligence();
}


void mines_set_intelligence(Minefield *f, int i)
{
  castback(f)->set_intelligence(i);
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

int mines_rows(const Minefield *f)
{
  return castback(f)->rows();
}

int mines_cols(const Minefield *f)
{
  return castback(f)->cols();
}

} // extern "C"

