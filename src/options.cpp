#include "options.h"


// --------------------------------------------------
// PartIDFilter
// --------------------------------------------------
PartIDFilter::PartIDFilter ()
    : _active (false)
{
}



PartIDFilter::PartIDFilter (const char* data)
{
    // here we must parse filter values
}


void PartIDFilter::appendValue (unsigned int val)
{
    if (val >= _data.size ())
        _data.resize (val+1);
    _data[val] = true;
    _active = true;
}


void PartIDFilter::appendRegion (unsigned int min, unsigned int max)
{
    if (max >= _data.size ())
        _data.resize (max+1);

    for (int i = min; i <= max; i++)
        _data[i] = true;
    _active = true;
}


bool PartIDFilter::check (unsigned int val)
{
    if (!_active)
        return true;
    
    if (val >= _data.size ())
        _data.resize (val+1);
    
    return _data[val];
}

