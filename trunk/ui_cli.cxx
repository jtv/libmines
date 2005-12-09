#include <iostream>

#include "gamelogic.hxx"

using namespace std;


namespace
{
void coordsbar(int cols)
{
  cout << "\t ";
  for (int i=0; i<cols; ++i)
  {
    if (i<10) cout << ' ';
    cout << i;
  }
  cout << endl;
}

} // namespace


int main()
{
  int rows=20, cols=30, mines=200;

  try
  {
    cout << "Creating minefield of "
         <<rows<<"x"<<cols<<" fields, "<<mines<<" mines." << endl;
    Lake L(rows,cols,mines,3);
    while (L.to_go())
    {
      cout << "Clear patches to go: " << L.to_go() << "..." << endl;
      coordsbar(cols);
      cout << endl;
      for (int r=-1; r<=rows; ++r)
      {
	if (r>=0 && r<rows) cout << r;
	cout << '\t';
	for (int c=-1; c<=cols; ++c) cout << L.status_at(r,c) << ' ';
	if (r >= 0 && r<rows)
	{
	  if (r <= 9) cout << ' ';
	  cout << r;
	}
	/*
	cout << '\t';
	for (int c=-1; c<=cols; ++c) cout << L.at(r,c).near_hiddenmines();
	cout << '\t';
	for (int c=-1; c<=cols; ++c) cout << L.at(r,c).near_unknown();
	cout << '\t';
	for (int c=-1; c<=cols; ++c) cout << (L.at(r,c).mined()?'*':'^');
	*/
	cout << endl;
      }
      cout << endl;
      coordsbar(cols);
      cout <<
	"Please enter row and column (zero-based, separated by space) of next "
	"patch you think is mine-free, or make either coordinate negative if "
	"you want to indicate a mine: "
	<< endl;
      int r, c;
      cin >> r >> c;
      cout << endl;
      set<Coords> changes;
      bool thinksismine = false;
      if (r < 0 || c < 0)
      {
	thinksismine = true;
	r = abs(r);
	c = abs(c);
      }
      if (r >= rows || c >= cols) cout<<"Out of range!"<<endl;
      else L.probe(r,c,changes,thinksismine);
    }

    cout << "Field cleared.  Congratulations!" << endl;
  }
  catch (const Boom &b)
  {
    cout << "** BOOM! **" << endl
         << "Failed after " << b.moves << " moves." << endl;
    if (b.mined) cout << "The fatal mine was at: ";
    else cout << "This location was not mined like you said: ";
    cout << '(' << b.position.row << ',' << b.position.col << ')' << endl;
  }
  catch (const exception &e)
  {
    cerr << e.what() << endl;
    return 1;
  }
  return 0;
}

