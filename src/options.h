#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include <set>
#include <vector>


class PartIDFilter
{
private:
    std::vector<bool> _data;
    bool _active;
    
public:
    PartIDFilter ();
    PartIDFilter (const char* text);

    void appendValue (unsigned int val);
    void appendRegion (unsigned int min, unsigned int max);

    bool check (unsigned int val);
};


class StateOptions
{
private:
    bool _keepDeleted;
    bool _pvd_mode;
    PartIDFilter* _pid_filter;
    
public:
    StateOptions (bool keepDeleted, bool pvd_mode, PartIDFilter* pid_filter = 0)
        : _keepDeleted (keepDeleted),
          _pvd_mode (pvd_mode),
          _pid_filter (pid_filter)
        { };

    bool keepDeleted () const
        { return _keepDeleted; };

    bool pvdMode () const
        { return _pvd_mode; };

    bool partIDCheck (unsigned int partID)
        { return _pid_filter ? _pid_filter->check (partID) : true; };
};


#endif
